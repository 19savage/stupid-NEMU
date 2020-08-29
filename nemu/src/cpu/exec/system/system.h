#ifndef __SYSTEM_H__
#define __SYSTEM_H__

make_helper(lgdt);
//make_helper(lidt);
make_helper(mov_r2cr);
make_helper(mov_cr2r);
//make_helper(int_);
//make_helper(cli);
//make_helper(sti);
//make_helper(iret);
//make_helper(hlt);

#endif
