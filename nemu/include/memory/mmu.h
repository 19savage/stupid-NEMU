#ifndef __MMU_H__
#define __MMU_H__

lnaddr_t segment_translate(swaddr_t, uint8_t);
hwaddr_t page_translate(lnaddr_t);

#endif
