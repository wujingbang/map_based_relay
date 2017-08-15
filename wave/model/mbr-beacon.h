#ifndef MBR_BEACON_H
#define MBR_BEACON_H

#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/mgt-headers.h"        // from wifi module
#include "ns3/wifi-mac-header.h"
#include "ns3/mesh-information-element-vector.h"

namespace ns3 {

/**
 * \brief Beacon is beacon header + list of arbitrary information elements
 *
 * It is supposed that distinct mesh protocols can use beacons to transport
 * their own information elements.
 */
class MbrBeacon
{
public:
  /**
   * C-tor
   *
   * \param us beacon interval in microseconds
   */
	MbrBeacon (uint64_t us);
  /// Read standard Wifi beacon header
  MgtBeaconHeader BeaconHeader () const { return m_header; }

  /**
   * Create wifi header for beacon frame.
   *
   * \param address is sender address
   * \param mpAddress is mesh point address
   */
  WifiMacHeader CreateHeader (Mac48Address address);
  /// Returns a beacon interval of wifi beacon
  Time GetBeaconInterval () const;
  /// Create frame = { beacon header + all information elements sorted by ElementId () }
  Ptr<Packet> CreatePacket ();
  /// Add information element
  void AddInformationElement (Ptr<WifiInformationElement> ie);

private:
  /// Beacon header
  MgtBeaconHeader m_header;
  MeshInformationElementVector m_elements;
};

}

#endif /* MBR_BEACON_H */
