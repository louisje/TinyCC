#include "cc0.h"

#ifdef CONFIG_TCC_ASM

#ifdef TCC_TARGET_I386
#include "i386-asm.c"
#endif
#include "tccasm.c"

#else
void asm_instr(void)
{
    error("inline asm() not supported");
}
void asm_global_instr(void)
{
    error("inline asm() not supported");
}
#endif
