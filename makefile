resistance: resistance.c load_node.c utils.c fifo.c
	mpicc -pthread resistance.c load_node.c utils.c fifo.c -o resistance