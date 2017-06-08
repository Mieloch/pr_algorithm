resistance: resistance.c load_node.c
	mpicc -pthread resistance.c load_node.c -o resistance