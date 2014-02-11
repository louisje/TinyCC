#include "cc0.h"

// ========= ccc 加入 ============
#include "global.c"
#include "lib0.c"
#include "sym_section.c"
#include "compile.c"
#include "macro.c"
#include "asm.c"

//==================== 錯誤處理 ===========================
/* error without aborting current compilation */
void error_noabort(const char *fmt, ...)
{
    TCCState *s1 = tcc_state;
    va_list ap;

    va_start(ap, fmt);
    error1(s1, 0, fmt, ap);
    va_end(ap);
}

void error(const char *fmt, ...)
{
    TCCState *s1 = tcc_state;
    va_list ap;

    va_start(ap, fmt);
    error1(s1, 0, fmt, ap);
    va_end(ap);
    /* better than nothing: in some cases, we accept to handle errors */
    if (s1->error_set_jmp_enabled) {
        longjmp(s1->error_jmp_buf, 1);
    } else {
        /* XXX: eliminate this someday */
        exit(1);
    }
}

void expect(const char *msg)
{
    error("%s expected", msg);
}

void warning(const char *fmt, ...)
{
    TCCState *s1 = tcc_state;
    va_list ap;

    if (s1->warn_none)
        return;

    va_start(ap, fmt);
    error1(s1, 1, fmt, ap);
    va_end(ap);
}

void error1(TCCState *s1, int is_warning, const char *fmt, va_list ap)
{
    char buf[2048];
    BufferedFile **f;
    
    buf[0] = '\0';
    if (file) {
        for(f = s1->include_stack; f < s1->include_stack_ptr; f++)
            strcat_printf(buf, sizeof(buf), "In file included from %s:%d:\n", 
                          (*f)->filename, (*f)->line_num);
        if (file->line_num > 0) {
            strcat_printf(buf, sizeof(buf), 
                          "%s:%d: ", file->filename, file->line_num);
        } else {
            strcat_printf(buf, sizeof(buf),
                          "%s: ", file->filename);
        }
    } else {
        strcat_printf(buf, sizeof(buf),
                      "tcc: ");
    }
    if (is_warning)
        strcat_printf(buf, sizeof(buf), "warning: ");
    strcat_vprintf(buf, sizeof(buf), fmt, ap);

    if (!s1->error_func) {
        /* default case: stderr */
        fprintf(stderr, "%s\n", buf);
    } else {
        s1->error_func(s1->error_opaque, buf);
    }
    if (!is_warning || s1->warn_error)
        s1->nb_errors++;
}

/********************************************************/
/* global variables */

#ifdef TCC_TARGET_I386
#include "i386-gen.c"
#endif

#ifdef TCC_TARGET_ARM
#include "arm-gen.c"
#endif

#ifdef TCC_TARGET_C67
#include "c67-gen.c"
#endif

#ifdef TCC_TARGET_X86_64
#include "x86_64-gen.c"
#endif

/********************************************************/

/* we use our own 'finite' function to avoid potential problems with
   non standard math libs */
/* XXX: endianness dependent */
int ieee_finite(double d)
{
    int *p = (int *)&d;
    return ((unsigned)((p[1] | 0x800fffff) + 1)) >> 31;
}

/* extract the basename of a file */
char *tcc_basename(const char *name)
{
    char *p = strchr(name, 0);
    while (p > name && !IS_PATHSEP(p[-1]))
        --p;
    return p;
}

char *tcc_fileextension (const char *name)
{
    char *b = tcc_basename(name);
    char *e = strrchr(b, '.');
    return e ? e : strchr(b, 0);
}

#ifdef _WIN32
char *normalize_slashes(char *path)
{
    char *p;
    for (p = path; *p; ++p)
        if (*p == '\\')
            *p = '/';
    return path;
}

void tcc_set_lib_path_w32(TCCState *s)
{
    /* on win32, we suppose the lib and includes are at the location
       of 'tcc.exe' */
    char path[1024], *p;
    GetModuleFileNameA(NULL, path, sizeof path);
    p = tcc_basename(normalize_slashes(strlwr(path)));
    if (p - 5 > path && 0 == strncmp(p - 5, "/bin/", 5))
        p -= 5;
    else if (p > path)
        p--;
    *p = 0;
    tcc_set_lib_path(s, path);
}
#endif

void set_pages_executable(void *ptr, unsigned long length)
{
#ifdef _WIN32
    unsigned long old_protect;
    VirtualProtect(ptr, length, PAGE_EXECUTE_READWRITE, &old_protect);
#else
    unsigned long start, end;
    start = (unsigned long)ptr & ~(PAGESIZE - 1);
    end = (unsigned long)ptr + length;
    end = (end + PAGESIZE - 1) & ~(PAGESIZE - 1);
    mprotect((void *)start, end - start, PROT_READ | PROT_WRITE | PROT_EXEC);
#endif            
}

