#include "node_structure.h"
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void send(int data, int receiver, int my_id) {
    pid_t pid = fork();
    if(pid == 0) {
        printf("%d: Sending number %d to %d\n", my_id, data, receiver);
        MPI_Send(&data, 1, MPI_INT, receiver, 0, MPI_COMM_WORLD);
        printf("%d: Number sent\n", my_id);
    } else {

    }
}

int main(int argc, char** argv) {
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);
    // Find out rank, size
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // We are assuming at least 2 processes for this task
    if (world_size < 4) {
        fprintf(stderr, "World size must be greater than 3 for %s\n", argv[0]);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }


    struct NODE my_node;
    int number = 0;
    if (world_rank == 0) {

        my_node = (NODE){.children = {1,2,3}, .is_acceptor = 'y', .has_dvd = 'n', .children_length = 3, .siblings_length = 0};

    } else if (world_rank == 1) {

        my_node = (NODE){.siblings = {2,3}, .is_acceptor = 'n', .has_dvd = 'y', .children_length = 0, .siblings_length = 2, .parent = 0};

    } else if (world_rank == 2) {

        my_node = (NODE){.siblings = {1,3}, .is_acceptor = 'n', .has_dvd = 'n', .children_length = 0, .siblings_length = 2, .parent = 0};

    } else if (world_rank == 3) {

        my_node = (NODE){.siblings = {1,2}, .is_acceptor = 'n', .has_dvd = 'n', .children_length = 0, .siblings_length = 2, .parent = 0};
    }

    if (my_node.is_acceptor == 'y') {
        number = 1;
        for(int i = 0; i < my_node.children_length; i++) {
            send(number, my_node.children[i], world_rank);
        }
    } else if (my_node.is_acceptor == 'n') {
        MPI_Recv(&number, 1, MPI_INT, my_node.parent, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("%d: Received number %d from %d\n", world_rank, number, my_node.parent);
    }

    MPI_Finalize();
}