#ifndef NODE_STRUCTURE
#define NODE_STRUCTURE

struct node {
    int parent;
    int * children;
    int * siblings;
    int children_length;
    int siblings_length;
};

#endif