void tcc_set_error_func(TCCState *s, void *error_opaque,
                        void (*error_func)(void *opaque, const char *msg))
{
    s->error_opaque = error_opaque;
    s->error_func = error_func;
}

void skip(int c)
{
    if (tok != c)
        error("'%c' expected", c);
    next();
}

void test_lvalue(void)
{
    if (!(vtop->r & VT_LVAL))
        expect("lvalue");
}

/* I/O layer */

BufferedFile *tcc_open(TCCState *s1, const char *filename)
{
    int fd;
    BufferedFile *bf;

    if (strcmp(filename, "-") == 0)
        fd = 0, filename = "stdin";
    else
        fd = open(filename, O_RDONLY | O_BINARY);
    if ((s1->verbose == 2 && fd >= 0) || s1->verbose == 3)
        printf("%s %*s%s\n", fd < 0 ? "nf":"->",
               (s1->include_stack_ptr - s1->include_stack), "", filename);
    if (fd < 0)
        return NULL;
    bf = tcc_malloc(sizeof(BufferedFile));
    bf->fd = fd;
    bf->buf_ptr = bf->buffer;
    bf->buf_end = bf->buffer;
    bf->buffer[0] = CH_EOB; /* put eob symbol */
    pstrcpy(bf->filename, sizeof(bf->filename), filename);
#ifdef _WIN32
    normalize_slashes(bf->filename);
#endif
    bf->line_num = 1;
    bf->ifndef_macro = 0;
    bf->ifdef_stack_ptr = s1->ifdef_stack_ptr;
    //    printf("opening '%s'\n", filename);
    return bf;
}

void tcc_close(BufferedFile *bf)
{
    total_lines += bf->line_num;
    close(bf->fd);
    tcc_free(bf);
}

#include "tccpp.c"
#include "tccgen.c"

// ccc delete

#include "tccelf.c"

#ifdef TCC_TARGET_COFF
#include "tcccoff.c"
#endif

#ifdef TCC_TARGET_PE
#include "tccpe.c"
#endif

#ifdef CONFIG_TCC_BACKTRACE
/* print the position in the source file of PC value 'pc' by reading
   the stabs debug information */
void rt_printline(unsigned long wanted_pc)
{
    Stab_Sym *sym, *sym_end;
    char func_name[128], last_func_name[128];
    unsigned long func_addr, last_pc, pc;
    const char *incl_files[INCLUDE_STACK_SIZE];
    int incl_index, len, last_line_num, i;
    const char *str, *p;

    fprintf(stderr, "0x%08lx:", wanted_pc);

    func_name[0] = '\0';
    func_addr = 0;
    incl_index = 0;
    last_func_name[0] = '\0';
    last_pc = 0xffffffff;
    last_line_num = 1;
    sym = (Stab_Sym *)stab_section->data + 1;
    sym_end = (Stab_Sym *)(stab_section->data + stab_section->data_offset);
    while (sym < sym_end) {
        switch(sym->n_type) {
            /* function start or end */
        case N_FUN:
            if (sym->n_strx == 0) {
                /* we test if between last line and end of function */
                pc = sym->n_value + func_addr;
                if (wanted_pc >= last_pc && wanted_pc < pc)
                    goto found;
                func_name[0] = '\0';
                func_addr = 0;
            } else {
                str = stabstr_section->data + sym->n_strx;
                p = strchr(str, ':');
                if (!p) {
                    pstrcpy(func_name, sizeof(func_name), str);
                } else {
                    len = p - str;
                    if (len > sizeof(func_name) - 1)
                        len = sizeof(func_name) - 1;
                    memcpy(func_name, str, len);
                    func_name[len] = '\0';
                }
                func_addr = sym->n_value;
            }
            break;
            /* line number info */
        case N_SLINE:
            pc = sym->n_value + func_addr;
            if (wanted_pc >= last_pc && wanted_pc < pc)
                goto found;
            last_pc = pc;
            last_line_num = sym->n_desc;
            /* XXX: slow! */
            strcpy(last_func_name, func_name);
            break;
            /* include files */
        case N_BINCL:
            str = stabstr_section->data + sym->n_strx;
        add_incl:
            if (incl_index < INCLUDE_STACK_SIZE) {
                incl_files[incl_index++] = str;
            }
            break;
        case N_EINCL:
            if (incl_index > 1)
                incl_index--;
            break;
        case N_SO:
            if (sym->n_strx == 0) {
                incl_index = 0; /* end of translation unit */
            } else {
                str = stabstr_section->data + sym->n_strx;
                /* do not add path */
                len = strlen(str);
                if (len > 0 && str[len - 1] != '/')
                    goto add_incl;
            }
            break;
        }
        sym++;
    }

    /* second pass: we try symtab symbols (no line number info) */
    incl_index = 0;
    {
        ElfW(Sym) *sym, *sym_end;
        int type;

        sym_end = (ElfW(Sym) *)(symtab_section->data + symtab_section->data_offset);
        for(sym = (ElfW(Sym) *)symtab_section->data + 1; 
            sym < sym_end;
            sym++) {
            type = ELFW(ST_TYPE)(sym->st_info);
            if (type == STT_FUNC) {
                if (wanted_pc >= sym->st_value &&
                    wanted_pc < sym->st_value + sym->st_size) {
                    pstrcpy(last_func_name, sizeof(last_func_name),
                            strtab_section->data + sym->st_name);
                    goto found;
                }
            }
        }
    }
    /* did not find any info: */
    fprintf(stderr, " ???\n");
    return;
 found:
    if (last_func_name[0] != '\0') {
        fprintf(stderr, " %s()", last_func_name);
    }
    if (incl_index > 0) {
        fprintf(stderr, " (%s:%d", 
                incl_files[incl_index - 1], last_line_num);
        for(i = incl_index - 2; i >= 0; i--)
            fprintf(stderr, ", included from %s", incl_files[i]);
        fprintf(stderr, ")");
    }
    fprintf(stderr, "\n");
}

