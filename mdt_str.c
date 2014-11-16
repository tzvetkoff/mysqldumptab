#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

#include "mdt_str.h"

struct mdt_str_t *
mdt_str_alloc()
{
    struct mdt_str_t *result = calloc(1, sizeof(struct mdt_str_t));

    if (!result) {
        fprintf(stderr, "Cannot allocate dynamic string\n");
        exit(EXIT_FAILURE);
    }

    result->len = 0;
    result->siz = 0x400;
    result->ptr = calloc(0x400 + 1, 1);

    if (!result->ptr) {
        free(result);
        fprintf(stderr, "Cannot allocate dynamic string\n");
        exit(EXIT_FAILURE);
    }

    return result;
}

void
mdt_str_free(struct mdt_str_t *mdt_str)
{
    free(mdt_str->ptr);
    free(mdt_str);
}

void
mdt_str_grow(struct mdt_str_t *mdt_str, int amount)
{
    char *ptr;

    ptr = realloc(mdt_str->ptr, mdt_str->siz + amount);
    if (!ptr) {
        fprintf(stderr, "Cannot resize dynamic string to %d\n", mdt_str->siz +
            amount);
        mdt_str_free(mdt_str);
        exit(EXIT_FAILURE);
    }

    mdt_str->siz += amount;
    mdt_str->ptr = ptr;
}

void
mdt_str_concat(struct mdt_str_t *mdt_str, const char *fmt, ...)
{
    va_list ap;
    int len, amount;

    /* estimate size */
    va_start(ap, fmt);
    len = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);

    /* check if we need to grow the string */
    if (mdt_str->len + len > mdt_str->siz) {
        amount = floor((float)(mdt_str->len + len - mdt_str->siz) /
            (float)0x400) * 0x400;
        mdt_str_grow(mdt_str, amount);
    }

    va_start(ap, fmt);
    vsnprintf(mdt_str->ptr + mdt_str->len, mdt_str->siz - mdt_str->len, fmt, ap);
    mdt_str->len += len;
    va_end(ap);
}
