#include "load_node.h"
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define RESOURCE_OWNER_TAG 100
#define RESOURCE_TRANSFER_TAG 101

#define BERSERK 666
int random_resource_owner(int world_size){
	time_t tt;
	srand(time(&tt));
	return (rand() % (world_size-1)) +1; // <1,n)
}
void check_world_size(int size){
	  // We are assuming at least 2 processes for this task
  if (size < 2) {
    fprintf(stderr, "World size must be greater than 1!\n");
    MPI_Abort(MPI_COMM_WORLD, 1);
  }
}
void b_cast_resource_owner(int world_size){
	int owner_pid = random_resource_owner(world_size);
	printf("Resource owner pid is = %d\n", owner_pid);
	int i=1;
	for(i=1;i<world_size;i++){
		MPI_Send(&owner_pid, 1, MPI_INT, i, RESOURCE_OWNER_TAG, MPI_COMM_WORLD);
	}
} 
void* resource_transfer_listener(void * node_data){
	int resource_id;
	MPI_Status status;
	node* my_node = (node*) node_data;
	while(1){
    	MPI_Recv(&resource_id, 1, MPI_INT, MPI_ANY_SOURCE, RESOURCE_TRANSFER_TAG, MPI_COMM_WORLD, &status); 
    	printf("Process = %d, recived berserk from process = %d\n", my_node->id, status.MPI_SOURCE);
		if(my_node->id == 0){

		}else{
			printf("Process = %d pass resource to parent = %d\n", my_node->id,my_node->parent);
			MPI_Send(&resource_id, 1, MPI_INT, my_node->parent, RESOURCE_TRANSFER_TAG, MPI_COMM_WORLD);
		}

	}
	pthread_exit(NULL);
}

int create_transfer_listener(node* my_node){
	 pthread_t t;
     int rc = pthread_create(&t, NULL, resource_transfer_listener, (void *)my_node);
		if (rc){
		      printf("ERROR; return code from pthread_create() is %d\n", rc);
		      exit(-1);
		}

    return t;
}
int main(int argc, char** argv) {
	MPI_Init(NULL, NULL);
	int world_rank;
	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	check_world_size(world_size);

	node* my_node = load_node(world_rank);
	print_node(my_node);
	sleep(1); //just for print

	int transfer_listener_id  = create_transfer_listener(my_node);
	
	if (world_rank == 0) {
		b_cast_resource_owner(world_size);
	} else{
		int owner_pid;
		MPI_Recv(&owner_pid, 1, MPI_INT, 0, RESOURCE_OWNER_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
		//printf("Process %d received recource owner = %d from process 0\n", world_rank, owner_pid);
		if(owner_pid == world_rank){
			int berserk_id = BERSERK;
			printf("Process = %d send berserk to parent = %d\n", world_rank, my_node->parent);
			MPI_Send(&berserk_id, 1, MPI_INT, my_node->parent, RESOURCE_TRANSFER_TAG, MPI_COMM_WORLD);
		}
	}
	void *status;
    pthread_join(transfer_listener_id, &status);
	MPI_Finalize();
}