#ifdef __i386__
/* fix for glibc 2.1 */
#ifndef REG_EIP
#define REG_EIP EIP
#define REG_EBP EBP
#endif

/* return the PC at frame level 'level'. Return non zero if not found */
int rt_get_caller_pc(unsigned long *paddr, 
                            ucontext_t *uc, int level)
{
    unsigned long fp;
    int i;

    if (level == 0) {
#if defined(__FreeBSD__)
        *paddr = uc->uc_mcontext.mc_eip;
#elif defined(__dietlibc__)
        *paddr = uc->uc_mcontext.eip;
#else
        *paddr = uc->uc_mcontext.gregs[REG_EIP];
#endif
        return 0;
    } else {
#if defined(__FreeBSD__) 
        fp = uc->uc_mcontext.mc_ebp;
#elif defined(__dietlibc__)
        fp = uc->uc_mcontext.ebp;
#else
        fp = uc->uc_mcontext.gregs[REG_EBP];
#endif
        for(i=1;i<level;i++) {
            /* XXX: check address validity with program info */
            if (fp <= 0x1000 || fp >= 0xc0000000)
                return -1;
            fp = ((unsigned long *)fp)[0];
        }
        *paddr = ((unsigned long *)fp)[1];
        return 0;
    }
}
#elif defined(__x86_64__)
/* return the PC at frame level 'level'. Return non zero if not found */
int rt_get_caller_pc(unsigned long *paddr,
                            ucontext_t *uc, int level)
{
    unsigned long fp;
    int i;

    if (level == 0) {
        /* XXX: only support linux */
        *paddr = uc->uc_mcontext.gregs[REG_RIP];
        return 0;
    } else {
        fp = uc->uc_mcontext.gregs[REG_RBP];
        for(i=1;i<level;i++) {
            /* XXX: check address validity with program info */
            if (fp <= 0x1000)
                return -1;
            fp = ((unsigned long *)fp)[0];
        }
        *paddr = ((unsigned long *)fp)[1];
        return 0;
    }
}
#else
#warning add arch specific rt_get_caller_pc()
int rt_get_caller_pc(unsigned long *paddr,
                            ucontext_t *uc, int level)
{
    return -1;
}
#endif

/* emit a run time error at position 'pc' */
void rt_error(ucontext_t *uc, const char *fmt, ...)
{
    va_list ap;
    unsigned long pc;
    int i;

    va_start(ap, fmt);
    fprintf(stderr, "Runtime error: ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    for(i=0;i<num_callers;i++) {
        if (rt_get_caller_pc(&pc, uc, i) < 0)
            break;
        if (i == 0)
            fprintf(stderr, "at ");
        else
            fprintf(stderr, "by ");
        rt_printline(pc);
    }
    exit(255);
    va_end(ap);
}

