#include "load_node.h"
#include "utils.h"
#include "fifo.h"
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>


#define RESOURCE_OWNER_TAG 100
#define RESOURCE_TRANSFER_TAG 101
#define ORGANIZE_MEETING 102
#define START_MEETING 103
#define END_MEETING 104
#define ACCEPTANCE_REQUEST_TAG 105
#define ASSIGNED_ACCEPTANCE_TAG 106
#define RESOURCE_REQUEST_TAG 107
#define RESOURCE_ASSIGN_TAG 108
#define BERSERK 666
pthread_mutex_t resource_mutex;
pthread_mutex_t using_resource_mutex;

pthread_mutex_t acceptance_fifo_mutex;
pthread_mutex_t resource_fifo_mutex;
pthread_mutex_t sent_resource_mutex;
pthread_mutex_t acceptance_receive_mutex;
pthread_cond_t acceptance_receive_cv;
pthread_cond_t resource_cv;

node_state* my_node_state;
//LISTENERS
void* resource_transfer_listener(void * t){ // nasluchiwanie na przekazywanie zasobu w gore 
	int resource_id;
	MPI_Status status;
	node* my_node = my_node_state->node_data;
	while(1){
    	MPI_Recv(&resource_id, 1, MPI_INT, MPI_ANY_SOURCE, RESOURCE_TRANSFER_TAG, MPI_COMM_WORLD, &status); 
    	printf("[RESOURCE_TRANSFER] Process[%d] recived berserk from process[%d]\n", my_node->id, status.MPI_SOURCE);
		if(my_node->id == 0){
			my_node_state->resource_owner = 0;
		}else{
			printf("[RESOURCE_TRANSFER] Process[%d] pass resource to parent[%d]\n", my_node->id,my_node->parent);
			MPI_Send(&resource_id, 1, MPI_INT, my_node->parent, RESOURCE_TRANSFER_TAG, MPI_COMM_WORLD);
			my_node_state->resource_owner = my_node->parent;
		}
	}
	pthread_exit(NULL);
}
void* meeting_acc_listener(void* t){ //nasluchiwanie na akceptacje spotkania
	MPI_Status status;
	node* my_node = my_node_state -> node_data;
	int ack_count = 0;
	int number = 1;
	while(ack_count < my_node->siblings_length){
		printf("[MEETING_ACK] Process[%d] need %d more ack\n", my_node->id, my_node->siblings_length-ack_count);
    	MPI_Recv(&number, 1, MPI_INT, MPI_ANY_SOURCE, ORGANIZE_MEETING, MPI_COMM_WORLD, &status); 
		ack_count++;
		printf("[MEETING_ACK] Process[%d] got meeting ack from process[%d]\n", my_node->id,status.MPI_SOURCE);
	}
	pthread_exit(NULL);
}

void* resource_assign_listener(void* t){
	int number;
	MPI_Status status;
	while(1){
		MPI_Recv(&number, 1, MPI_INT, MPI_ANY_SOURCE, RESOURCE_ASSIGN_TAG, MPI_COMM_WORLD, &status); 
		pthread_mutex_lock(&resource_fifo_mutex);
		int first_in_queue =  pop(my_node_state->resource_request_fifo);
		pthread_mutex_unlock(&resource_fifo_mutex);	
		
		if(first_in_queue == my_node_state->node_data->id){
			printf("[RESOURE_ASSIGN] Process[%d] receive his resource assignment\n", my_node_state->node_data->id);
			pthread_mutex_lock(&resource_mutex);
			my_node_state->resource_owner = my_node_state->node_data->id;
   			pthread_cond_signal(&resource_cv);
   			
			pthread_mutex_unlock(&resource_mutex);

   		}
   		else{
   			pthread_mutex_lock(&resource_mutex);
   			printf("[RESOURE_ASSIGN] Process[%d] receive resource acceptance from process[%d] and pass it to process[%d]\n", my_node_state->node_data->id,status.MPI_SOURCE, first_in_queue);
   			MPI_Send(&number, 1, MPI_INT, first_in_queue, RESOURCE_ASSIGN_TAG, MPI_COMM_WORLD);   	
	
			my_node_state->resource_owner = first_in_queue;
   			
			pthread_mutex_lock(&resource_fifo_mutex);
			while(first_in_queue = pop(my_node_state->resource_request_fifo) != -1){
				printf("[RESOURE_REQUEST] Process[%d] pass resource request to process[%d]\n", my_node_state->node_data->id, my_node_state->resource_owner);
   				MPI_Send(&number, 1, MPI_INT, my_node_state->resource_owner, RESOURCE_REQUEST_TAG, MPI_COMM_WORLD);   	
			} 
			pthread_mutex_unlock(&resource_fifo_mutex);

		pthread_mutex_lock(&sent_resource_mutex);
		my_node_state->sent_resource_request = 0;
		pthread_mutex_unlock(&sent_resource_mutex);


			pthread_mutex_unlock(&resource_mutex);
   		}

	}
		
}


