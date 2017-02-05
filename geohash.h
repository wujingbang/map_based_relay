#ifndef GEOHASH_H_
#define GEOHASH_H_

#include "mbr.h"


    typedef enum
    {
        GEOHASH_NORTH = 0,
        GEOHASH_EAST,
        GEOHASH_WEST,
        GEOHASH_SOUTH,
        GEOHASH_SOUTH_WEST,
        GEOHASH_SOUTH_EAST,
        GEOHASH_NORT_WEST,
        GEOHASH_NORT_EAST
    } GeoDirection;

    typedef struct
    {
            uint64_t bits;
            uint8_t step;
    } GeoHashBits;


    typedef struct
    {
            GeoHashBits north;
            GeoHashBits east;
            GeoHashBits west;
            GeoHashBits south;
            GeoHashBits north_east;
            GeoHashBits south_east;
            GeoHashBits north_west;
            GeoHashBits south_west;
    } GeoHashNeighbors;


    int geohash_get_neighbors(GeoHashBits hash, GeoHashNeighbors* neighbors);
    int geohash_get_neighbor(GeoHashBits hash, GeoDirection direction, GeoHashBits* neighbor);
    int geohash_get_neighbors_in_set(GeoHashBits hash, u64 *geohashset);

    GeoHashBits geohash_next_leftbottom(GeoHashBits bits);
    GeoHashBits geohash_next_rightbottom(GeoHashBits bits);
    GeoHashBits geohash_next_lefttop(GeoHashBits bits);
    GeoHashBits geohash_next_righttop(GeoHashBits bits);

#endif /* GEOHASH_H_ */
