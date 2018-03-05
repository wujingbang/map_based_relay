## Description of files
 1. netlink_user.c : (user space) creates a simple road map and push the information to the MBR module.
 1. rcv.c : (user space) receives beacon of neighbors and maintains neighbor list for MBR through memory sharing.
 1. sendA.c, sendB.c£¬sendC.c : (user space) sends beacons for nodes (MAC addr is hard coded).

## Usage
 1. When MBR module is loaded, run netlink_user to create test road map for MBR;
 1. Run rcv to receive beacons;
 1. run sendX to send beacons. 