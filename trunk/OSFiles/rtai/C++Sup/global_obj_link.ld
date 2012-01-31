OUTPUT_FORMAT("elf32-i386")
SECTIONS
{
	.text 0x100000:
	{
		code = .;_code = .;__code = .;
		*(.text)
		. = ALIGN(4096);
	}

	.linker_rtaicsuplib_data_ :
	{
		__CTOR_LIST__ = .;LONG((__CTOR_END__ - __CTOR_LIST__) / 4 - 2) *(.ctors) LONG(0) __CTOR_END__ = .;
		__DTOR_LIST__ = .;LONG((__DTOR_END__ - __DTOR_LIST__) / 4 - 2) *(.dtors) LONG(0) __DTOR_END__ = .;
		linker_rtaicsuplib_data_ = .;_linker_rtaicsuplib_data_ = .;__linker_rtaicsuplib_data_ = .;
		*(.linker_rtaicsuplib_data_)
		. = ALIGN(4096);
	}
	
	.bss :
	{
		bss = .;_bss = .; __bss = .;
		*(.bss)
		. = ALIGN(4096);
	}

	end = .; _end = .; __end = .;
}
