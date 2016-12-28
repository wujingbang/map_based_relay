#include <linux/kernel.h>
#include "debug.h"

void mbr_printk(const char *level, const int debug_mask,
		const char *fmt, ...)
{
	struct va_format vaf;
	va_list args;

	va_start(args, fmt);

	vaf.fmt = fmt;
	vaf.va = &args;

	printk("%smbr: %pV", level, &vaf);

	va_end(args);
}
