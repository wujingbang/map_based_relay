/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "gpsr-packet.h"
#include "ns3/address-utils.h"
#include "ns3/packet.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("GpsrPacket");

namespace ns3 {
namespace gpsr {

NS_OBJECT_ENSURE_REGISTERED (TypeHeader);

TypeHeader::TypeHeader (MessageType t = GPSRTYPE_HELLO)
  : m_type (t),
    m_valid (true)
{
}

TypeId
TypeHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::gpsr::TypeHeader")
    .SetParent<Header> ()
    .AddConstructor<TypeHeader> ()
  ;
  return tid;
}

TypeId
TypeHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

uint32_t
TypeHeader::GetSerializedSize () const
{
  return 1;
}

void
TypeHeader::Serialize (Buffer::Iterator i) const
{
  i.WriteU8 ((uint8_t) m_type);
}

uint32_t
TypeHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  uint8_t type = i.ReadU8 ();
  m_valid = true;
  switch (type)
    {
    case GPSRTYPE_HELLO:
    case GPSRTYPE_POS:
      {
        m_type = (MessageType) type;
        break;
      }
    default:
      m_valid = false;
    }
  uint32_t dist = i.GetDistanceFrom (start);
  NS_ASSERT (dist == GetSerializedSize ());
  return dist;
}

void
TypeHeader::Print (std::ostream &os) const
{
  switch (m_type)
    {
    case GPSRTYPE_HELLO:
      {
        os << "HELLO";
        break;
      }
    case GPSRTYPE_POS:
      {
        os << "POSITION";
        break;
      }
    default:
      os << "UNKNOWN_TYPE";
    }
}

bool
TypeHeader::operator== (TypeHeader const & o) const
{
  return (m_type == o.m_type && m_valid == o.m_valid);
}

std::ostream &
operator<< (std::ostream & os, TypeHeader const & h)
{
  h.Print (os);
  return os;
}

//-----------------------------------------------------------------------------
// HELLO
//-----------------------------------------------------------------------------
HelloHeader::HelloHeader (double originPosx, double originPosy)
  : m_originPosx (originPosx),
    m_originPosy (originPosy)
{
}

NS_OBJECT_ENSURE_REGISTERED (HelloHeader);

TypeId
HelloHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::gpsr::HelloHeader")
    .SetParent<Header> ()
    .AddConstructor<HelloHeader> ()
  ;
  return tid;
}

TypeId
HelloHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

uint32_t
HelloHeader::GetSerializedSize () const
{
  return 16;
}

void
HelloHeader::Serialize (Buffer::Iterator i) const
{
  NS_LOG_DEBUG ("Serialize X " << m_originPosx << " Y " << m_originPosy);

  uint64_t x,y;
  memcpy(&x, &m_originPosx, 8);
  memcpy(&y, &m_originPosy, 8);
  i.WriteHtonU64 (x);
  i.WriteHtonU64 (y);

}

uint32_t
HelloHeader::Deserialize (Buffer::Iterator start)
{

  Buffer::Iterator i = start;
  uint64_t x,y;
  x = i.ReadNtohU64 ();
  y = i.ReadNtohU64 ();
  memcpy(&m_originPosx, &x, 8);
  memcpy(&m_originPosy, &y, 8);

  NS_LOG_DEBUG ("Deserialize X " << m_originPosx << " Y " << m_originPosy);

  uint32_t dist = i.GetDistanceFrom (start);
  NS_ASSERT (dist == GetSerializedSize ());
  return dist;
}

void
HelloHeader::Print (std::ostream &os) const
{
  os << " PositionX: " << m_originPosx
     << " PositionY: " << m_originPosy;
}

std::ostream &
operator<< (std::ostream & os, HelloHeader const & h)
{
  h.Print (os);
  return os;
}



bool
HelloHeader::operator== (HelloHeader const & o) const
{
  return (m_originPosx == o.m_originPosx && m_originPosy == o.m_originPosy);
}





