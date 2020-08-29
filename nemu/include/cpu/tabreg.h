#ifndef __TABREG_H__
#define __TABREG_H__

typedef struct TableRegister {
	uint32_t limit      : 16;
	uint32_t base       : 32;
} TabReg;

#endif
