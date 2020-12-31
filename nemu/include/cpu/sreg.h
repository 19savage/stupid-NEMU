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

typedef struct {
	union {
		struct {
			uint16_t limit1;
			uint16_t base1;
		};
		uint32_t first;
	};
	union {
		struct {
			uint32_t base2 : 8;
			uint32_t type  : 5;
			uint32_t dpl   : 2;
			uint32_t p     : 1;
			uint32_t limit2: 4;
			uint32_t avl   : 1;
			uint32_t       : 1;
			uint32_t b     : 1;
			uint32_t g     : 1;
			uint32_t base3 : 8;
		};
		uint32_t second;
	};
}SegmentDes;

extern const char* sregs[];
#endif
