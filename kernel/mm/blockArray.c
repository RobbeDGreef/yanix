#include <libk/string.h>
#include <kernel.h>
#include <mm/blockArray.h>
#include <mm/heap.h>



blockArray *createEmptyBlockArray(uint32_t maxsize){
	blockArray *ret = (blockArray*) kmalloc(sizeof(blockArray));
	ret->array = (block*) kmalloc(sizeof(block)*maxsize);
	memset((void*)ret->array, 0, sizeof(block)*maxsize); 	// clear the whole array and overwrite them with 0's
	ret->size = 0;
	ret->maxsize = maxsize;
	return ret;
}

static void insertBlockArray(blockArray *array, block b, uint32_t index) {

	block tmp = array->array[index];
	block tmp2;
	array->array[index] = b;
	index++;
	while (index<=array->size){
		tmp2 = array->array[index];
		array->array[index] = tmp;
		tmp = tmp2;
		index++;
	}
}

void insertNewBlock(blockArray *array, block b) {
	for (uint32_t i = 0; i < array->size; i++) {
		if (b.size < array->array[i].size) {
			// insert here and push the rest of the items up
			insertBlockArray(array, b, i);
			array->size++;
			return;
		}
	}
	// append to the end i guess??
	if (array->size+1 <= array->maxsize){
		array->array[array->size] = b;
		array->size++;
	} else {
		printk(KERN_CRIT "\nmax size of array reached cannot append");
	}
}

void removeBlock(blockArray *array, uint32_t index) {
	// todo: code this
	
	// !!! DECREMENT SIZE !!!
	
	while (index < array->size) {
		array->array[index] = array->array[index+1];
		index++;
	}
	array->size--;

}

signed int findBlock(blockArray *array, block b) {
	for (uint32_t i = 0; i<array->size; i++){
		if ((b.start == array->array[i].start) && (b.size == array->array[i].size)) {
			// the size check is actually unnecesairy i think
			return i;
		}
	}
	// not found
	return -1;
}

signed int findBlockWithStart(blockArray *array, uint32_t start){
	for (uint32_t i = 0; i<array->size; i++){
		
		if ((start == array->array[i].start)) {
			return i;
		}
	}
	// not found
	return -1;
}

signed int findBlockWithEnd(blockArray *array, uint32_t end){
	for (uint32_t i = 0; i<array->size; i++){
		
		if ((end == array->array[i].end)) {
			return i;
		}
	}
	// not found
	return -1;
}

uint32_t alignedAddressInBlock(block b){
	uint32_t addr = (b.start & 0xFFFFF000) +0x1000;
	if (b.start <= addr && addr <= b.end){
		return addr;
	} else {
		return 0;
	}
}