/* signal handler for fatal errors */
void sig_error(int signum, siginfo_t *siginf, void *puc)
{
    ucontext_t *uc = puc;

    switch(signum) {
    case SIGFPE:
        switch(siginf->si_code) {
        case FPE_INTDIV:
        case FPE_FLTDIV:
            rt_error(uc, "division by zero");
            break;
        default:
            rt_error(uc, "floating point exception");
            break;
        }
        break;
    case SIGBUS:
    case SIGSEGV:
        if (rt_bound_error_msg && *rt_bound_error_msg)
            rt_error(uc, *rt_bound_error_msg);
        else
            rt_error(uc, "dereferencing invalid pointer");
        break;
    case SIGILL:
        rt_error(uc, "illegal instruction");
        break;
    case SIGABRT:
        rt_error(uc, "abort() called");
        break;
    default:
        rt_error(uc, "caught signal %d", signum);
        break;
    }
    exit(255);
}

#endif

/* copy code into memory passed in by the caller and do all relocations
   (needed before using tcc_get_symbol()).
   returns -1 on error and required size if ptr is NULL */
int tcc_relocate(TCCState *s1, void *ptr)
{
    Section *s;
    unsigned long offset, length, mem;
    int i;

    if (0 == s1->runtime_added) {
        s1->runtime_added = 1;
        s1->nb_errors = 0;
#ifdef TCC_TARGET_PE
        pe_add_runtime(s1);
        relocate_common_syms();
        tcc_add_linker_symbols(s1);
#else
        tcc_add_runtime(s1);
        relocate_common_syms();
        tcc_add_linker_symbols(s1);
        build_got_entries(s1);
#endif
    }

    offset = 0, mem = (unsigned long)ptr;
    for(i = 1; i < s1->nb_sections; i++) {
        s = s1->sections[i];
        if (0 == (s->sh_flags & SHF_ALLOC))
            continue;
        length = s->data_offset;
        s->sh_addr = mem ? (mem + offset + 15) & ~15 : 0;
        offset = (offset + length + 15) & ~15;
    }

    /* relocate symbols */
    relocate_syms(s1, 1);
    if (s1->nb_errors)
        return -1;

#ifdef TCC_TARGET_X86_64
    s1->runtime_plt_and_got_offset = 0;
    s1->runtime_plt_and_got = (char *)(mem + offset);
    /* double the size of the buffer for got and plt entries
       XXX: calculate exact size for them? */
    offset *= 2;
#endif

    if (0 == mem)
        return offset + 15;

    /* relocate each section */
    for(i = 1; i < s1->nb_sections; i++) {
        s = s1->sections[i];
        if (s->reloc)
            relocate_section(s1, s);
    }

    for(i = 1; i < s1->nb_sections; i++) {
        s = s1->sections[i];
        if (0 == (s->sh_flags & SHF_ALLOC))
            continue;
        length = s->data_offset;
        // printf("%-12s %08x %04x\n", s->name, s->sh_addr, length);
        ptr = (void*)s->sh_addr;
        if (NULL == s->data || s->sh_type == SHT_NOBITS)
            memset(ptr, 0, length);
        else
            memcpy(ptr, s->data, length);
        /* mark executable sections as executable in memory */
        if (s->sh_flags & SHF_EXECINSTR)
            set_pages_executable(ptr, length);
    }
#ifdef TCC_TARGET_X86_64
    set_pages_executable(s1->runtime_plt_and_got,
                         s1->runtime_plt_and_got_offset);
#endif
    return 0;
}

/* launch the compiled program with the given arguments */
int tcc_run(TCCState *s1, int argc, char **argv)
{
    int (*prog_main)(int, char **);
    void *ptr;
    int ret;

    ret = tcc_relocate(s1, NULL);
    if (ret < 0)
        return -1;
    ptr = tcc_malloc(ret);
    tcc_relocate(s1, ptr);

    prog_main = tcc_get_symbol_err(s1, "main");
    
    if (s1->do_debug) {
#ifdef CONFIG_TCC_BACKTRACE
        struct sigaction sigact;
        /* install TCC signal handlers to print debug info on fatal
           runtime errors */
        sigact.sa_flags = SA_SIGINFO | SA_RESETHAND;
        sigact.sa_sigaction = sig_error;
        sigemptyset(&sigact.sa_mask);
        sigaction(SIGFPE, &sigact, NULL);
        sigaction(SIGILL, &sigact, NULL);
        sigaction(SIGSEGV, &sigact, NULL);
        sigaction(SIGBUS, &sigact, NULL);
        sigaction(SIGABRT, &sigact, NULL);
#else        
        error("debug mode not available");
#endif
    }

#ifdef CONFIG_TCC_BCHECK
    if (s1->do_bounds_check) {
        void (*bound_init)(void);

        /* set error function */
        rt_bound_error_msg = tcc_get_symbol_err(s1, "__bound_error_msg");

        /* XXX: use .init section so that it also work in binary ? */
        bound_init = (void *)tcc_get_symbol_err(s1, "__bound_init");
        bound_init();
    }
#endif
    ret = (*prog_main)(argc, argv);
    tcc_free(ptr);
    return ret;
}

