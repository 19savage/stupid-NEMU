#include "cpu/exec/template-start.h"
#include "nemu.h"
#include "memory/segment.h"

DATA_TYPE concat(tmp_pop_, SUFFIX)();

make_helper(concat(iret_, SUFFIX)) {
	cpu.eip = concat(tmp_pop_, SUFFIX) ();
	cpu.cs.val = concat(tmp_pop_, SUFFIX) ();
	cpu.eflags.val = concat(tmp_pop_, SUFFIX) ();
	if(cpu.cr0.protect_enable) {
		load_sreg_cache(R_CS);
	}
	print_asm("iret");
	return 0;
}

#include "cpu/exec/template-end.h"
