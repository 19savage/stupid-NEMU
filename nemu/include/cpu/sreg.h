#ifndef __SREG_H__
#define __SREG_H__

#include "common.h"
#include "cpu/reg.h"

typedef struct SegmentSelector {
	union {
		/* the visible part */
		struct {
			uint32_t rpl    :2;
			uint32_t ti     :1;
			uint32_t index  :13;
		};
		uint16_t val;
	};
	struct {
		/* the invisible part */
		uint32_t base;
		uint32_t limit;
		uint32_t type				:5;
		uint32_t privilege_level	:2;
		uint32_t soft_use			:1;
	};
} SegSel;

extern const char* sregs[];
#endif
