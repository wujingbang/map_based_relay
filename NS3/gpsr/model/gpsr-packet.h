/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef GPSRPACKET_H
#define GPSRPACKET_H

#include <iostream>
#include "ns3/header.h"
#include "ns3/enum.h"
#include "ns3/ipv4-address.h"
#include <map>
#include "ns3/nstime.h"
#include "ns3/vector.h"

namespace ns3 {
namespace gpsr {



enum MessageType
{
  GPSRTYPE_HELLO  = 1,         //!< GPSRTYPE_HELLO
  GPSRTYPE_POS = 2,            //!< GPSRTYPE_POS
};

/**
 * \ingroup gpsr
 * \brief GPSR types
 */
class TypeHeader : public Header
{
public:
  /// c-tor
  TypeHeader (MessageType t);

  ///\name Header serialization/deserialization
  //\{
  static TypeId GetTypeId ();
  TypeId GetInstanceTypeId () const;
  uint32_t GetSerializedSize () const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  void Print (std::ostream &os) const;
  //\}

  /// Return type
  MessageType Get () const
  {
    return m_type;
  }
  /// Check that type if valid
  bool IsValid () const
  {
    return m_valid; //FIXME that way it wont work
  }
  bool operator== (TypeHeader const & o) const;
private:
  MessageType m_type;
  bool m_valid;
};

std::ostream & operator<< (std::ostream & os, TypeHeader const & h);

class HelloHeader : public Header
{
public:
  /// c-tor
  HelloHeader (double originPosx = 0, double originPosy = 0);

  ///\name Header serialization/deserialization
  //\{
  static TypeId GetTypeId ();
  TypeId GetInstanceTypeId () const;
  uint32_t GetSerializedSize () const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  void Print (std::ostream &os) const;
  //\}

  ///\name Fields
  //\{
  void SetOriginPosx (double posx)
  {
    m_originPosx = posx;
  }
  double GetOriginPosx () const
  {
    return m_originPosx;
  }
  void SetOriginPosy (double posy)
  {
    m_originPosy = posy;
  }
  double GetOriginPosy () const
  {
    return m_originPosy;
  }
  //\}


  bool operator== (HelloHeader const & o) const;
private:
  double         m_originPosx;          ///< Originator Position x
  double         m_originPosy;          ///< Originator Position x
};

std::ostream & operator<< (std::ostream & os, HelloHeader const &);

class PositionHeader : public Header
{
public:
  /// c-tor
  PositionHeader();
  PositionHeader (double srcPosx, double srcPosy,
		  double dstPosx, double dstPosy, uint32_t updated,
		  double recPosx, double recPosy, uint8_t inRec,
		  double lastPosx, double lastPosy);

  ///\name Header serialization/deserialization
  //\{
  static TypeId GetTypeId ();
  TypeId GetInstanceTypeId () const;
  uint32_t GetSerializedSize () const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  void Print (std::ostream &os) const;
  //\}

  ///\name Fields
  //\{
  void SetDstPosx (double posx)
  {
    m_dstPosx = posx;
  }
  double GetDstPosx () const
  {
    return m_dstPosx;
  }
  void SetDstPosy (double posy)
  {
    m_dstPosy = posy;
  }
  double GetDstPosy () const
  {
    return m_dstPosy;
  }
  void SetUpdated (uint32_t updated)
  {
    m_updated = updated;
  }
  uint32_t GetUpdated () const
  {
    return m_updated;
  }
  void SetRecPosx (double posx)
  {
    m_recPosx = posx;
  }
  double GetRecPosx () const
  {
    return m_recPosx;
  }
  void SetRecPosy (double posy)
  {
    m_recPosy = posy;
  }
  double GetRecPosy () const
  {
    return m_recPosy;
  }
  void SetInRec (uint8_t rec)
  {
    m_inRec = rec;
  }
  uint8_t GetInRec () const
  {
    return m_inRec;
  }
  void SetLastPosx (double posx)
  {
    m_lastPosx = posx;
  }
  double GetLastPosx () const
  {
    return m_lastPosx;
  }
  void SetLastPosy (double posy)
  {
    m_lastPosy = posy;
  }
  double GetLastPosy () const
  {
    return m_lastPosy;
  }
  double
  getSrcPosx () const
  {
	return m_srcPosx;
  }

  void
  setSrcPosx (double srcPosx)
  {
	m_srcPosx = srcPosx;
  }

  double
  getSrcPosy () const
  {
	return m_srcPosy;
  }

  void
  setSrcPosy (double srcPosy)
  {
	m_srcPosy = srcPosy;
  }


  bool operator== (PositionHeader const & o) const;


private:
  double         m_dstPosx;          ///< Destination Position x
  double         m_dstPosy;          ///< Destination Position x
  uint32_t         m_updated;          ///< Time of last update
  double         m_recPosx;          ///< x of position that entered Recovery-mode
  double         m_recPosy;          ///< y of position that entered Recovery-mode
  uint8_t          m_inRec;          ///< 1 if in Recovery-mode, 0 otherwise
  double         m_lastPosx;          ///< x of position of previous hop
  double         m_lastPosy;          ///< y of position of previous hop

  double	m_srcPosx;
  double	m_srcPosy;
};

std::ostream & operator<< (std::ostream & os, PositionHeader const &);

}
}
#endif /* GPSRPACKET_H */
