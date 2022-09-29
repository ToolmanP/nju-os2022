#ifndef __PMM_H
#define __PMM_H

#define BITMAPLEVELS 17
#define BITMAPMAX (1<<BITMAPLEVELS)
#define MALLOCMAX (1<<14)
#define MALLOC_FAILURE ((uintptr_t)-1)
#define STACKSIZE (PGSIZE<<1)

#endif