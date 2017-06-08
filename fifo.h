#ifndef FIFO_H
#define FIFO_H

typedef struct{
    int elements[100]; // kolejka pomiesci n-1 
    int start;
    int end;
}fifo;
int put(fifo* queue, int elem);
int pop(fifo* queue);

#endif