#ifndef geoSVR_NEIGHBOR_H
#define geoSVR_NEIGHBOR_H

#include <map>
#include <cassert>
#include <stdint.h>
#include "ns3/ipv4.h"
#include "ns3/timer.h"
#include <sys/types.h>
#include "ns3/node.h"
#include "ns3/node-list.h"
#include "ns3/mobility-model.h"
#include "ns3/vector.h"
#include "ns3/wifi-mac-header.h"
#include "ns3/random-variable-stream.h"
#include <complex>
#include <list>

#include "ns3/ptr.h"
#include "ns3/node.h"
#include "geoSVR-map.h"

namespace ns3 {
namespace geoSVR {

#define MSVR_MAX_NB_AGING 4.0

struct msvr_ninfo {
  
    /* NOTE: define a struct to store them */
    double x;
    double y;
    double speed;
    double heading;
    int edgeid;
    Ipv4Address dst;
};

struct msvr_nbentry {
    struct msvr_ninfo nbe_ninfo;
    Time m_expireTime;
};

class Neighbors
{
public:
  /// c-tor
  Neighbors ();
  Neighbors (bool nbFromMbr);

void Init();
void Clear();

void Update(struct msvr_ninfo *ip);
void Purge();

struct msvr_nbentry *find_entry_via_ipaddress(Ipv4Address dst);
bool isNeighbor(Ipv4Address dst, Vector srcpos, double range);

Ipv4Address find_furthest_nhop(int roadid1,int roadid2, int edgeid3, double x1, double y1, double x2, double y2, double range);
Ipv4Address find_next_hop(int roadid1, int roadid2, int roadid3, double x1, double y1, double x2, double y2);

void Print();

bool
isNbFromMbr () const
{
  return m_nbFromMbr;
}

void
setNbFromMbr (bool nbFromMbr)
{
  m_nbFromMbr = nbFromMbr;
}

Ptr<Node>
getNode () const
{
  return m_node;
}

void
setNode (Ptr<Node> node)
{
  m_node = node;
}

Callback<void, WifiMacHeader const &> GetTxErrorCallback () const
{
  return m_txErrorCallback;
}

private:
  std::list<msvr_nbentry> nbl;
  bool m_nbFromMbr;
  Ptr<Node> m_node;
  Time m_entryLifeTime;
  //std::map<Ipv4Address, std::pair<Vector, Time> > m_table;
  // TX error callback
  Callback<void, WifiMacHeader const &> m_txErrorCallback;
  // Process layer 2 TX error notification
  void ProcessTxError (WifiMacHeader const&);
  //std::map<Ipv4Address, std::pair<Vector, Time> >  rng_planarize();
  //double angle(Vector v1, Vector v2);
  //int intersect(Ipv4Address theother, Vector srcPos, Vector dstPos);


};

}   // gpsr
} // ns3
#endif /* geoSVR_PTABLE_H */
