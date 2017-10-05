#include "gpsr-ptable.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include <algorithm>

#include "ns3/mbr-neighbor-app.h"
#include "ns3/core-module.h"

NS_LOG_COMPONENT_DEFINE ("GpsrTable");


namespace ns3 {
namespace gpsr {

/*
  GPSR position table
*/
PositionTable::PositionTable ()
{
  m_nbFromMbr = false;
  m_txErrorCallback = MakeCallback (&PositionTable::ProcessTxError, this);
  m_entryLifeTime = Seconds (1); //FIXME fazer isto parametrizavel de acordo com tempo de hello

}

PositionTable::PositionTable (bool nbFromMbr)
{
  m_nbFromMbr = nbFromMbr;
  m_txErrorCallback = MakeCallback (&PositionTable::ProcessTxError, this);
  m_entryLifeTime = Seconds (1); //FIXME fazer isto parametrizavel de acordo com tempo de hello

}

Time 
PositionTable::GetEntryUpdateTime (Ipv4Address id)
{
  if (id == Ipv4Address::GetZero ())
    {
      return Time (Seconds (0));
    }
  if (m_nbFromMbr)
    {
//      Ptr<Application> app = m_node->GetApplication(0);
//      Ptr<mbr::MbrNeighborApp> nbapp = DynamicCast<mbr::MbrNeighborApp> (app);
      Ptr<mbr::MbrNeighborApp> nbapp;
      for (uint32_t j = 0; j < m_node->GetNApplications (); j++)
	{
	  nbapp = DynamicCast<mbr::MbrNeighborApp> (m_node->GetApplication(j));
	  if (nbapp)
	    break;
	}
      NS_ASSERT(nbapp);
      return nbapp->getNb()->GetSettingTime(id);
    }
  else
    {
      std::map<Ipv4Address, std::pair<Vector, Time> >::iterator i = m_table.find (id);
      return i->second.second;
    }
}

/**
 * \brief Adds entry in position table
 */
void 
PositionTable::AddEntry (Ipv4Address id, Vector position)
{
  if (m_nbFromMbr)
    return;

  std::map<Ipv4Address, std::pair<Vector, Time> >::iterator i = m_table.find (id);
  if (i != m_table.end () || id.IsEqual (i->first))
    {
      m_table.erase (id);
      m_table.insert (std::make_pair (id, std::make_pair (position, Simulator::Now ())));
      return;
    }
  

  m_table.insert (std::make_pair (id, std::make_pair (position, Simulator::Now ())));
}

/**
 * \brief Deletes entry in position table
 */
void PositionTable::DeleteEntry (Ipv4Address id)
{
  if (m_nbFromMbr)
    return;

  m_table.erase (id);
}

/**
 * \brief Gets position from position table
 * \param id Ipv4Address to get position from
 * \return Position of that id or NULL if not known
 */
Vector 
PositionTable::GetPosition (Ipv4Address id)
{
  if (m_nbFromMbr)
    {
      Ptr<mbr::MbrNeighborApp> nbapp;
      for (uint32_t j = 0; j < m_node->GetNApplications (); j++)
	{
	  nbapp = DynamicCast<mbr::MbrNeighborApp> (m_node->GetApplication(j));
	  if (nbapp)
	    break;
	}
//      Ptr<Application> app = m_node->GetApplication(0);
//      Ptr<mbr::MbrNeighborApp> nbapp = DynamicCast<mbr::MbrNeighborApp> (app);
      NS_ASSERT(nbapp);
      return nbapp->getNb()->GetCartesianPositionFromIp(id);
    }
  else
    {
      NodeList::Iterator listEnd = NodeList::End ();
      for (NodeList::Iterator i = NodeList::Begin (); i != listEnd; i++)
	{
	  Ptr<Node> node = *i;
	  if (node->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal () == id)
	    {
	      return node->GetObject<MobilityModel> ()->GetPosition ();
	    }
	}
      return PositionTable::GetInvalidPosition ();
    }
}

/**
 * \brief Checks if a node is a neighbour
 * \param id Ipv4Address of the node to check
 * \return True if the node is neighbour, false otherwise
 */
bool
PositionTable::isNeighbour (Ipv4Address id)
{
  if (m_nbFromMbr)
    {
      for (uint32_t j = 0; j < m_node->GetNApplications (); j++)
	{
	  Ptr<mbr::MbrNeighborApp> nbapp = DynamicCast<mbr::MbrNeighborApp> (m_node->GetApplication(j));
	  if (nbapp)
	    return nbapp->getNb()->IsNeighbor(id);
	}
//      Ptr<Application> app = m_node->GetApplication(0);
//      Ptr<mbr::MbrNeighborApp> nbapp = DynamicCast<mbr::MbrNeighborApp> (app);
//      return nbapp->getNb()->IsNeighbor(id);
    }
  else
    {
      std::map<Ipv4Address, std::pair<Vector, Time> >::iterator i = m_table.find (id);
      if (i != m_table.end () || id.IsEqual (i->first))
	{
	  return true;
	}
    }

  return false;
}


/**
 * \brief remove entries with expired lifetime
 */
void 
PositionTable::Purge ()
{
  if (m_nbFromMbr)
    return;

  if(m_table.empty ())
    {
      return;
    }

  std::list<Ipv4Address> toErase;

  std::map<Ipv4Address, std::pair<Vector, Time> >::iterator i = m_table.begin ();
  std::map<Ipv4Address, std::pair<Vector, Time> >::iterator listEnd = m_table.end ();
  
  for (; !(i == listEnd); i++)
    {

      if (m_entryLifeTime + GetEntryUpdateTime (i->first) <= Simulator::Now ())
        {
          toErase.insert (toErase.begin (), i->first);

        }
    }
  toErase.unique ();

  std::list<Ipv4Address>::iterator end = toErase.end ();

  for (std::list<Ipv4Address>::iterator it = toErase.begin (); it != end; ++it)
    {

      m_table.erase (*it);

    }
}

/**
 * \brief clears all entries
 */
void 
PositionTable::Clear ()
{
  if (m_nbFromMbr)
    return;

  m_table.clear ();
}

/**
 * \brief Gets next hop according to GPSR protocol
 * \param position the position of the destination node
 * \param nodePos the position of the node that has the packet
 * \return Ipv4Address of the next hop, Ipv4Address::GetZero () if no nighbour was found in greedy mode
 */
Ipv4Address 
PositionTable::BestNeighbor (Vector position, Vector nodePos)
{
  Ipv4Address bestFoundID;
  double bestFoundDistance;
  double initialDistance = CalculateDistance (nodePos, position);

  if (m_nbFromMbr)
    {
      Ptr<mbr::MbrNeighborApp> nbapp;
      for (uint32_t j = 0; j < m_node->GetNApplications (); j++)
	{
	  nbapp = DynamicCast<mbr::MbrNeighborApp> (m_node->GetApplication(j));
	  if (nbapp)
	    break;
	}
      NS_ASSERT(nbapp);
      if (nbapp->getNb()->NeighborEmpty())
	{
	  NS_LOG_DEBUG ("BestNeighbor table is empty; Position: " << position);
	  return Ipv4Address::GetZero ();
	}

      Vector p;
      bestFoundID = nbapp->getNb()->GetIp(0);
      bestFoundDistance = CalculateDistance (nbapp->getNb()->GetCartesianPosition(0), position);
      int i;
      for (i = 1; i < nbapp->getNb()->GetTableSize(); i++)
        {
	  double dist = CalculateDistance (nbapp->getNb()->GetCartesianPosition(i), position);
          if (bestFoundDistance > dist)
            {
              bestFoundID = nbapp->getNb()->GetIp(i);
              bestFoundDistance = dist;
            }
        }
    }
  else
    {
      Purge ();
      if (m_table.empty ())
	{
	  NS_LOG_DEBUG ("BestNeighbor table is empty; Position: " << position);
	  return Ipv4Address::GetZero ();
	}     //if table is empty (no neighbours)

      bestFoundID = m_table.begin ()->first;
      bestFoundDistance = CalculateDistance (m_table.begin ()->second.first, position);
      std::map<Ipv4Address, std::pair<Vector, Time> >::iterator i;
      for (i = m_table.begin (); !(i == m_table.end ()); i++)
	{
	  if (bestFoundDistance > CalculateDistance (i->second.first, position))
	    {
	      bestFoundID = i->first;
	      bestFoundDistance = CalculateDistance (i->second.first, position);
	    }
	}
    }

  if(initialDistance > bestFoundDistance)
    return bestFoundID;
  else
    return Ipv4Address::GetZero (); //so it enters Recovery-mode

}

std::map<Ipv4Address, std::pair<Vector, Time> >
PositionTable::rng_planarize()
{
  std::map<Ipv4Address, std::pair<Vector, Time> > result;
  Vector myPos;
  Ptr<MobilityModel> MM = m_node->GetObject<MobilityModel> ();
  myPos.x = MM->GetPosition ().x;
  myPos.y = MM->GetPosition ().y;

  if (m_nbFromMbr)
    {
      int temp, index;
      Ptr<mbr::MbrNeighborApp> nbapp;
      for (uint32_t j = 0; j < m_node->GetNApplications (); j++)
	{
	  nbapp = DynamicCast<mbr::MbrNeighborApp> (m_node->GetApplication(j));
	  if (nbapp)
	    break;
	}

      for (index = 0; index < nbapp->getNb()->GetTableSize(); index++)
	{
	  double mdis = CalculateDistance(myPos, nbapp->getNb()->GetCartesianPosition(index));
	  for (temp = index+1; temp < nbapp->getNb()->GetTableSize(); temp++)
	    {
	      if (nbapp->getNb()->GetIp(index) != nbapp->getNb()->GetIp(temp))
		{
		  double tempdis1 = CalculateDistance(myPos, nbapp->getNb()->GetCartesianPosition(temp));
		  double tempdis2 = CalculateDistance(nbapp->getNb()->GetCartesianPosition(index),
						      nbapp->getNb()->GetCartesianPosition(temp));
		  if(tempdis1 < mdis && tempdis2 < mdis)
		    break;
		}
	    }
	}
      if (temp == nbapp->getNb()->GetTableSize())
	{
	  result.insert(std::make_pair (nbapp->getNb()->GetIp(index),
					std::make_pair (nbapp->getNb()->GetCartesianPosition(index),
							Simulator::Now ())));
	}

    }
  else
    {
      std::map<Ipv4Address, std::pair<Vector, Time> >::iterator temp, index;


      Purge ();
      for (index = m_table.begin (); index != m_table.end (); index++)
	{
	  double mdis = CalculateDistance(myPos, index->second.first);
	  for (temp = index; temp != m_table.end(); temp++)
	    {
	      if (temp->first != index->first)
		{
		  double tempdis1 = CalculateDistance(myPos, temp->second.first);
		  double tempdis2 = CalculateDistance(index->second.first, temp->second.first);
		  if(tempdis1 < mdis && tempdis2 < mdis)
		    break;
		}
	    }
	  if (temp == m_table.end())
	    {
	      result.insert(std::make_pair (index->first, std::make_pair (index->second.first, Simulator::Now ())));
	    }
	}
    }
  return result;
}

double
PositionTable::angle(Vector v1, Vector v2){
  double x1 = v1.x;
  double y1 = v1.y;
  double x2 = v2.x;
  double y2 = v2.y;
  double line_len = sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));

