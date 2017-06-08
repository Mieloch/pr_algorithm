resistance: resistance.c load_node.c rand_utils.c fifo.c
	mpicc -pthread resistance.c load_node.c rand_utils.c listeners.c fifo.c -o resistance