#include "load_node.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>

int main(void){
    node* my_node = load_node();
    print_arr(my_node->siblings, "siblings");
    print_arr(my_node->children, "children");
    printf("parent = %d ", my_node->parent);
    printf("id = %d", my_node->id);
    printf("\n\n");
    return 0;
}