#ifndef DEBUG_H
#define DEBUG_H


#include "relay.h"

enum MBR_DEBUG {
	MBR_DBG_RESET		= 0x00000001,
	MBR_DBG_QUEUE		= 0x00000002,
	MBR_DBG_EEPROM		= 0x00000004,
	MBR_DBG_CALIBRATE	= 0x00000008,
	MBR_DBG_INTERRUPT	= 0x00000010,
	MBR_DBG_REGULATORY	= 0x00000020,
	MBR_DBG_ANI		= 0x00000040,
	MBR_DBG_XMIT		= 0x00000080,
	MBR_DBG_BEACON		= 0x00000100,
	MBR_DBG_CONFIG		= 0x00000200,
	MBR_DBG_FATAL		= 0x00000400,
	MBR_DBG_PS		= 0x00000800,
	MBR_DBG_BTCOEX		= 0x00001000,
	MBR_DBG_WMI		= 0x00002000,
	MBR_DBG_BSTUCK		= 0x00004000,
	MBR_DBG_MCI		= 0x00008000,
	MBR_DBG_DFS		= 0x00010000,
	MBR_DBG_WOW		= 0x00020000,
	MBR_DBG_CHAN_CTX	= 0x00040000,
	MBR_DBG_DYNACK		= 0x00080000,
	MBR_DBG_ANY		= 0xffffffff
};

#define MBR_DBG_DEFAULT (MBR_DBG_ANY)

void mbr_printk(const char *level, const int debug_mask,
		const char *fmt, ...);


#ifdef CONFIG_DEBUG

#define mbr_dbg(level, dbg_mask, fmt, ...)				\
do {									\
	if (level & MBR_DBG_##dbg_mask)			\
		mbr_printk(KERN_ALERT, level, fmt, ##__VA_ARGS__);	\
} while (0)

#else

#define _mbr_dbg(level, dbg_mask, fmt, ...)				\
do {									\
} while (0)

#define mbr_dbg(level, dbg_mask, fmt, ...)				\
	_mbr_dbg(level, MBR_DBG_##dbg_mask, fmt, ##__VA_ARGS__)


#endif /* CONFIG_DEBUG */
#endif /* DEBUG_H */
