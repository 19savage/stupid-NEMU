#include "cpu/exec/helper.h"

void load_sreg(uint8_t, uint16_t);

/*make_helper(mov_sreg2rm_w) {
	int len = decode_r2rm_w(eip + 1);
	uint8_t sreg = op_src->reg;
	assert(sreg < 6 && sreg != R_CS);
	write_operand_w(op_dest, cpu.sreg[sreg].val);
	print_asm("movw %%%s,%s", sregs[sreg], op_dest->str);
	return len + 1;
}*/

make_helper(mov_rm2sreg_w) {
	int len = decode_rm2r_w(eip + 1);
	uint8_t sreg = op_dest->reg;
	load_sreg(sreg, op_src->val);
	print_asm("movw %s,%%%s", op_src->str, sregs[sreg]);
	return len + 1;
}

