#include <mm/blockArray.h>
#include <mm/heap.h>
#include <mm/paging.h>

#include <lib/function.h>
#include <drivers/video/videoText.h>

#include <stdint.h>
#include <stddef.h>

extern uint32_t end_of_bin;
volatile uint32_t placement_address = (uint32_t) &end_of_bin; // this should actually point to the end of the kernel in memory but that should be set at linktime
heap_t *kernelHeap = 0;
extern page_directory_t *g_kernel_directory;

#define True  1
#define False 0

/**
 * @brief      Debug function to see the heap status
 *
 * @param      heap  The heap
 */
static void heapstatus(heap_t *heap){
    print("\nheap status ");print("(size: ");print_int(heap->size);print("): ");
    print("\n    blocks: ");print_int(heap->array->size);
    print("\n    start:  ");print_hex(heap->start);
    print("\n    end:    ");print_hex(heap->start+heap->maxsize);
}




/**
 * @brief      Creates a heap.
 *
 * @param[in]  start     The start of the heap
 * @param[in]  maxsize   The maxsize of the heap
 * @param[in]  maxindex  The maximum amount of indexes in the bitmap of the heap
 * @param[in]  user      Whether it is usermode heap or not
 *
 * @return     A pointer to a heap structure
 */
heap_t *createHeap(uint32_t start, uint32_t maxsize, uint32_t maxindex, bool user)
{
    heap_t *heap    = (heap_t*) kmalloc_base(sizeof(heap_t), 1, 0);
    heap->array     = (blockArray *) createEmptyBlockArray(maxindex);
    heap->start     = start;
    heap->maxsize   = maxsize;
    heap->user      = user; 
    return heap;
}

/**
 * @brief      Initializes the kernel heap
 */
void init_kheap()
{
    kernelHeap = createHeap(KHEAP_START, KHEAP_MAXSIZE, KHEAP_MAXINDEX, False);
}

/**
 * @brief      Tries to expand a heap with given size
 *
 * @param      heap  The heap
 * @param[in]  size  The size
 */
static void expand(heap_t *heap, size_t size)
{
    uint32_t iter = 0;
    // paging stuff
    for (;iter < size; iter+=1000){
        // todo: we should never only use writeable block we should implement readonly into our heap
        alloc_frame(get_page(heap->start+heap->size+iter, 1, (page_directory_t*) g_kernel_directory), (heap->user)?0:1, True); // readonly now always false
    }
    heap->size = heap->size + size;
}

/**
 * @brief      Allocates a block of memory on a heap
 *
 * @param      heap     The heap
 * @param[in]  aligned  The aligned
 * @param[in]  size     The size
 *
 * @return     pointer to the memory
 */
void *alloc(heap_t *heap, bool aligned, uint32_t size)
{ 
    blockArray *array = heap->array;
    if (array->size != 0){
        for (uint32_t i = 0; i < array->size; i++){
            if (array->array[i].ishole == True){
                if (aligned == False){
                    if (array->array[i].size == size) {
                        // fits perfectly
                        array->array[i].ishole = False;
                        return (void*)array->array[i].start;
                    } else if (size < array->array[i].size) {
                        block tmp, tmp2;

                        tmp.start  = array->array[i].start;
                        tmp.end    = array->array[i].start + size;
                        tmp.size   = size;
                        tmp.ishole = False;

                        tmp2.start  = array->array[i].start + size;
                        tmp2.end    = array->array[i].end;
                        tmp2.size   = array->array[i].size - size;
                        tmp2.ishole = True;
                        
                        removeBlock(array, i);
                        
                        insertNewBlock(array, tmp);
                        insertNewBlock(array, tmp2);
                        return (void*) tmp.start;

                    }
                } else {
                    if (array->array[i].size == size && (array->array[i].start && 0xFFFFF000)) {
                        // fits perfectly and is page aligned
                        array->array[i].ishole = False;
                        return (void*)array->array[i].start;
                    } else if (size < array->array[i].size) {
                        uint32_t a = alignedAddressInBlock(array->array[i]);
                        
                        if (a == 0){  // if a == 0 (no aligned addr in block)
                            continue;
                        } else if (array->array[i].end < a + size){ // a + size is larger than the end of the block
                            continue;
                        }

                        if (a == array->array[i].start){
                            block tmp, tmp2;

                            tmp.start  = array->array[i].start;
                            tmp.end    = array->array[i].start + size;
                            tmp.size   = size;
                            tmp.ishole = False;

                            tmp2.start  = array->array[i].start + size;
                            tmp2.end    = array->array[i].end;
                            tmp2.size   = array->array[i].size - size;
                            tmp2.ishole = True;
                            
                            removeBlock(array, i);
                            
                            insertNewBlock(array, tmp);
                            insertNewBlock(array, tmp2);
                            return (void*) tmp.start;
                        }
                        else {
                            // three blocks

                            block tmp, tmp2;
                            tmp.start  = array->array[i].start;
                            tmp.end    = a;
                            tmp.size   = a - array->array[i].start;
                            tmp.ishole = True;

                            tmp2.start  = a;
                            tmp2.end    = a+size;
                            tmp2.size   = size;
                            tmp2.ishole = False;
                            if (size+a < array->array[i].end){
                                block tmp3;
                                tmp3.start  = a+size;
                                tmp3.end    = array->array[i].end;
                                tmp3.size   = array->array[i].end - (a+size);
                                tmp3.ishole = True;
                                insertNewBlock(array, tmp3);
                            }
                            removeBlock(array, i);
                            insertNewBlock(array, tmp);
                            insertNewBlock(array, tmp2);
                            return (void*) tmp2.start;
                        }
                    }
                }
            }
        }
    }
    // can we expand the heap?
    if (size + heap->size <= heap->maxsize && aligned == False){
        // expand heap
        // now add a new block
        block b;
        b.start     = heap->start + heap->size; // this will become the biggest block in the heap so it should be appended at the end
        b.size      = size;
        b.ishole    = False;
        b.end       = b.start + size;
        expand(heap, size);
        insertNewBlock(array, b);
        return (void*) b.start;
    
    } else if (aligned == True && ((heap->size & 0xFFFFF000) + 0x1000 + size) <= heap->maxsize) {
        block b, tmp;
        tmp.start = 0; // just a flag
        
        b.start     = ((heap->start + heap->size) & 0xFFFFF000) + 0x1000;
        b.size      = size;
        b.ishole    = False;
        b.end       = b.start + b.size;
        
        if (heap->start + heap->size != (((heap->start+heap->size) & 0xFFFFF000)+0x1000)){
            tmp.start   = heap->start + heap->size;
            tmp.size    = b.start - (heap->start + heap->size);
            tmp.end     = b.start;
            tmp.ishole  = True;

        }
        if (tmp.start != 0){
            expand(heap, (((heap->size&0xFFFFF000)+0x1000)-heap->size)+size);
            insertNewBlock(array, tmp);
            insertNewBlock(array, b);
        } else {
            expand(heap, size);
            insertNewBlock(array, b);
        }
        return (void*) b.start;
    
    } else {
        // return error
        print("we got an error (cannot expand heap)");
        print("heap size would be: "); print_int(heap->size + size);
        print("\nheap maxsize is: "); print_int(heap->maxsize);
        //print("\nadded size is: "); print_hex(size);
        heapstatus(kernelHeap);
        return 0;
    }

}


