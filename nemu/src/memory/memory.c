#include "nemu.h"
#include "common.h"
#include "memory/cache.h"
#include "memory/mmu.h"
#include "memory/page.h"

uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);

/* Memory accessing interfaces */

uint32_t hwaddr_read(hwaddr_t addr, size_t len) {
#ifdef ENABLE_CACHE
	return cache_read(addr, len, &L1_dcache) & (~0u >> ((4 - len) << 3));
#else
	return dram_read(addr, len) & (~0u >> ((4 - len) << 3));
#endif
}

void hwaddr_write(hwaddr_t addr, size_t len, uint32_t data) {
#ifdef ENABLE_CACHE
	cache_write(addr, len, data, &L1_dcache);
#else
	dram_write(addr, len, data);
#endif
}

uint32_t lnaddr_read(lnaddr_t addr, size_t len) {
	
	uint32_t data;
	if(cpu.cr0.paging) {
		hwaddr_t hwaddr = page_translate(addr);
		uint32_t remain_byte = PAGE_SIZE - (addr & PAGE_MASK);
		if(remain_byte < len) {
			/* data cross the page boundary */
			data = hwaddr_read(hwaddr, remain_byte);

			hwaddr = page_translate(addr + remain_byte);
			data |= hwaddr_read(hwaddr, len - remain_byte) << (remain_byte << 3);
		}
		else {
			data = hwaddr_read(hwaddr, len);
		}
	}
	else {
		
		data = hwaddr_read(addr, len);
	}

	return data;
}

void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data) {
	if(cpu.cr0.paging) {
		hwaddr_t hwaddr = page_translate(addr);
		uint32_t remain_byte = PAGE_SIZE - (addr & PAGE_MASK);
		if(remain_byte < len) {
			/* data cross the page boundary */
			uint32_t cut = PAGE_SIZE - (addr & PAGE_MASK);
			assert(cut < 4);
			hwaddr_write(hwaddr, cut, data);

			hwaddr = page_translate(addr + cut);
			hwaddr_write(hwaddr, len - cut, data >> (cut << 3));
		}
		else {
			hwaddr_write(hwaddr, len, data);
		}
	}
	else {
		hwaddr_write(addr, len, data);
	}
}

uint32_t swaddr_read(swaddr_t addr, size_t len, uint8_t sreg) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	lnaddr_t lnaddr = segment_translate(addr, sreg);
	return lnaddr_read(lnaddr, len);
}

void swaddr_write(swaddr_t addr, size_t len, uint32_t data, uint8_t sreg) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	lnaddr_t lnaddr = segment_translate(addr, sreg);
	lnaddr_write(lnaddr, len, data);
}

