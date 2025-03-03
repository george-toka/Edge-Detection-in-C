#include <rt_misc.h>

 __value_in_regs struct __initial_stackheap __user_initial_stackheap( 
 unsigned R0, unsigned SP, unsigned R2, unsigned SL){    
 
 struct __initial_stackheap config;     


 config.heap_base   = 0x01C75F4;       
 //config.heap_limit  = 0x000058C0 + 0x00132000;  // 1.2 MB for heap

 config.stack_base  = 0x01C85F4;       
 //config.stack_limit = config.stack_base - 0x00010000;  // 64 KB stack size

 
 return config;} 