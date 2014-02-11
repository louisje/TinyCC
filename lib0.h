#ifndef LIB0_H
#define LIB0_H

#define FALSE 0
#define false 0
#define TRUE 1
#define true 1

typedef int BOOL;

#define error0(msg) printf(msg)

typedef struct CString {
    int size; // size in bytes 
    void *data; // either 'char *' or 'nwchar_t *'
    int size_allocated;
    void *data_allocated; // if non NULL, data has been malloced
} CString;

void swap(int *p, int *q);
inline int isid(int c);
inline int isnum(int c);
inline int isoct(int c);
inline int toup(int c);

char *pstrcpy(char *buf, int buf_size, const char *s);
char *pstrcat(char *buf, int buf_size, const char *s);
char *pstrdup(const char *str);

void pfree(void *ptr);
void *pmalloc(unsigned long size);
void *pmallocz(unsigned long size);
void *prealloc(void *ptr, unsigned long size);

void strcat_vprintf(char *buf, int buf_size, const char *fmt, va_list ap);
void strcat_printf(char *buf, int buf_size, const char *fmt, ...);
void cstr_realloc(CString *cstr, int new_size);
inline void cstr_ccat(CString *cstr, int ch);
void cstr_cat(CString *cstr, const char *str);
void cstr_wccat(CString *cstr, int ch);
void cstr_new(CString *cstr);
void cstr_free(CString *cstr);
void add_char(CString *cstr, int c);

#define cstr_reset(cstr) cstr_free(cstr)

void dynarray_add(void ***ptab, int *nb_ptr, void *data);
void dynarray_reset(void *pp, int *n);

#endif