void tcc_cleanup(void)
{
    int i, n;

    if (NULL == tcc_state)
        return;
    tcc_state = NULL;

    /* free -D defines */
    free_defines(NULL);

    /* free tokens */
    n = tok_ident - TOK_IDENT;
    for(i = 0; i < n; i++)
        tcc_free(table_ident[i]);
    tcc_free(table_ident);

    /* free sym_pools */
    dynarray_reset(&sym_pools, &nb_sym_pools);
    /* string buffer */
    cstr_free(&tokcstr);
    /* reset symbol stack */
    sym_free_first = NULL;
    /* cleanup from error/setjmp */
    macro_ptr = NULL;
}

TCCState *tcc_new(void)
{
    TCCState *s;

    tcc_cleanup();

    s = tcc_mallocz(sizeof(TCCState));
    if (!s)
        return NULL;
    tcc_state = s;
    s->output_type = TCC_OUTPUT_MEMORY;
    s->tcc_lib_path = CONFIG_TCCDIR;

    preprocess_new();

    /* we add dummy defines for some special macros to speed up tests
       and to have working defined() */
    define_push(TOK___LINE__, MACRO_OBJ, NULL, NULL);
    define_push(TOK___FILE__, MACRO_OBJ, NULL, NULL);
    define_push(TOK___DATE__, MACRO_OBJ, NULL, NULL);
    define_push(TOK___TIME__, MACRO_OBJ, NULL, NULL);

    /* standard defines */
    tcc_define_symbol(s, "__STDC__", NULL);
    tcc_define_symbol(s, "__STDC_VERSION__", "199901L");
#if defined(TCC_TARGET_I386)
    tcc_define_symbol(s, "__i386__", NULL);
#endif
#if defined(TCC_TARGET_X86_64)
    tcc_define_symbol(s, "__x86_64__", NULL);
#endif
#if defined(TCC_TARGET_ARM)
    tcc_define_symbol(s, "__ARM_ARCH_4__", NULL);
    tcc_define_symbol(s, "__arm_elf__", NULL);
    tcc_define_symbol(s, "__arm_elf", NULL);
    tcc_define_symbol(s, "arm_elf", NULL);
    tcc_define_symbol(s, "__arm__", NULL);
    tcc_define_symbol(s, "__arm", NULL);
    tcc_define_symbol(s, "arm", NULL);
    tcc_define_symbol(s, "__APCS_32__", NULL);
#endif
#ifdef TCC_TARGET_PE
    tcc_define_symbol(s, "_WIN32", NULL);
#else
    tcc_define_symbol(s, "__unix__", NULL);
    tcc_define_symbol(s, "__unix", NULL);
#if defined(__linux)
    tcc_define_symbol(s, "__linux__", NULL);
    tcc_define_symbol(s, "__linux", NULL);
#endif
#endif
    /* tiny C specific defines */
    tcc_define_symbol(s, "__TINYC__", NULL);

    /* tiny C & gcc defines */
    tcc_define_symbol(s, "__SIZE_TYPE__", "unsigned int");
    tcc_define_symbol(s, "__PTRDIFF_TYPE__", "int");
#ifdef TCC_TARGET_PE
    tcc_define_symbol(s, "__WCHAR_TYPE__", "unsigned short");
#else
    tcc_define_symbol(s, "__WCHAR_TYPE__", "int");
#endif
    
#ifndef TCC_TARGET_PE
    /* default library paths */
    tcc_add_library_path(s, CONFIG_SYSROOT "/usr/local/lib");
    tcc_add_library_path(s, CONFIG_SYSROOT "/usr/lib");
    tcc_add_library_path(s, CONFIG_SYSROOT "/lib");
#endif

    /* no section zero */
    dynarray_add((void ***)&s->sections, &s->nb_sections, NULL);

    /* create standard sections */
    text_section = new_section(s, ".text", SHT_PROGBITS, SHF_ALLOC | SHF_EXECINSTR);
    data_section = new_section(s, ".data", SHT_PROGBITS, SHF_ALLOC | SHF_WRITE);
    bss_section = new_section(s, ".bss", SHT_NOBITS, SHF_ALLOC | SHF_WRITE);

    /* symbols are always generated for linking stage */
    symtab_section = new_symtab(s, ".symtab", SHT_SYMTAB, 0,
                                ".strtab",
                                ".hashtab", SHF_PRIVATE); 
    strtab_section = symtab_section->link;
    
    /* private symbol table for dynamic symbols */
    s->dynsymtab_section = new_symtab(s, ".dynsymtab", SHT_SYMTAB, SHF_PRIVATE,
                                      ".dynstrtab", 
                                      ".dynhashtab", SHF_PRIVATE);
    s->alacarte_link = 1;

#ifdef CHAR_IS_UNSIGNED
    s->char_is_unsigned = 1;
#endif
#if defined(TCC_TARGET_PE) && 0
    /* XXX: currently the PE linker is not ready to support that */
    s->leading_underscore = 1;
#endif
    return s;
}

