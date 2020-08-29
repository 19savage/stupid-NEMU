#include "cpu/exec/helper.h"
//#include "cpu/intr.h"

void load_sreg(uint8_t, uint16_t);
void init_all_tlb();

make_helper(lgdt) {
	int len = decode_rm_l(eip + 1);
	cpu.gdtr.limit = lnaddr_read(op_src->addr, 2);
	cpu.gdtr.base = lnaddr_read(op_src->addr + 2, 4);

	print_asm("lgdt %s", op_src->str);
	return len + 1;
}

/*make_helper(lidt) {
	int len = decode_rm_l(eip + 1);
	cpu.idtr.limit = lnaddr_read(op_src->addr, 2);
	cpu.idtr.base = lnaddr_read(op_src->addr + 2, 4);

	print_asm("lidt %s", op_src->str);
	return len + 1;
}*/

make_helper(mov_r2cr) {
	int len = decode_r2rm_l(eip + 1);
	assert(op_dest->type == OP_TYPE_REG);
	if(op_src->reg == 0) {
		uint32_t temp = cpu.cr0.val;
		cpu.cr0.val = reg_l(op_dest->reg);
		if(cpu.cr0.protect_enable) {
			if((temp & 0x1) == 0) {
				cpu.cs.base = 0;
				cpu.cs.limit = 0xffffffff;
				cpu.cs.type = 0x1a;
				cpu.cs.privilege_level = 0;
				cpu.cs.soft_use = 0;
			}
		}
	}
	else if(op_src->reg == 3) {
		cpu.cr3.val = reg_l(op_dest->reg);
		init_all_tlb();
	}
	else { assert(0); }

	print_asm("movl %%%s,%%cr%d", regsl[op_dest->reg], op_src->reg);
	return len + 1;
}

make_helper(mov_cr2r) {
	int len = decode_r2rm_l(eip + 1);
	assert(op_dest->type == OP_TYPE_REG);
	if(op_dest->reg == 0) { reg_l(op_src->reg) = cpu.cr0.val; }
	else if(op_dest->reg == 3) { reg_l(op_src->reg) = cpu.cr3.val; }
	else { assert(0); }

	print_asm("movl %%cr%d,%%%s", op_src->reg, regsl[op_dest->reg]);
	return len + 1;
}

/*make_helper(int_) {
	uint8_t NO = instr_fetch(eip + 1, 1);
	Log_write("%8x:   cd %02x %*.sint $%#x\n", cpu.eip, NO, 50 - (12 + 3 * 2), "", NO);
	cpu.eip += 2;
	raise_intr(NO, 0);

	return 2;
}

make_helper(cli) {
	cpu.eflags.IF = 0;
	print_asm("cli");
	return 1;
}

make_helper(sti) {
	cpu.eflags.IF = 1;
	print_asm("sti");
	return 1;
}

make_helper(iret) {
	cpu.eip = swaddr_read(cpu.esp, 4, R_SS) - 1;
	load_sreg(R_CS, swaddr_read(cpu.esp + 4, 4, R_SS));
	cpu.eflags.val = swaddr_read(cpu.esp + 8, 4, R_SS);
	cpu.esp += 12;

	print_asm("iret");
	return 1;
}

make_helper(hlt) {
	if(cpu.eflags.IF) {
		while(cpu.INTR == 0) ;
	}
	else {
		Assert(0, "hlt with cli!");
	}
	print_asm("hlt");
	return 1;
}*/
