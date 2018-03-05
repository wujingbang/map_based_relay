# map_based_relay
## MBR in Linux: [Linux](Linux)
 1. (to be added)
## MBR in NS-3: [NS3](NS-3)
### About GeoSVR
 1. Source code comes from https://github.com/gnehzuil/GeoSVR
 1. We transplant the source code from ns-2 to ns-3. 
### About the obstacle shadowing model (OBM)
 Source code comes from S. E. Carpenter and M. L. Sichitiu, ¡°An obstacle model implementation for evaluating radio shadowing with ns-3,¡± in Proc. WNS3, 2015, pp.17¨C24.
#### Dependency of the obstacle shadowing model
```
sudo apt-get install libcgal-dev
sudo cp NS3/wave/model/proj/64bit/lib/libproj.so /usr/lib
```

#### Basic performace of channels in ns-3 when OBM is implemented
 1. \alpha = 9 dB and \beta = 0.4 dB/m.
 1. Two nodes, scenario as shown in the following figure:
 ![image](NS3/ns3-ch-basic-scenario.jpg)
 1. Performance as shown in the following figure:
 ![image](NS3/ns3-ch-basic-performance.jpg)