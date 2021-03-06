#include <kernel/task.h>
#include "fair.h"

/* runqueue's head can be init task. thnik about it */
static DEFINE_LIST_HEAD(cfs_rq);

struct task *cfs_pick_next(struct scheduler *cfs)
{
	struct list *head = cfs->rq;

	if (head->next == head) /* empty */
		return NULL;

	return get_container_of(head->next, struct task, rq);
}

void cfs_rq_add(struct scheduler *cfs, struct task *new)
{
	if (!new || get_task_state(new))
		return;

	struct list *curr, *head;
	struct task *task;

	head = cfs->rq;

	for (curr = head->next; curr != head; curr = curr->next) {
		task = get_container_of(curr, struct task, rq);

		if (task->se.vruntime > new->se.vruntime) {
			list_add(&new->rq, curr->prev);
			break;
		}
	}

	if (curr == head) /* if empty or the last entry */
		list_add(&new->rq, head->prev);

	cfs->nr_running++;
}

void cfs_rq_del(struct scheduler *cfs, struct task *task)
{
	list_del(&task->rq);
	cfs->nr_running--;
}

void cfs_init(struct scheduler *cfs)
{
	cfs->vruntime_base = 0;
	cfs->nr_running    = 0;
	cfs->rq            = (void *)&cfs_rq;
}
