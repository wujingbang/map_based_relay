#include "geoSVR-neighbors.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include <algorithm>
#include <list>

#include "ns3/core-module.h"
#include "ns3/mbr-neighbor-app.h"

NS_LOG_COMPONENT_DEFINE ("geoSVRNeighbors");

struct point {
    double x;
    double y;
};

namespace ns3 {
namespace geoSVR {


double
msvr_cal_dist(struct point p1, struct point p2)
{
    return sqrt(pow((p1.x - p2.x), 2) + pow((p1.y - p2.y), 2));
}

double
msvr_cal_dist(double x1, double y1, double x2, double y2)
{
    point p1, p2;

    p1.x = x1;
    p1.y = y1;
    p2.x = x2;
    p2.y = y2;

    return msvr_cal_dist(p1, p2);
}

Neighbors::Neighbors ()
{
  m_nbFromMbr = false;
  m_txErrorCallback = MakeCallback (&Neighbors::ProcessTxError, this);
  m_entryLifeTime = Seconds (2); //FIXME fazer isto parametrizavel de acordo com tempo de hello
}

Neighbors::Neighbors (bool nbFromMbr)
{
  m_nbFromMbr = nbFromMbr;
  m_txErrorCallback = MakeCallback (&Neighbors::ProcessTxError, this);
  m_entryLifeTime = Seconds (2); //FIXME fazer isto parametrizavel de acordo com tempo de hello

}

void
Neighbors::Init()
{
    // empty
}

void
Neighbors::Clear()
{
    nbl.clear ();
}

void
Neighbors::Update(struct msvr_ninfo *ip)
{
    struct msvr_nbentry ep;

    for (std::list<msvr_nbentry>::iterator iter = nbl.begin();
         iter != nbl.end(); ++iter) {
        if ((iter->nbe_ninfo.dst).IsEqual(ip->dst)) {
            /*iter->nbe_ninfo.n_id = ip->n_id;*/
            iter->nbe_ninfo.x = ip->x;
            iter->nbe_ninfo.y = ip->y; 
            iter->nbe_ninfo.speed = ip->speed;
            iter->nbe_ninfo.heading = ip->heading;
            iter->nbe_ninfo.edgeid = ip->edgeid;
            iter->m_expireTime = std::max (m_entryLifeTime + Simulator::Now (), iter->m_expireTime);
            //memcpy(&ep.nbe_ninfo.n_dst, &ip->n_dst, sizeof(struct in_addr));
            //gettimeofday(&iter->nbe_ts, NULL);
            return;
        }
    }

    memset(&ep, 0, sizeof(struct msvr_nbentry));

    ep.nbe_ninfo.x = ip->x;
    ep.nbe_ninfo.y = ip->y;
    ep.nbe_ninfo.speed = ip->speed;
    ep.nbe_ninfo.heading = ip->heading;
    ep.nbe_ninfo.edgeid = ip->edgeid;
    ep.nbe_ninfo.dst = ip->dst;
    ep.m_expireTime = m_entryLifeTime + Simulator::Now ();
    //gettimeofday(&ep.nbe_ts, NULL);

    nbl.push_front(ep);
    Purge();
}


struct CloseNeighbor
{
  bool operator() (const msvr_nbentry & nb) const
  {
    return ((nb.m_expireTime < Simulator::Now ()));
  }
};

void
Neighbors::Purge()
{
    if (nbl.empty ())
        return;

    CloseNeighbor pred;
    nbl.erase (std::remove_if (nbl.begin (), nbl.end (), pred), nbl.end ());

//    std::list<msvr_nbentry>::iterator iter1 = std::remove_if (nbl.begin (), nbl.end (), pred);
//    std::list<msvr_nbentry>::iterator iter2;
//    for(iter2 = iter1; iter2 != nbl.end(); ++iter2)
//    {
//    	std::cout<<iter2->m_expireTime << " "<< Simulator::Now ()<<std::endl;
//    }
}

struct msvr_nbentry *
Neighbors::find_entry_via_ipaddress(Ipv4Address dst)
{
    Purge();
    for (std::list<msvr_nbentry>::iterator iter = nbl.begin();
         iter != nbl.end(); ++iter) {
        if ((iter->nbe_ninfo.dst).IsEqual(dst))
            return &(*iter);
    }

    return NULL;
}

bool
Neighbors::isNeighbor(Ipv4Address dst, Vector srcpos, double range)
{
  if (m_nbFromMbr)
    {
      for (uint32_t j = 0; j < m_node->GetNApplications (); j++)
	{
	  Ptr<mbr::MbrNeighborApp> nbapp = DynamicCast<mbr::MbrNeighborApp> (m_node->GetApplication(j));
	  if (nbapp)
	  {
		  Vector nbloc;
		  double dist;
		  bool ret = nbapp->getNb()->IsNeighbor(dst, nbloc);
		  if (ret == true)
		  {
			  dist = msvr_cal_dist(srcpos.x, srcpos.y, nbloc.x, nbloc.y);
			  if (dist <= range)
				  return true;
			  else
				  return false;
		  }

	    return false;

	  }
	}
//      Ptr<Application> app = m_node->GetApplication(0);
//      Ptr<mbr::MbrNeighborApp> nbapp = DynamicCast<mbr::MbrNeighborApp> (app);
//      return nbapp->getNb()->IsNeighbor(id);
    }
  else
    {
      Purge();
      for (std::list<msvr_nbentry>::iterator iter = nbl.begin();
	   iter != nbl.end(); ++iter) {
	  if ((iter->nbe_ninfo.dst).IsEqual(dst))
	      return true;
      }
    }
    return false;
}

Ipv4Address
Neighbors::find_furthest_nhop(int edgeid1, int edgeid2, int edgeid3, double x1, double y1, double x2, double y2, double range)
{
  double bestdistance = -1;
  double this2nb_distance, this2nextroad_distance, nb2nextroad_distance, delta;
  if (m_nbFromMbr)
    {
      MsvrMap map;
      Ipv4Address ipfound = Ipv4Address::GetZero ();
      Ptr<mbr::MbrNeighborApp> nbapp;
      for (uint32_t j = 0; j < m_node->GetNApplications (); j++)
	{
	  nbapp = DynamicCast<mbr::MbrNeighborApp> (m_node->GetApplication(j));
	  if (nbapp)
	    break;
	}
      NS_ASSERT(nbapp);
      for (int i = 1; i < nbapp->getNb()->GetTableSize(); i++)
		{
		  Vector v = nbapp->getNb()->GetCartesianPosition(i);
		  this2nb_distance = msvr_cal_dist(x1, y1, v.x, v.y);
		  if (this2nb_distance > range)
			  continue;

		  this2nextroad_distance = msvr_cal_dist(x1, y1, x2, y2);
		  nb2nextroad_distance = msvr_cal_dist(v.x, v.y, x2, y2);
		  delta = this2nextroad_distance - nb2nextroad_distance;
		  if (delta <= 20)
			  continue;

		  int edgeid = map.getRoadByPos(v.x, v.y).id_;
		  if (edgeid == edgeid1) {
			  if (delta > bestdistance) {
				  bestdistance = delta;
				  ipfound = nbapp->getNb()->GetIp(i);
			  }
		  } else if (edgeid2 != -1 && edgeid == edgeid2) {
			  if (delta > bestdistance) {
				  bestdistance = delta;
				  ipfound = nbapp->getNb()->GetIp(i);
			  }
		  } else if (edgeid == edgeid3){
			  if (delta > bestdistance) {
				  bestdistance = delta;
				  ipfound = nbapp->getNb()->GetIp(i);
			  }
		  }
		}
	  return ipfound;
    }
  else
    {
	  return find_next_hop(edgeid1, edgeid2, edgeid3, x1, y1, x2, y2);

    }
}

Ipv4Address
Neighbors::find_next_hop(int edgeid1, int edgeid2, int edgeid3, double x1, double y1, double x2, double y2)
{
  double len1 = 200.0;
  double rlen = 200.0;
  double len2 = msvr_cal_dist(x1, y1, x2, y2);
  double goal = 195.0;
  if (m_nbFromMbr)
    {
      MsvrMap map;
      Ipv4Address ipfound = Ipv4Address::GetZero ();
      Ptr<mbr::MbrNeighborApp> nbapp;
      for (uint32_t j = 0; j < m_node->GetNApplications (); j++)
	{
	  nbapp = DynamicCast<mbr::MbrNeighborApp> (m_node->GetApplication(j));
	  if (nbapp)
	    break;
	}
      NS_ASSERT(nbapp);
      for (int i = 1; i < nbapp->getNb()->GetTableSize(); i++)
	{
	  Vector v = nbapp->getNb()->GetCartesianPosition(i);
	  int edgeid = map.getRoadByPos(v.x, v.y).id_;
	  if (edgeid == edgeid1)
	    {
	      double tmplen = msvr_cal_dist(x1, y1, v.x, v.y);
	      double dstlen = msvr_cal_dist(x2, y2, v.x, v.y);
	      if (abs(tmplen - goal) < len1 &&  dstlen < len2 && tmplen < rlen) {
			  len1 = abs(tmplen - goal);
			  ipfound = nbapp->getNb()->GetIp(i);
	      }
	    }
	   else if (edgeid2 != -1 && edgeid == edgeid2)
	     {
		double tmplen = msvr_cal_dist(x1, y1, v.x, v.y);
		double dstlen = msvr_cal_dist(x2, y2, v.x, v.y);
		if (abs(tmplen - goal) < len1 && dstlen < len2 && tmplen < rlen)
		  {
		    len1 = abs(tmplen - goal);
		    ipfound = nbapp->getNb()->GetIp(i);
		  }
	     }
	   else if (edgeid == edgeid3)
	     {
		double tmplen = msvr_cal_dist(x1, y1,
			v.x, v.y);
		double dstlen = msvr_cal_dist(x2, y2,
			v.x, v.y);
		if (abs(tmplen - goal) < len1 &&  dstlen < len2 && tmplen < rlen)
		  {
		    len1 = abs(tmplen - goal);
		    ipfound = nbapp->getNb()->GetIp(i);
		  }
	     }
	  }
	  return ipfound;
    }
  else
    {

      Purge();
      struct msvr_nbentry *res = NULL;

      //msvr_nbl_print(nbl);//added by yyq
      for (std::list<msvr_nbentry>::iterator iter = nbl.begin();
	   iter != nbl.end(); ++iter) {

	  if (iter->nbe_ninfo.edgeid == edgeid1) {
	      double tmplen = msvr_cal_dist(x1, y1,
		      iter->nbe_ninfo.x, iter->nbe_ninfo.y);
	      double dstlen = msvr_cal_dist(x2, y2,
		      iter->nbe_ninfo.x, iter->nbe_ninfo.y);
	      //printf("teplen: %f dstlen: %f  len1: %f len2: %f\n" , tmplen, dstlen, len1, len2 );
	      if (abs(tmplen - goal) < len1 &&
		  dstlen < len2 && tmplen < rlen) {
		  len1 = abs(tmplen - goal);
		  res = &(*iter);
	      }
	  } else if (edgeid2 != -1 &&
		     iter->nbe_ninfo.edgeid == edgeid2) {
	      double tmplen = msvr_cal_dist(x1, y1,
		      iter->nbe_ninfo.x, iter->nbe_ninfo.y);
	      double dstlen = msvr_cal_dist(x2, y2,
		      iter->nbe_ninfo.x, iter->nbe_ninfo.y);
	      //printf("teplen: %f dstlen: %f  len1: %f len2: %f\n" , tmplen, dstlen, len1, len2 );
	      if (abs(tmplen - goal) < len1 &&
		  dstlen < len2 && tmplen < rlen) {
		  len1 = abs(tmplen - goal);
		  res = &(*iter);
	      }
	  } else if (iter->nbe_ninfo.edgeid == edgeid3) {
	      double tmplen = msvr_cal_dist(x1, y1,
		      iter->nbe_ninfo.x, iter->nbe_ninfo.y);
	      double dstlen = msvr_cal_dist(x2, y2,
		      iter->nbe_ninfo.x, iter->nbe_ninfo.y);
	      //printf("teplen: %f dstlen: %f  len1: %f len2: %f\n" , tmplen, dstlen, len1, len2 );
	      if (abs(tmplen - goal) < len1 &&
		  dstlen < len2 && tmplen < rlen) {
		  len1 = abs(tmplen - goal);
		  res = &(*iter);
	      }
	  }
      }
      if (res == NULL)
	  return Ipv4Address::GetZero ();
      else
	  return res->nbe_ninfo.dst;

    }
}

void
Neighbors::Print()
{
//    fprintf(stderr, "====\n");
//    for (std::list<msvr_nbentry>::iterator iter = nbl.begin();
//         iter != nbl.end(); ++iter) {
//        fprintf(stderr, "(%f, %f):(%f %f) edgeid %d ",
//                iter->nbe_ninfo.x, iter->nbe_ninfo.y,
//                iter->nbe_ninfo.speed, iter->nbe_ninfo.heading,
//                iter->nbe_ninfo.edgeid);
//        (iter->nbe_ninfo.dst).Print(std::cout);
//        std::cout << std::endl;
//        fflush(stdout);
//    }
//    fprintf(stderr, "====\n");
}

void Neighbors::ProcessTxError (WifiMacHeader const & hdr)
{
}


}   // geoSVR
} // ns3
