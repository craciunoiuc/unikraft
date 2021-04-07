SECTIONS
{
	.uk_store_libs_list : {
		PROVIDE(uk_store_libs = .);
		KEEP (*(.uk_store_libs_list));
		QUAD(-0x1);
	}
}
INSERT AFTER .text;
