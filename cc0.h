#ifndef CC0_H
#define CC0_H

//================  TCC.H ================================
#define _GNU_SOURCE
#include "config.h"

#ifdef CONFIG_TCCBOOT

#include "tccboot.h"
#define CONFIG_TCC_STATIC

#else

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <signal.h>
#include <fcntl.h>
#include <setjmp.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <sys/timeb.h>
#include <io.h> /* open, close etc. */
#include <direct.h> /* getcwd */
#define inline __inline
#define inp next_inp
#endif

#ifndef _WIN32
#include <unistd.h>
#include <sys/time.h>
#include <sys/ucontext.h>
#include <sys/mman.h>
#endif

#endif /* !CONFIG_TCCBOOT */

#ifndef PAGESIZE
#define PAGESIZE 4096
#endif

#include "elf.h"
#include "stab.h"

#ifndef O_BINARY
#define O_BINARY 0
#endif

/* parser debug */
//#define PARSE_DEBUG
/* preprocessor debug */
//#define PP_DEBUG
/* include file debug */
//#define INC_DEBUG

//#define MEM_DEBUG

/* assembler debug */
//#define ASM_DEBUG

/* target selection */
//#define TCC_TARGET_I386   /* i386 code generator */
//#define TCC_TARGET_ARM    /* ARMv4 code generator */
//#define TCC_TARGET_C67    /* TMS320C67xx code generator */
//#define TCC_TARGET_X86_64 /* x86-64 code generator */

/* default target is I386 */
#if !defined(TCC_TARGET_I386) && !defined(TCC_TARGET_ARM) && \
    !defined(TCC_TARGET_C67) && !defined(TCC_TARGET_X86_64)
#define TCC_TARGET_I386
#endif

#if !defined(_WIN32) && !defined(TCC_UCLIBC) && !defined(TCC_TARGET_ARM) && \
    !defined(TCC_TARGET_C67) && !defined(TCC_TARGET_X86_64)
#define CONFIG_TCC_BCHECK /* enable bound checking code */
#endif

#if defined(_WIN32) && !defined(TCC_TARGET_PE)
#define CONFIG_TCC_STATIC
#endif

/* define it to include assembler support */
#if !defined(TCC_TARGET_ARM) && !defined(TCC_TARGET_C67) && \
    !defined(TCC_TARGET_X86_64)
#define CONFIG_TCC_ASM
#endif

/* object format selection */
#if defined(TCC_TARGET_C67)
#define TCC_TARGET_COFF
#endif

#if !defined(_WIN32) && !defined(CONFIG_TCCBOOT)
#define CONFIG_TCC_BACKTRACE
#endif

/* path to find crt1.o, crti.o and crtn.o. Only needed when generating
   executables or dlls */
#define CONFIG_TCC_CRT_PREFIX CONFIG_SYSROOT "/usr/lib"

#define INCLUDE_STACK_SIZE  32
#define IFDEF_STACK_SIZE    64
#define VSTACK_SIZE         256
#define STRING_MAX_SIZE     1024
#define PACK_STACK_SIZE     8

#define TOK_HASH_SIZE       8192 /* must be a power of two */
#define TOK_ALLOC_INCR      512  /* must be a power of two */
#define TOK_MAX_SIZE        4 /* token max size in int unit when stored in string */

#ifdef TCC_TARGET_PE
typedef unsigned short nwchar_t;
#else
typedef int nwchar_t;
#endif


#define FUNC_CALL(r) (((func_attr_t*)&(r))->func_call)
#define FUNC_EXPORT(r) (((func_attr_t*)&(r))->func_export)
#define FUNC_ARGS(r) (((func_attr_t*)&(r))->func_args)
#define INLINE_DEF(r) (*(int **)&(r))
/* -------------------------------------------------- */

#define SYM_STRUCT     0x40000000 /* struct/union/enum symbol space */
#define SYM_FIELD      0x20000000 /* struct/union field symbol space */
#define SYM_FIRST_ANOM 0x10000000 /* first anonymous sym */

/* stored in 'Sym.c' field */
#define FUNC_NEW       1 /* ansi function prototype */
#define FUNC_OLD       2 /* old function prototype */
#define FUNC_ELLIPSIS  3 /* ansi function prototype with ... */

/* stored in 'Sym.r' field */
#define FUNC_CDECL     0 /* standard c call */
#define FUNC_STDCALL   1 /* pascal c call */
#define FUNC_FASTCALL1 2 /* first param in %eax */
#define FUNC_FASTCALL2 3 /* first parameters in %eax, %edx */
#define FUNC_FASTCALL3 4 /* first parameter in %eax, %edx, %ecx */
#define FUNC_FASTCALLW 5 /* first parameter in %ecx, %edx */

/* field 'Sym.t' for macros */
#define MACRO_OBJ      0 /* object like macro */
#define MACRO_FUNC     1 /* function like macro */

