#ifndef __ARMv7M_SYSCLK_H__
#define __ARMv7M_SYSCLK_H__

#include <io.h>

/* Systick */
#define SYSTICK_INT		2
#define SYSTICK_FLAG()		(STK_CTRL & 0x10000)
#define SYSTICK_MAX		((1 << 24) - 1) /* 24-bit timer */

#define reset_sysclk()		(STK_VAL = 0)
#define set_sysclk(v)		(STK_LOAD = v)

#define stop_sysclk()		(STK_CTRL &= ~3)
#define run_sysclk()		(STK_CTRL = (STK_CTRL & ~3) | SYSTICK_INT | 1)

#define get_sysclk()		(STK_VAL)
#define get_sysclk_max()	(STK_LOAD + 1)

int sysclk_init();

#undef  INCPATH
#define INCPATH			<asm/mach-MACHINE/clock.h>
#include INCPATH

#endif /* __ARMv7M_SYSCLK_H__ */
