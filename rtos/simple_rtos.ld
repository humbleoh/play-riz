/*
 * STM32F103C8T6 (BluePill) 链接脚本
 * Flash: 64KB (0x08000000 - 0x0800FFFF)
 * SRAM:  20KB (0x20000000 - 0x20004FFF)
 */

ENTRY(reset_handler)

/* 内存定义 */
MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
    SRAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 20K
}

/* 栈大小定义 */
_stack_size = 0x400;  /* 1KB 栈 */

/* 段定义 */
SECTIONS
{
    /* 中断向量表 */
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } >FLASH

    /* 程序代码 */
    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.glue_7)
        *(.glue_7t)
        *(.eh_frame)
        
        KEEP (*(.init))
        KEEP (*(.fini))
        
        . = ALIGN(4);
        _etext = .;
    } >FLASH

    /* 只读数据 */
    .rodata :
    {
        . = ALIGN(4);
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
    } >FLASH

    /* ARM异常处理索引 */
    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } >FLASH
    
    .ARM :
    {
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
    } >FLASH

    /* 初始化数据的加载地址 */
    _sidata = LOADADDR(.data);

    /* 初始化数据段 */
    .data :
    {
        . = ALIGN(4);
        _sdata = .;        /* 数据段开始 */
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;        /* 数据段结束 */
    } >SRAM AT> FLASH

    /* 未初始化数据段 */
    .bss :
    {
        . = ALIGN(4);
        _sbss = .;         /* BSS段开始 */
        __bss_start__ = _sbss;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;         /* BSS段结束 */
        __bss_end__ = _ebss;
    } >SRAM

    /* 用户堆栈 */
    ._user_heap_stack :
    {
        . = ALIGN(8);
        PROVIDE ( end = . );
        PROVIDE ( _end = . );
        . = . + _stack_size;
        . = ALIGN(8);
        _estack = .;       /* 栈顶 */
    } >SRAM

    /* 调试信息 */
    .ARM.attributes 0 : { *(.ARM.attributes) }
    .debug_info     0 : { *(.debug_info) }
    .debug_abbrev   0 : { *(.debug_abbrev) }
    .debug_aranges  0 : { *(.debug_aranges) }
    .debug_ranges   0 : { *(.debug_ranges) }
    .debug_line     0 : { *(.debug_line) }
    .debug_str      0 : { *(.debug_str) }
    .debug_loc      0 : { *(.debug_loc) }
    .debug_macinfo  0 : { *(.debug_macinfo) }
    .debug_frame    0 : { *(.debug_frame) }
    .debug_pubnames 0 : { *(.debug_pubnames) }
    .debug_pubtypes 0 : { *(.debug_pubtypes) }
}

/* 检查栈溢出 */
ASSERT(_estack <= ORIGIN(SRAM) + LENGTH(SRAM), "Stack overflow!")

/* 提供符号给C代码使用 */
PROVIDE(_stack_top = ORIGIN(SRAM) + LENGTH(SRAM));
PROVIDE(_heap_start = _ebss);
PROVIDE(_heap_end = _estack - _stack_size);