  double sin_theta, cos_theta;
  double theta;

  if(line_len == 0.0){
    NS_LOG_DEBUG("2 nodes are the same\n");
    return -1.0;
  }

  sin_theta = (y2-y1)/line_len;
  cos_theta = (x2-x1)/line_len;

  theta = acos(cos_theta);

  if(sin_theta<0){
    theta = 2*PI - theta;
  }

  return theta;
}

/* To check the line from me to theother, and the line from source
 * and destination is intersecting each other or not
 * Note: 2 line segments intersects each other if they have a common
 *       point, BUT here, if the common point is the end point,
 *       we don't count it.
 */
int
PositionTable::intersect(Ipv4Address theother, Vector srcPos, Vector dstPos){
  //line 1 (x1,y1)--(x2,y2) is the segment
  //line 2 (x3,y3)--(x4,y4) is the xD

  NS_ASSERT(isNeighbour(theother));
  Vector oPos = GetPosition(theother);
  Ptr<MobilityModel> MM = m_node->GetObject<MobilityModel> ();

  double x1 = MM->GetPosition ().x;
  double y1 = MM->GetPosition ().y;
  double x2 = oPos.x;
  double y2 = oPos.y;
  double x3 = srcPos.x;
  double y3 = srcPos.y;
  double x4 = dstPos.x;
  double y4 = dstPos.y;

  double a1 = y2 - y1;
  double b1 = x1 - x2;
  double c1 = x2*y1 - x1*y2;

  double a2 = y4 - y3;
  double b2 = x3 - x4;
  double c2 = x4*y3 - x3*y4;

  double denom = a1*b2 - a2*b1;

  double x;//, y; //the result;

  if(denom == 0) {
    return 0; //parallel lines;
  }

  x = (b1*c2 - b2*c1)/denom;
 // y = (a2*c1 - a1*c2)/denom;

  if(x > MIN(x1, x2) && x < MAX(x1, x2) &&
     x > MIN(x3, x4) && x < MAX(x3, x4))
    return 1;
  else return 0;
}


