ENTRY(boot)

SECTIONS {
	. = 0x80200000;

	.text : {
		KEEP(*(.text.boot));
		*(.text .text*);
	}

	.rodata : ALIGN(4) {
		*(.rodata .rodata*);
	}
	.data : ALIGN(4) {
		*(.data .data*)
	}
	.bss : ALIGN(4) {
		__bss_start = .;
		*(.bss .bss* .sbss .sbss*);
		__bss_end = .;
	}

	. = ALIGN(4);
	. += 128 * 1024; /* 128KB */
	__stack_top = .;
}