/**
 * @brief      Frees up a block of memory from a heap
 *
 * @param      heap     The heap
 * @param      address  The address
 *
 * @return     Success code
 */
int free(heap_t *heap, uint32_t *address)
{
    /*
    since you as user only have the start address of your allocated space and not the block object
    we need to search for the block object with the findBlockWithStart function and when found need
    to mark it as a hole and we're done
    ...
    except that we aren't we actually need to check whether there is another block in the array that ends with our
    start address that is a hole aswell and do the same thing for the endaddress (equal to its start)
    */

    signed int index = findBlockWithStart(heap->array, (uint32_t) address);
    if (index == -1){
        print("got error: could not find address in heap\n");
        print("Watch out this results in this block of memory lost and thus we leeked memory!\n");
        return -1;
    }

    heap->array->array[index].ishole = True;

    signed int lind = findBlockWithEnd(heap->array, heap->array->array[index].start);
    signed int rind = findBlockWithStart(heap->array, heap->array->array[index].end);


    if (heap->array->array[lind].ishole == True && heap->array->array[rind].ishole && lind != -1 && rind != -1) {
        // both appendable 
        
        block newblock;
        newblock.start  = heap->array->array[lind].start;
        newblock.size   = heap->array->array[lind].size + heap->array->array[index].size + heap->array->array[rind].size;
        newblock.end    = heap->array->array[rind].end;
        newblock.ishole = True;
        removeBlock(heap->array, lind);
        removeBlock(heap->array, index);
        removeBlock(heap->array, rind);
        insertNewBlock(heap->array, newblock);

    } else if (heap->array->array[lind].ishole == True && lind != -1){
        // only the left is appendable
        
        block newblock;
        newblock.start  = heap->array->array[lind].start;
        newblock.size   = heap->array->array[lind].size + heap->array->array[index].size;
        newblock.end    = heap->array->array[index].end;
        newblock.ishole = True;
        removeBlock(heap->array, lind);
        removeBlock(heap->array, index);
        insertNewBlock(heap->array, newblock);

    } else if (heap->array->array[rind].ishole == True && lind != -1){
        // only the right is appendable
        
        block newblock;
        newblock.start  = heap->array->array[rind].start;
        newblock.size   = heap->array->array[rind].size + heap->array->array[index].size;
        newblock.end    = heap->array->array[index].end;
        newblock.ishole = True;
        removeBlock(heap->array, rind);
        removeBlock(heap->array, index);
        insertNewBlock(heap->array, newblock);
    }
    return 0;
}



/**
 * @brief      Extensive kernel malloc function
 *
 * @param[in]  size             The size
 * @param[in]  aligned          The aligned
 * @param      physicaladdress  The physicaladdress
 *
 * @return     Pointer to memory
 */
void *kmalloc_base(size_t size, int aligned, phys_addr_t *physicaladdress)
{
    if (kernelHeap != 0) {
        uintptr_t *pointer = alloc(kernelHeap, aligned, size);
        if (physicaladdress != 0 && pointer != 0){
            page_t *page = get_page((off_t)pointer, 0, g_kernel_directory);
            *physicaladdress = page->frame*0x1000 + ((unsigned int)pointer&0xFFF);
        }

        return (void*) pointer;
    } else {
        if (aligned == 1 && (placement_address & 0xFFFFF000)){
            placement_address &= 0xFFFFF000;
            placement_address += 0x1000;
        }
        if (physicaladdress) {
            *physicaladdress = placement_address;
        }

        placement_address += size;
        return (void*) (placement_address-size);
    }
}

/**
 * @brief      Simple kernel kmalloc function
 *
 * @param[in]  size  The size
 *
 * @return     Pointer to memory
 */
void *kmalloc(size_t size)
{
    return kmalloc_base(size, 0, 0);
}

/**
 * @brief      Frees a block of memory
 *
 * @param      addr  The address of the memory block
 *
 * @return     success of failure
 */
int kfree(void *addr)
{
    return free(kernelHeap, addr);
}