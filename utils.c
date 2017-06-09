#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int rand_1_to_bound(int bound){
	time_t tt;
	srand(time(&tt));
	return (rand() % (bound-1)) +1; // <1,n)
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

int arr_len(int* arr){
    if(arr == NULL){
        return 0;
    }
    return sizeof(arr)/sizeof(arr[0]) +1;
}