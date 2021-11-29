#include <string.h>

#include "ctx.h"

void ctx_make(struct ctx *ctx, void *entry, void *stack) {
        memset(ctx, 0, sizeof(*ctx));

        unsigned casted_entry = *(unsigned*) entry;
        if(*(unsigned*) entry == 0xfa1e0ff3) stack -= 8;
        ctx->rsp = (unsigned long) stack;
        ctx->rsp -= 8;
        *(unsigned long *)ctx->rsp = (unsigned long) entry;
}

