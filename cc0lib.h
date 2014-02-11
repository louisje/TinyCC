#ifndef CC0LIB_H
#define CC0LIB_H

#include "tcc.h"
#include "symbol.h"

#define tcc_free(p) pfree(p)
#define tcc_malloc(size) pmalloc(size)
#define tcc_mallocz(size) pmallocz(size)
#define tcc_realloc(ptr, size) prealloc(ptr, size)
#define tcc_strdup(str) pstrdup(str)

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

void error_noabort(const char *fmt, ...);
void error(const char *fmt, ...);
void expect(const char *msg);
void warning(const char *fmt, ...);
void error1(TCCState *s1, int is_warning, const char *fmt, va_list ap);

void tcc_set_lib_path_w32(TCCState *s);
int tcc_set_flag(TCCState *s, const char *flag_name, int value);
void tcc_print_stats(TCCState *s, int64_t total_time);

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

#endif