/* field 'Sym.r' for C labels */
#define LABEL_DEFINED  0 /* label is defined */
#define LABEL_FORWARD  1 /* label is forward defined */
#define LABEL_DECLARED 2 /* label is declared but never used */

/* type_decl() types */
#define TYPE_ABSTRACT  1 /* type without variable */
#define TYPE_DIRECT    2 /* type with variable */

/* The current value can be: */
#define VT_VALMASK   0x00ff
#define VT_CONST     0x00f0  /* constant in vc 
                              (must be first non register value) */
#define VT_LLOCAL    0x00f1  /* lvalue, offset on stack */
#define VT_LOCAL     0x00f2  /* offset on stack */
#define VT_CMP       0x00f3  /* the value is stored in processor flags (in vc) */
#define VT_JMP       0x00f4  /* value is the consequence of jmp true (even) */
#define VT_JMPI      0x00f5  /* value is the consequence of jmp false (odd) */
#define VT_LVAL      0x0100  /* var is an lvalue */
#define VT_SYM       0x0200  /* a symbol value is added */
#define VT_MUSTCAST  0x0400  /* value must be casted to be correct (used for
                                char/short stored in integer registers) */
#define VT_MUSTBOUND 0x0800  /* bound checking must be done before
                                dereferencing value */
#define VT_BOUNDED   0x8000  /* value is bounded. The address of the
                                bounding function call point is in vc */
#define VT_LVAL_BYTE     0x1000  /* lvalue is a byte */
#define VT_LVAL_SHORT    0x2000  /* lvalue is a short */
#define VT_LVAL_UNSIGNED 0x4000  /* lvalue is unsigned */
#define VT_LVAL_TYPE     (VT_LVAL_BYTE | VT_LVAL_SHORT | VT_LVAL_UNSIGNED)

/* types */
#define VT_INT        0  /* integer type */
#define VT_BYTE       1  /* signed byte type */
#define VT_SHORT      2  /* short type */
#define VT_VOID       3  /* void type */
#define VT_PTR        4  /* pointer */
#define VT_ENUM       5  /* enum definition */
#define VT_FUNC       6  /* function type */
#define VT_STRUCT     7  /* struct/union definition */
#define VT_FLOAT      8  /* IEEE float */
#define VT_DOUBLE     9  /* IEEE double */
#define VT_LDOUBLE   10  /* IEEE long double */
#define VT_BOOL      11  /* ISOC99 boolean type */
#define VT_LLONG     12  /* 64 bit integer */
#define VT_LONG      13  /* long integer (NEVER USED as type, only
                            during parsing) */
#define VT_BTYPE      0x000f /* mask for basic type */
#define VT_UNSIGNED   0x0010  /* unsigned type */
#define VT_ARRAY      0x0020  /* array type (also has VT_PTR) */
#define VT_BITFIELD   0x0040  /* bitfield modifier */
#define VT_CONSTANT   0x0800  /* const modifier */
#define VT_VOLATILE   0x1000  /* volatile modifier */
#define VT_SIGNED     0x2000  /* signed type */

/* storage */
#define VT_EXTERN  0x00000080  /* extern definition */
#define VT_STATIC  0x00000100  /* static variable */
#define VT_TYPEDEF 0x00000200  /* typedef definition */
#define VT_INLINE  0x00000400  /* inline definition */

#define VT_STRUCT_SHIFT 16   /* shift for bitfield shift values */

/* type mask (except storage) */
#define VT_STORAGE (VT_EXTERN | VT_STATIC | VT_TYPEDEF | VT_INLINE)
#define VT_TYPE    (~(VT_STORAGE))

/* token values */

/* warning: the following compare tokens depend on i386 asm code */
#define TOK_ULT 0x92
#define TOK_UGE 0x93
#define TOK_EQ  0x94
#define TOK_NE  0x95
#define TOK_ULE 0x96
#define TOK_UGT 0x97
#define TOK_Nset 0x98
#define TOK_Nclear 0x99
#define TOK_LT  0x9c
#define TOK_GE  0x9d
#define TOK_LE  0x9e
#define TOK_GT  0x9f

#define TOK_LAND  0xa0
#define TOK_LOR   0xa1

