#ifndef CACHE_H
#define CACHE_H

#define BLOCK_L1_SIZE_B  6
#define BLOCK_L1_WAY_SIZE_B 3
#define BLOCK_L1_SET_SIZE_B 7
#define BLOCK_L1_SIZE ( 1 << BLOCK_L1_SIZE_B)
#define BLOCK_L1_WAY_SIZE ( 1 << BLOCK_L1_WAY_SIZE_B)
#define BLOCK_L1_SET_SIZE ( 1 << BLOCK_L1_SET_SIZE_B)


#define BLOCK_L2_SIZE_B 6
#define BLOCK_L2_WAY_SIZE_B 4
#define BLOCK_L2_SET_SIZE_B 12
#define BLOCK_L2_SIZE ( 1 << BLOCK_L2_SIZE_B)
#define BLOCK_L2_WAY_SIZE ( 1 << Block_L2_WAY_SIZE_B)
#define BLOCK_L2_SET_SIZE ( 1 << Block_L2_SET_SIZE_B)


struct Cache {
	bool valid;
	int tag;
	uint8_t data[BLOCK_L1_SIZE];
}cache[BLOCK_L1_WAY_SIZE*BLOCK_L1_SET_SIZE]
/* cache block storage size : 64B
   whole cache size : 64KB
   8-way set-associative
   mark bit : valid 
   replace algorithm: srand;
   write through;  
   not write allocate;
*/

struct Cache2{
	bool valid,dirty;
	int tag;
	uint8_t data[BLOCK_L2_SIZE];
}cache2[BLOCK_L2_WAY_SIZE*BLOCK_L2_SET_SIZE]
/* cache block storage size : 64B
   whole cache size : 4MBL
   16-way set-associative
   mark bit : valid , dirty
   replace algorithm : srandl
   write back;
   write allocate;
*/

void init_cache();
uint32_t cache_read(hwaddr_t addr);
void cache_write(hwaddr_t addr, size_t len , uint32_t data);
uint32_t cache2_read(hwaddr_t addr);
void cache2_write(hwaddr_t addr,size_t len, uint32_t data_;

#endif
