SECTIONS
{
	.uk_store_libs_list : {
		PROVIDE(uk_store_libs = .);
		KEEP(*(.uk_store_libs_list));
	}
}
INSERT AFTER .text;

/* 1 memory holder for the dynamic lists for the tree lists */

/* another one for the starting address for the static entries + end address */
