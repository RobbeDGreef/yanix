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
- [x] For some reason figlet can't run without previous input, i think it has to do
	  with our vfs system and the stdin file
- [ ] Another f-ing memory overflow bug, happens when i try to run /bin/fork
	  See Current bug track for more info
- [x] Implement a new multitasking system because it is just crap

# Possible reasons for future bugs

- Paging system does not necessairilly allocate continues blocks of real memory
  This can be a problem if the paging system allocates page tables,
  I am not sure though

# Current idea

fork creates a new task space as usual

stack trace

*IRQ happens*
- irq handler
- timer
- scheduler
- task_yield
- task_switch
- task_switch arch

# Current bug

So yeah i know what is going on but now why
so
the CPU tripple faults and doesn't call any other exception
for the simple reason that:
we switch task and load in a corrupted virtual memtable
and thus the cpu tries to call the exception handler for double
fault and can't find it
and thus tripple faults
my best guess is that we somehow memory overflowed 
into our virtual memtable (since it is on the heap it could be anything) and thus this is gonna be fun.... NOT

okay so 
i think we have a "race condition" because 
the 0xC0059000 task is corrupted and thus the tripple
fault depends on when it is called

however we gotta figure out why this motherfucker is corrupted
it is the task created by the user space application but that 
shouldn't matter
so i am going to keep looking i guess
its not like i've been looking for weeks now
