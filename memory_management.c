#include "user/memory_management.h"
#include "kernel/types.h"
#include "user/user.h"

#define NULL ((void*)0)
#define DATA_SIZE sizeof(block_data)

// Memory allocater and freer created by Joshua Goundry
// joshuagoundry@gmail.com

// Struct used for keeping track of heap memory blocks
struct block_data {
    int size;
    _Bool free;
    struct block_data *next;
    struct block_data *prev;
};

typedef struct block_data block_data;

// Linked list head for block data
block_data *data_head = NULL; 

void* _malloc(int size) {
    // TODO: Align size
    if (size == 0)
        return NULL;

    // Seeks valid block in linked list, returns location if successful
    void *block = find_block(size);
    if (block != NULL)
        return block;

    // Allocates memory on the heap, returns location of successful
    block = allocate_memory(size);
    if (block != NULL)
        return block;

    // Returns NULL if _malloc failed
    return NULL;
}

void _free(void *ptr) {
    if (ptr == NULL)
        return;

    // Gets block data struct for ptr, sets block to free
    block_data *data = ptr - DATA_SIZE;
    data->free = 1;

    // Merges all previous adjacent blocks if free
    while (data->prev && data->prev->free) {
        data->prev->size += DATA_SIZE + data->size;   // ammend size
        if (data->next) {                   // ammend next and prev pointers
            data->prev->next = data->next;
            data->next->prev = data->prev;
        }
        else {                              // ammend next pointer if NULL
            data->prev->next = NULL;
        }
        data = data->prev;  // move pointer to newly located block data
    }

    // Merges all next adjacent blocks if free
    while (data->next && data->next->free) {
        data->size += DATA_SIZE + data->next->size; // ammend size
        if (data->next->next) {             // ammend next and prev pointers
            data->next->next->prev = data;
            data->next = data->next->next;
        }
        else {                              // ammend next pointer if NULL
            data->next = NULL;
        }                                                                                                               
    }
    return;
}

void* allocate_memory(int size) {
    // Call sbrk to extend heap
    void *prev_brk = sbrk(size + DATA_SIZE);    // requested size + size of struct metadata
    if (prev_brk == (void *) -1)                // sbrk failed if -1 returned
        return NULL;

    // Create block data for allocated memory
    block_data *data = prev_brk;
    data->size = size;
    data->free = 0;
    data->next = NULL;
    data->prev = NULL;

    // Add as list head if list is empty else add to end of list
    if (data_head == NULL)
        data_head = data;

    else {
        block_data *current = data_head;
        while (current) {
            if (current->next == NULL) {
                current->next = data;
                data->prev = current;
                break;
            }
            current = current->next;
        }
    }
    // Return void * to requested memory location
    return prev_brk + DATA_SIZE;
}

void* find_block(int size) {
    // Returns NULL if list is empty
    if (data_head == NULL)
        return NULL;
    
    // Iterates current through linked list to find valid block
    block_data *current = data_head;
    while (current) {
        if (current->free == 1) {
            // If block has extra space for another block, split block
            if (size + DATA_SIZE < current->size) {
                // Create new block
                int extra_space = current->size - size - DATA_SIZE;
                block_data *new_block = (void *) current + DATA_SIZE + size;
                new_block->size = extra_space;
                new_block->free = 1;
                new_block->next = current->next;
                new_block->prev = current;

                // Change original block and return it
                current->free = 0;
                current->size = size;
                current->next = new_block;
                return (void *) current + DATA_SIZE;
            }
            else if (size <= current->size) {   // If block has enough requested space
                current->free = 0;
                return (void *) current + DATA_SIZE;    // Return memory location as a void *
            }
        }
        // Iterate current block
        current = current->next;
    }
    // If valid block not found return NULL
    return NULL;
}