#include "memory/cache.h"

#include <stdlib.h>

#define exp2(x) (1 << (x))
#define mask_with_len(x) (exp2(x) - 1)

uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);

enum { WRITE_THROUGH, WRITE_BACK };

Cache L1_icache;
Cache L1_dcache;
Cache L2_cache;

//static inline 
bool cache_query(Cache *c, hwaddr_t addr, CacheLine **hit_cl) {
	uint32_t key = (addr & c->tag_mask) | CL_VALID;
	int set = (addr & c->set_mask) >> c->line_size_width;
	CacheLine *cl = c->line + (set << c->associativity_width);
	CacheLine *cl_test = cl + c->last_access[set];

	if(cl_test->key_val == key) {
		// cache hit
		*hit_cl = cl_test;
		return true;
	}

	cl_test = cl + c->associativity - 1;
	
	for(; cl_test >= cl; cl_test --) {
		// check each ways in the set 
		if(cl_test->key_val == key) {
			// cache hit
			*hit_cl = cl_test;
			c->last_access[set] = cl_test - cl;
			return true;
		}
	}

	/* cache miss */
	c->last_access[set] = rand() & mask_with_len(c->associativity_width);
	*hit_cl = cl + c->last_access[set];	// victim cache line
	return false;
}

//static inline
CacheLine* cache_fetch(Cache *c, hwaddr_t addr) {
	CacheLine *cl;
   	if( cache_query(c, addr, &cl) ) {
		/* cache hit */
		return cl;
	}

	/* cache miss */
	int i;
	if (cl->valid && cl->dirty) {
		/* write back */
		int cl_idx = cl - c->line;
		hwaddr_t addr0 = get_tag(cl->key_val) + ((cl_idx >> c->associativity_width) << c->line_size_width);

		for(i = 0; i < c->line_size; i += 4) {
			c->next_level_write(addr0 + i, 4, unalign_rw(cl->sram + i, 4));
		}
	}
	/* cache fill */
	uint32_t line_start_addr = addr & ~c->offset_mask;
	for(i = 0; i < c->line_size; i += 4) {
		unalign_rw(cl->sram + i, 4) = c->next_level_read(line_start_addr + i, 4);
	}

	cl->key_val = (addr & c->tag_mask) | CL_VALID;
	cl->dirty = false;
	return cl;
}

uint32_t cache_read(hwaddr_t addr, size_t len, Cache *c) {
	uint32_t line_offset = addr & c->offset_mask;
	CacheLine *cl = cache_fetch(c, addr);
	uint8_t *p = cl->sram + line_offset;
	uint32_t remain_byte = c->line_size - line_offset;
	if(remain_byte < len) {
		/* data cross the cache line boundary */
		CacheLine *next_cl = cache_fetch(c, addr + c->line_size);
		switch(remain_byte) {  // always be 1, 2, or 3
			case 1: return unalign_rw(p, 1) | (unalign_rw(next_cl->sram, 3) << 8);
			case 2: return unalign_rw(p, 2) | (unalign_rw(next_cl->sram, 2) << 16);
			case 3: return unalign_rw(p, 3) | (unalign_rw(next_cl->sram, 1) << 24);
		}
	}

	return unalign_rw(p, 4);
}

void cache_write(hwaddr_t addr, size_t len, uint32_t data, Cache *c) {
	uint32_t line_offset = addr & c->offset_mask;
	CacheLine *cl = cache_fetch(c, addr);
	uint8_t *p = cl->sram + line_offset;
	uint32_t remain_byte = c->line_size - line_offset;
	if(remain_byte < len) {
		/* data cross the cache line boundary */
		CacheLine *next_cl = cache_fetch(c, addr + c->line_size);
		switch(remain_byte) {  // always be 1, 2, or 3
			case 3: unalign_rw(p + 1, 2) = ((data >> 8) & 0xffff);
			case 1: unalign_rw(p, 1) = data & 0xff; break;
			case 2: unalign_rw(p, 2) = (data & 0xffff); break;
		}

		uint8_t *q = next_cl->sram;
		uint8_t *p_data = (void *)&data + remain_byte;
		uint32_t remain_byte2 = len - remain_byte;
		if(remain_byte2 -- > 0) {
			*q = *p_data;
			if(remain_byte2 -- > 0) {
				*(++ q) = *(++ p_data);
				if(remain_byte2 -- > 0) {
					*(++ q) = *(++ p_data);
				}
			}
		}

		if(c->write_policy == WRITE_THROUGH) {
			cl->dirty = next_cl->dirty = false;
			c->next_level_write(addr & ~0x3, 4, unalign_rw((size_t)p & ~0x3, 4));
			c->next_level_write(addr + remain_byte, 4, unalign_rw(next_cl->sram, 4));
		}
		else {
			cl->dirty = next_cl->dirty = true;
		}
	}
	else {
		switch(len) {
			case 3: unalign_rw(p + 1, 2) = ((data >> 8) & 0xffff);
			case 1: unalign_rw(p, 1) = data & 0xff; break;
			case 2: unalign_rw(p, 2) = (data & 0xffff); break;
			case 4: unalign_rw(p, 4) = data; break;
		}
		cl->dirty = (c->write_policy == WRITE_BACK);
		if(!cl->dirty) {
			c->next_level_write(addr, len, data);
		}
	}

#ifndef ONLY_ONE_L1_CACHE
	if(c->coherency != NULL) {
		c->coherency(addr, len);
	}
#endif
}

