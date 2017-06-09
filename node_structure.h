#ifndef NODE_STRUCTURE
#define NODE_STRUCTURE

typedef struct{
    int id;
    int parent;
    int children[3];
    int siblings[3];
    int children_length;
    int siblings_length;
}node;

#endif
