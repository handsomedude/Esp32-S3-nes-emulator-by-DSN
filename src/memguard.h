#ifndef _MEMGUARD_H_
#define _MEMGUARD_H_

#include <stdlib.h>
#include <stdbool.h>  
#include <string.h>  
 
#define mem_checkblocks()
#define mem_checkleaks()
#define mem_cleanup()
#define mem_term()
 
void *mem_alloc(int size, bool prefer_fast_memory);
void mem_free(void *ptr);
void *mem_calloc(int nmemb, int size, bool prefer_fast_memory);
 
#define NOFRENDO_MALLOC(s) mem_alloc(s, false)
#define NOFRENDO_FREE(p)   mem_free(p)
#define NOFRENDO_STRDUP(s) strdup(s)

#ifdef __cplusplus
extern "C" {
#endif
 
int nofrendo_log_printf(const char *format, ...);
 
#define log_printf nofrendo_log_printf

#ifdef __cplusplus
}
#endif

#endif