SECTIONS
{
	.uk_store_entries_list : {
		. = ALIGN(0x8);
		PROVIDE(uk_store_entries_start = .);
		KEEP(*(.uk_store_entries_list));
		KEEP(*(.uk_store_entries_list.*));
		PROVIDE(uk_store_entries_end = .);
	}

	.uk_store_libs_list : {
		. = ALIGN(0x8);
		PROVIDE(uk_store_libs_start = .);
		KEEP(*(.uk_store_libs_list));
		KEEP(*(.uk_store_libs_list.*));
		PROVIDE(uk_store_libs_end = .);
	}
}
INSERT AFTER .text;

