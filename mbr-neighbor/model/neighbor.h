/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 IITP RAS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Based on
 *      NS-2 AODV model developed by the CMU/MONARCH group and optimized and
 *      tuned by Samir Das and Mahesh Marina, University of Cincinnati;
 *
 *      AODV-UU implementation by Erik Nordström of Uppsala University
 *      http://core.it.uu.se/core/index.php/AODV-UU
 *
 * Authors: Elena Buchatskaia <borovkovaes@iitp.ru>
 *          Pavel Boyko <boyko@iitp.ru>
 */

#ifndef MBRNEIGHBOR_H
#define MBRNEIGHBOR_H

#include "ns3/mbr-common.h"

#include "ns3/simulator.h"
#include "ns3/timer.h"
#include "ns3/ipv4-address.h"
#include "ns3/callback.h"
#include "ns3/wifi-mac-header.h"
#include "ns3/arp-cache.h"
#include <vector>

namespace ns3
{
namespace mbr
{
/**
 * \ingroup mbr
 * \brief maintain list of active neighbors
 */
class Neighbors
{
public:
  /// c-tor
  Neighbors (Time delay);
  /// Neighbor description
  struct Neighbor
  {
    Ipv4Address m_neighborAddress;
    Mac48Address m_hardwareAddress;
    Time m_expireTime;
    bool close;
    uint64_t m_geohash;
    uint16_t m_direction;
    double m_x;
    double m_y;



    Neighbor (Ipv4Address ip, Mac48Address mac, Time t, uint64_t geohash, uint16_t direction, double x, double y) :
      m_neighborAddress (ip), m_hardwareAddress (mac), m_expireTime (t),close (false),
      m_geohash(geohash), m_direction(direction), m_x(x), m_y(y)
    {
    }
  };
  /// Return expire time for neighbor node with address addr, if exists, else return 0.
  Time GetExpireTime (Ipv4Address addr);
  /// Check that node with address addr  is neighbor
  bool IsNeighbor (Ipv4Address addr);
  /// Update expire time for entry with address addr, if it exists, else add new entry
  void Update (Ipv4Address addr, Time expire, const uint8_t *mac, uint64_t geohash, uint16_t direction, double x, double y);
  /// Remove all expired entries
  void Purge ();
  /// Schedule m_ntimer.
  void ScheduleTimer ();
  /// Remove all entries
  void Clear () { m_nb.clear (); }

  uint64_t GetGeohashFromMacInNb(uint8_t* mac,double *x, double *y);
  int GetnbFromsetRandom(Mac48Address *mac, GeoHashSetCoordinate *geohashset);
  int GetnbFromsetBest(Mac48Address *ret_mac, GeoHashSetCoordinate *geohashset);

  /// Get callback to ProcessTxError
//  Callback<void, WifiMacHeader const &> GetTxErrorCallback () const { return m_txErrorCallback; }

private:

  int GetDtime(uint64_t geohash, uint16_t direct, GeoHashSetCoordinate *geohashset);
  /// TX error callback
//  Callback<void, WifiMacHeader const &> m_txErrorCallback;
  /// Timer for neighbor's list. Schedule Purge().
  Timer m_ntimer;
  /// vector of entries
  std::vector<Neighbor> m_nb;

  /// Find MAC address by IP using list of ARP caches
//  Mac48Address LookupMacAddress (Ipv4Address);
  /// Process layer 2 TX error notification
//  void ProcessTxError (WifiMacHeader const &);
};

}
}

#endif /* MBRNEIGHBOR_H */
