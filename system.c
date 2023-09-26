#include "system.h"

#include <stdlib.h>
#include <stdio.h>

void print_awaiting(struct system *s) {
	printf("Printing awaiting queue...\n");
	struct task *current_task = s->awaiting_sentinel;
	while(1) {
		printf("%c -> %c -> %c | (c = %d, a = %d, p = %d, pol = %d)\n", current_task->prev->name, current_task->name, current_task->next->name, current_task->c, current_task->a, current_task->p, current_task->pol);
		current_task = current_task->next;
		if(current_task == s->awaiting_sentinel)
			break;
	}
}

void print_awaiting_reversed(struct system *s) {

	printf("Printing awaiting queue reversed...\n");
	struct task *current_task = s->awaiting_sentinel;
	while(1) {
		printf("%c -> %c -> %c | (c = %d, a = %d, p = %d, pol = %d)\n", current_task->prev->name, current_task->name, current_task->next->name, current_task->c, current_task->a, current_task->p, current_task->pol);
		current_task = current_task->prev;
		if(current_task == s->awaiting_sentinel)
			break;
	}
}

void print_execution(struct system *s) {
	printf("Printing execution queue...\n");
	struct task *current_task = s->execution_sentinel;
	while(1) {
		printf("%c -> %c -> %c | (c = %d, a = %d, p = %d, pol = %d)\n", current_task->prev->name, current_task->name, current_task->next->name, current_task->c, current_task->a, current_task->p, current_task->pol);
		current_task = current_task->next;
		if(current_task == s->execution_sentinel)
			break;
	}
}

void print_scheduling_queues(struct system *s) {
	printf("Printing scheduling queues...\n");
	struct scheduling_queue *current_sq = s->scheduling_sentinels->next;
	while(1) {
		if(current_sq == s->scheduling_sentinels) break;
		printf("Printing scheduling queue with priority %d: ", current_sq->priority);
		struct scheduling_node *current_sn = current_sq->scheduling_sentinel->next;
		while(1) {
			if(current_sn == current_sq->scheduling_sentinel) break;
			printf("Task %c ", current_sn->t->name);
			current_sn = current_sn->next;
		}
		printf("\n");
		current_sq = current_sq->next;
	}
}

void add_to_awaiting(struct system *s, struct task *t) {
	// Add task to awaiting queue if a > 0 (ordered by arrival time)
	struct task *current_task = s->awaiting_sentinel->next;
	while(1) {
		if(current_task == s->awaiting_sentinel) {
			// If it has reached the end of the queue
			t->prev = s->awaiting_sentinel->prev;
			t->next = s->awaiting_sentinel;

			s->awaiting_sentinel->prev->next = t;
			s->awaiting_sentinel->prev = t;
			break;
		} else {
			if((t->a < current_task->a)) {
				t->next = current_task;
				t->prev = current_task->prev;

				current_task->prev->next = t;
				current_task->prev = t;
				break;
			}
		}
		current_task = current_task->next;
	}
}

void add_to_execution(struct system *s, struct task *t) {
	// Add task to executing queue if a == 0 (inversed ordered by priority (max 1))
	struct task *current_task = s->execution_sentinel->next;
	while(1) {
		if(current_task == s->execution_sentinel) {
			// If it has reached the end of the queue
			t->prev = s->execution_sentinel->prev;
			t->next = s->execution_sentinel;

			s->execution_sentinel->prev->next = t;
			s->execution_sentinel->prev = t;
			break;
		} else {
			if((t->p < current_task->p)) {
				t->next = current_task;
				t->prev = current_task->prev;

				current_task->prev->next = t;
				current_task->prev = t;
				break;
			}
		}
		current_task = current_task->next;
	}

}

