#ifndef GLOBAL_C
#define GLOBAL_C

#include "cc0.h"

/* global variables */

/* XXX: get rid of this ASAP */
struct TCCState *tcc_state;

/* display benchmark infos */
int total_lines;
int total_bytes;

/* parser */
struct BufferedFile *file;
int ch, tok;
CValue tokc;
CString tokcstr; /* current parsed string, if any */
/* additional informations about token */
int tok_flags;
#define TOK_FLAG_BOL   0x0001 /* beginning of line before */
#define TOK_FLAG_BOF   0x0002 /* beginning of file before */
#define TOK_FLAG_ENDIF 0x0004 /* a endif was found matching starting #ifdef */
#define TOK_FLAG_EOF   0x0008 /* end of file */

int *macro_ptr, *macro_ptr_allocated;
int *unget_saved_macro_ptr;
int unget_saved_buffer[TOK_MAX_SIZE + 1];
int unget_buffer_enabled;
int parse_flags;
#define PARSE_FLAG_PREPROCESS 0x0001 /* activate preprocessing */
#define PARSE_FLAG_TOK_NUM    0x0002 /* return numbers instead of TOK_PPNUM */
#define PARSE_FLAG_LINEFEED   0x0004 /* line feed is returned as a
                                        token. line feed is also
                                        returned at eof */
#define PARSE_FLAG_ASM_COMMENTS 0x0008 /* '#' can be used for line comment */
#define PARSE_FLAG_SPACES     0x0010 /* next() returns space tokens (for -E) */

Sym *sym_free_first;
void **sym_pools;
int nb_sym_pools;

Section *text_section, *data_section, *bss_section; /* predefined sections */
Section *cur_text_section; /* current section where function code is
                              generated */
#ifdef CONFIG_TCC_ASM
Section *last_text_section; /* to handle .previous asm directive */
#endif
/* bound check related sections */
Section *bounds_section; /* contains global data bound description */
Section *lbounds_section; /* contains local data bound description */
/* symbol sections */
Section *symtab_section, *strtab_section;

/* debug sections */
Section *stab_section, *stabstr_section;

/* loc : local variable index
   ind : output code index
   rsym: return symbol
   anon_sym: anonymous symbol index
*/
int rsym, anon_sym, ind, loc;
/* expression generation modifiers */
int const_wanted; /* true if constant wanted */
int nocode_wanted; /* true if no code generation wanted for an expression */
int global_expr;  /* true if compound literals must be allocated
                            globally (used during initializers parsing */
CType func_vt; /* current function return type (used by return
                         instruction) */
int func_vc;
int last_line_num, last_ind, func_ind; /* debug last line number and pc */
int tok_ident;
TokenSym **table_ident;
TokenSym *hash_ident[TOK_HASH_SIZE];
char token_buf[STRING_MAX_SIZE + 1];
char *funcname;
Sym *global_stack, *local_stack;
Sym *define_stack;
Sym *global_label_stack, *local_label_stack;

SValue vstack[VSTACK_SIZE], *vtop;
/* some predefined types */
CType char_pointer_type, func_old_type, int_type;

/* use GNU C extensions */
int gnu_ext = 1;

/* use Tiny C extensions */
int tcc_ext = 1;

/* max number of callers shown if error */
#ifdef CONFIG_TCC_BACKTRACE
int num_callers = 6;
const char **rt_bound_error_msg;
#endif

#endif