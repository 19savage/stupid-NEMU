#include "cpu/exec/helper.h"

make_helper(cld) {
	cpu.eflags.DF = 0;
	print_asm("cld");
	return 1;
}

make_helper(std) {
	cpu.eflags.DF = 1;
	print_asm("std");
	return 1;
}