void process_task(struct task* t, struct system *s) {
	
	// Processing a task is done in the reading of the input and means adding the read task to the awaiting queue (while ordering by arrival time) if its arrival time != 0 
	// or add it to the execution queue (while ordering by priority) if its arrival time == 0.
	// Processing atask also means allocating a new scheduling queue for its priority level and scheduling politic (FIFO or RR) and adding the task to it.
	// Obs.: If there is already a scheduling queue for its priority level, the task is simply appended to it
	
	if(t->a > 0) add_to_awaiting(s, t);
	else add_to_execution(s, t);

	// About scheduling queue and scheduling nodes: the system has a scheduling queue sentinel that can lead to all the scheduling queues created on this processes,
	// each with its respective priority level and scheduling policy. Each scheduling queue, therefore, has a scheduling node sentinel that can lead to all members of this
	// scheduling queue. However, these members are not tasks themselves, but scheduling nodes. As a matter of fact, a scheduling node represents a task in this context of
	// scheduling queues, such is that a scheduling node has only a pointer to the task it represents.
	//
	// Obs.: The system was actually built that way because of two simple reasons: (1) bad software project planning, (2) arriving queue and execution queue are actually disjoint sets,
	// which means that a task having pointers to its previous and next elements do not imply inconsistence, given a task is either on one or the other queue at any point in time. 
	// However this does not occur with scheduling queues, given that, at any point in time, a task might be either at awaiting or execution queue, but always in a scheduling queue, which implies 
	// they must have two different entities to represent their previous and next elements in the different queues, which will not ever be the same for both queues. 
	// That is why scheduling nodes are required.

	struct scheduling_queue *current_sq = s->scheduling_sentinels->next;
	while(1) {
		if(current_sq == s->scheduling_sentinels) {
			// Reached the end of the queue, meaning no scheduling queue for this task's priority 
			// level has been created yet.
			struct scheduling_queue *new_sq = malloc(sizeof(struct scheduling_queue));	
			new_sq->priority = t->p;
			
			struct scheduling_node *ss = malloc(sizeof(struct scheduling_node));
			ss->t = NULL;
			ss->next = ss;
			ss->prev = ss;
			new_sq->scheduling_sentinel = ss;
			
			struct scheduling_node *new_sn = malloc(sizeof(struct scheduling_node));
			new_sn->t = t;
			new_sn->next = ss;
			ss->next = new_sn;
			new_sn->prev = ss;
			ss->prev = new_sn;
			
			current_sq = s->scheduling_sentinels->next;
			while(1) {
				if(current_sq == s->scheduling_sentinels) {
					new_sq->next = current_sq;
					new_sq->prev = current_sq->prev;
					current_sq->prev->next = new_sq;
					current_sq->prev = new_sq;
					break;
				} else {
					if((new_sq->priority < current_sq->priority)) {
						new_sq->next = current_sq;
						new_sq->prev = current_sq->prev;
						current_sq->prev->next = new_sq;
						current_sq->prev = new_sq;
						break;
					}
				}
				current_sq = current_sq->next;
			}
			
			break;
		} else {
			if(t->p == current_sq->priority) {
				struct scheduling_node *new_sn = malloc(sizeof(struct scheduling_node));
				new_sn->t = t;
				current_sq->scheduling_sentinel->prev->next = new_sn;
				new_sn->prev = current_sq->scheduling_sentinel->prev;
				new_sn->next = current_sq->scheduling_sentinel;
				current_sq->scheduling_sentinel->prev = new_sn;
				break;
			} else current_sq = current_sq->next;
		}
	}
}

struct task ** check_for_arriving_tasks(struct system *s, int * len) {
		struct task **arriving_tasks = malloc(0);
		struct task *current_task = s->awaiting_sentinel->next;
		
		int i = 0;

		*(len) = 0;
		
		while(1) {
			if(current_task == s->awaiting_sentinel) {
				break;
			} else {
				if(current_task->a == 0) {
					arriving_tasks = realloc(arriving_tasks, (++(*(len))) * sizeof(struct task *));
					arriving_tasks[i] = current_task;
					i++;
				} else break;
			}
			current_task = current_task->next;
		}
		
		return arriving_tasks;
}

struct task * check_for_ending_task(struct system *s) {
		return s->execution_sentinel->next != NULL && s->execution_sentinel->next->c == 0 ? s->execution_sentinel->next : NULL;
}


void move_to_execution(struct system *s, struct task *t) {
	// Detatching task from awaiting list
	t->prev->next = t->next;
	t->next->prev = t->prev;

	add_to_execution(s, t);
}

void remove_from_execution(struct system *s, struct task *t) {

	struct scheduling_queue *current_sq = s->scheduling_sentinels->next;
	while(1) {
		if(current_sq == s->scheduling_sentinels) {
			break;
		} else {
			if(current_sq->priority == t->p) {
				struct scheduling_node *current_sn = current_sq->scheduling_sentinel->next;
				while(1) {
					if(current_sn == current_sq->scheduling_sentinel) {
						break;
					} else {
						if(current_sn->t->name == t->name) {
							current_sn->prev->next = current_sn->next;
							current_sn->next->prev = current_sn->prev;
							free(current_sn);
							break;
						} else { current_sn = current_sn->next; }
					}
				}
				break;
			} 
			else { current_sq = current_sq->next; }	
		};
	}

	t->prev->next = t->next;
	t->next->prev = t->prev;
	free(t);	
}

