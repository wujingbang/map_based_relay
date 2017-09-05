#ifndef UTILS_H
#define UTILS_H

#ifdef __cplusplus
extern "C"
{
#endif

extern void * mbr_malloc(int size);
extern void mbr_free(void *p);
extern double radian(double d);
extern double get_distance(double lat1, double lng1, double lat2, double lng2);


#ifdef __cplusplus
}
#endif
#endif /* UTILS_H */
