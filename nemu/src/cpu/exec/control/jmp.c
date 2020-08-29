#include "cpu/exec/helper.h"

#define DATA_BYTE 1
#include "jmp-template.h"
#undef DATA_BYTE

#define DATA_BYTE 4
#include "jmp-template.h"
#undef DATA_BYTE

void load_sreg(uint8_t, uint16_t);

make_helper(jmp_intersegment) {
	swaddr_t addr = instr_fetch(eip + 1, 4);
	uint16_t cs = instr_fetch(eip + 5, 2);

	cpu.eip = addr - (1 + 4 + 2);
	load_sreg(R_CS, cs);

	print_asm("ljmp $%#x,$%#x", cs, addr);
	return 1 + 4 + 2;
}
