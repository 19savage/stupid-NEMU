#include "common.h"
#include "burst.h"
#include "memory/cache.h"
#include <stdlib.h>
void ddr3read(hwaddr_t addr, void *data);
void ddr3write(hwaddr_t addr, void *data , uint8_t *mask);
void dram_write(hwaddr_t addr, size_t len, uint32_t data);

void init_cache(){
	int i;
	for( i = 0 ; i < BLOCK_L1_WAY_SIZE*BLOCK_L1_SET_SIZE ; i++){
	cache[i].valid = false;
	cache[i].tag = 0;
	memset (cache[i].data,0,BLOCK_L1_SIZE);
	}
	for( i = 0 ; i < BLOCK_L2_WAY_SIZE*BLOCK_L2_SET_SIZE ; i++){
	cache2[i].valid = false;
	cache2[i].tag = 0;
	memset (cache2[i].data , 0, BLOCK_L2_SIZE);
	}
}

uint32_t cache_read(hwaddr_t addr){
	uint32_t tag = addr >> (BLOCK_L1_SET_SIZE_B + BLOCK_L1_SIZE_B);
	uint32_t set = addr >> (BLOCK_L1_SIZE_B);
	set &= ( BLOCK_L1_SET_SIZE  -1 );
	
	int i ;
	for ( i = set * BLOCK_L1_WAY_SIZE ;  i < (set+1) * BLOCK_L1_WAY_SIZE ; i ++ ){
		if( cache[i].valid && cache[i].tag == tag)
			return i;
		}
	// rep till the next set;

	
	//cache read miss
	int j = cache2_read( addr);
	srand(i);
	i  = BLOCK_L1_WAY_SIZE*set + rand() % BLOCK_L1_WAY_SIZE;
	memcpy (cache[i].data ,cache2[j].data ,BLOCK_L1_SIZE);

	cache[i].valid = true;
	cache[i].tag = tag;
	return i;
}

uint32_t cache2_read(hwaddr_t addr){
	uint32_t tag = addr >> (BLOCK_L2_SET_SIZE_B + BLOCK_L2_SIZE_B);
	uint32_t set = addr >> (BLOCK_L2_SIZE_B);
	set &= ( BLOCK_L2_SET_SIZE - 1 );
	uint32_t block = (addr >> BLOCK_L2_SIZE_B ) << BLOCK_L2_SIZE_B;

	int i;
	for ( i = set*BLOCK_L2_WAY_SIZE ; i < (set+1) * BLOCK_L2_WAY_SIZE ; i ++ ){
		if ( cache2[i].valid && cache2[i].tag == tag)
			return i;
		}
	
	
	//cache2 read miss
	srand(i);
	i = BLOCK_L2_WAY_SIZE*set + rand()% BLOCK_L2_WAY_SIZE;
	if(cache2[i].dirty && cache2[i].valid){
		uint32_t addr2 = (cache2[i].tag << (BLOCK_L2_SET_SIZE_B + BLOCK_L2_SIZE_B)) | (set)<< BLOCK_L2_SIZE_B;
		uint8_t mask[2*BURST_LEN];
		memset(mask,1,2*BURST_LEN);
		int j;
		for( j = 0 ; j < BLOCK_L2_SIZE/BURST_LEN; j++ )
			ddr3write(addr2+j*BURST_LEN, cache2[i].data+j*BURST_LEN , mask);
	}
	int p;
	for ( p=0 ; p < BLOCK_L2_SIZE/BURST_LEN ; p++){
		ddr3read(block+ p*BURST_LEN, cache2[i].data+p*BURST_LEN);
	}
	cache2[i].valid = true;
	cache2[i].tag = tag;
	cache2[i].dirty = false;

	return i;
}


void cache_write(hwaddr_t addr, size_t len , uint32_t data){
	uint32_t tag = addr>>(BLOCK_L1_SET_SIZE_B+ BLOCK_L1_SIZE_B);
	uint32_t set = addr>>(BLOCK_L1_SIZE_B);
	set &=(BLOCK_L1_SET_SIZE-1);
	uint32_t offset = addr&(BLOCK_L1_SIZE - 1);

	int i;
	for(i = set*BLOCK_L1_WAY_SIZE ; i < (set +1 )*BLOCK_L1_WAY_SIZE ; i++){
		if(cache[i].valid && cache[i].tag == tag){
		   if(offset + len > BLOCK_L1_SIZE) {
			memcpy(cache[i].data+offset , &data, BLOCK_L1_SIZE-offset);
			cache2_write(addr,BLOCK_L1_SIZE-offset, data);
			cache_write(addr+BLOCK_L1_SIZE-offset, len-(BLOCK_L1_SIZE-offset), data>>(BLOCK_L1_SIZE-offset));
		}
		else {
			memcpy(cache[i].data+offset,&data,len);
			cache2_write(addr,len,data);
		     }
	
		return ;
		}
	}
	cache2_write(addr,len,data);
}


void cache2_write(hwaddr_t addr, size_t len, uint32_t data){
	uint32_t tag = addr >> (BLOCK_L2_SET_SIZE_B + BLOCK_L2_SIZE_B);
	uint32_t set = addr >> (BLOCK_L2_SIZE_B);
	set &= ( BLOCK_L2_SET_SIZE - 1);
	uint32_t offset = addr&(BLOCK_L2_SIZE -1);
	
	int i;
	for ( i = set*BLOCK_L2_WAY_SIZE ; i<(set+1)*BLOCK_L2_WAY_SIZE ; i++){
		if(cache2[i].valid && cache2[i].tag == tag){
			cache2[i].dirty = true;
			if(offset + len > BLOCK_L2_SIZE){
				memcpy(cache2[i].data+offset,&data,BLOCK_L2_SIZE-offset);
				cache2_write(addr+BLOCK_L2_SIZE-offset,len-(BLOCK_L2_SIZE-offset),data>>(BLOCK_L2_SIZE-offset));
			}
			else
			{
				memcpy(cache2[i].data+offset,&data,len);
			}
			return ;
		}
	}

	int j = cache2_read(addr);
	cache2[i].dirty = true;
	memcpy(cache2[j].data+offset,&data,len);
}


	
                        
	                      
	
