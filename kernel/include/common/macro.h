#ifndef __COMMON_H

#define CONCAT(x,y) x ## y
#define CONCAT2 CONCAT
#define CONCAT3(x,y,z) CONCAT(CONCAT(x,y),z)
#define CONCAT4(x,y,z,w) CONCAT(CONCAT3(x,y,z),w)
#define noinline __attribute__((noinline))

#endif