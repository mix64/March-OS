ENTRY(main)

SECTIONS {
	. = 0x10000000000;
	.text	: {*(.text)}
	.rodata	: {*(.rodata)}
	. = ALIGN(0x1000);
	.data	: {*(.data)}
	.bss	: {*(.bss)}
	. = ALIGN(0x1000);
}