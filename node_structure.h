#ifndef NODE_STRUCTURE
#define NODE_STRUCTURE

typedef struct NODE {
    int parent;
    int children[3];
    int siblings[3];
    int children_length;
    int siblings_length;
    char is_acceptor;
    char has_dvd;
} NODE;

#endif