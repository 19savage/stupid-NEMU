#include "cpu/exec/template-start.h"

#define instr pop

static void do_execute(){
	OPERAND_W(op_src , swaddr_read(cpu.eip , 4 , R_SS));
	cpu.eip += 4;
	print_asm_template1();
}

make_instr_helper(r)

#include "cpu/exec/template-end.h"
