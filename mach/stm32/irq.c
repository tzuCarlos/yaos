#include <foundation.h>
#include <kernel/task.h>
#include <kernel/sched.h>
#include <asm/context.h>

#ifdef CONFIG_DEBUG
int sched_overhead;
#endif

/* A register that is not yet saved in stack gets used by compiler optimization.
 * If I put all the registers that are not yet saved in clobber list,
 * it changes code ordering that ruins stack, showing weird behavior.
 * Make it in an assembly file or do some study. */
void __attribute__((naked, used, optimize("O0"))) pendsv_handler()
{
#ifdef CONFIG_DEBUG
	sched_overhead = get_systick();
#endif
	/* schedule_prepare() saves the current context and
	 * guarantees not to be preempted while schedule_finish()
	 * does the opposite. */
	schedule_prepare();
	update_curr();
	schedule_core();
	schedule_finish();
#ifdef CONFIG_DEBUG
	sched_overhead -= get_systick();
#endif
	__asm__ __volatile__("bx lr");
}

void __attribute__((naked)) sys_schedule()
{
	SCB_ICSR |= 1 << 28; /* raising pendsv for scheduling */
	__asm__ __volatile__("bx lr");
}

#ifdef CONFIG_SYSCALL
#include <kernel/syscall.h>

#ifdef CONFIG_DEBUG
unsigned syscall_count = 0;
#endif

void __attribute__((naked)) svc_handler()
{
#ifdef CONFIG_DEBUG
	__asm__ __volatile__("add %0, %1, #1"
			: "=r"(syscall_count)
			: "0"(syscall_count)
			: "r0", "memory");
#endif
	__asm__ __volatile__(
			"mrs	r12, psp		\n\t"
			/* r0 must be the same value to the stacked one
			 * because of hardware mechanism. The meantime of
			 * entering into interrupt service routine
			 * nothing can change the value. But the problem
			 * is that it sometimes gets corrupted. how? why?
			 * So here I get r0 from stack. */
			"ldr	r0, [r12]		\n\t"
			/* if nr >= SYSCALL_NR */
			"cmp	r0, %0			\n\t"
			"it	ge			\n\t"
			/* then nr = 0 */
			"movge	r0, #0			\n\t"
			/* get the syscall address */
			"ldr	r3, =syscall_table	\n\t"
			"ldr	r3, [r3, r0, lsl #2]	\n\t"
			/* arguments in place */
			"ldr	r2, [r12, #12]		\n\t"
			"ldr	r1, [r12, #8]		\n\t"
			"ldr	r0, [r12, #4]		\n\t"
			"push	{lr}			\n\t"
			"push	{r12}			\n\t"
			"blx	r3			\n\t"
			"pop	{r12}			\n\t"
			"pop	{lr}			\n\t"
			/* store return value */
			"str	r0, [r12]		\n\t"
			"bx	lr			\n\t"
			:: "I"(SYSCALL_NR));
}
#endif /* CONFIG_SYSCALL */

void __attribute__((naked)) isr_default()
{
	unsigned sp, lr, psr, usp;

	sp  = GET_SP ();
	psr = GET_PSR();
	lr  = GET_LR ();
	usp = GET_USP();

	printk("\nKernel SP      0x%08x\n"
		"Stacked PSR    0x%08x\n"
		"Stacked PC     0x%08x\n"
		"Stacked LR     0x%08x\n"
		"Current LR     0x%08x\n"
		"Current PSR    0x%08x(vector number:%d)\n", sp,
		*(unsigned *)(sp + 28),
		*(unsigned *)(sp + 24),
		*(unsigned *)(sp + 20),
		lr, psr, psr & 0x1ff);
	printk("\nUser SP        0x%08x\n"
		"Stacked PSR    0x%08x\n"
		"Stacked PC     0x%08x\n"
		"Stacked LR     0x%08x\n",
		usp,
		*(unsigned *)(usp + 28),
		*(unsigned *)(usp + 24),
		*(unsigned *)(usp + 20));

	printk("\ncurrent->sp         0x%08x\n"
		"current->base       0x%08x\n"
		"current->heap       0x%08x\n"
		"current->kernel     0x%08x\n"
		"current->state      0x%08x\n"
		"current->irqflag    0x%08x\n"
		"current->addr       0x%08x\n"
		, current->mm.sp, current->mm.base, current->mm.heap,
		current->mm.kernel, current->state, current->irqflag,
		current->addr);

	printk("\ncurrent context\n");
	int i;
	for (i = 0; i < NR_CONTEXT; i++)
		printk("[%02d] 0x%08x\n", i, current->mm.sp[i]);

	printk("\nSCB_ICSR  0x%08x\n"
		"SCB_CFSR  0x%08x\n"
		"SCB_HFSR  0x%08x\n"
		"SCB_MMFAR 0x%08x\n"
		"SCB_BFAR  0x%08x\n",
		SCB_ICSR, SCB_CFSR, SCB_HFSR, SCB_MMFAR, SCB_BFAR);

	/* led for debugging */
	SET_PORT_CLOCK(ENABLE, PORTD);
	SET_PORT_PIN(PORTD, 2, PIN_OUTPUT_50MHZ);
	while (1) {
		PUT_PORT(PORTD, GET_PORT(PORTD) ^ 4);
		mdelay(100);
	}
}

#include <kernel/lock.h>
static DEFINE_SPINLOCK(nvic_lock);
void SET_IRQ(int on, unsigned irq_nr)
{
	spin_lock(nvic_lock);
	__SET_IRQ(on&1, irq_nr);
	spin_unlock(nvic_lock);
}