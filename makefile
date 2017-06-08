resistance: resistance.c load_node.c rand_utils.c listeners.c
	mpicc -pthread resistance.c load_node.c rand_utils.c listeners.c -o resistance