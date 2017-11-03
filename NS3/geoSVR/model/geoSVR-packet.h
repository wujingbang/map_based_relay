/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef GEOSVRPACKET_H
#define GEOSVRPACKET_H

#include <iostream>
#include "ns3/header.h"
#include "ns3/enum.h"
#include "ns3/ipv4-address.h"
#include <map>
#include "ns3/nstime.h"
#include "ns3/vector.h"


namespace ns3 {
namespace geoSVR {

enum MessageType
{
  GEOSVRTYPE_HELLO,         //!< geoSVRTYPE_HELLO
  GEOSVRTYPE_DATAPACKET,            //!< geoSVRTYPE_DATAPACKET
};

/**
 * \ingroup geoSVR
 * \brief  geoSVR types
 */
class TypeHeader : public Header
{
public:
  /// c-tor
  TypeHeader (MessageType t = GEOSVRTYPE_HELLO);

  ///\name Header serialization/deserialization
  //\{
  static TypeId GetTypeId ();
  TypeId GetInstanceTypeId () const;
  uint32_t GetSerializedSize () const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  void Print(std::ostream &os) const;
  //\}
 
  /// Return type
  MessageType Get () const { return m_type; }
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

// -------------------------
// HelloHeader
// -------------------------

class HelloHeader : public Header
{
public:
  /// c-tor
  HelloHeader (double x = 0, double y = 0, double s = 0, double h = 0, Ipv4Address dst = Ipv4Address ());

  ///\name Header serialization/deserialization
  //\{
  static TypeId GetTypeId ();
  TypeId GetInstanceTypeId () const;
  uint32_t GetSerializedSize () const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  void Print(std::ostream &os) const;
  //\}

  ///\name Fields
  //\{
  void SetX (double x) { m_x = x; }
  double GetX () const { return m_x; }

  void SetY (double y) { m_y = y; }
  double GetY () const { return m_y; }

  void SetS (double s){ m_s = s; }
  double GetS () const { return m_s; }

  double GetH () const { return m_h; }
  void SetH (double h) { m_h = h; }

  Ipv4Address GetDst () const { return m_dst; }
  void SetDst (Ipv4Address dst) { m_dst = dst; }

  //\}
  bool operator== (HelloHeader const & o) const;

private:
  double         m_x;          
  double         m_y;          
  double         m_s;
  double         m_h;
  Ipv4Address   m_dst;
};

std::ostream & operator<< (std::ostream & os, HelloHeader const &);

// -------------------------
// DatapacketHeader 
// -------------------------

class DatapacketHeader : public Header 
{
public:
  /// c-tor
  DatapacketHeader (
		  	  	  	uint32_t       length = 0,
		  	  	  	uint32_t       m1 = 0,
                    uint32_t       t1 = 0,
                    uint32_t       c1 = 0,
                    //uint32_t     m_seq,
                    double         sx = 0,
                    double         sy = 0,
                    double         dx = 0,
                    double         dy = 0,
                    uint8_t      *path = NULL);
              

  // Header serialization/deserialization
  static TypeId GetTypeId ();
  TypeId GetInstanceTypeId () const;
  uint32_t GetSerializedSize () const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  void Print(std::ostream &os) const;


  // Fields
  //void SetSeq (uint32_t seq) { m_seq = seq; }
  //uint32_t GetSeq () const { return m_seq; }

  void SetLength (uint32_t length) { m_length = length; }
  uint32_t GetLength () const { return m_length; }

  void SetSx (double sx) { m_sx = sx; }
  double GetSx () const { return m_sx; }

  void SetSy (double sy) { m_sy = sy; }
  double GetSy () const { return m_sy; }

  void SetDx (double dx) { m_dx = dx; }
  double GetDx () const { return m_dx; }

  void SetDy (double dy) { m_dy = dy; }
  double GetDy () const { return m_dy; }

  uint8_t* GetPath() const { return m_path;}


  bool encode_path(const std::vector<int>& paths);
  bool decode_path(std::vector<int>& paths);


  bool operator== (DatapacketHeader const & o) const;

private:
  uint32_t		  m_length; // path length
  uint32_t        m : 1;
  uint32_t        t : 2;
  uint32_t        c : 1;
  //uint32_t       m_seq;
  double          m_sx;
  double          m_sy;
  double          m_dx;
  double          m_dy;
  uint8_t       *m_path;
// char           app[0];
};

std::ostream & operator<< (std::ostream & os, DatapacketHeader const &);

} 
}
#endif