void* resource_request_listener(void* t){ // nasluchiwanie na prosby o zasob
	int number;
	MPI_Status status;
	while(1){
		MPI_Recv(&number, 1, MPI_INT, MPI_ANY_SOURCE, RESOURCE_REQUEST_TAG, MPI_COMM_WORLD, &status); 



		printf("[RESOURE_REQUEST] Process[%d] received resource request from process[%d]\n", my_node_state->node_data->id, status.MPI_SOURCE);
		pthread_mutex_lock(&sent_resource_mutex);
		if(my_node_state->node_data->id != my_node_state->resource_owner){
			pthread_mutex_lock(&resource_fifo_mutex);
	   		put(my_node_state->resource_request_fifo, status.MPI_SOURCE);
			pthread_mutex_unlock(&resource_fifo_mutex);

			if(my_node_state->sent_resource_request == 0){
				printf("[RESOURE_REQUEST] Process[%d] pass resource request to process[%d]\n", my_node_state->node_data->id, my_node_state->resource_owner);
	   			MPI_Send(&number, 1, MPI_INT, my_node_state->resource_owner, RESOURCE_REQUEST_TAG, MPI_COMM_WORLD);   	
				my_node_state->sent_resource_request = 1;
			}
		}else if(my_node_state->node_data->id == my_node_state->resource_owner){
			printf("Resource owner process[%d] got resource request\n",my_node_state->node_data->id);
			pthread_mutex_lock(&using_resource_mutex);
			//sekcja
			pthread_mutex_lock(&resource_fifo_mutex);
   			int first_in_queue = pop(my_node_state->resource_request_fifo);
			pthread_mutex_unlock(&resource_fifo_mutex);

			printf("[RESOURCE_ASSIGN] Process[%d] surrender resource and pass it to process[%d]\n", my_node_state->node_data->id, status.MPI_SOURCE);
			MPI_Send(&number, 1, MPI_INT, status.MPI_SOURCE, RESOURCE_ASSIGN_TAG, MPI_COMM_WORLD);   	
			my_node_state->resource_owner = status.MPI_SOURCE;

			pthread_mutex_unlock(&using_resource_mutex);
			pthread_mutex_lock(&resource_fifo_mutex);
			while(first_in_queue = pop(my_node_state->resource_request_fifo) != -1){
				printf("[RESOURE_REQUEST] Process[%d] pass resource request to process[%d]\n", my_node_state->node_data->id, my_node_state->resource_owner);
   				MPI_Send(&number, 1, MPI_INT, my_node_state->resource_owner, RESOURCE_REQUEST_TAG, MPI_COMM_WORLD);   	
			} 
			pthread_mutex_unlock(&resource_fifo_mutex);
		
		}	
		pthread_mutex_unlock(&sent_resource_mutex);

	}
}


void request_for_resource(){
		int number = 1;
	   	pthread_mutex_lock(&resource_fifo_mutex);
   		put(my_node_state->resource_request_fifo, my_node_state->node_data->id);
		pthread_mutex_unlock(&resource_fifo_mutex);

		printf("[RESOURCE_REQUEST] process[%d] ask process[%d] for resource\n", my_node_state->node_data->id, my_node_state->resource_owner);
		
		MPI_Send(&number, 1, MPI_INT, my_node_state->resource_owner, RESOURCE_REQUEST_TAG, MPI_COMM_WORLD);

   		pthread_mutex_lock(&resource_mutex);
   		while(my_node_state->resource_owner != my_node_state->node_data->id){
   			pthread_cond_wait(&resource_cv, &resource_mutex);
   		}
	   	pthread_mutex_lock(&using_resource_mutex);
	
		pthread_mutex_unlock(&resource_mutex);

		printf("[RESOURCE_REQUEST] Process[%d] got resource\n", my_node_state->node_data->id);
}



void* acceptance_request_listener(void* t){ // nasluchiwanie na prosby o zgode
	int number;
	MPI_Status status;
	while(1){
		MPI_Recv(&number, 1, MPI_INT, MPI_ANY_SOURCE, ACCEPTANCE_REQUEST_TAG, MPI_COMM_WORLD, &status); 
		if(my_node_state->node_data->id == 0){
			printf("[ACCEPTANCE_REQUEST] Acceptor process[%d] received acceptance request from process[%d]\n", my_node_state->node_data->id, status.MPI_SOURCE);
			MPI_Send(&number, 1, MPI_INT, status.MPI_SOURCE, ASSIGNED_ACCEPTANCE_TAG, MPI_COMM_WORLD);
		}
		else{
			pthread_mutex_lock(&acceptance_fifo_mutex);
			//printf("[start]put acceptance_request_fifo\n");
			put(my_node_state->acceptance_request_fifo, status.MPI_SOURCE);
			//printf("[end]put acceptance_request_fifo\n");
			pthread_mutex_unlock(&acceptance_fifo_mutex);
			
			printf("[ACCEPTANCE_REQUEST] Process[%d] received acceptance request from process[%d] and pass it to process[%d]\n", my_node_state->node_data->id, status.MPI_SOURCE, my_node_state->node_data->parent);
			MPI_Send(&number, 1, MPI_INT, my_node_state->node_data->parent, ACCEPTANCE_REQUEST_TAG, MPI_COMM_WORLD);
		}
	}
}