void tcc_delete(TCCState *s1)
{
    int i;

    tcc_cleanup();

    /* free all sections */
    for(i = 1; i < s1->nb_sections; i++)
        free_section(s1->sections[i]);
    dynarray_reset(&s1->sections, &s1->nb_sections);

    for(i = 0; i < s1->nb_priv_sections; i++)
        free_section(s1->priv_sections[i]);
    dynarray_reset(&s1->priv_sections, &s1->nb_priv_sections);
        
    /* free any loaded DLLs */
    for ( i = 0; i < s1->nb_loaded_dlls; i++) {
        DLLReference *ref = s1->loaded_dlls[i];
        if ( ref->handle )
            dlclose(ref->handle);
    }
    
    /* free loaded dlls array */
    dynarray_reset(&s1->loaded_dlls, &s1->nb_loaded_dlls);

    /* free library paths */
    dynarray_reset(&s1->library_paths, &s1->nb_library_paths);

    /* free include paths */
    dynarray_reset(&s1->cached_includes, &s1->nb_cached_includes);
    dynarray_reset(&s1->include_paths, &s1->nb_include_paths);
    dynarray_reset(&s1->sysinclude_paths, &s1->nb_sysinclude_paths);

    tcc_free(s1);
}

int tcc_add_include_path(TCCState *s1, const char *pathname)
{
    char *pathname1;
    
    pathname1 = tcc_strdup(pathname);
    dynarray_add((void ***)&s1->include_paths, &s1->nb_include_paths, pathname1);
    return 0;
}

int tcc_add_sysinclude_path(TCCState *s1, const char *pathname)
{
    char *pathname1;
    
    pathname1 = tcc_strdup(pathname);
    dynarray_add((void ***)&s1->sysinclude_paths, &s1->nb_sysinclude_paths, pathname1);
    return 0;
}

int tcc_add_file_internal(TCCState *s1, const char *filename, int flags)
{
    const char *ext;
    ElfW(Ehdr) ehdr;
    int fd, ret;
    BufferedFile *saved_file;

    /* find source file type with extension */
    ext = tcc_fileextension(filename);
    if (ext[0])
        ext++;

    /* open the file */
    saved_file = file;
    file = tcc_open(s1, filename);
    if (!file) {
        if (flags & AFF_PRINT_ERROR) {
            error_noabort("file '%s' not found", filename);
        }
        ret = -1;
        goto fail1;
    }

    if (flags & AFF_PREPROCESS) {
        ret = tcc_preprocess(s1);
    } else if (!ext[0] || !PATHCMP(ext, "c")) {
        /* C file assumed */
        ret = tcc_compile(s1);
    } else 
#ifdef CONFIG_TCC_ASM
    if (!strcmp(ext, "S")) {
        /* preprocessed assembler */
        ret = tcc_assemble(s1, 1);
    } else if (!strcmp(ext, "s")) {
        /* non preprocessed assembler */
        ret = tcc_assemble(s1, 0);
    } else 
#endif
#ifdef TCC_TARGET_PE
    if (!PATHCMP(ext, "def")) {
        ret = pe_load_def_file(s1, file->fd);
    } else
#endif
    {
        fd = file->fd;
        /* assume executable format: auto guess file type */
        ret = read(fd, &ehdr, sizeof(ehdr));
        lseek(fd, 0, SEEK_SET);
        if (ret <= 0) {
            error_noabort("could not read header");
            goto fail;
        } else if (ret != sizeof(ehdr)) {
            goto try_load_script;
        }

        if (ehdr.e_ident[0] == ELFMAG0 &&
            ehdr.e_ident[1] == ELFMAG1 &&
            ehdr.e_ident[2] == ELFMAG2 &&
            ehdr.e_ident[3] == ELFMAG3) {
            file->line_num = 0; /* do not display line number if error */
            if (ehdr.e_type == ET_REL) {
                ret = tcc_load_object_file(s1, fd, 0);
            } else if (ehdr.e_type == ET_DYN) {
                if (s1->output_type == TCC_OUTPUT_MEMORY) {
#ifdef TCC_TARGET_PE
                    ret = -1;
#else
                    void *h;
                    h = dlopen(filename, RTLD_GLOBAL | RTLD_LAZY);
                    if (h)
                        ret = 0;
                    else
                        ret = -1;
#endif
                } else {
                    ret = tcc_load_dll(s1, fd, filename, 
                                       (flags & AFF_REFERENCED_DLL) != 0);
                }
            } else {
                error_noabort("unrecognized ELF file");
                goto fail;
            }
        } else if (memcmp((char *)&ehdr, ARMAG, 8) == 0) {
            file->line_num = 0; /* do not display line number if error */
            ret = tcc_load_archive(s1, fd);
        } else 
#ifdef TCC_TARGET_COFF
        if (*(uint16_t *)(&ehdr) == COFF_C67_MAGIC) {
            ret = tcc_load_coff(s1, fd);
        } else
#endif
#ifdef TCC_TARGET_PE
        if (pe_test_res_file(&ehdr, ret)) {
            ret = pe_load_res_file(s1, fd);
        } else
#endif
        {
            /* as GNU ld, consider it is an ld script if not recognized */
        try_load_script:
            ret = tcc_load_ldscript(s1);
            if (ret < 0) {
                error_noabort("unrecognized file type");
                goto fail;
            }
        }
    }
 the_end:
    tcc_close(file);
 fail1:
    file = saved_file;
    return ret;
 fail:
    ret = -1;
    goto the_end;
}

