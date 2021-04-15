SECTIONS
{
	.uk_store_libs_list : {
		PROVIDE(uk_store_libs = .);
		KEEP(*(.uk_store_libs_list));
	}
}
INSERT AFTER .text;
