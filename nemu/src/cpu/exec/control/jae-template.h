#include "cpu/exec/template-start.h"

#define instr jae

make_helper(concat(jae_i_, SUFFIX)) {
	int length = concat(decode_i_, SUFFIX) (eip + 1);
	print_asm("jae %x", (DATA_TYPE_S)op_src->imm + cpu.eip + length + 1);
	if(cpu.eflags.CF == 0) cpu.eip += (DATA_TYPE_S)op_src->imm;
	return length + 1;
}

#include "cpu/exec/template-end.h"
