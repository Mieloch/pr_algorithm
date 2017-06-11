
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
		if((queue->end +1)%10 == queue->start){
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
// int main(){
// 	fifo* queue = malloc(sizeof(fifo));
// 	printf("pop %d\n", pop(queue));
// 		printf("pop %d\n", pop(queue));

// 	// printf("put %d\n", put(queue,2));;
// 	// printf("put %d\n", put(queue,3));;
// 	// printf("put %d\n", put(queue,4));;
// 	// printf("put %d\n", put(queue,5));;
// 	// printf("pop %d\n", pop(queue));
// 	// printf("pop %d\n", pop(queue));
// 	// printf("pop %d\n", pop(queue));
// 	// printf("put %d\n", put(queue,1));
// 	// printf("put %d\n", put(queue,2));;
// 	// printf("put %d\n", put(queue,3));;
// 	// printf("put %d\n", put(queue,4));;
// 	// printf("put %d\n", put(queue,5));;
// 	// printf("pop %d\n", pop(queue));
// 	// printf("pop %d\n", pop(queue));
// 	// printf("pop %d\n", pop(queue));
// 	// printf("pop %d\n", pop(queue));
// 	// printf("pop %d\n", pop(queue));
// 	// 	printf("put %d\n", put(queue,1));
// 	// printf("put %d\n", put(queue,2));;
// 	// printf("put %d\n", put(queue,3));;
// 	// printf("put %d\n", put(queue,4));;
// 	// printf("put %d\n", put(queue,5));;
// 	// printf("pop %d\n", pop(queue));
// 	// printf("pop %d\n", pop(queue));
// 	// printf("pop %d\n", pop(queue));
// 	// printf("pop %d\n", pop(queue));
// 	// printf("pop %d\n", pop(queue));


// }

 
