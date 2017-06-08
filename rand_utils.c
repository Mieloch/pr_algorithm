#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int rand_1_to_bound(int bound){
	time_t tt;
	srand(time(&tt));
	return (rand() % (bound-1)) +1; // <1,n)
}