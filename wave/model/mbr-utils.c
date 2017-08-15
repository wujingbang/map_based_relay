#include "mbr-utils.h"

#ifdef LINUX_KERNEL
#include <linux/slab.h>

void * mbr_malloc(int size)
{
	return kmalloc(size, GFP_KERNEL);
}

void mbr_free(const void *p)
{
	return kfree(p);
}

#else

#include<stdlib.h>

void * mbr_malloc(int size)
{
	return malloc(size);
}

void mbr_free(void *p)
{
	return free(p);
}

#endif /* LINUX_KERNEL */


