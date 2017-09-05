
#include "debug.h"

#ifdef LINUX_KERNEL

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

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

void mbr_print_file(const char *level, const int debug_mask,
		const char *fmt, ...)
{
	struct file *filp;
	mm_segment_t fs;
	char data[255];
	struct va_format vaf;
	va_list args;

	va_start(args, fmt);

	vaf.fmt = fmt;
	vaf.va = &args;

	sprintf(data, "%pV", &vaf);

	va_end(args);


	filp = filp_open(TRACE_FILE, O_RDWR| O_APPEND | O_CREAT, 0644);
	if(IS_ERR(filp))
	{
		printk(KERN_ALERT "mbr_print_file: file open error!\n");
		return;
	}

	fs=get_fs();
	set_fs(KERNEL_DS);
	filp->f_op->write(filp, data, strlen(data),&filp->f_pos);
	set_fs(fs);
	filp_close(filp, NULL);
}

#else
void mbr_printk(const char *level, const int debug_mask,
		const char *fmt, ...) {

}

void mbr_print_file(const char *level, const int debug_mask,
		const char *fmt, ...) {

}

#endif
