SECTIONS
{
  .uk_store_buffer_list : {
    PROVIDE(uk_store_buffer = .);
    KEEP (*(.uk_store_buffer_list))
    LONG(0x0)
  }
}
INSERT AFTER .text;
