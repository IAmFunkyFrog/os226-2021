
#include <stddef.h>

#include "pool.h"

void pool_init(struct pool *p, void *mem, unsigned long nmemb, unsigned long membsz) {
    *p = POOL_INITIALIZER(p, nmemb, membsz);
}

void *pool_alloc(struct pool *p) {
    if(p->free != NULL) {
        struct pool_free_block *fb = p->free;
        p->free = fb->next;
        return fb;
    }
    else if(p->freestart < p->freeend) {
        void *b = p->freestart;
        p->freestart += p->membsz;
        return b;
    }
	return NULL;
}

void pool_free(struct pool *p, void *ptr) {
    if(ptr >= p->mem && ptr <= p->freeend) {
        struct pool_free_block *newb = ptr;
        *newb = (struct pool_free_block){.next = p->free};
        p->free = newb;
    }
}
