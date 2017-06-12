#ifndef RAND_UTILS_H
#define RAND_UTILS_H

int rand_1_to_bound(int bound);
int min_from_arr(int* arr, int size);
void print_arr(int * arr, char* arr_name, int size);
int create_listener(void* listener_function);
void check_world_size(int size);
void check_thread_support(int provided);
#endif