int tcc_add_file(TCCState *s, const char *filename)
{
    if (s->output_type == TCC_OUTPUT_PREPROCESS)
        return tcc_add_file_internal(s, filename, AFF_PRINT_ERROR | AFF_PREPROCESS);
    else
        return tcc_add_file_internal(s, filename, AFF_PRINT_ERROR);
}

int tcc_add_library_path(TCCState *s, const char *pathname)
{
    char *pathname1;
    
    pathname1 = tcc_strdup(pathname);
    dynarray_add((void ***)&s->library_paths, &s->nb_library_paths, pathname1);
    return 0;
}

/* find and load a dll. Return non zero if not found */
/* XXX: add '-rpath' option support ? */
int tcc_add_dll(TCCState *s, const char *filename, int flags)
{
    char buf[1024];
    int i;

    for(i = 0; i < s->nb_library_paths; i++) {
        snprintf(buf, sizeof(buf), "%s/%s", 
                 s->library_paths[i], filename);
        if (tcc_add_file_internal(s, buf, flags) == 0)
            return 0;
    }
    return -1;
}

/* the library name is the same as the argument of the '-l' option */
int tcc_add_library(TCCState *s, const char *libraryname)
{
    char buf[1024];
    int i;
    
    /* first we look for the dynamic library if not static linking */
    if (!s->static_link) {
#ifdef TCC_TARGET_PE
        snprintf(buf, sizeof(buf), "%s.def", libraryname);
#else
        snprintf(buf, sizeof(buf), "lib%s.so", libraryname);
#endif
        if (tcc_add_dll(s, buf, 0) == 0)
            return 0;
    }

    /* then we look for the static library */
    for(i = 0; i < s->nb_library_paths; i++) {
        snprintf(buf, sizeof(buf), "%s/lib%s.a", 
                 s->library_paths[i], libraryname);
        if (tcc_add_file_internal(s, buf, 0) == 0)
            return 0;
    }
    return -1;
}

int tcc_add_symbol(TCCState *s, const char *name, void *val)
{
    add_elf_sym(symtab_section, (unsigned long)val, 0, 
                ELFW(ST_INFO)(STB_GLOBAL, STT_NOTYPE), 0,
                SHN_ABS, name);
    return 0;
}