struct task * interrogate_scheduling_queue(struct system *s, struct task *t) {
	struct task * to_be_runned;
	struct scheduling_queue *current_sq = s->scheduling_sentinels->next;
	while(1) {
		if(current_sq == s->scheduling_sentinels) {	
			break;
		} else {
			if(current_sq->priority == t->p) {
				struct scheduling_node *current_sn = current_sq->scheduling_sentinel->next;
				if(current_sn->t->pol == 1) {
				// FIFO - Only returns the first element of the scheduling queue, without moving it; 
					to_be_runned = current_sn->t;
				} else if(current_sn->t->pol == 2) {
				// RR - Unlike FIFO, Round-Robin returns the first element but moves it to the end of the queue.
					to_be_runned = current_sq->scheduling_sentinel->next->t;
					if (current_sn->next != current_sq->scheduling_sentinel) {
						current_sn->next->prev = current_sq->scheduling_sentinel;
						current_sq->scheduling_sentinel->prev->next = current_sn;

						current_sq->scheduling_sentinel->next = current_sn->next;
						current_sn->next = current_sq->scheduling_sentinel;
						
						current_sn->prev = current_sq->scheduling_sentinel->prev;
						current_sq->scheduling_sentinel->prev = current_sn;
					}
				} else	to_be_runned = NULL;
			       	break;
			} else current_sq = current_sq->next;
		}
	}
	return to_be_runned;	
}

void free_scheduling_queues(struct system *s) {
	struct scheduling_queue *current_sq = s->scheduling_sentinels->next;
	struct scheduling_queue *next_sq_to_be_freed = current_sq;
	while(1) {
		current_sq = next_sq_to_be_freed;
		if(current_sq == s->scheduling_sentinels) break;
		current_sq->prev->next = current_sq->next;
		current_sq->next->prev = current_sq->prev;
		
		next_sq_to_be_freed = current_sq->next;
		
		free(current_sq->scheduling_sentinel);
		free(current_sq);

	}
}

char *run(struct system *s) {
	
	// Running the system after giving its initial conditions consists of a loop with numbers decrementation and manipulation of the queues, 
	// where at the end of any completed cycle the task name is appended to the string which represents the scale of execution. 
	//
	// A cycle can be summarized as follows:
	// - Gather all arriving tasks (arrival time == 0) from awaiting queue and the ending task (computation == 0), if there is any, from execution queue;
	// - Remove those arriving tasks from awaiting queue and move all to execution queue while ordering it by priority.
	// - Decrement by one the arrival time of all those tasks which remained on the arriving queue.
	// - Remove the ending task from the execution queue. This also means removing this task from its scheduling queue.
	// - Take the first element of the execution queue and interrogate it about its scheduling policy: 
	// 		"Which task from your queue is the next one to be runned accordingly to your policy? Gimme its name and keep doing your damn job >:("
	// - Run the task obtained from the interrogatory and decrement by one its computation. 
	//
	// Obs.: As said ealier, running a task actually means appending its name to the string which represents the scale of execution that is being built up by the scheduler. 

	int out_index = 0;
	int out_len = 1;
	char *out = malloc(out_len);

	while(1) {

		int i, len_a;
		struct task ** arriving_tasks = check_for_arriving_tasks(s, &len_a);
		struct task * ending_task = check_for_ending_task(s);
		
		char running_name;
	
		struct task * current_task;	
		struct task * running;
		struct task * to_be_runned;

		for(i = 0; i < len_a; i++)
			move_to_execution(s, arriving_tasks[i]);
		
		if(ending_task != NULL)
			remove_from_execution(s, ending_task);	

		current_task = s->awaiting_sentinel->next;
		while(1) {
			if(current_task == s->awaiting_sentinel) break;
			(current_task->a)--;
			current_task = current_task->next;
		}

		running = s->execution_sentinel->next;
		
		if(running == s->execution_sentinel) {
			if(s->awaiting_sentinel->next == s->awaiting_sentinel) running_name = 1;
			else running_name = '.';
		} else {
			to_be_runned = interrogate_scheduling_queue(s, running);
			if(to_be_runned != running) {
				to_be_runned->prev->next = to_be_runned->next;
				to_be_runned->next->prev = to_be_runned->prev;
				to_be_runned->prev = s->execution_sentinel;
				to_be_runned->next = running;
				running->prev = to_be_runned;
				s->execution_sentinel->next = to_be_runned;
			}
			(to_be_runned->c)--;
			running_name = to_be_runned->name;
		}
		
		free(arriving_tasks);

		if(running_name == 1) break;

		out = realloc(out, ++out_len);
		out[out_index++] = running_name;
		

	}

	out[out_len - 1] = '\0';
	return out;
}
