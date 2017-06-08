#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <mpi.h>
#include "node_structure.h"
#include "node_state.h"
#include "msg_tag.h"


void* resource_transfer_listener(void * state){
	int resource_id;
	MPI_Status status;
	node_state* my_node_state = (node_state*) state;
	node* my_node = my_node_state->node_data;
	while(1){
    	MPI_Recv(&resource_id, 1, MPI_INT, MPI_ANY_SOURCE, RESOURCE_TRANSFER_TAG, MPI_COMM_WORLD, &status); 
    	printf("Process = %d, recived berserk from process = %d\n", my_node->id, status.MPI_SOURCE);
		if(my_node->id == 0){
			my_node_state->resource_owner = 0;
		}else{
			printf("Process = %d pass resource to parent = %d\n", my_node->id,my_node->parent);
			MPI_Send(&resource_id, 1, MPI_INT, my_node->parent, RESOURCE_TRANSFER_TAG, MPI_COMM_WORLD);
			my_node_state->resource_owner = my_node->parent;
		}
	}
	pthread_exit(NULL);
}
