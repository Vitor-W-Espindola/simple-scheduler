#include "system.h"

#include <stdlib.h>
#include <stdio.h>

void print_awaiting(struct system *s) {
	printf("Printing awaiting queue...\n");
	struct task *current_task = s->awaiting_sentinel;
	while(1) {
		printf("%c -> %c -> %c | (c = %d, a = %d, p = %d)\n", current_task->prev->name, current_task->name, current_task->next->name, current_task->c, current_task->a, current_task->p);
		current_task = current_task->next;
		if(current_task == s->awaiting_sentinel)
			break;
	}
}

void print_awaiting_reversed(struct system *s) {

	printf("Printing awaiting queue reversed...\n");
	struct task *current_task = s->awaiting_sentinel;
	while(1) {
		printf("%c -> %c -> %c | (c = %d, a = %d, p = %d)\n", current_task->prev->name, current_task->name, current_task->next->name, current_task->c, current_task->a, current_task->p);
		current_task = current_task->prev;
		if(current_task == s->awaiting_sentinel)
			break;
	}
}

void print_execution(struct system *s) {
	printf("Printing execution queue...\n");
	struct task *current_task = s->execution_sentinel;
	while(1) {
		printf("%c -> %c -> %c | (c = %d, a = %d, p = %d)\n", current_task->prev->name, current_task->name, current_task->next->name, current_task->c, current_task->a, current_task->p);
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
	printf("Received task for processing: %d %d %d %d\n", t->c, t->a, t->p, t->pol);
	
	if(t->a > 0) add_to_awaiting(s, t);
	else add_to_execution(s, t);

	// Create or add a scheduling queue for the task with its policy
	struct scheduling_queue *current_sq = s->scheduling_sentinels;
	while(1) {
		if((current_sq->next == s->scheduling_sentinels) && t->p != current_sq->priority) {
			// If it has reached the end of the queue

			// Define and initializes a new scheduling queue for this priority level
			struct scheduling_queue *new_sq = malloc(sizeof(struct scheduling_queue));
			
			new_sq->priority = t->p;
			new_sq->pol = t->pol;
			
			// Creates sentinel node for scheduling queue
			struct scheduling_node *ss = malloc(sizeof(struct scheduling_node));
			ss->t = NULL;
			ss->next = ss;
			ss->prev = ss;
			new_sq->scheduling_sentinel = ss;
			
			// Create scheduling node for task and add it to its new scheduling queue
			struct scheduling_node *new_sn = malloc(sizeof(struct scheduling_node));
			new_sn->t = t;
			new_sn->next = ss;
			ss->next = new_sn;
			new_sn->prev = ss;
			ss->prev = new_sn;
			
			// Add new scheduling queue in list (ordered by priority)
			current_sq = s->scheduling_sentinels->next;
			while(1) {
				if(current_sq == s->scheduling_sentinels) {
					// If it has reached the end of the queue	
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
				// If there is already a priority queue for this task's priority level
				// it goes to the end of the queue
				struct scheduling_node *new_sn = malloc(sizeof(struct scheduling_node));
				new_sn->t = t;

				current_sq->scheduling_sentinel->prev->next = new_sn;
				new_sn->prev = current_sq->scheduling_sentinel->prev;
				new_sn->next = current_sq->scheduling_sentinel;
				current_sq->scheduling_sentinel->prev = new_sn;
				break;
			}
		}
		current_sq = current_sq->next;
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
				}
			}
			current_task = current_task->next;
		}
		
		return arriving_tasks;
}

struct task ** check_for_ending_tasks(struct system *s, int * len) {
		struct task **ending_tasks = malloc(0);
		struct task *current_task = s->execution_sentinel->next;
		
		int i = 0;

		*(len) = 0;
		
		while(1) {
			if(current_task == s->execution_sentinel) {
				break;
			} else {
				if(current_task->c == 0) {
					ending_tasks = realloc(ending_tasks, (++(*(len))) * sizeof(struct task *));
					ending_tasks[i] = current_task;
					i++;
					break;
				}
			}
			current_task = current_task->next;
		}
		
		
		return ending_tasks;
}


void move_to_execution(struct system *s, struct task *t) {
	// Detatching task from awaiting list
	t->prev->next = t->next;
	t->next->prev = t->prev;

	add_to_execution(s, t);
}

