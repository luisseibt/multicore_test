#define SIMDEV_CORE_DONE (*(volatile unsigned int *)(0x10000000))
#define SIMDEV_SOUT (*(volatile unsigned int *)(0x10000008)) // for multicore simdev 
// #define SIMDEV_SOUT (*(volatile unsigned int *)(0x10000028)) // for regualr simdev