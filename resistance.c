#include "load_node.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>

int main(void){
    node* my_node = load_node();
    print_node(my_node);
    return 0;
}