#include "trap.h"

const char str[] = "Hello , world!\n";

int main(){
	asm volatile ( "movl $4 , %eax;"
			"movl $1 , %ebx;"
			"movl $str , %ecx;"
			"movl $14 , %edx;"
			"int $0x80");
	return 0;
}
