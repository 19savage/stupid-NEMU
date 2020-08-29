#include "memory/segment.h"

#include "nemu.h"

uint32_t lnaddr_read(lnaddr_t, size_t);

void load_sreg(uint8_t sreg, uint16_t sel) {
	assert(sreg < 6);
	cpu.sreg[sreg].val = sel;
	assert(cpu.sreg[sreg].ti == 0);		// only GDT is simulated

	uint32_t buf[2];
	lnaddr_t base = cpu.gdtr.base + (sel & ~0x7);
	buf[0] = lnaddr_read(base, 4);
	buf[1] = lnaddr_read(base + 4, 4);
//	Log("base = %08x, buf[0] = %08x, buf[1] = %08x", base, buf[0], buf[1]);

	SegDesc *s = (void *)buf;
	assert(s->present);
	assert(s->operation_size == 0);
	assert(s->granularity == 1);
	cpu.sreg[sreg].base = s->base_15_0 | (s->base_23_16 << 16) | (s->base_31_24 << 24);
	cpu.sreg[sreg].limit = ((s->limit_15_0 | (s->limit_19_16 << 16)) << 12) | 0xfff;
	cpu.sreg[sreg].type = s->type | (s->segment_type << 4);
	cpu.sreg[sreg].privilege_level = s->privilege_level;
	cpu.sreg[sreg].soft_use = s->soft_use;
}

lnaddr_t segment_translate(swaddr_t addr, uint8_t sreg) {
#ifdef DEBUG
	assert(sreg < 6);
#endif
	return (cpu.cr0.protect_enable ? cpu.sreg[sreg].base : 0) + addr;
}
