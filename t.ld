/****************
qemu with -kernel file.bin ==> start to execute from 0x10000 ==>
file.bin must be loaded to 0x10000 ==> . = 0x10000
************/
ENTRY(reset)
SECTIONS
{
  . = 0x10000;
  .text : { ts.o *(.text) }
  .data : { *(.data) }
  .ARM : {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  }
  .bss : {
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
   *(.bss) 
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
   }
  . = ALIGN(8);
/*  .data : { panda.o }        */
  /*.data : { porkcar.o }     */
 _end = . ;
    end = .;
  
  . = . + 0xe000000; /*4kB of SVC stack space */
  irq_stack = .;
  . = . + 0x4000; /* 4kB of IRQ stack space */
  svc_stack = .;
  stack_top = .;
 /* . = . + 128M-0x10000;            SVC stack */
  /*stack_top = 128M-0x10000;
  irq_stack = stack_top - 0x4000;
  svc_stack = stack_top;
   */
}

