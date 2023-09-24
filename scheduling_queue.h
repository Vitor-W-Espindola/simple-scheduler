#include "task.h"

struct scheduling_node {
	struct task *t;

	struct scheduling_node *next;
	struct scheduling_node *prev;
};

typedef void (*fetch)(void);

struct scheduling_queue {
	short int priority;
	short int pol; //SCHED_FIFO - 1, SCHED_RR - 2

	struct scheduling_node *scheduling_sentinel;
	struct scheduling_queue *next;
	struct scheduling_queue *prev;		
};
