struct task {
	char name;
	short int c, a, p, pol;
	
	struct scheduling_node *sn;	
	struct task *next;
	struct task *prev;
};