void* aassigned_acceptance_listener(void* t){ // nasluchiwanie na uzyskane zgody
	int adress;
	MPI_Status status;
	while(1){
		MPI_Recv(&adress, 1, MPI_INT, MPI_ANY_SOURCE, ASSIGNED_ACCEPTANCE_TAG, MPI_COMM_WORLD, &status); 
		printf("[ASSIGNED_ACCEPTANCE] Process[%d] received assigned acceptance from process[%d]\n", my_node_state->node_data->id, status.MPI_SOURCE);
		pthread_mutex_lock(&acceptance_fifo_mutex);
		//printf("[start]pop acceptance_request_fifo\n");
		sleep(3);
		//print_arr(my_node_state->acceptance_request_fifo->elements, "request_fifo");
		int first_in_queue = pop(my_node_state->acceptance_request_fifo);
		//printf("[end]pop acceptance_request_fifo\n");

		pthread_mutex_unlock(&acceptance_fifo_mutex);
		
		if(first_in_queue == my_node_state->node_data->id){
			pthread_mutex_lock(&acceptance_receive_mutex);
			my_node_state->wait_for_acceptance = 0;
			pthread_cond_signal(&acceptance_receive_cv);
			pthread_mutex_unlock(&acceptance_receive_mutex);


		}else{
			printf("[ASSIGNED_ACCEPTANCE] Process[%d] pass acceptance to to process[%d]\n", my_node_state->node_data->id, first_in_queue);
			MPI_Send(&adress, 1, MPI_INT, first_in_queue, ASSIGNED_ACCEPTANCE_TAG, MPI_COMM_WORLD);
		}
	}
}

void wait_for_acceptor_acceptance(){
		int number = 1;
	   	pthread_mutex_lock(&acceptance_fifo_mutex);
   		put(my_node_state->acceptance_request_fifo, my_node_state->node_data->id);
		pthread_mutex_unlock(&acceptance_fifo_mutex);
		printf("[ACCEPTANCE_REQUEST] process[%d] is waiting for acceptance\n", my_node_state->node_data->id);
		MPI_Send(&number, 1, MPI_INT, my_node_state->node_data->id, ACCEPTANCE_REQUEST_TAG, MPI_COMM_WORLD);
   		
   		pthread_mutex_lock(&acceptance_receive_mutex);
   		my_node_state->wait_for_acceptance = 1;
   		while(my_node_state->wait_for_acceptance ==1){
   			pthread_cond_wait(&acceptance_receive_cv, &acceptance_receive_mutex);
   		}	
		pthread_mutex_unlock(&acceptance_receive_mutex);
		printf("[ASSIGNED_ACCEPTANCE] Process[%d] got his acceptance\n", my_node_state->node_data->id);
}


void wait_for_all_meeting_ack(){
		node* my_node = my_node_state -> node_data;
	if(my_node->siblings_length == 0){
		printf("[ORGANIZE_MEETING] Process[%d] cant organize meeting alone\n", my_node->id);
		return;
	}
	int i;
	int number = -1;
	int transfer_listener_id  = create_listener(meeting_acc_listener);


	for(i=0;i<my_node->siblings_length;i++){
		printf("[ORGANIZE_MEETING] Process[%d] asking process[%d] for meeting\n", my_node->id,my_node->siblings[i]);
		MPI_Send(&number, 1, MPI_INT, my_node->siblings[i], ORGANIZE_MEETING, MPI_COMM_WORLD);
	}

	void *status;
    pthread_join(transfer_listener_id, &status);
}

