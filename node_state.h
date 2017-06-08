#ifndef NODE_STATE_STRUCTURE_H
#define NODE_STATE_STRUCTURE_H
#include "node_structure.h"
// strutkura pelniaca role pamieci globalnej pomiedzy watkami, tutaj chyba kolejki, wskaznik na wlasciciela tokena, itd
typedef struct{
    node* node_data;
    int resource_owner;
}node_state;

#endif