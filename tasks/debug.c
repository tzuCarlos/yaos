#ifdef CONFIG_DEBUGGER
#include <foundation.h>
#include <kernel/task.h>

#define GPIO_PIN_INDEX			10

static void __attribute__((naked)) isr_break()
{
	unsigned int *sp, *lr, *psr, *usp;

	sp  = (unsigned int *)__get_sp();
	psr = (unsigned int *)__get_psr();
	lr  = (unsigned int *)__get_ret_addr();
	usp = (unsigned int *)__get_usp();

	printk("\nKernel SP      0x%08x\n"
		"Stacked PSR    0x%08x\n"
		"Stacked PC     0x%08x\n"
		"Stacked LR     0x%08x\n"
		"Current LR     0x%08x\n"
		, sp, sp[7], sp[6], sp[5], lr, psr);

	printk("\nUser SP        0x%08x\n"
		"Stacked PSR    0x%08x\n"
		"Stacked PC     0x%08x\n"
		"Stacked LR     0x%08x\n"
		, usp, usp[7], usp[6], usp[5]);

	printk("\ncurrent->sp         0x%08x\n"
		"current->base       0x%08x\n"
		"current->heap       0x%08x\n"
		"current->kernel     0x%08x\n"
		"current->kernel->sp 0x%08x\n"
		"current->state      0x%08x\n"
		"current->irqflag    0x%08x\n"
		"current->addr       0x%08x\n"
		"current             0x%08x\n"
		, current->mm.sp, current->mm.base, current->mm.heap,
		current->mm.kernel.base, current->mm.kernel.sp, current->state,
		current->irqflag, current->addr, current);

	ret_from_gpio_int(GPIO_PIN_INDEX);
	__set_pc(lr);
}

static void inspect()
{
	int vector_nr;

	vector_nr = gpio_init(GPIO_PIN_INDEX,
			GPIO_MODE_INPUT | GPIO_CONF_PULL | GPIO_INT_FALLING);

	register_isr(vector_nr, isr_break);
}
REGISTER_TASK(inspect, TASK_PRIVILEGED, DEFAULT_PRIORITY);
#endif
