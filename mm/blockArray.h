#ifndef BLOCKARRAY_H
#define BLOCKARRAY_H
 
#include <stdint.h>

typedef struct {
	uint32_t start;
	uint32_t end;
	uint32_t size;
	int ishole;
} block;

typedef struct {
	block* array;
	uint32_t size; // amount of indexes (if you want the absolute size do sizeof(block)*blockArray->size)
	uint32_t maxsize;
} blockArray;

void removeBlock(blockArray *array, uint32_t index);
void insertNewBlock(blockArray *array, block b);
blockArray *createEmptyBlockArray(uint32_t maxsize);
signed int findBlock(blockArray *array, block b);
signed int findBlockWithStart(blockArray *array, uint32_t start);
signed int findBlockWithEnd(blockArray *array, uint32_t end);
uint32_t alignedAddressInBlock(block b);
#endif