
#include <stdio.h>
#include <stdlib.h>
#include "fifo.h"
#include "utils.h"
int put(fifo* queue, int elem) 
{
	if(queue->end == queue->start) //tylko gdy pusta
	{
		queue->elements[queue->end] = elem;
		queue->end = (queue->end +1)%10;
	}else{
		if((queue->end+1)%10 == queue->start){
			return -1; //pelna
		}else{
			queue->elements[queue->end] = elem;
			queue->end = (queue->end +1)%10;
		}
	}
	return 1;
}

int pop(fifo* queue)
{ 
	if(queue->start == queue->end){
		return -1; //pusta
	}
	// printf("fifo pop start = %d\n", queue->start);
	int first = queue->elements[queue->start];
	queue->start = (queue->start +1)%10;
	return first;
}

int fifo_is_empty(fifo* queue) {
	if(queue->end == queue->start){
		return 1;
	}
	else{
		return 0;
	}
}
void print_fifo(fifo* queue){
	int i = queue->start;
	int j = queue->end;
	int elem;
	while(i != j){
			 elem = queue->elements[i];
			i = (i +1)%10;
			printf("%d, ", elem);
	}
	printf("\n");
	// printf("fifo pop start = %d\n", queue->start);
}
// int main(){
// 	fifo* queue = malloc(sizeof(fifo));
// 	queue->start=0;
// 	queue->end=0;

// 	printf("put %d\n", put(queue,2));;
// 	// printf("put %d\n", put(queue,3));;
// 		// printf("put %d\n", put(queue,33));;
// // 
// 	// printf("put %d\n", put(queue,55));;;

// 	print_fifo(queue);


// }

 