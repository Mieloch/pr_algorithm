#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utils.h"
#include <pthread.h>
#include <mpi.h>

int rand_1_to_bound(int bound){
	time_t tt;
	srand(time(&tt));
	return (rand() % (bound-1)) +1; // <1,n)
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

int arr_len(int* arr){
    if(arr == NULL){
        return 0;
    }
    return sizeof(arr)/sizeof(arr[0]);
}

int create_listener(void* listener_function){
	 pthread_t t;
     int rc = pthread_create(&t, NULL, listener_function, (void *)t);
		if (rc){
		      printf("ERROR; return code from pthread_create() is %d\n", rc);
		      exit(-1);
		}
    return t;
}


void print_arr(int * arr, char* arr_name){
    printf("%s = ", arr_name);
    if(arr == NULL){
        printf("none ");
        return;
    }
    int len = arr_len(arr);
    int i = 0;
    for(i=0;i<len;i++){
        printf("%d, ", arr[i]);
    }
}

int min_from_arr(int* arr){
	if(arr == NULL){
		return -1;
	}
	int i=0;
	int min = arr[0];
	for(i=1;i<arr_len(arr);i++){
		if(arr[i] < min){
			min = arr[i];
		}
	}
	return min;
}

