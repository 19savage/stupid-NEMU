#include "common.h"
#include "memory.h"
#include <string.h>

#define VMEM_ADDR 0xa0000
#define SCR_SIZE (320 * 200)
#define NR_PT ((SCR_SIZE + PT_SIZE - 1) / PT_SIZE)  // number of page tables to cover the vmem
static PTE vmem_ptable[NR_PT][NR_PTE] align_to_page;

/* Use the function to get the start address of user page directory. */
PDE* get_updir();

void create_video_mapping() {
	/* TODO: create an identical mapping from virtual memory area 
	 * [0xa0000, 0xa0000 + SCR_SIZE) to physical memory area 
	 * [0xa0000, 0xa0000 + SCR_SIZE) for user program. You may define
	 * some page tables to create this mapping.
	 */
	//panic("please implement me");
	PDE *updir = get_updir();

	uint32_t va = VMEM_ADDR;
	int len = SCR_SIZE;
	int pt_idx = 0;

	for(; len > 0; len -= PAGE_SIZE, va += PAGE_SIZE) {
		int pde_idx = va / PT_SIZE;
		PTE *pt;

		if(updir[pde_idx].present == 0) {
			pt = vmem_ptable[pt_idx ++];
			updir[pde_idx].val = make_pde(va_to_pa(pt));
			memset(pt, 0, NR_PTE * sizeof(NR_PTE));
		}
		else {
			pt = (void *)pa_to_va(updir[pde_idx].val & ~0xfff);
		}

		int pte_idx = (va % PT_SIZE) / PAGE_SIZE;
		if(pt[pte_idx].present == 0) {
			pt[pte_idx].val = make_pte(va);
		}
	}
}

void video_mapping_write_test() {
	int i;
	uint32_t *buf = (void *)VMEM_ADDR;
	for(i = 0; i < SCR_SIZE / 4; i ++) {
		buf[i] = i;
	}
}

void video_mapping_read_test() {
	int i;
	uint32_t *buf = (void *)VMEM_ADDR;
	for(i = 0; i < SCR_SIZE / 4; i ++) {
		assert(buf[i] == i);
	}
}

void video_mapping_clear() {
	memset((void *)VMEM_ADDR, 0, SCR_SIZE);
}

