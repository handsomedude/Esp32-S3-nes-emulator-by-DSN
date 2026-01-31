#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <esp32-hal-psram.h>
#include "memguard.h"
 
void *mem_alloc(int size, bool prefer_fast_memory) {
    void* ptr = NULL;
     
    if (psramFound()) {
        ptr = ps_malloc(size);
    }
     
    if (!ptr) {
        ptr = malloc(size);
    }
     
    if (ptr) {
        memset(ptr, 0, size);
    }
    
    return ptr;
}

void *mem_calloc(int nmemb, int size, bool prefer_fast_memory) {
    return mem_alloc(nmemb * size, prefer_fast_memory);
}

void mem_free(void *ptr) {
    free(ptr);
}