#include <stdio.h>
#include <stdlib.h>
#include <time.h>
int random_resource_owner(int bound){
	time_t tt;
	srand(time(&tt));
	return (rand() % (bound-1)) +1; // <1,n)
}