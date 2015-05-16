#ifndef __RAM_H__
#define __RAM_H__

extern unsigned int _start_ram;
extern unsigned int _end_ram;
#define IS_RAM(x) (((void*)x >= (void*)&_start_ram) && ((void*)x < (void*)&_end_ram))
#endif
