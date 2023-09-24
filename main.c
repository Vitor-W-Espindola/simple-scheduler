#include "system.h"

#include <stdio.h>
#include <stdlib.h>

int main() {
	
	struct system sys, *s = &sys;
	
	// Creation of awaiting and execution sentinels
	struct task as, es;
	as.name = '!';
	as.c = as.a = as.p = as.pol = -1;
	as.next = as.prev = &as;
	es.name = '@';
	es.c = es.a = es.p = es.pol = -1;
	es.next = es.prev = &es;
	s->awaiting_sentinel = &as;
	s->execution_sentinel = &es;

	// Creation of scheduling queue "master" sentinel
	struct scheduling_queue sqs;
	sqs.priority = -1;
	sqs.pol = -1;
	sqs.scheduling_sentinel = NULL;
	sqs.next = sqs.prev = &sqs;
	s->scheduling_sentinels = &sqs;	

	while(1) {
	
		char *out;
		
		// Initial conditions	
		int i, len;

		scanf("%d", &len);	

		if(len == 0) break;
		
		for(i = 0; i < len; i++) {
			struct task *t = malloc(sizeof(struct task));
			t->name = (97 + i);
			scanf("%d %d %d %d", &(t->c), &(t->a), &(t->p), &(t->pol));
			process_task(t, s);
		}

		print_execution(s);

		out = run(s); // IMPULSE RESPONSE !!! KABOOM !!!
		
		printf("output: %s\n\n", out);

		free(out);
	}

	printf("\nHello, World!\n");
	return 0;

}
