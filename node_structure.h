#ifndef NODE_STRUCTURE
#define NODE_STRUCTURE

typedef struct{
    int id;
    int parent;
    int* children;
    int* siblings;
    int children_length;
    int siblings_length;
}node;

#endif
