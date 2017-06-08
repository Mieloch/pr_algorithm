#include "load_node.h"
#include "utils.h"
#include "fifo.h"
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>



#define RESOURCE_OWNER_TAG 100
#define RESOURCE_TRANSFER_TAG 101
#define ORGANIZE_MEETING 102
#define START_MEETING 103
#define END_MEETING 104


#define BERSERK 666

pthread_mutex_t mutex_resource;
node_state* my_node_state;
//LISTENERS
void* resource_transfer_listener(void * t){ // nasluchiwanie na przekazywanie zasobu w gore 
	int resource_id;
	MPI_Status status;
	node* my_node = my_node_state->node_data;
	while(1){
    	MPI_Recv(&resource_id, 1, MPI_INT, MPI_ANY_SOURCE, RESOURCE_TRANSFER_TAG, MPI_COMM_WORLD, &status); 
    	printf("Process[%d], recived berserk from process[%d]\n", my_node->id, status.MPI_SOURCE);
		if(my_node->id == 0){
			my_node_state->resource_owner = 0;
		}else{
			printf("Process[%d] pass resource to parent[%d]\n", my_node->id,my_node->parent);
			MPI_Send(&resource_id, 1, MPI_INT, my_node->parent, RESOURCE_TRANSFER_TAG, MPI_COMM_WORLD);
			my_node_state->resource_owner = my_node->parent;
		}
	}
	pthread_exit(NULL);
}
void* meeting_acc_listener(void* t){
	MPI_Status status;
	node* my_node = my_node_state -> node_data;
	int ack_count = 0;
	int number = 1;
	while(ack_count < my_node->siblings_length){
		printf("Process[%d] need %d more ack\n", my_node->id, my_node->siblings_length-ack_count);
    	MPI_Recv(&number, 1, MPI_INT, MPI_ANY_SOURCE, ORGANIZE_MEETING, MPI_COMM_WORLD, &status); 
		ack_count++;
		printf("Process[%d] got meeting ack from process[%d]\n", my_node->id,status.MPI_SOURCE);
	}
	pthread_exit(NULL);

}


void check_world_size(int size){
	  // We are assuming at least 2 processes for this task
  if (size < 2) {
    fprintf(stderr, "World size must be greater than 1!\n");
    MPI_Abort(MPI_COMM_WORLD, 1);
  }
}
void check_thread_support(int provided){
 	if (provided!=MPI_THREAD_MULTIPLE) {
        fprintf(stderr, "NO THREAD SUPPORT!!\n");
        exit(-1);
	}
}
void b_cast_resource_owner(int world_size){
	int owner_pid = rand_1_to_bound(world_size);
	printf("Resource owner pid is = %d\n", owner_pid);
	int i=1;
	for(i=1;i<world_size;i++){
		MPI_Send(&owner_pid, 1, MPI_INT, i, RESOURCE_OWNER_TAG, MPI_COMM_WORLD);
	}
} 

int create_transfer_listener(){
	 pthread_t t;
     int rc = pthread_create(&t, NULL, resource_transfer_listener, (void *)t);
		if (rc){
		      printf("ERROR; return code from pthread_create() is %d\n", rc);
		      exit(-1);
		}
    return t;
}
int create_meeting_acc_listener(){
	 pthread_t t;
     int rc = pthread_create(&t, NULL, meeting_acc_listener, (void *)t);
		if (rc){
		      printf("ERROR; return code from pthread_create() is %d\n", rc);
		      exit(-1);
		}
    return t;
}
void organize_meeting(){
	node* my_node = my_node_state -> node_data;
	if(my_node->siblings_length == 0){
		printf("Process[%d] cant organize meeting alone\n", my_node->id);
		return;
	}
	int i;
	int number = -1;
	int transfer_listener_id  = create_meeting_acc_listener();


	for(i=0;i<my_node->siblings_length;i++){
		printf("Process[%d] asking process[%d] for meeting\n", my_node->id,my_node->siblings[i]);
		MPI_Send(&number, 1, MPI_INT, my_node->siblings[i], ORGANIZE_MEETING, MPI_COMM_WORLD);
	}

	void *status;
    pthread_join(transfer_listener_id, &status);
   	printf("Process[%d] has all ack\n",my_node->id);
   	if(my_node->id < min_from_arr(my_node->siblings)){
   		printf("Process[%d] is organizator\n", my_node->id);
   		// todo zasob i zgoda
   		for(i=0;i<my_node->siblings_length;i++){
			printf("Process[%d] send start meeting to process[%d]\n", my_node->id,my_node->siblings[i]);
			MPI_Send(&number, 1, MPI_INT, my_node->siblings[i], START_MEETING, MPI_COMM_WORLD);
		}
		//todo byc moze inne procesy musza potwierdzic start i czekamy na ich ack
		sleep(10); //pal kuce
		for(i=0;i<my_node->siblings_length;i++){
			printf("Process[%d] send end meeting to process[%d]\n", my_node->id,my_node->siblings[i]);
			MPI_Send(&number, 1, MPI_INT, my_node->siblings[i], END_MEETING, MPI_COMM_WORLD);
		}
   	}
   	else{
   		MPI_Status status;
   		printf("Process[%d] wait for start meeting\n", my_node->id);
    	MPI_Recv(&number, 1, MPI_INT, MPI_ANY_SOURCE, START_MEETING, MPI_COMM_WORLD, &status); 
   		printf("Process[%d] start meeting arrived from process[%d]\n",my_node->id, status.MPI_SOURCE);
   	 	//pal kuce
   		MPI_Recv(&number, 1, MPI_INT, MPI_ANY_SOURCE, END_MEETING, MPI_COMM_WORLD, &status);
   		printf("Process[%d] end meeting arrived from process[%d]\n",my_node->id, status.MPI_SOURCE);
   	}
   	printf("Process[%d] meeting is over\n", my_node->id);

}

int main(int argc, char** argv) {
	//init
	int provided;
 	MPI_Init_thread( 0, 0, MPI_THREAD_MULTIPLE, &provided );
 	check_thread_support(provided);
 	int world_rank;
	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	check_world_size(world_size);
    pthread_mutex_init(&mutex_resource, NULL);
	my_node_state= init_node(world_rank);
	int transfer_listener_id  = create_transfer_listener();
	

	if (world_rank == 0) {
		b_cast_resource_owner(world_size);
	} else{
		int owner_pid;
		MPI_Recv(&owner_pid, 1, MPI_INT, 0, RESOURCE_OWNER_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
		//printf("Process %d received recource owner = %d from process 0\n", world_rank, owner_pid);
		if(owner_pid == world_rank){
			int berserk_id = BERSERK;
			printf("Process[%d] send berserk to parent[%d]\n", world_rank, my_node_state->node_data->parent);
			MPI_Send(&berserk_id, 1, MPI_INT, my_node_state->node_data->parent, RESOURCE_TRANSFER_TAG, MPI_COMM_WORLD);
		}
	}

	sleep(1); //just to pretty printing print
	sleep(rand_1_to_bound(5)); //just to pretty printing print
	if(world_rank == 1 || world_rank == 2 || world_rank == 3){
		organize_meeting();
	}


	void *status;
    pthread_join(transfer_listener_id, &status);
	MPI_Finalize();
}
