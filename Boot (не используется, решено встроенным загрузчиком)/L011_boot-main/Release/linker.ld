MEMORY{
	ROM(rx) 		: ORIGIN = 0x08000000, LENGTH = 1K
	SRAM(rwx)		: ORIGIN = 0x20000000, LENGTH = 1K
	SRAM_PRG(rwx)	: ORIGIN = 0x20000000 + LENGTH(SRAM), LENGTH = 1K
}

_estack = LENGTH(SRAM) + ORIGIN(SRAM);

SECTIONS{
	.isr_vector : {
	KEEP(*(.isr_vector))	
	} >ROM
	
	.text : {
        . = ALIGN(4);
        *(.text)
		*(.rodata)
    } >ROM
}