void remove_from_execution(struct system *s, struct task *t) {

	struct scheduling_queue *current_sq = s->scheduling_sentinels->next;
	// Find its scheduling queue
	while(1) {
		if(current_sq == s->scheduling_sentinels) {
			break;
		} else {
			if(current_sq->priority == t->p) {
				// Find its schediling node
				struct scheduling_node *current_sn = current_sq->scheduling_sentinel->next;
				while(1) {
					if(current_sn == current_sq->scheduling_sentinel) {
						break;
					} else {
						if(current_sn->t->name == t->name) {
							// Remove it from the list
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
	struct scheduling_queue *current_sq = s->scheduling_sentinels->next;
	while(1) {
		if(current_sq == s->scheduling_sentinels) {	
			break;
		} else {
			if(current_sq->priority == t->p) {
				if(current_sq->pol == 1) {
				// FIFO
					return current_sq->scheduling_sentinel->next->t; 
				}
				else if(current_sq->pol == 2) {
				// RR
					// First element of the queue goes to the end
					struct scheduling_node *current_sn = current_sq->scheduling_sentinel->next;
					
					// Change node after current_sn
					if (current_sn->next != current_sq->scheduling_sentinel) {
						current_sn->next->prev = current_sq->scheduling_sentinel;
						current_sq->scheduling_sentinel->prev->next = current_sn;

						// Change sentinel node and current node
						current_sq->scheduling_sentinel->next = current_sn->next;
						current_sn->next = current_sq->scheduling_sentinel;
						
						current_sn->prev = current_sq->scheduling_sentinel->prev;
						current_sq->scheduling_sentinel->prev = current_sn;
					}
					return current_sn->t;
				}
			} 
			else current_sq = current_sq->next;
				
		};
	}	
	return t;
}

int update_execution(struct system *s, struct task **arriving_tasks, struct task **ending_tasks, int len_a, int len_e) {

		printf("Updating execution...\n");

		int i;
		struct task * running;
		struct task * next_to_be_runned;
		
		// Removing tasks of execution queue
		for(i = 0; i < len_e; i++)
			remove_from_execution(s, ending_tasks[i]);
		
		// a's--
		struct task *current_task = s->awaiting_sentinel->next;
		while(1) {
			if(current_task == s->awaiting_sentinel) break;
			(current_task->a)--;
			current_task = current_task->next;
		}
		
		// Move arriving tasks to execution queue
		for(i = 0; i < len_a; i++) {
			move_to_execution(s, arriving_tasks[i]);
		}

		running = s->execution_sentinel->next;
		
		if(running == s->execution_sentinel) {
			if(s->awaiting_sentinel->next == s->awaiting_sentinel) return 1;
			else return '.';
		} else {
		
			// c--
			// (running->c)--;
			next_to_be_runned = interrogate_scheduling_queue(s, running);
			(next_to_be_runned->c)--;
			if(next_to_be_runned != running) {
				// change next_to_be_running neighbours
				next_to_be_runned->prev->next = next_to_be_runned->next;
				next_to_be_runned->next->prev = next_to_be_runned->prev;
				
				// put next_to_be_running in front of execution queue
				next_to_be_runned->prev = s->execution_sentinel;
				next_to_be_runned->next = running;

				running->prev = next_to_be_runned;
				s->execution_sentinel->next = next_to_be_runned;
			}
		}

		
		
		printf("NEXT TO BE RUNNED %c\n", next_to_be_runned->name);	
		
		return next_to_be_runned->name;
}

void free_scheduling_queues(struct system *s) {
	struct scheduling_queue *current_sq = s->scheduling_sentinels->next;
	struct scheduling_queue *next_sq_to_be_freed = current_sq;
	while(1) {
		current_sq = next_sq_to_be_freed;
		if(current_sq == s->scheduling_sentinels) break;
		printf("Freeing scheduling queue with priority %d...\n", current_sq->priority);
		current_sq->prev->next = current_sq->next;
		current_sq->next->prev = current_sq->prev;
		
		next_sq_to_be_freed = current_sq->next;
		
		free(current_sq->scheduling_sentinel);
		free(current_sq);

	}
}

char *run(struct system *s) {
	
	int out_index = 0;
	int out_len = 1;
	char *out = malloc(out_len);

	printf("Running system...\n");
	while(1) {
		int i, len_a, len_e = 0;
		struct task ** arriving_tasks = check_for_arriving_tasks(s, &len_a);
		struct task ** ending_tasks = check_for_ending_tasks(s, &len_e);

		
		if(len_a > 0) {
			printf("Listing arriving tasks...\n");
			for(i = 0; i < len_a; i++) printf("\tTask named %c\n", arriving_tasks[i]->name);
		}

		if(len_e > 0) { 
			printf("Listing ending tasks...\n");
			for(i = 0; i < len_e; i++) printf("\tTask named %c\n", ending_tasks[i]->name);
		}

		
		char running_name = update_execution(s, arriving_tasks, ending_tasks, len_a, len_e);
		
		free(arriving_tasks);
		free(ending_tasks);

		printf("\n\n");
		// print_awaiting(s);
		print_execution(s);
		// print_scheduling_queues(s);

		if(running_name == 1) break;

		out = realloc(out, ++out_len);
		out[out_index++] = running_name;
	}
	out[out_len - 1] = '\0';
	
	free_scheduling_queues(s);
	printf("Are they gone??\n");
	print_scheduling_queues(s);
	return out;
}

/*
void stop(struct system *s) {
	
	// Freeing awaiting queue
	struct task * current_task = s->awaiting_sentinel->next;
	struct task * task_to_be_freed;
	while(1) {
		if(current_task == s->awaiting_sentinel)
			break;

		current_task->next->prev = current_task->prev;
		current_task->prev->next = current_task->next;

		task_to_be_freed = current_task;
		current_task = current_task->next;
		
		printf("Freeing task %c...\n", task_to_be_freed->name);
		free(task_to_be_freed);
	} 

	printf("Stopping system...\b");
}

void reset(struct system *s) {
	
	// Freeing awaiting queue
	
	struct task * current_task = s->awaiting_sentinel->next;
	struct task * task_to_be_freed;
	while(1) {
		if(current_task == s->awaiting_sentinel)
			break;

		current_task->next->prev = current_task->prev;
		current_task->prev->next = current_task->next;

		task_to_be_freed = current_task;
		current_task = current_task->next;
		
		printf("Freeing task %c...\n", task_to_be_freed->name);
		free(task_to_be_freed);
	} 
	
	

};
*/
