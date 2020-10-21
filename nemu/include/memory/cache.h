#ifndef __CACHE_H_
#define __CACHE_H_

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
#define BLOCK_L2_WAY_SIZE ( 1 << BLOCK_L2_WAY_SIZE_B)
#define BLOCK_L2_SET_SIZE ( 1 << BLOCK_L2_SET_SIZE_B)


typedef struct {
	bool valid;
	int tag;
	uint8_t data[BLOCK_L1_SIZE];
}Cache;
/* cache block storage size : 64B
   whole cache size : 64KB
   8-way set-associative
   mark bit : valid 
   replace algorithm: srand;
   write through;  
   not write allocate;
*/

typedef struct {
	bool valid,dirty;
	int tag;
	uint8_t data[BLOCK_L2_SIZE];
}Cache2;
/* cache block storage size : 64B
   whole cache size : 4MBL
   16-way set-associative
   mark bit : valid , dirty
   replace algorithm : srandl
   write back;
   write allocate;
*/
Cache cache[BLOCK_L1_WAY_SIZE*BLOCK_L2_SET_SIZE];
Cache2 cache2[BLOCK_L2_WAY_SIZE*BLOCK_L2_SET_SIZE];

void init_cache();
uint32_t cache_read(hwaddr_t addr);
void cache_write(hwaddr_t addr, size_t len , uint32_t data);
uint32_t cache2_read(hwaddr_t addr);
void cache2_write(hwaddr_t addr,size_t len, uint32_t data);

#endif