#define TOK_DEC   0xa2
#define TOK_MID   0xa3 /* inc/dec, to void constant */
#define TOK_INC   0xa4
#define TOK_UDIV  0xb0 /* unsigned division */
#define TOK_UMOD  0xb1 /* unsigned modulo */
#define TOK_PDIV  0xb2 /* fast division with undefined rounding for pointers */
#define TOK_CINT   0xb3 /* number in tokc */
#define TOK_CCHAR 0xb4 /* char constant in tokc */
#define TOK_STR   0xb5 /* pointer to string in tokc */
#define TOK_TWOSHARPS 0xb6 /* ## preprocessing token */
#define TOK_LCHAR    0xb7
#define TOK_LSTR     0xb8
#define TOK_CFLOAT   0xb9 /* float constant */
#define TOK_LINENUM  0xba /* line number info */
#define TOK_CDOUBLE  0xc0 /* double constant */
#define TOK_CLDOUBLE 0xc1 /* long double constant */
#define TOK_UMULL    0xc2 /* unsigned 32x32 -> 64 mul */
#define TOK_ADDC1    0xc3 /* add with carry generation */
#define TOK_ADDC2    0xc4 /* add with carry use */
#define TOK_SUBC1    0xc5 /* add with carry generation */
#define TOK_SUBC2    0xc6 /* add with carry use */
#define TOK_CUINT    0xc8 /* unsigned int constant */
#define TOK_CLLONG   0xc9 /* long long constant */
#define TOK_CULLONG  0xca /* unsigned long long constant */
#define TOK_ARROW    0xcb
#define TOK_DOTS     0xcc /* three dots */
#define TOK_SHR      0xcd /* unsigned shift right */
#define TOK_PPNUM    0xce /* preprocessor number */

#define TOK_SHL   0x01 /* shift left */
#define TOK_SAR   0x02 /* signed shift right */
  
/* assignement operators : normal operator or 0x80 */
#define TOK_A_MOD 0xa5
#define TOK_A_AND 0xa6
#define TOK_A_MUL 0xaa
#define TOK_A_ADD 0xab
#define TOK_A_SUB 0xad
#define TOK_A_DIV 0xaf
#define TOK_A_XOR 0xde
#define TOK_A_OR  0xfc
#define TOK_A_SHL 0x81
#define TOK_A_SAR 0x82

#ifndef offsetof
#define offsetof(type, field) ((size_t) &((type *)0)->field)
#endif

#ifndef countof
#define countof(tab) (sizeof(tab) / sizeof((tab)[0]))
#endif

#define TOK_EOF       (-1)  /* end of file */
#define TOK_LINEFEED  10    /* line feed */

/* all identificators and strings have token above that */
#define TOK_IDENT 256

/* only used for i386 asm opcodes definitions */
#define DEF_ASM(x) DEF(TOK_ASM_ ## x, #x)