//-----------------------------------------------------------------------------
// Position
//-----------------------------------------------------------------------------
PositionHeader::PositionHeader()
  : m_dstPosx (0.0),
    m_dstPosy (0.0),
    m_updated (0),
    m_recPosx (0.0),
    m_recPosy (0.0),
    m_inRec (0),
    m_lastPosx (0.0),
    m_lastPosy (0.0),
    m_srcPosx (0.0),
    m_srcPosy (0.0)
{

}
PositionHeader::PositionHeader (double srcPosx, double srcPosy,
				double dstPosx, double dstPosy, uint32_t updated,
				double recPosx, double recPosy, uint8_t inRec,
				double lastPosx, double lastPosy)
  : m_dstPosx (dstPosx),
    m_dstPosy (dstPosy),
    m_updated (updated),
    m_recPosx (recPosx),
    m_recPosy (recPosy),
    m_inRec (inRec),
    m_lastPosx (lastPosx),
    m_lastPosy (lastPosy),
    m_srcPosx (srcPosx),
    m_srcPosy (srcPosy)
{
}

NS_OBJECT_ENSURE_REGISTERED (PositionHeader);

TypeId
PositionHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::gpsr::PositionHeader")
    .SetParent<Header> ()
    .AddConstructor<PositionHeader> ()
  ;
  return tid;
}

TypeId
PositionHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

uint32_t
PositionHeader::GetSerializedSize () const
{
  return 69;//53;
}

void
PositionHeader::Serialize (Buffer::Iterator i) const
{
  uint64_t x,y;
  memcpy(&x, &m_srcPosx, 8);
  memcpy(&y, &m_srcPosy, 8);
  i.WriteHtonU64 (x);
  i.WriteHtonU64 (y);
  memcpy(&x, &m_dstPosx, 8);
  memcpy(&y, &m_dstPosy, 8);
  i.WriteHtonU64 (x);
  i.WriteHtonU64 (y);
  i.WriteU32 (m_updated);
  memcpy(&x, &m_recPosx, 8);
  memcpy(&y, &m_recPosy, 8);
  i.WriteHtonU64 (x);
  i.WriteHtonU64 (y);
  i.WriteU8 (m_inRec);
  memcpy(&x, &m_lastPosx, 8);
  memcpy(&y, &m_lastPosy, 8);
  i.WriteHtonU64 (x);
  i.WriteHtonU64 (y);
}

uint32_t
PositionHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  uint64_t x,y;
  x = i.ReadNtohU64 ();
  y = i.ReadNtohU64 ();
  memcpy(&m_srcPosx, &x, 8);
  memcpy(&m_srcPosy, &y, 8);
  x = i.ReadNtohU64 ();
  y = i.ReadNtohU64 ();
  memcpy(&m_dstPosx, &x, 8);
  memcpy(&m_dstPosy, &y, 8);
  m_updated = i.ReadU32 ();
  x = i.ReadNtohU64 ();
  y = i.ReadNtohU64 ();
  memcpy(&m_recPosx, &x, 8);
  memcpy(&m_recPosy, &y, 8);
  m_inRec = i.ReadU8 ();
  x = i.ReadNtohU64 ();
  y = i.ReadNtohU64 ();
  memcpy(&m_lastPosx, &x, 8);
  memcpy(&m_lastPosy, &y, 8);

  uint32_t dist = i.GetDistanceFrom (start);
  NS_ASSERT (dist == GetSerializedSize ());
  return dist;
}

void
PositionHeader::Print (std::ostream &os) const
{
  os << " PositionX: "  << m_dstPosx
     << " PositionY: " << m_dstPosy
     << " Updated: " << m_updated
     << " RecPositionX: " << m_recPosx
     << " RecPositionY: " << m_recPosy
     << " inRec: " << m_inRec
     << " LastPositionX: " << m_lastPosx
     << " LastPositionY: " << m_lastPosy;
}

std::ostream &
operator<< (std::ostream & os, PositionHeader const & h)
{
  h.Print (os);
  return os;
}



bool
PositionHeader::operator== (PositionHeader const & o) const
{
  return (m_srcPosx == o.m_srcPosx && m_srcPosy == o.m_srcPosy &&m_dstPosx == o.m_dstPosx && m_dstPosy == o.m_dstPosy && m_updated == o.m_updated && m_recPosx == o.m_recPosx && m_recPosy == o.m_recPosy && m_inRec == o.m_inRec && m_lastPosx == o.m_lastPosx && m_lastPosy == o.m_lastPosy);
}


}
}