Ipv4Address
PositionTable::peri_nexthop(Vector lastPos, Vector srcPos, Vector dstPos)
{
  std::map<Ipv4Address, std::pair<Vector, Time> > planar_neighbors;
  std::map<Ipv4Address, std::pair<Vector, Time> >::iterator temp;
  double alpha=999, minangle=999;
  Ipv4Address nexthop;
  Vector nexthopPos;

  planar_neighbors = rng_planarize();

  Vector myPos;
  Ptr<MobilityModel> MM = m_node->GetObject<MobilityModel> ();
  myPos.x = MM->GetPosition ().x;
  myPos.y = MM->GetPosition ().y;

  if (lastPos.x != -1 && lastPos.y != -1)
    {
//      NS_ASSERT(isNeighbour(last));
//      Vector lastPos = GetPosition(last);
      alpha = angle(myPos, lastPos);
    }
  else
    {
      alpha = angle(myPos, dstPos);
    }

  for (temp = planar_neighbors.begin(); temp != planar_neighbors.end(); temp++)
    {
      if (temp->second.first.x != lastPos.x && temp->second.first.y != lastPos.y)
	{
	  double delta;
	  delta = angle(myPos, temp->second.first);
	  delta = delta - alpha;
	  if(delta < 0.0) {
	    delta = 2*PI + delta;
	  }
	  if(delta < minangle){
	    minangle = delta;
	    nexthop = temp->first;
	    nexthopPos = temp->second.first;
	  }
	}
    }
  if (planar_neighbors.size() > 1 && intersect(nexthop, srcPos, dstPos))
    {
      return peri_nexthop(nexthopPos, srcPos, dstPos);
    }
  return nexthop;

}