int tcc_set_output_type(TCCState *s, int output_type)
{
    char buf[1024];

    s->output_type = output_type;

    if (!s->nostdinc) {
        /* default include paths */
        /* XXX: reverse order needed if -isystem support */
#ifndef TCC_TARGET_PE
        tcc_add_sysinclude_path(s, CONFIG_SYSROOT "/usr/local/include");
        tcc_add_sysinclude_path(s, CONFIG_SYSROOT "/usr/include");
#endif
        snprintf(buf, sizeof(buf), "%s/include", s->tcc_lib_path);
        tcc_add_sysinclude_path(s, buf);
#ifdef TCC_TARGET_PE
        snprintf(buf, sizeof(buf), "%s/include/winapi", s->tcc_lib_path);
        tcc_add_sysinclude_path(s, buf);
#endif
    }

    /* if bound checking, then add corresponding sections */
#ifdef CONFIG_TCC_BCHECK
    if (s->do_bounds_check) {
        /* define symbol */
        tcc_define_symbol(s, "__BOUNDS_CHECKING_ON", NULL);
        /* create bounds sections */
        bounds_section = new_section(s, ".bounds", 
                                     SHT_PROGBITS, SHF_ALLOC);
        lbounds_section = new_section(s, ".lbounds", 
                                      SHT_PROGBITS, SHF_ALLOC);
    }
#endif

    if (s->char_is_unsigned) {
        tcc_define_symbol(s, "__CHAR_UNSIGNED__", NULL);
    }

    /* add debug sections */
    if (s->do_debug) {
        /* stab symbols */
        stab_section = new_section(s, ".stab", SHT_PROGBITS, 0);
        stab_section->sh_entsize = sizeof(Stab_Sym);
        stabstr_section = new_section(s, ".stabstr", SHT_STRTAB, 0);
        put_elf_str(stabstr_section, "");
        stab_section->link = stabstr_section;
        /* put first entry */
        put_stabs("", 0, 0, 0, 0);
    }

    /* add libc crt1/crti objects */
#ifndef TCC_TARGET_PE
    if ((output_type == TCC_OUTPUT_EXE || output_type == TCC_OUTPUT_DLL) &&
        !s->nostdlib) {
        if (output_type != TCC_OUTPUT_DLL)
            tcc_add_file(s, CONFIG_TCC_CRT_PREFIX "/crt1.o");
        tcc_add_file(s, CONFIG_TCC_CRT_PREFIX "/crti.o");
    }
#endif

#ifdef TCC_TARGET_PE
    snprintf(buf, sizeof(buf), "%s/lib", s->tcc_lib_path);
    tcc_add_library_path(s, buf);
#endif

    return 0;
}

#define WD_ALL    0x0001 /* warning is activated when using -Wall */
#define FD_INVERT 0x0002 /* invert value before storing */

typedef struct FlagDef {
    uint16_t offset;
    uint16_t flags;
    const char *name;
} FlagDef;

const FlagDef warning_defs[] = {
    { offsetof(TCCState, warn_unsupported), 0, "unsupported" },
    { offsetof(TCCState, warn_write_strings), 0, "write-strings" },
    { offsetof(TCCState, warn_error), 0, "error" },
    { offsetof(TCCState, warn_implicit_function_declaration), WD_ALL,
      "implicit-function-declaration" },
};

int set_flag(TCCState *s, const FlagDef *flags, int nb_flags,
                    const char *name, int value)
{
    int i;
    const FlagDef *p;
    const char *r;

    r = name;
    if (r[0] == 'n' && r[1] == 'o' && r[2] == '-') {
        r += 3;
        value = !value;
    }
    for(i = 0, p = flags; i < nb_flags; i++, p++) {
        if (!strcmp(r, p->name))
            goto found;
    }
    return -1;
 found:
    if (p->flags & FD_INVERT)
        value = !value;
    *(int *)((uint8_t *)s + p->offset) = value;
    return 0;
}


/* set/reset a warning */
int tcc_set_warning(TCCState *s, const char *warning_name, int value)
{
    int i;
    const FlagDef *p;

    if (!strcmp(warning_name, "all")) {
        for(i = 0, p = warning_defs; i < countof(warning_defs); i++, p++) {
            if (p->flags & WD_ALL)
                *(int *)((uint8_t *)s + p->offset) = 1;
        }
        return 0;
    } else {
        return set_flag(s, warning_defs, countof(warning_defs),
                        warning_name, value);
    }
}

const FlagDef flag_defs[] = {
    { offsetof(TCCState, char_is_unsigned), 0, "unsigned-char" },
    { offsetof(TCCState, char_is_unsigned), FD_INVERT, "signed-char" },
    { offsetof(TCCState, nocommon), FD_INVERT, "common" },
    { offsetof(TCCState, leading_underscore), 0, "leading-underscore" },
};

/* set/reset a flag */
int tcc_set_flag(TCCState *s, const char *flag_name, int value)
{
    return set_flag(s, flag_defs, countof(flag_defs),
                    flag_name, value);
}

/* set CONFIG_TCCDIR at runtime */
void tcc_set_lib_path(TCCState *s, const char *path)
{
    s->tcc_lib_path = tcc_strdup(path);
}

void tcc_print_stats(TCCState *s, int64_t total_time)
{
    double tt;
    tt = (double)total_time / 1000000.0;
    if (tt < 0.001)
        tt = 0.001;
    if (total_bytes < 1)
        total_bytes = 1;
    printf("%d idents, %d lines, %d bytes, %0.3f s, %d lines/s, %0.1f MB/s\n", 
           tok_ident - TOK_IDENT, total_lines, total_bytes,
           tt, (int)(total_lines / tt),
           total_bytes / tt / 1000000.0);
}