#define DEF_BWL(x) \
 DEF(TOK_ASM_ ## x ## b, #x "b") \
 DEF(TOK_ASM_ ## x ## w, #x "w") \
 DEF(TOK_ASM_ ## x ## l, #x "l") \
 DEF(TOK_ASM_ ## x, #x)

#define DEF_WL(x) \
 DEF(TOK_ASM_ ## x ## w, #x "w") \
 DEF(TOK_ASM_ ## x ## l, #x "l") \
 DEF(TOK_ASM_ ## x, #x)

#define DEF_FP1(x) \
 DEF(TOK_ASM_ ## f ## x ## s, "f" #x "s") \
 DEF(TOK_ASM_ ## fi ## x ## l, "fi" #x "l") \
 DEF(TOK_ASM_ ## f ## x ## l, "f" #x "l") \
 DEF(TOK_ASM_ ## fi ## x ## s, "fi" #x "s")

#define DEF_FP(x) \
 DEF(TOK_ASM_ ## f ## x, "f" #x ) \
 DEF(TOK_ASM_ ## f ## x ## p, "f" #x "p") \
 DEF_FP1(x)

#define DEF_ASMTEST(x) \
 DEF_ASM(x ## o) \
 DEF_ASM(x ## no) \
 DEF_ASM(x ## b) \
 DEF_ASM(x ## c) \
 DEF_ASM(x ## nae) \
 DEF_ASM(x ## nb) \
 DEF_ASM(x ## nc) \
 DEF_ASM(x ## ae) \
 DEF_ASM(x ## e) \
 DEF_ASM(x ## z) \
 DEF_ASM(x ## ne) \
 DEF_ASM(x ## nz) \
 DEF_ASM(x ## be) \
 DEF_ASM(x ## na) \
 DEF_ASM(x ## nbe) \
 DEF_ASM(x ## a) \
 DEF_ASM(x ## s) \
 DEF_ASM(x ## ns) \
 DEF_ASM(x ## p) \
 DEF_ASM(x ## pe) \
 DEF_ASM(x ## np) \
 DEF_ASM(x ## po) \
 DEF_ASM(x ## l) \
 DEF_ASM(x ## nge) \
 DEF_ASM(x ## nl) \
 DEF_ASM(x ## ge) \
 DEF_ASM(x ## le) \
 DEF_ASM(x ## ng) \
 DEF_ASM(x ## nle) \
 DEF_ASM(x ## g)

#define TOK_ASM_int TOK_INT

enum tcc_token {
    TOK_LAST = TOK_IDENT - 1,
#define DEF(id, str) id,
#include "tcctok.h"
#undef DEF
};

#define TOK_UIDENT TOK_DEFINE

#ifdef _WIN32
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#ifndef __GNUC__
  #define strtold (long double)strtod
  #define strtof (float)strtod
  #define strtoll (long long)strtol
#endif
#elif defined(TCC_UCLIBC) || defined(__FreeBSD__) || defined(__DragonFly__) \
    || defined(__OpenBSD__)
/* currently incorrect */
long double strtold(const char *nptr, char **endptr)
{
    return (long double)strtod(nptr, endptr);
}
float strtof(const char *nptr, char **endptr)
{
    return (float)strtod(nptr, endptr);
}
#else
/* XXX: need to define this to use them in non ISOC99 context */
extern float strtof (const char *__nptr, char **__endptr);
extern long double strtold (const char *__nptr, char **__endptr);
#endif

#ifdef _WIN32
#define IS_PATHSEP(c) (c == '/' || c == '\\')
#define IS_ABSPATH(p) (IS_PATHSEP(p[0]) || (p[0] && p[1] == ':' && IS_PATHSEP(p[2])))
#define PATHCMP stricmp
#else
#define IS_PATHSEP(c) (c == '/')
#define IS_ABSPATH(p) IS_PATHSEP(p[0])
#define PATHCMP strcmp
#endif

#ifdef CONFIG_TCC_BACKTRACE
extern int num_callers;
extern const char **rt_bound_error_msg;
#endif

#include "lib0.h"
#include "libtcc.h"

//================ ��Ƶ��c struct ==========================
// MACRO ����
struct macro_level {
    struct macro_level *prev;
    int *p;
};

//SYMBOL SECTION �Ÿ����q
#define SYM_POOL_NB (8192 / sizeof(Sym))

typedef struct TCCSyms {
    char *str;
    void *ptr;
} TCCSyms;

/* type definition */
typedef struct CType {
    int t;
    struct Sym *ref;
} CType;

/* symbol management */
typedef struct Sym {
    int v;    /* symbol token */
    long r;    /* associated register */
    long c;    /* associated number */
    CType type;    /* associated type */
    struct Sym *next; /* next related symbol */
    struct Sym *prev; /* prev symbol in stack */
    struct Sym *prev_tok; /* previous symbol for this token */
} Sym;

/* token symbol management */
typedef struct TokenSym {
    struct TokenSym *hash_next;
    struct Sym *sym_define; /* direct pointer to define */
    struct Sym *sym_label; /* direct pointer to label */
    struct Sym *sym_struct; /* direct pointer to structure */
    struct Sym *sym_identifier; /* direct pointer to identifier */
    int tok; /* token number */
    int len;
    char str[1];
} TokenSym;

/* constant value */
typedef union CValue {
    long double ld;
    double d;
    float f;
    int i;
    unsigned int ui;
    unsigned int ul; /* address (should be unsigned long on 64 bit cpu) */
    long long ll;
    unsigned long long ull;
    struct CString *cstr;
    void *ptr;
    int tab[1];
} CValue;

/* value on stack */
typedef struct SValue {
    CType type;      /* type */
    unsigned short r;      /* register + flags */
    unsigned short r2;     /* second register, used for 'long long'
                              type. If not used, set to VT_CONST */
    CValue c;              /* constant, if VT_CONST */
    struct Sym *sym;       /* symbol, if (VT_SYM | VT_CONST) */
} SValue;

/* section definition */
/* XXX: use directly ELF structure for parameters ? */
/* special flag to indicate that the section should not be linked to
   the other ones */
#define SHF_PRIVATE 0x80000000

/* special flag, too */
#define SECTION_ABS ((void *)1)

typedef struct Section {
    unsigned long data_offset; /* current data offset */
    unsigned char *data;       /* section data */
    unsigned long data_allocated; /* used for realloc() handling */
    int sh_name;             /* elf section name (only used during output) */
    int sh_num;              /* elf section number */
    int sh_type;             /* elf section type */
    int sh_flags;            /* elf section flags */
    int sh_info;             /* elf section info */
    int sh_addralign;        /* elf section alignment */
    int sh_entsize;          /* elf entry size */
    unsigned long sh_size;   /* section size (only used during output) */
    unsigned long sh_addr;      /* address at which the section is relocated */
    unsigned long sh_offset;    /* file offset */
    int nb_hashed_syms;      /* used to resize the hash table */
    struct Section *link;    /* link to another section */
    struct Section *reloc;   /* corresponding section for relocation, if any */
    struct Section *hash;     /* hash table for symbols */
    struct Section *next;
    char name[1];           /* section name */
} Section;

#define IO_BUF_SIZE 8192

typedef struct BufferedFile {
    uint8_t *buf_ptr;
    uint8_t *buf_end;
    int fd;
    int line_num;    /* current line number - here to simplify code */
    int ifndef_macro;  /* #ifndef macro / #endif search */
    int ifndef_macro_saved; /* saved ifndef_macro */
    int *ifdef_stack_ptr; /* ifdef_stack value at the start of the file */
    char inc_type;          /* type of include */
    char inc_filename[512]; /* filename specified by the user */
    char filename[1024];    /* current filename - here to simplify code */
    unsigned char buffer[IO_BUF_SIZE + 1]; /* extra size for CH_EOB char */
} BufferedFile;

#define CH_EOB   '\\'       /* end of buffer or '\0' char in file */
#define CH_EOF   (-1)   /* end of file */

/* parsing state (used to save parser state to reparse part of the
   source several times) */
typedef struct ParseState {
    int *macro_ptr;
    int line_num;
    int tok;
    CValue tokc;
} ParseState;

/* used to record tokens */
typedef struct TokenString {
    int *str;
    int len;
    int allocated_len;
    int last_line_num;
} TokenString;

/* include file cache, used to find files faster and also to eliminate
   inclusion if the include file is protected by #ifndef ... #endif */
typedef struct CachedInclude {
    int ifndef_macro;
    int hash_next; /* -1 if none */
    char type; /* '"' or '>' to give include type */
    char filename[1]; /* path specified in #include */
} CachedInclude;

#define CACHED_INCLUDES_HASH_SIZE 512

#ifdef CONFIG_TCC_ASM
typedef struct ExprValue {
    uint32_t v;
    Sym *sym;
} ExprValue;

#define MAX_ASM_OPERANDS 30
typedef struct ASMOperand {
    int id; /* GCC 3 optionnal identifier (0 if number only supported */
    char *constraint;
    char asm_str[16]; /* computed asm string for operand */
    SValue *vt; /* C value of the expression */
    int ref_index; /* if >= 0, gives reference to a output constraint */
    int input_index; /* if >= 0, gives reference to an input constraint */
    int priority; /* priority, used to assign registers */
    int reg; /* if >= 0, register number used for this operand */
    int is_llong; /* true if double register value */
    int is_memory; /* true if memory operand */
    int is_rw;     /* for '+' modifier */
} ASMOperand;

#endif

typedef struct DLLReference {
    int level;
    void *handle;
    char name[1];
} DLLReference;

/* GNUC attribute definition */
typedef struct AttributeDef {
    int aligned;
    int packed; 
    Section *section;
    int func_attr; /* calling convention, exports, ... */
} AttributeDef;

/* -------------------------------------------------- */
/* gr: wrappers for casting sym->r for other purposes */
typedef struct {
    unsigned
      func_call : 8,
      func_args : 8,
      func_export : 1;
} func_attr_t;

/********************************************************/
struct TCCState {
    int output_type;
 
    BufferedFile **include_stack_ptr;
    int *ifdef_stack_ptr;

    /* include file handling */
    char **include_paths;
    int nb_include_paths;
    char **sysinclude_paths;
    int nb_sysinclude_paths;
    CachedInclude **cached_includes;
    int nb_cached_includes;

    char **library_paths;
    int nb_library_paths;

    /* array of all loaded dlls (including those referenced by loaded
       dlls) */
    DLLReference **loaded_dlls;
    int nb_loaded_dlls;

    /* sections */
    Section **sections;
    int nb_sections; /* number of sections, including first dummy section */

    Section **priv_sections;
    int nb_priv_sections; /* number of private sections */

    /* got handling */
    Section *got;
    Section *plt;
    unsigned long *got_offsets;
    int nb_got_offsets;
    /* give the correspondance from symtab indexes to dynsym indexes */
    int *symtab_to_dynsym;

    /* temporary dynamic symbol sections (for dll loading) */
    Section *dynsymtab_section;
    /* exported dynamic symbol section */
    Section *dynsym;

    int nostdinc; /* if true, no standard headers are added */
    int nostdlib; /* if true, no standard libraries are added */
    int nocommon; /* if true, do not use common symbols for .bss data */

    /* if true, static linking is performed */
    int static_link;

    /* soname as specified on the command line (-soname) */
    const char *soname;

    /* if true, all symbols are exported */
    int rdynamic;

    /* if true, only link in referenced objects from archive */
    int alacarte_link;

    /* address of text section */
    unsigned long text_addr;
    int has_text_addr;
    
    /* output format, see TCC_OUTPUT_FORMAT_xxx */
    int output_format;

    /* C language options */
    int char_is_unsigned;
    int leading_underscore;
    
    /* warning switches */
    int warn_write_strings;
    int warn_unsupported;
    int warn_error;
    int warn_none;
    int warn_implicit_function_declaration;

    /* display some information during compilation */
    int verbose;
    /* compile with debug symbol (and use them if error during execution) */
    int do_debug;
    /* compile with built-in memory and bounds checker */
    int do_bounds_check;
    /* give the path of the tcc libraries */
    const char *tcc_lib_path;

    /* error handling */
    void *error_opaque;
    void (*error_func)(void *opaque, const char *msg);
    int error_set_jmp_enabled;
    jmp_buf error_jmp_buf;
    int nb_errors;

    /* tiny assembler state */
    Sym *asm_labels;

    /* see include_stack_ptr */
    BufferedFile *include_stack[INCLUDE_STACK_SIZE];

    /* see ifdef_stack_ptr */
    int ifdef_stack[IFDEF_STACK_SIZE];

    /* see cached_includes */
    int cached_includes_hash[CACHED_INCLUDES_HASH_SIZE];

    /* pack stack */
    int pack_stack[PACK_STACK_SIZE];
    int *pack_stack_ptr;

    /* output file for preprocessing */
    FILE *outfile;

    /* for tcc_relocate */
    int runtime_added;

#ifdef TCC_TARGET_X86_64
    /* write PLT and GOT here */
    char *runtime_plt_and_got;
    unsigned int runtime_plt_and_got_offset;
#endif
};

// ================= ��ƭ쫬 function prototype =======
// ================= ������� ======================
#define tcc_free(p) pfree(p)
#define tcc_malloc(size) pmalloc(size)
#define tcc_mallocz(size) pmallocz(size)
#define tcc_realloc(ptr, size) prealloc(ptr, size)
#define tcc_strdup(str) pstrdup(str)

// =========== ��ƫŧi (function prototype) =======
void error_noabort(const char *fmt, ...);
void error(const char *fmt, ...);
void expect(const char *msg);
void warning(const char *fmt, ...);
void error1(TCCState *s1, int is_warning, const char *fmt, va_list ap);

void tcc_set_lib_path_w32(TCCState *s);
int tcc_set_flag(TCCState *s, const char *flag_name, int value);
void tcc_print_stats(TCCState *s, int64_t total_time);
char *normalize_slashes(char *path);
char *tcc_basename(const char *name);
char *tcc_fileextension (const char *name);
/* tccpp.c */
void next(void);
char *get_tok_str(int v, CValue *cv);

/* tccgen.c */
void parse_expr_type(CType *type);
void expr_type(CType *type);
void unary_type(CType *type);
void block(int *bsym, int *csym, int *case_sym, int *def_sym, 
                  int case_reg, int is_expr);
int expr_const(void);
void expr_eq(void);
void gexpr(void);
void gen_inline_functions(void);
void decl(int l);
void decl_initializer(CType *type, Section *sec, unsigned long c, 
                             int first, int size_only);
void decl_initializer_alloc(CType *type, AttributeDef *ad, int r, 
                                   int has_init, int v, int scope);
int gv(int rc);
void gv2(int rc1, int rc2);
void move_reg(int r, int s);
void save_regs(int n);
void save_reg(int r);
void vpop(void);
void vswap(void);
void vdup(void);
int get_reg(int rc);
int get_reg_ex(int rc,int rc2);

void gen_op(int op);
void force_charshort_cast(int t);
void gen_cast(CType *type);
void vstore(void);
void mk_pointer(CType *type);

/* type handling */
int type_size(CType *type, int *a);
inline CType *pointed_type(CType *type);
int pointed_size(CType *type);
int lvalue_type(int t);
int parse_btype(CType *type, AttributeDef *ad);
void type_decl(CType *type, AttributeDef *ad, int *v, int td);
int compare_types(CType *type1, CType *type2, int unqualified);
int is_compatible_types(CType *type1, CType *type2);
int is_compatible_parameter_types(CType *type1, CType *type2);

int ieee_finite(double d);
void vpushi(int v);
void vpushll(long long v);
void vrott(int n);
void vnrott(int n);
void lexpand_nr(void);
void vpush_global_sym(CType *type, int v);
void vset(CType *type, int r, int v);
void type_to_str(char *buf, int buf_size, 
                 CType *type, const char *varstr);

/* section generation */
void section_realloc(Section *sec, unsigned long new_size);
void *section_ptr_add(Section *sec, unsigned long size);
void put_extern_sym(Sym *sym, Section *section, 
                           unsigned long value, unsigned long size);
void greloc(Section *s, Sym *sym, unsigned long addr, int type);
int put_elf_str(Section *s, const char *sym);
int put_elf_sym(Section *s, 
                       unsigned long value, unsigned long size,
                       int info, int other, int shndx, const char *name);
int add_elf_sym(Section *s, unsigned long value, unsigned long size,
                       int info, int other, int sh_num, const char *name);
void put_elf_reloc(Section *symtab, Section *s, unsigned long offset,
                          int type, int symbol);
void put_stabs(const char *str, int type, int other, int desc, 
                      unsigned long value);
void put_stabs_r(const char *str, int type, int other, int desc, 
                        unsigned long value, Section *sec, int sym_index);
void put_stabn(int type, int other, int desc, int value);
void put_stabd(int type, int other, int desc);
int tcc_add_dll(TCCState *s, const char *filename, int flags);

#define AFF_PRINT_ERROR     0x0001 /* print error if file not found */
#define AFF_REFERENCED_DLL  0x0002 /* load a referenced dll from another dll */
#define AFF_PREPROCESS      0x0004 /* preprocess file */
int tcc_add_file_internal(TCCState *s, const char *filename, int flags);

/* tcccoff.c */
int tcc_output_coff(TCCState *s1, FILE *f);

/* tccpe.c */
void *resolve_sym(TCCState *s1, const char *sym, int type);
int pe_load_def_file(struct TCCState *s1, int fd);
int pe_test_res_file(void *v, int size);
int pe_load_res_file(struct TCCState *s1, int fd);
void pe_add_runtime(struct TCCState *s1);
void pe_guess_outfile(char *objfilename, int output_type);
int pe_output_file(struct TCCState *s1, const char *filename);

/* tccasm.c */
#ifdef CONFIG_TCC_ASM
void asm_expr(TCCState *s1, ExprValue *pe);
int asm_int_expr(TCCState *s1);
int find_constraint(ASMOperand *operands, int nb_operands, 
                           const char *name, const char **pp);

int tcc_assemble(TCCState *s1, int do_preprocess);
#endif

void asm_instr(void);
void asm_global_instr(void);

// ============== ���q Section =====================
Section *new_section(TCCState *s1, const char *name, int sh_type, int sh_flags);
void free_section(Section *s);
void section_realloc(Section *sec, unsigned long new_size);
void *section_ptr_add(Section *sec, unsigned long size);
Section *find_section(TCCState *s1, const char *name);
void greloc(Section *s, Sym *sym, unsigned long offset, int type);
// ============== �Ÿ� Symbol =====================
Sym *sym_find(int v);
Sym *sym_push(int v, CType *type, int r, int c);
Sym *get_sym_ref(CType *type, Section *sec,
                        unsigned long offset, unsigned long size);
Sym *external_global_sym(int v, CType *type, int r);
void put_extern_sym2(Sym *sym, Section *section, 
                            unsigned long value, unsigned long size,
                            int can_add_underscore);
void put_extern_sym(Sym *sym, Section *section, 
                           unsigned long value, unsigned long size);
// ============== �sĶ�� Compiler =======================
int tcc_compile(TCCState *s1);
int tcc_compile_string(TCCState *s, const char *str);
// ============== �e�B�z�� Macro (PreProcessor) =========
void tcc_define_symbol(TCCState *s1, const char *sym, const char *value);
void tcc_undefine_symbol(TCCState *s1, const char *sym);
void preprocess_init(TCCState *s1);
void next_nomacro(void);
void next_nomacro_spc(void);
void macro_subst(TokenString *tok_str, Sym **nested_list,
                 const int *macro_str, struct macro_level **can_read_stream);
TokenSym *tok_alloc_new(TokenSym **pts, const char *str, int len);
TokenSym *tok_alloc(const char *str, int len);
char *get_tok_str(int v, CValue *cv);
int tcc_peekc_slow(BufferedFile *bf);
int handle_eob(void);
inline void inp(void);
int handle_stray_noerror(void);
void handle_stray(void);
int handle_stray1(uint8_t *p);
void minp(void);
uint8_t *parse_line_comment(uint8_t *p);
uint8_t *parse_comment(uint8_t *p);
inline void skip_spaces(void);
inline int check_space(int t, int *spc);
uint8_t *parse_pp_string(uint8_t *p, int sep, CString *str);
void preprocess_skip(void);
void save_parse_state(ParseState *s);
void restore_parse_state(ParseState *s);
inline int tok_ext_size(int t);
inline void tok_str_new(TokenString *s);
void tok_str_free(int *str);
int *tok_str_realloc(TokenString *s);
void tok_str_add(TokenString *s, int t);
void tok_str_add2(TokenString *s, int t, CValue *cv);
void tok_str_add_tok(TokenString *s);
inline void define_push(int v, int macro_type, int *str, Sym *first_arg);
void define_undef(Sym *s);
inline Sym *define_find(int v);
void free_defines(Sym *b);
Sym *label_find(int v);
Sym *label_push(Sym **ptop, int v, int flags);
void label_pop(Sym **ptop, Sym *slast);
int expr_preprocess(void);
void tok_print(int *str);
void parse_define(void);
inline int hash_cached_include(int type, const char *filename);
CachedInclude *search_cached_include(TCCState *s1,
                                     int type, const char *filename);
inline void add_cached_include(TCCState *s1, int type, 
                                      const char *filename, int ifndef_macro);
void pragma_parse(TCCState *s1);
void preprocess(int is_bof);
void parse_escape_string(CString *outstr, const uint8_t *buf, int is_long);
void bn_lshift(unsigned int *bn, int shift, int or_val);
void bn_zero(unsigned int *bn);
void parse_number(const char *p);
inline void next_nomacro1(void);
void next_nomacro_spc(void);
void next_nomacro(void);
int *macro_arg_subst(Sym **nested_list, int *macro_str, Sym *args);
int macro_subst_tok(TokenString *tok_str,
                    Sym **nested_list, Sym *s, struct macro_level **can_read_stream);
inline int *macro_twosharps(const int *macro_str);
void macro_subst(TokenString *tok_str, Sym **nested_list, 
                 const int *macro_str, struct macro_level ** can_read_stream);
void next(void);
inline void unget_tok(int last_tok);
void preprocess_init(TCCState *s1);
void preprocess_new();
int tcc_preprocess(TCCState *s1);

// ============== ���� =======================
void skip(int c);
void test_lvalue(void);

/* true if float/double/long double type */
inline int is_float(int t)
{
    int bt;
    bt = t & VT_BTYPE;
    return bt == VT_LDOUBLE || bt == VT_DOUBLE || bt == VT_FLOAT;
}

/* space exlcuding newline */
inline int is_space(int ch)
{
    return ch == ' ' || ch == '\t' || ch == '\v' || ch == '\f' || ch == '\r';
}

// ================= �����ܼ� ==========================
/* global variables */

/* XXX: get rid of this ASAP */
extern struct TCCState *tcc_state;

/* display benchmark infos */
int total_lines;
int total_bytes;

/* parser */
extern struct BufferedFile *file;
extern int ch, tok;
extern CValue tokc;
extern CString tokcstr; /* current parsed string, if any */
/* additional informations about token */
extern int tok_flags;
#define TOK_FLAG_BOL   0x0001 /* beginning of line before */
#define TOK_FLAG_BOF   0x0002 /* beginning of file before */
#define TOK_FLAG_ENDIF 0x0004 /* a endif was found matching starting #ifdef */
#define TOK_FLAG_EOF   0x0008 /* end of file */

extern int *macro_ptr, *macro_ptr_allocated;
extern int *unget_saved_macro_ptr;
extern int unget_saved_buffer[TOK_MAX_SIZE + 1];
extern int unget_buffer_enabled;
extern int parse_flags;
#define PARSE_FLAG_PREPROCESS 0x0001 /* activate preprocessing */
#define PARSE_FLAG_TOK_NUM    0x0002 /* return numbers instead of TOK_PPNUM */
#define PARSE_FLAG_LINEFEED   0x0004 /* line feed is returned as a
                                        token. line feed is also
                                        returned at eof */
#define PARSE_FLAG_ASM_COMMENTS 0x0008 /* '#' can be used for line comment */
#define PARSE_FLAG_SPACES     0x0010 /* next() returns space tokens (for -E) */

/* symbol allocator */
extern Sym *sym_free_first;
extern void **sym_pools;
extern int nb_sym_pools;
 
extern Section *text_section, *data_section, *bss_section; /* predefined sections */
extern Section *cur_text_section; /* current section where function code is
                              generated */
#ifdef CONFIG_TCC_ASM
extern Section *last_text_section; /* to handle .previous asm directive */
#endif
/* bound check related sections */
extern Section *bounds_section; /* contains global data bound description */
extern Section *lbounds_section; /* contains local data bound description */
/* symbol sections */
extern Section *symtab_section, *strtab_section;

/* debug sections */
extern Section *stab_section, *stabstr_section;

/* loc : local variable index
   ind : output code index
   rsym: return symbol
   anon_sym: anonymous symbol index
*/
extern int rsym, anon_sym, ind, loc;
/* expression generation modifiers */
extern int const_wanted; /* true if constant wanted */
extern int nocode_wanted; /* true if no code generation wanted for an expression */
extern int global_expr;  /* true if compound literals must be allocated
                            globally (used during initializers parsing */
extern CType func_vt; /* current function return type (used by return
                         instruction) */
extern int func_vc;
extern int last_line_num, last_ind, func_ind; /* debug last line number and pc */
extern int tok_ident;
extern TokenSym **table_ident;
extern TokenSym *hash_ident[TOK_HASH_SIZE];
extern char token_buf[STRING_MAX_SIZE + 1];
extern char *funcname;
extern Sym *global_stack, *local_stack;
extern Sym *define_stack;
extern Sym *global_label_stack, *local_label_stack;

extern SValue vstack[VSTACK_SIZE], *vtop;
/* some predefined types */
extern CType char_pointer_type, func_old_type, int_type;

/* use GNU C extensions */
extern int gnu_ext;

/* use Tiny C extensions */
extern int tcc_ext;

/* max number of callers shown if error */
#ifdef CONFIG_TCC_BACKTRACE
extern int num_callers = 6;
extern const char **rt_bound_error_msg;
#endif

#include "global.c"

#endif
