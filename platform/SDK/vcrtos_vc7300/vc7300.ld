OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
SEARCH_DIR(.)

/* This is only used by gdb to understand where to start */
ENTRY(reset_handler_default)

MEMORY
{
    rom (rx)    : ORIGIN = 0x00000000, LENGTH = 512K
    ram (w!rx)  : ORIGIN = 0x20000000, LENGTH = 128K
}

/* Section Definitions */
SECTIONS
{
    .text :
    {
        . = ALIGN(4);
        _sfixed = .;
        _isr_vectors = DEFINED(_isr_vectors) ? _isr_vectors : . ;
        KEEP(*(SORT(.vectors*)))
        *(.text .text.* .gnu.linkonce.t.*)
        *(.glue_7t) *(.glue_7)
        *(.rodata .rodata* .gnu.linkonce.r.*)
        *(.ARM.extab* .gnu.linkonce.armextab.*)

        /* Support C constructors, and C destructors in both user code
           and the C library. This also provides support for C++ code. */
        . = ALIGN(4);
        KEEP(*(.init))
        . = ALIGN(4);
        __preinit_array_start = .;
        KEEP (*(.preinit_array))
        __preinit_array_end = .;

        . = ALIGN(4);
        __init_array_start = .;
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array))
        __init_array_end = .;

        . = ALIGN(0x4);
        KEEP (*crtbegin.o(.ctors))
        KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
        KEEP (*(SORT(.ctors.*)))
        KEEP (*crtend.o(.ctors))

        . = ALIGN(4);
        KEEP(*(.fini))

        . = ALIGN(4);
        __fini_array_start = .;
        KEEP (*(.fini_array))
        KEEP (*(SORT(.fini_array.*)))
        __fini_array_end = .;

        KEEP (*crtbegin.o(.dtors))
        KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
        KEEP (*(SORT(.dtors.*)))
        KEEP (*crtend.o(.dtors))

        . = ALIGN(4);
        _efixed = .;            /* End of text section */
    } > rom

    /* .ARM.exidx is sorted, so has to go in its own output section.  */
    PROVIDE_HIDDEN (__exidx_start = .);
    .ARM.exidx :
    {
      *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > rom
    PROVIDE_HIDDEN (__exidx_end = .);

    /* exception handling */
    . = ALIGN(4);
    .eh_frame :
    {
        KEEP (*(.eh_frame))
    } > rom

    . = ALIGN(4);
    _etext = .;

    /* stack section */
    .stack (NOLOAD):
    {
        KEEP (*(.puf))
        . = ALIGN(8);
        _sstack = .;
        KEEP (*(.isr_stack))
        . = ALIGN(8);
        _estack = .;
    } > ram

    .relocate :
    {
        . = ALIGN(4);
        _srelocate = .;
        *(.ramfunc .ramfunc.*);
        *(.data .data.*);
        KEEP (*(.openocd .openocd.*))
        . = ALIGN(4);
        _erelocate = .;
    } > ram AT> rom

    /* .bss section which is used for uninitialized data */
    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        _sbss = . ;
        _szero = .;
        *(.bss .bss.*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = . ;
        _ezero = .;
    } > ram

    /* heap section */
    . = ALIGN(4);
    _sheap = . ;
    /* provide `end` need it by libnosys.a(sbrk.o): in function `_sbrk` */
    end = _sheap;
    _eheap = ORIGIN(ram) + LENGTH(ram);

    /* Populate information about ram size */
    _sram = ORIGIN(ram);
    _eram = ORIGIN(ram) + LENGTH(ram);
}
