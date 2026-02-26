/* link.ld */
OUTPUT_ARCH( "riscv" )
ENTRY( _start )

SECTIONS
{
  . = 0x80000000; /* RAM Start */
  
  .text : {
    *(.text)      /* Code */
  }
  
  .data : {
    *(.data)      /* Global Variables */
  }
  
  .bss : {
    *(.bss)       /* Uninitialized Variables */
  }
}