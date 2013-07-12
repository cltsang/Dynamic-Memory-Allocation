#include <stdio.h>
#include <unistd.h>

typedef struct header {
	struct header *ptr;
	unsigned int size;
} Header;


void * malloc_simple	(unsigned int nbytes);
void * malloc_first_fit	(unsigned int nbytes);
void * malloc_best_fit	(unsigned int nbytes);
void * malloc_worst_fit	(unsigned int nbytes);
void free_simple	(void *addr);
void free_no_tail	(void *addr);
