
#include "geohash.h"

static inline uint8_t get_bit(uint64_t bits, uint8_t pos)
{
    return (bits >> pos) & 0x01;
}


static int geohash_move_x(GeoHashBits* hash, int8_t d)
{
    if (d == 0)
        return 0;
    uint64_t x = hash->bits & 0xaaaaaaaaaaaaaaaaLL;
    uint64_t y = hash->bits & 0x5555555555555555LL;

    uint64_t zz = 0x5555555555555555LL >> (64 - hash->step * 2);
    if (d > 0)
    {
        x = x + (zz + 1);
    }
    else
    {
        x = x | zz;
        x = x - (zz + 1);
    }
    x &= (0xaaaaaaaaaaaaaaaaLL >> (64 - hash->step * 2));
    hash->bits = (x | y);
    return 0;
}

static int geohash_move_y(GeoHashBits* hash, int8_t d)
{
    if (d == 0)
        return 0;
    uint64_t x = hash->bits & 0xaaaaaaaaaaaaaaaaLL;
    uint64_t y = hash->bits & 0x5555555555555555LL;

    uint64_t zz = 0xaaaaaaaaaaaaaaaaLL >> (64 - hash->step * 2);
    if (d > 0)
    {
        y = y + (zz + 1);
    }
    else
    {
        y = y | zz;
        y = y - (zz + 1);
    }
    y &= (0x5555555555555555LL >> (64 - hash->step * 2));
    hash->bits = (x | y);
    return 0;
}

int geohash_get_neighbors(GeoHashBits hash, GeoHashNeighbors* neighbors)
{
    geohash_get_neighbor(hash, GEOHASH_NORTH, &neighbors->north);
    geohash_get_neighbor(hash, GEOHASH_EAST, &neighbors->east);
    geohash_get_neighbor(hash, GEOHASH_WEST, &neighbors->west);
    geohash_get_neighbor(hash, GEOHASH_SOUTH, &neighbors->south);
    geohash_get_neighbor(hash, GEOHASH_SOUTH_WEST, &neighbors->south_west);
    geohash_get_neighbor(hash, GEOHASH_SOUTH_EAST, &neighbors->south_east);
    geohash_get_neighbor(hash, GEOHASH_NORT_WEST, &neighbors->north_west);
    geohash_get_neighbor(hash, GEOHASH_NORT_EAST, &neighbors->north_east);
    return 0;
}

int geohash_get_neighbors_in_set(GeoHashBits hash, u64 *geohashset)
{
	GeoHashNeighbors neighbors;
	geohash_get_neighbors(hash, &neighbors);

	geohashset[0] = hash.bits;
	geohashset[1] = neighbors.north.bits;
	geohashset[2] = neighbors.east.bits;
	geohashset[3] = neighbors.west.bits;
	geohashset[4] = neighbors.south.bits;
	geohashset[5] = neighbors.south_west.bits;
	geohashset[6] = neighbors.south_east.bits;
	geohashset[7] = neighbors.north_west.bits;
	geohashset[8] = neighbors.north_east.bits;

    return 0;
}

int geohash_get_neighbor(GeoHashBits hash, GeoDirection direction, GeoHashBits* neighbor)
{
    if (NULL == neighbor)
    {
        return -1;
    }
    *neighbor = hash;
    switch (direction)
    {
        case GEOHASH_NORTH:
        {
            geohash_move_x(neighbor, 0);
            geohash_move_y(neighbor, 1);
            break;
        }
        case GEOHASH_SOUTH:
        {
            geohash_move_x(neighbor, 0);
            geohash_move_y(neighbor, -1);
            break;
        }
        case GEOHASH_EAST:
        {
            geohash_move_x(neighbor, 1);
            geohash_move_y(neighbor, 0);
            break;
        }
        case GEOHASH_WEST:
        {
            geohash_move_x(neighbor, -1);
            geohash_move_y(neighbor, 0);
            break;
        }
        case GEOHASH_SOUTH_WEST:
        {
            geohash_move_x(neighbor, -1);
            geohash_move_y(neighbor, -1);
            break;
        }
        case GEOHASH_SOUTH_EAST:
        {
            geohash_move_x(neighbor, 1);
            geohash_move_y(neighbor, -1);
            break;
        }
        case GEOHASH_NORT_WEST:
        {
            geohash_move_x(neighbor, -1);
            geohash_move_y(neighbor, 1);
            break;
        }
        case GEOHASH_NORT_EAST:
        {
            geohash_move_x(neighbor, 1);
            geohash_move_y(neighbor, 1);
            break;
        }
        default:
        {
            return -1;
        }
    }
    return 0;
}

GeoHashBits geohash_next_leftbottom(GeoHashBits bits)
{
    GeoHashBits newbits = bits;
    newbits.step++;
    newbits.bits <<= 2;
    return newbits;
}
GeoHashBits geohash_next_rightbottom(GeoHashBits bits)
{
    GeoHashBits newbits = bits;
    newbits.step++;
    newbits.bits <<= 2;
    newbits.bits += 2;
    return newbits;
}
GeoHashBits geohash_next_lefttop(GeoHashBits bits)
{
    GeoHashBits newbits = bits;
    newbits.step++;
    newbits.bits <<= 2;
    newbits.bits += 1;
    return newbits;
}

GeoHashBits geohash_next_righttop(GeoHashBits bits)
{
    GeoHashBits newbits = bits;
    newbits.step++;
    newbits.bits <<= 2;
    newbits.bits += 3;
    return newbits;
}
