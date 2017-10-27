/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "geoSVR-packet.h"
#include "ns3/address-utils.h"
#include "ns3/packet.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("geoSVRPacket");

namespace ns3 {
namespace geoSVR {

NS_OBJECT_ENSURE_REGISTERED (TypeHeader);

TypeHeader::TypeHeader (MessageType t)
  : m_type (t),
    m_valid (true)
{                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             
}

TypeId
TypeHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::geoSVR::TypeHeader")
    .SetParent<Header> ()
	.SetGroupName("geoSVR")
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
    case GEOSVRTYPE_HELLO:
    case GEOSVRTYPE_DATAPACKET:
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
    case GEOSVRTYPE_HELLO:
      {
        os << "HELLO";
        break;
      }
    case GEOSVRTYPE_DATAPACKET:
      {
        os << "DATAPACKET";
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

HelloHeader::HelloHeader (double x, double y, double s, double h, Ipv4Address dst) : m_x (x), m_y (y), m_s (s), m_h (h), m_dst (dst)
{
}

NS_OBJECT_ENSURE_REGISTERED (HelloHeader);

TypeId
HelloHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::geoSVR::HelloHeader")
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
  return 36;
}

void
HelloHeader::Serialize (Buffer::Iterator i) const
{
  NS_LOG_DEBUG ("Serialize X " << m_x << " Y " << m_y);

  uint64_t x, y, s, h;
  memcpy(&x, &m_x, 8);
  memcpy(&y, &m_y, 8);
  memcpy(&s, &m_s, 8);
  memcpy(&h, &m_h, 8);
  i.WriteHtonU64 (x);
  i.WriteHtonU64 (y);
  i.WriteHtonU64 (s);
  i.WriteHtonU64 (h);
  WriteTo (i, m_dst);
}

uint32_t
HelloHeader::Deserialize (Buffer::Iterator start)
{

  Buffer::Iterator i = start;
  uint64_t x, y, s, h;
  x = i.ReadNtohU64 ();
  y = i.ReadNtohU64 ();
  s = i.ReadNtohU64 ();
  h = i.ReadNtohU64 ();
  memcpy(&m_x, &x, 8);
  memcpy(&m_y, &y, 8);
  memcpy(&m_s, &s, 8);
  memcpy(&m_h, &h, 8);
  ReadFrom (i, m_dst);

  NS_LOG_DEBUG ("Deserialize X " << m_x << " Y " << m_y);
  uint32_t dist = i.GetDistanceFrom (start);
  NS_ASSERT (dist == GetSerializedSize ());
  return dist;
}

void
HelloHeader::Print (std::ostream &os) const
{
  os << " PositionX: " << m_x
     << " PositionY: " << m_y
     << " Speed: " << m_s
     << " Direction: " << m_h;
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
  return (m_x == o.m_x && m_y == o.m_y && m_s == o.m_s && m_h == o.m_h && m_dst == o.m_dst);
}


//-----------------------------------------------------------------------------
// Datapacket
//-----------------------------------------------------------------------------
DatapacketHeader::DatapacketHeader (uint32_t length, uint32_t m1, uint32_t t1, uint32_t c1,
                                    //uint32_t  m_seq,
                                    double sx, double sy, double dx, double dy, char  *path) :
                                    m_length (length), m (m1), t (t1), c (c1), m_sx (sx), m_sy (sy), m_dx (dx), m_dy (dy), m_path (path)
{
}

NS_OBJECT_ENSURE_REGISTERED (DatapacketHeader);

TypeId
DatapacketHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::geoSVR::DatapacketHeader")
    .SetParent<Header> ()
    .SetGroupName("geoSVR")
    .AddConstructor<DatapacketHeader> ()
  ;
  return tid;
}

TypeId
DatapacketHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

uint32_t
DatapacketHeader::GetSerializedSize () const
{
  return 48 + m_length;
}

void
DatapacketHeader::Serialize (Buffer::Iterator i) const
{
  uint64_t sx, sy, dx, dy;
  i.WriteHtonU32 (m_length);
  i.WriteHtonU32 (m);
  i.WriteHtonU32 (t);
  i.WriteHtonU32 (c);
  memcpy(&sx, &m_sx, 8);
  memcpy(&sy, &m_sy, 8);
  memcpy(&dx, &m_dx, 8);
  memcpy(&dy, &m_dy, 8);
  i.WriteHtonU64 (sx);
  i.WriteHtonU64 (sy);
  i.WriteHtonU64 (dx);
  i.WriteHtonU64 (dy);
  for(uint32_t j = 1; j <= m_length; ++j)
	  i.WriteU8 (m_path[j]);

  free(m_path);

}

uint32_t
DatapacketHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  uint64_t sx, sy, dx, dy;
  m_length = i.ReadNtohU32 ();
  m = i.ReadNtohU32 ();
  t = i.ReadNtohU32 ();
  c = i.ReadNtohU32 ();
  sx = i.ReadNtohU64 ();
  sy = i.ReadNtohU64 ();
  dx = i.ReadNtohU64 ();
  dy = i.ReadNtohU64 ();
  memcpy(&m_sx, &sx, 8);
  memcpy(&m_sy, &sy, 8);
  memcpy(&m_dx, &dx, 8);
  memcpy(&m_dy, &dy, 8);
  if(m_length != 0)
  {
	  m_path = (char *)malloc((m_length+1) * sizeof(char));
	  m_path[0] = m_length;
	  for(uint32_t j = 1; j <= m_length; ++j)
		  m_path[j] = i.ReadU8();
  }

  uint32_t dist = i.GetDistanceFrom (start);
  NS_ASSERT (dist == GetSerializedSize ());
  return dist;
}

// encode and decode path

bool
DatapacketHeader::encode_path(const std::vector<int>& paths)
{
    char *enpath = NULL;

    enpath = (char *)malloc(sizeof(char) * paths.size() * 2);

    enpath[0] = paths.size() * 2 - 1;

    m_length = enpath[0];

    int i;
    std::vector<int>::const_iterator iter;
    for (i = 1, iter = paths.begin();
         iter != paths.end(); ++iter, i += 2) {
        enpath[i] = *iter;
        enpath[i + 1] = ',';
    }

    if(m_path != NULL)
    	free(m_path);
    m_path = enpath;

    if(m_path == NULL)
    	return false;
    else
    	return true;
}

bool
DatapacketHeader::decode_path(std::vector<int>& paths)
{
    char *enpaths = m_path;
    if(enpaths == NULL)
    	return false;

    int len = enpaths[0];

    for (int i = 1; i <= len; i += 2) {
        paths.push_back(enpaths[i]);
    }

    return true;
}

void
DatapacketHeader::Print (std::ostream &os) const
{
  os << " sx: " << m_sx
     << " sy: " << m_sy
     << " dx: " << m_dx
     << " dy: " << m_dy
	 << " path: ";
  for(uint32_t i = 1; i <= m_length; ++i)
	  os << m_path[i] << ' ';
  os << std::endl;
}

std::ostream &
operator<< (std::ostream & os, DatapacketHeader const & h)
{
  h.Print (os);
  return os;
}

bool
DatapacketHeader::operator== (DatapacketHeader const & o) const
{
  return (m_sx == o.m_sx &&
          m_sy == o.m_sy &&
          m_dx == o.m_dy &&
          m_dy == o.m_dy);
}

}
}





