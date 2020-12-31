#include "cpu/exec/helper.h"

#include "nemu.h"

void device_update();

make_helper(hlt) {
	while(!cpu.eflags.IF || !cpu.INTR) {
		device_update();
	}
	print_asm("hlt");
	return 1;
}
