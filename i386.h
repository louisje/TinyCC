#ifndef I386_H
#define I386_H

/* number of available registers */
#define NB_REGS             4

/* a register can belong to several classes. The classes must be
   sorted from more general to more precise (see gv2() code which does
   assumptions on it). */
#define RC_INT     0x0001 /* generic integer register */
#define RC_FLOAT   0x0002 /* generic float register */
#define RC_EAX     0x0004
#define RC_ST0     0x0008 
#define RC_ECX     0x0010
#define RC_EDX     0x0020
#define RC_IRET    RC_EAX /* function return: integer register */
#define RC_LRET    RC_EDX /* function return: second integer register */
#define RC_FRET    RC_ST0 /* function return: float register */

/* pretty names for the registers */
enum {
    TREG_EAX = 0,
    TREG_ECX,
    TREG_EDX,
    TREG_ST0,
};

int reg_classes[NB_REGS] = {
    /* eax */ RC_INT | RC_EAX,
    /* ecx */ RC_INT | RC_ECX,
    /* edx */ RC_INT | RC_EDX,
    /* st0 */ RC_FLOAT | RC_ST0,
};

/* return registers for function */
#define REG_IRET TREG_EAX /* single word int return register */
#define REG_LRET TREG_EDX /* second word return register (for long long) */
#define REG_FRET TREG_ST0 /* float return register */

/* defined if function parameters must be evaluated in reverse order */
#define INVERT_FUNC_PARAMS

/* defined if structures are passed as pointers. Otherwise structures
   are directly pushed on stack. */
//#define FUNC_STRUCT_PARAM_AS_PTR

/* pointer size, in bytes */
#define PTR_SIZE 4

/* long double size and alignment, in bytes */
#define LDOUBLE_SIZE  12
#define LDOUBLE_ALIGN 4
/* maximum alignment (for aligned attribute support) */
#define MAX_ALIGN     8

/******************************************************/
/* ELF defines */

#define EM_TCC_TARGET EM_386

/* relocation type for 32 bit data relocation */
#define R_DATA_32   R_386_32
#define R_JMP_SLOT  R_386_JMP_SLOT
#define R_COPY      R_386_COPY

#define ELF_START_ADDR 0x08048000
#define ELF_PAGE_SIZE  0x1000

void g(int c);
void o(unsigned int c);
void gen_le32(int c);
void gsym_addr(int t, int a);
void gsym(int t);
int oad(int c, int s);
void gen_addr32(int r, Sym *sym, int c);
void gen_modrm(int op_reg, int r, Sym *sym, int c);
void load(int r, SValue *sv);
void store(int r, SValue *v);
void gadd_sp(int val);
void gcall_or_jmp(int is_jmp);
void gfunc_call(int nb_args);
void gfunc_prolog(CType *func_type);
void gfunc_epilog(void);
int gjmp(int t);
void gjmp_addr(int a);
int gtst(int inv, int t);
void gen_opi(int op);
void gen_opf(int op);
void gen_cvt_itof(int t);
void gen_cvt_ftoi(int t);
void gen_cvt_ftof(int t);
void ggoto(void);
void gen_bounded_ptr_add(void);
void gen_bounded_ptr_deref(void);

#endif