static inline uint32_t L1_next_level_read(hwaddr_t addr, size_t len) {
	return cache_read(addr, len, &L2_cache);
}

static inline void L1_next_level_write(hwaddr_t addr, size_t len, uint32_t data) {
	cache_write(addr, len, data, &L2_cache);
}

static inline void coherency(hwaddr_t addr, size_t len) {
	assert(len == 1 || len == 2 || len == 4);
	Cache *c = &L1_icache;

	CacheLine *cl;
   	if( cache_query(c, addr, &cl) ) {
		/* cache hit */
		cl->key_val = 0;
	}

	uint32_t line_offset = addr & c->offset_mask;
	uint32_t remain_byte = c->line_size - line_offset;
	if(remain_byte < len) {
		if( cache_query(c, addr + c->line_size, &cl) ) {
			/* cache hit */
			cl->key_val = 0;
		}
	}
}

static void init_cache(Cache *c) {
	int i;
	for(i = 0; i < c->nr_line; i ++) {
		c->line[i].key_val = 0;
	}
}

static void make_cache(Cache *c, 
		uint32_t tag_mask, uint32_t line_mask, uint32_t set_mask, uint32_t offset_mask,
		int line_size_width, int total_size_width, int associativity_width, 
		int write_policy, void (* coherency) (hwaddr_t, size_t),
		uint32_t (* next_level_read) (hwaddr_t, size_t),
		void (* next_level_write) (hwaddr_t, size_t, uint32_t)) {
	
	c->line_size_width = line_size_width;
	c->associativity_width = associativity_width;
	c->set_size_width = total_size_width - line_size_width - associativity_width;

	c->line_size = exp2(line_size_width);
	c->nr_line = exp2(total_size_width - line_size_width);
	c->associativity = exp2(associativity_width);
	c->nr_set = exp2(total_size_width - line_size_width - associativity_width);

	c->line_mask = (c->nr_line - 1) << line_size_width;
	c->set_mask = mask_with_len(c->set_size_width) << line_size_width;
	c->offset_mask = mask_with_len(line_size_width);
	c->tag_mask = ~(c->set_mask | c->offset_mask);

	c->write_policy = write_policy;
	c->coherency = coherency;
	c->next_level_read = next_level_read;
	c->next_level_write = next_level_write;

	assert(c->associativity <= c->nr_line);

	c->line = malloc(sizeof(CacheLine) * c->nr_line);
	c->last_access = malloc(sizeof(int) * c->nr_set);

	int i;
	for(i = 0; i < c->nr_line; i ++) {
		c->line[i].key_val = 0;
//		c->line[i].valid = false;
		c->line[i].sram = malloc(c->line_size);
	}

	for(i = 0; i < c->nr_set; i ++) {
		c->last_access[i] = 0;
	}
}

void make_all_cache() {
	make_cache(&L1_icache, 
			0xffffe000, 0x0000ffc0, 0x00001fc0, 0x0000003f,
			6, 16, 3, WRITE_THROUGH, NULL, L1_next_level_read, L1_next_level_write);
	make_cache(&L1_dcache, 
			0xffffe000, 0x0000ffc0, 0x00001fc0, 0x0000003f,
			6, 16, 3, WRITE_THROUGH, coherency, L1_next_level_read, L1_next_level_write);
	make_cache(&L2_cache, 
			0xfffc0000, 0x003fffc0, 0x0003ffc0, 0x0000003f,
			6, 22, 4, WRITE_BACK, NULL, dram_read, dram_write);
}

void init_all_cache() {
	init_cache(&L1_icache);
	init_cache(&L1_dcache);
	init_cache(&L2_cache);
}
