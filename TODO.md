# Regular

- [ ] Make the kernel multiboot compliant
- [ ] Make the bootloader multiboot compliant
- [x] New memory manager since the old one is starting to show bugs?
	- After writing the new mem manager i figured out where the bugs
	  came from, it were just two memory overflow bugs in 
	  the ext2 driver and in the tty system. Anyway the new memory
	  manager is faster and better than the last one so this one will
	  stay.
- [ ] New memory manager freeing does not remerge blocks yet
- [x] Paging system is tripping the f out, I need to figure out why
	- Bug fixed, it was because kmalloc_base() didn't set the physical address.
	  The reason was because of our new page_fault() system, it tried to find
	  the physical address with the frame address of get_page but the pages
	  didn't have any pages assigned yet.



# Possible reasons for future bugs

- Paging system does not necessairilly allocate continues blocks of real memory
  This can be a problem if the paging system allocates page tables,
  I am not sure though
