#ifndef __TASK_H__
#define __TASK_H__

#define STACK_SIZE			1024 /* bytes, kernel stack */
#define USER_SPACE_SIZE			1024 /* bytes */
#define HEAP_SIZE			(USER_SPACE_SIZE >> 2) /* one fourth */
#define USER_STACK_SIZE			(USER_SPACE_SIZE - HEAP_SIZE)

#define STACK_SENTINEL			0xdeafc0de

struct mm {
	unsigned int *base;
	unsigned int *sp;
	unsigned int *heap;
	/* heap is located in the bottom of stack memory and expands up to brk.
	 * and to obtain stack bottom, "bottom = sp & ~(stack size - 1)" while
	 * "top = bottom + stack size - 1". so "heap size = brk - bottom". but
	 * be aware that both of heap and brk can be changed dynamically. */

	/* kernel stack */
	struct {
		unsigned int *base;
		unsigned int *sp;
	} kernel;
};

/* type & flag */
#define TASK_USER			0x00
#define TASK_KERNEL			0x01
#define TASK_PRIVILEGED			TASK_KERNEL
#define TASK_STATIC			0x02
#define TASK_SYSCALL			0x04
#define TASK_CLONED			0x08
#define TASK_HANDLER			0x10
#define STACK_SHARED			0x80

#define set_task_flags(p, v)		((p)->flags = v)
#define get_task_flags(p)		((p)->flags)
#define set_task_type(p, v)		set_task_flags(p, v)
#define get_task_type(p)		get_task_flags(p)

/* state */
#define TASK_RUNNING			0x00
#define TASK_STOPPED			0x01
#define TASK_WAITING			0x02
#define TASK_SLEEPING			0x04
#define TASK_ZOMBIE			0x08

#define set_task_state(p, s)		((p)->state = s)
#define get_task_state(p)		((p)->state)

/* priority */
/* the lower number, the higher priority.
 *
 *  realtime |  normal
 * ----------|----------
 *   0 ~ 10  | 11 ~ 255
 *                `-----> 132 = default priority
 */
#define RT_PRIORITY			10
#define LOW_PRIORITY			(RT_PRIORITY + 245)
#define DEFAULT_PRIORITY		(RT_PRIORITY + 122)
#ifdef CONFIG_REALTIME
#define HIGH_PRIORITY			0
#else
#define HIGH_PRIORITY			(RT_PRIORITY + 1)
#endif

#define set_task_pri(p, v)		((p)->pri = v)
#define get_task_pri(p)			((p)->pri)
#define is_task_realtime(p)		(get_task_pri(p) <= RT_PRIORITY)

#include <types.h>
#include <kernel/sched.h>

struct task {
	/* `state` must be the first element in the structure as it is used
	 * for sanity check at the initialization */
	unsigned int state;
	unsigned int flags;
	unsigned int pri;
	void *addr;
	unsigned int irqflag;

	struct mm mm;

	struct task *parent;
	struct list children;
	struct list sibling;

	struct list rq;

	struct sched_entity se;
};

#define REGISTER_TASK(f, t, p) \
	static struct task task_##f \
	__attribute__((section(".user_task_list"), aligned(8), used)) = { \
		.state = TASK_ZOMBIE, \
		.flags = TASK_STATIC | t, \
		.pri   = p, \
		.addr  = f, \
	}

extern struct task *current;
extern struct task init;

struct task *make(unsigned int flags, void *addr, void *ref);
int clone(unsigned int flags, void *ref);
void set_task_dressed(struct task *task, unsigned int flags, void *addr);
int alloc_mm(struct task *new, void *ref, unsigned int flags);
void wrapper();
void destroy(struct task *task);
void kill_zombie();

struct task *find_task(unsigned int addr, struct task *head);

int sys_kill(unsigned int tid);
int sys_fork();

/* It used to return `(int)(p)`, task address, but it could probably lead
 * problem when the address is higher than 0x80000000 resulting in negative
 * value. We don't really make use of tid at the moment which is just task
 * address. So return just 1. */
#define get_task_tid(p)			1

#endif /* __TASK_H__ */
