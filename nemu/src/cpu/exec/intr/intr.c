#include "setjmp.h"
#include "nemu.h"
#include "memory/segment.h"

extern jmp_buf jbuf;

void tmp_push_l (uint32_t val);

void raise_intr(uint8_t NO){
	/*Start Interrupt*/	
	tmp_push_l(cpu.eflags.val);
	if(cpu.cr0.protect_enable == 0) {
		cpu.eflags.IF = 0;
		cpu.eflags.TF = 0;
	}
	tmp_push_l(cpu.cs.val);
	tmp_push_l(cpu.eip);
	
	/*Use NO to index the IDT*/
	uint32_t base = cpu.idtr.base;
	uint32_t index = base + (NO << 3);

	GATE gate;
	gate.value1 = lnaddr_read(index, 4);
	gate.value2 = lnaddr_read(index + 4, 4);

	cpu.cs.val = gate.selector;
	load_sreg_cache(R_CS);//load segment

	uint32_t addr = cpu.cs.base + (gate.offset1 | (gate.offset2 << 16));

	cpu.eip = addr;	
	
	/* Jump back to cpu_exec() */
	longjmp(jbuf,1);
}
