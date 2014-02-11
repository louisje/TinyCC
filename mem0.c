/* memory management */
#ifdef MEM_DEBUG
int mem_cur_size;
int mem_max_size;
unsigned malloc_usable_size(void*);
#endif

void tcc_free(void *ptr)
{
#ifdef MEM_DEBUG
    mem_cur_size -= malloc_usable_size(ptr);
#endif
    free(ptr);
}

void *tcc_malloc(unsigned long size)
{
    void *ptr;
    ptr = malloc(size);
    if (!ptr && size)
        error("memory full");
#ifdef MEM_DEBUG
    mem_cur_size += malloc_usable_size(ptr);
    if (mem_cur_size > mem_max_size)
        mem_max_size = mem_cur_size;
#endif
    return ptr;
}

void *tcc_mallocz(unsigned long size)
{
    void *ptr;
    ptr = tcc_malloc(size);
    memset(ptr, 0, size);
    return ptr;
}

void *tcc_realloc(void *ptr, unsigned long size)
{
    void *ptr1;
#ifdef MEM_DEBUG
    mem_cur_size -= malloc_usable_size(ptr);
#endif
    ptr1 = realloc(ptr, size);
#ifdef MEM_DEBUG
    /* NOTE: count not correct if alloc error, but not critical */
    mem_cur_size += malloc_usable_size(ptr1);
    if (mem_cur_size > mem_max_size)
        mem_max_size = mem_cur_size;
#endif
    return ptr1;
}

#define free(p) use_tcc_free(p)
#define malloc(s) use_tcc_malloc(s)
#define realloc(p, s) use_tcc_realloc(p, s)

void tcc_memstats(void)
{
#ifdef MEM_DEBUG
    printf("memory in use: %d\n", mem_cur_size);
#endif
}
