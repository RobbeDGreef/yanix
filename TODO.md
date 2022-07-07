# Regular

- [ ] Make the bootloader multiboot compliant
- [ ] Rename vector "copy" function to "duplicate"

# Possible reasons for future bugs

- Paging system does not necessairilly allocate continues blocks of real memory
  This can be a problem if the paging system allocates page tables,
  I am not sure though

# well... todo

threading is a terrible soup. We will have to change the whole multitasking 
system because we used to switch on task and now will switch on thread
this creates the problem of stacks. Each thread will have its own stack
duplicated from the caller thread. now thats all fine and dandy but that means
we need to create a system to reliably create stacks with a non-heap address
but the data does need to be, like, a different page n stuff. Yea i know it's bad
but once this is over we will have (nearly) no problem running SMP stuff.

why do the stacks have to be non heap ??? I don't remember and it would make 
everything a lot harder

lets just create a new stack and copy it from its contents
better yet, let's create a heap specifically for this
that would not be secure or something but it would be simple and i care more
about that currently

we are going to give a timeslice to a task
and then that task gives the individual threads CPU's ?
that's simple and very inefficient, .... perfect

okay so i was wrong wth

THREADS ARE NEVER COPIED stupid me
thread stacks can very well be in heap addresses because they start at an 
arbitrary entry point in the program meaning the stack is fresh and without 
ret calls. Whatever it's still complicated asf

the problem is really the first thread but now that i think abt it i really don't 
think we should've changed anything oml i could have just left the first thread as
it was and just spawning new ones with fresh stacks

HHHHHHHHHH