/**
 * \brief Gets next hop according to GPSR recovery-mode protocol (right hand rule)
 * \param previousHop the position of the node that sent the packet to this node
 * \param nodePos the position of the destination node
 * \return Ipv4Address of the next hop, Ipv4Address::GetZero () if no nighbour was found in greedy mode
 */
Ipv4Address
PositionTable::BestAngle (Vector previousHop, Vector nodePos)
{
  double tmpAngle;
  Ipv4Address bestFoundID = Ipv4Address::GetZero ();
  double bestFoundAngle = 360;
  NS_LOG_LOGIC("previousHop: "<<previousHop<<" myPos: "<<nodePos);

  Ptr<mbr::MbrNeighborApp> nbapp;
  if (!m_nbFromMbr)
    {

      Purge ();
    }

  std::map<Ipv4Address, std::pair<Vector, Time> > planar_neighbors;
  planar_neighbors = rng_planarize();

  if (m_table.empty ())
    {
      NS_LOG_DEBUG ("BestNeighbor table is empty; Position: " << nodePos);
      return Ipv4Address::GetZero ();
    }     //if table is empty (no neighbours)

  std::map<Ipv4Address, std::pair<Vector, Time> >::iterator i;

  for (i = planar_neighbors.begin(); i != planar_neighbors.end(); i++)
//      for (i = m_table.begin (); !(i == m_table.end ()); i++)
    {
      tmpAngle = GetAngle(nodePos, previousHop, i->second.first);
      if (bestFoundAngle > tmpAngle && tmpAngle != 0)
	{
	  bestFoundID = i->first;
	  bestFoundAngle = tmpAngle;
	}
    }
  if(bestFoundID == Ipv4Address::GetZero ()) //only if the only neighbour is who sent the packet
    {
      bestFoundID = m_table.begin ()->first;
    }



  return bestFoundID;
}


//Gives angle between the vector CentrePos-Refpos to the vector CentrePos-node counterclockwise
double 
PositionTable::GetAngle (Vector centrePos, Vector refPos, Vector node){
  double const pi = 4*atan(1);

  std::complex<double> A = std::complex<double>(centrePos.x,centrePos.y);
  std::complex<double> B = std::complex<double>(node.x,node.y);
  std::complex<double> C = std::complex<double>(refPos.x,refPos.y);   //Change B with C if you want angles clockwise

  std::complex<double> AB; //reference edge
  std::complex<double> AC;
  std::complex<double> tmp;
  std::complex<double> tmpCplx;

  std::complex<double> Angle;

  AB = B - A;
  AB = (real(AB)/norm(AB)) + (std::complex<double>(0.0,1.0)*(imag(AB)/norm(AB)));

  AC = C - A;
  AC = (real(AC)/norm(AC)) + (std::complex<double>(0.0,1.0)*(imag(AC)/norm(AC)));

  tmp = log(AC/AB);
  tmpCplx = std::complex<double>(0.0,-1.0);
  Angle = tmp*tmpCplx;
  Angle *= (180/pi);
  if (real(Angle)<0)
    Angle = 360+real(Angle);

  return real(Angle);

}





/**
 * \ProcessTxError
 */
void PositionTable::ProcessTxError (WifiMacHeader const & hdr)
{
}



//FIXME ainda preciso disto agr que o LS ja n está aqui???????

/**
 * \brief Returns true if is in search for destionation
 */
bool PositionTable::IsInSearch (Ipv4Address id)
{
  return false;
}

bool PositionTable::HasPosition (Ipv4Address id)
{
  return true;
}


}   // gpsr
} // ns3
