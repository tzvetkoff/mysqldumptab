#ifndef MDT_STR_H
#define MDT_STR_H 1

/* dynamic strings */
struct mdt_str_t {
    unsigned int len;
    unsigned int siz;
    char *ptr;
};

struct mdt_str_t *
mdt_str_alloc();

void
mdt_str_free(struct mdt_str_t *mdt_str);

void
mdt_str_clear(struct mdt_str_t *mdt_str);

void
mdt_str_grow(struct mdt_str_t *mdt_str, int amount);

void
mdt_str_concat(struct mdt_str_t *mdt_str, const char *fmt, ...);

#endif
