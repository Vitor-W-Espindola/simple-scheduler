#include "system.h"

#include <stdio.h>
#include <stdlib.h>

int main() {
	
	// Creation of structures system, awaiting, execution and schedule queue sentinels (all on stack)
	// Awaiting, execution and scheduling queues are double-linked lists with sentinel nodes, and I don't actually
	// know if calling them queues are correct or not.
	//
	// Obs.: This is just like creating the whole machinary and get ready to consume inputs 
	
	struct system sys, *s = &sys;
	
	struct task as, es;
	as.name = '!';
	as.c = as.a = as.p = as.pol = -1;
	as.next = as.prev = &as;
	es.name = '@';
	es.c = es.a = es.p = es.pol = -1;
	es.next = es.prev = &es;
	s->awaiting_sentinel = &as;
	s->execution_sentinel = &es;

	struct scheduling_queue sqs;
	sqs.priority = -1;
	sqs.scheduling_sentinel = NULL;
	sqs.next = sqs.prev = &sqs;
	s->scheduling_sentinels = &sqs;	

	while(1) {
	
		// Read the group of tasks to be scheduled (this a static scheduling, all task property will not change)
		//
		// Obs.: This is just like given the system its initial conditions before running it
		// Obs.: Input is written in a pre-defined format, so it is the reading
		
		char *out;
		int i, len;

		scanf("%d", &len);	

		if(len == 0) break;
		
		for(i = 0; i < len; i++) {
			struct task *t = malloc(sizeof(struct task));
			t->name = (65 + i);
			
			scanf("%d %d %d %d", &(t->c), &(t->a), &(t->p), &(t->pol));
			
			process_task(t, s);
		}
	
		// Have the initial conditions been set, all queues correctly created and tasks appended to them, the system is now ready to run.
		out = run(s);
		
		printf("%s\n\n", out);

		free(out);
	}

	return 0;

}
