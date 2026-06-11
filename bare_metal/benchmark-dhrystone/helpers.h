#define SIMDEV_CORE_DONE (*(volatile unsigned int *)(0x10000000))
#define SIMDEV_SOUT (*(volatile unsigned int *)(0x10000008)) // for multicore simdev 
#define SIMDEV_SOUT_CORE0 (*(volatile unsigned int*)0x10000008)
#define SIMDEV_SOUT_CORE1 (*(volatile unsigned int*)0x1000000C)
// #define SIMDEV_SOUT (*(volatile unsigned int *)(0x10000028)) // for regualr simdev