#ifndef NODE_STATE_STRUCTURE_H
#define NODE_STATE_STRUCTURE_H
#include "node_structure.h"
#include "fifo.h"
// strutkura pelniaca role pamieci globalnej pomiedzy watkami, tutaj chyba kolejki, wskaznik na wlasciciela tokena, itd
typedef struct{
    node* node_data;
    int resource_owner;
    int sent_resource_request;
    int wait_for_acceptance;
    fifo* resource_request_fifo;
    fifo* acceptance_request_fifo;
}node_state;

#endif