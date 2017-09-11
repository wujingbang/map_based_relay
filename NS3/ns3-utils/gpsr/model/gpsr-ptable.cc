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
  m_entryLifeTime = Seconds (2); //FIXME fazer isto parametrizavel de acordo com tempo de hello

}

PositionTable::PositionTable (bool nbFromMbr)
{
  m_nbFromMbr = nbFromMbr;
  m_txErrorCallback = MakeCallback (&PositionTable::ProcessTxError, this);
  m_entryLifeTime = Seconds (2); //FIXME fazer isto parametrizavel de acordo com tempo de hello

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
      return nbapp->getNb()->GetPositionFromIp(id);
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
      if (nbapp->getNb()->NeighborEmpty())
	{
	  NS_LOG_DEBUG ("BestNeighbor table is empty; Position: " << nodePos);
	  return Ipv4Address::GetZero ();
	}

      int i;
      for (i = 0; i < nbapp->getNb()->GetTableSize(); i++)
        {
	  tmpAngle = GetAngle(nodePos, previousHop, nbapp->getNb()->GetPosition(i));
	  if (bestFoundAngle > tmpAngle && tmpAngle != 0)
	    {
	      bestFoundID = nbapp->getNb()->GetIp(i);
	      bestFoundAngle = tmpAngle;
	    }
        }
      if(bestFoundID == Ipv4Address::GetZero ()) //only if the only neighbour is who sent the packet
        {
          bestFoundID = nbapp->getNb()->GetIp(0);
        }
    }
  else
    {
      Purge ();

      if (m_table.empty ())
	{
	  NS_LOG_DEBUG ("BestNeighbor table is empty; Position: " << nodePos);
	  return Ipv4Address::GetZero ();
	}     //if table is empty (no neighbours)

      std::map<Ipv4Address, std::pair<Vector, Time> >::iterator i;

      for (i = m_table.begin (); !(i == m_table.end ()); i++)
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
    }


  return bestFoundID;
}


//Gives angle between the vector CentrePos-Refpos to the vector CentrePos-node counterclockwise
double 
PositionTable::GetAngle (Vector centrePos, Vector refPos, Vector node){
  double const PI = 4*atan(1);

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
  Angle *= (180/PI);
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
