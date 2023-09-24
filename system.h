#include "scheduling_queue.h"

struct system {
	struct task *awaiting_sentinel;
	struct task *execution_sentinel;
	struct scheduling_queue *scheduling_sentinels;
};

void print_awaiting(struct system *s);
void print_awaiting_reversed(struct system *s);
void print_execution(struct system *s);
void print_scheduling_queues(struct system *s);
void process_task(struct task *t, struct system *s);
struct task ** check_for_arriving_tasks(struct system *s, int * len);
struct task ** check_for_ending_tasks(struct system *s, int *len);
void move_to_execution(struct system *s, struct task *t);
void remove_from_execution(struct system *s, struct task *t);
struct task * interrogate_scheduling_queue(struct system *s, struct task *t);
int update_execution(struct system *s, struct task **arriving_tasks, struct task **ending_tasks, int len_a, int len_e);
void free_scheduling_queues(struct system *s);
char *run(struct system *s);
void stop(struct system *s);
void reset(struct system *s);
