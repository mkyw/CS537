#include "mem.h"    
#include <stdbool.h>    
extern BLOCK_HEADER* first_header;

// return a pointer to the payload
// if a large enough free block isn't available, return NULL
void* Mem_Alloc(int size){
    // Return null for invalid input
    if (size <= 0) {
        return NULL;
    }   

    // Find a free block that's big enough
    int padding = 0;
    int blockSize = size + 8;
    while (blockSize % 16) {
        blockSize++;
        padding++;
    }   

    BLOCK_HEADER* ptr = first_header;
    while (ptr->size_alloc!=1) {
        if ((ptr->size_alloc&0xFFFFFFFE) >= blockSize) {
            // allocate the block
            int total_alloc = ptr->size_alloc - (ptr->size_alloc %2);
                ptr->payload = size;
            if (ptr->size_alloc - blockSize >= 16) {
                BLOCK_HEADER* next = (BLOCK_HEADER*) ((unsigned long) ptr + blockSize);
                next->size_alloc = total_alloc - blockSize;
                next->payload = total_alloc - blockSize -8;
            }
            ptr->size_alloc = blockSize + 1;
            return (void*) (ptr + 1);
        }
        
        if (ptr->size_alloc==1) {
            return NULL;
        } else {
            ptr = (BLOCK_HEADER*) ((unsigned long)ptr + ptr->size_alloc-(ptr->size_alloc%2));
        }   
    }   

    // return NULL if we didn't find a block
    return NULL;
}


// return 0 on success
// return -1 if the input ptr was invalid
int Mem_Free(void *ptr){
    if (ptr == NULL || (unsigned long)ptr %8 != 0) {
        return -1; 
    }   

    BLOCK_HEADER* header = ptr-8;
    BLOCK_HEADER* prev_block = header;
    if(header->size_alloc%2==0){
        return -1;
    }
    // traverse the list and check all pointers to find the correct block 
    // if you reach the end of the list without finding it return -1
    while(header->size_alloc!=1){
        // finding the previous block so we can coalesce
        if((unsigned long)header+header->size_alloc-header->size_alloc%2 == (unsigned long)(ptr-8)){
            prev_block = header;
        }
    // free the block 
        if((unsigned long)header==(unsigned long)(ptr-8)){
            break;
        }
        header = (BLOCK_HEADER*)((unsigned long)header+header->size_alloc-header->size_alloc%2);
    }
    if(header->size_alloc==1)return -1;
    header = ptr-8;
    // coalesce adjacent free blocks
    int block_size = header->size_alloc - header->size_alloc%2;
    if(((BLOCK_HEADER*)((unsigned long)header+block_size))->size_alloc%2==0){
        block_size+=((BLOCK_HEADER*)((unsigned long)header+block_size))->size_alloc;
    }
    if(prev_block->size_alloc%2==0 && (unsigned long)prev_block >= (unsigned long)first_header){
        block_size+=prev_block->size_alloc;
        header = prev_block;
    }

    // updating header
    header->size_alloc = block_size;
    header->payload = block_size-8;
    return 0;
    return -1;
}