void take_part_in_meeting(){
		node* my_node = my_node_state -> node_data;
		int number = -1;
	   	MPI_Status status;
   		printf("[ORGANIZE_MEETING] Process[%d] wait for start meeting\n", my_node->id);
    	MPI_Recv(&number, 1, MPI_INT, MPI_ANY_SOURCE, START_MEETING, MPI_COMM_WORLD, &status); 
   		printf("[ORGANIZE_MEETING] Process[%d] start meeting arrived from process[%d]\n",my_node->id, status.MPI_SOURCE);
   	 	//pal kuce
   		MPI_Recv(&number, 1, MPI_INT, MPI_ANY_SOURCE, END_MEETING, MPI_COMM_WORLD, &status);
   		printf("[ORGANIZE_MEETING] Process[%d] end meeting arrived from process[%d]\n",my_node->id, status.MPI_SOURCE);
}
void organize_meeting(){
	node* my_node = my_node_state -> node_data;
	int number = -1;
	int i;
   	printf("[ORGANIZE_MEETING] Process[%d] is organizator\n", my_node->id);
		
	wait_for_acceptor_acceptance();
	request_for_resource();
		//todo wait_for_resource

	for(i=0;i<my_node->siblings_length;i++){
		printf("[ORGANIZE_MEETING] Process[%d] send start meeting to process[%d]\n", my_node->id,my_node->siblings[i]);
		MPI_Send(&number, 1, MPI_INT, my_node->siblings[i], START_MEETING, MPI_COMM_WORLD);
	}
	//todo byc moze inne procesy musza potwierdzic start i czekamy na ich ack
	sleep(10); //pal kuce
	for(i=0;i<my_node->siblings_length;i++){
		printf("[ORGANIZE_MEETING] Process[%d] send end meeting to process[%d]\n", my_node->id,my_node->siblings[i]);
		MPI_Send(&number, 1, MPI_INT, my_node->siblings[i], END_MEETING, MPI_COMM_WORLD);
	}
	// release



	pthread_mutex_unlock(&using_resource_mutex);

}


void find_meeting(){
	node* my_node = my_node_state -> node_data;
	int number = -1;
	int i;
	wait_for_all_meeting_ack();

   	printf("[MEETING_ACK] Process[%d] has all ack\n",my_node->id);
   	if(my_node->id < min_from_arr(my_node->siblings)){
   		organize_meeting();
   	}
   	else{
   		take_part_in_meeting();
   	}
   	printf("[ORGANIZE_MEETING] Process[%d] meeting is over\n", my_node->id);

}


void b_cast_resource_owner(int world_size){
	int owner_pid = rand_1_to_bound(world_size);
	printf("Resource owner pid is = %d\n", owner_pid);
	int i=1;
	for(i=1;i<world_size;i++){
		MPI_Send(&owner_pid, 1, MPI_INT, i, RESOURCE_OWNER_TAG, MPI_COMM_WORLD);
	}
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
    pthread_mutex_init(&resource_mutex, NULL);
	pthread_mutex_init(&acceptance_fifo_mutex,NULL);
	pthread_mutex_init(&acceptance_receive_mutex,NULL);
	pthread_mutex_init(&sent_resource_mutex, NULL);
	pthread_mutex_init(&using_resource_mutex, NULL);

	pthread_cond_init(&acceptance_receive_cv, NULL);
	pthread_cond_init(&resource_cv, NULL);

	my_node_state= init_node(world_rank);
	int resource_transfer_listener_id  = create_listener(resource_transfer_listener);
	int acceptance_request_listener_id  = create_listener(acceptance_request_listener);
	int aassigned_acceptance_listener_id  = create_listener(aassigned_acceptance_listener);
	int resource_request_listener_id = create_listener(resource_request_listener);
	int resource_assign_listener_id = create_listener(resource_assign_listener);


	if (world_rank == 0) {
		b_cast_resource_owner(world_size);
	} else{
		int owner_pid;
		MPI_Recv(&owner_pid, 1, MPI_INT, 0, RESOURCE_OWNER_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
		//printf("Process %d received recource owner = %d from process 0\n", world_rank, owner_pid);
		if(owner_pid == world_rank){
			int berserk_id = BERSERK;
			printf("[RESOURCE_TRANSFER] Process[%d] send berserk to parent[%d]\n", world_rank, my_node_state->node_data->parent);
			MPI_Send(&berserk_id, 1, MPI_INT, my_node_state->node_data->parent, RESOURCE_TRANSFER_TAG, MPI_COMM_WORLD);
		}
	}

	sleep(1); //just to pretty printing print
	sleep(rand_1_to_bound(5)); //just to pretty printing print

	int k = 0;
	while(k<2){
		if(world_rank == 4 || world_rank == 5 || world_rank == 6 || world_rank == 12 || world_rank == 11 || world_rank == 10){
			find_meeting();
		}
		sleep(rand_1_to_bound(5));
		k++;
	}
	



	void *status;
    pthread_join(resource_transfer_listener_id, &status);
    pthread_join(acceptance_request_listener_id, &status);
    pthread_join(aassigned_acceptance_listener_id, &status);

	pthread_mutex_destroy(&acceptance_fifo_mutex);
	pthread_mutex_destroy(&resource_mutex);
	pthread_mutex_destroy(&acceptance_receive_mutex);
	pthread_cond_destroy(&acceptance_receive_cv);

	MPI_Finalize();
}
