#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "node_structure.h"
#include "node_state.h"
#include "utils.h"

void get_field(char* line, char* buff, int num){
    int len = strlen(line);
    int l_s = num;
    int lowe_sep_index = 0;
    {
        int i=0;
        int count = 0;
        for(i=0;i< len; i++){
            if(line[i] == ';'){
                count++;
            }
            if(count == l_s){
                lowe_sep_index = i+1;
                break;
            }
        }
    }
    int col_len = 0;
    int j =lowe_sep_index;
    while(line[j] != ';'){
        col_len++;
        j++;
    }
    memcpy(buff,&line[lowe_sep_index], col_len * sizeof(char));
    buff[col_len] = '\0';
}

int* column_to_int_arr(char* column){
    if(column[0] == 'x'){
        return NULL;
    }
    int i = 0;
    int sep_count = 0;
    while(column[i] != '\0'){
        if(column[i] == ','){
            sep_count++;
        }
        i++;
    }
    int len = sep_count + 1;
    int* arr = malloc(len * sizeof(int));

    int j =0;
    char* token = strtok(column, ",");
    while(token){
        arr[j] = atoi(token);
        j++;
        token = strtok(0,",");
    }
    return arr;

}

int find_id(char* line){
        char buff[3];
        get_field(line,buff,1);
        int id;
        if(column_to_int_arr(buff) == NULL){
            id = -1;
        }
        else{
            id = column_to_int_arr(buff)[0];
        }
        return id;
}

int* find_siblings(char* line){
    char buff[20];
    get_field(line,buff, 3);
    return column_to_int_arr(buff);
}

int find_parent(char* line){
    char buff[20];
    get_field(line, buff, 2);
    int parent;
    if(column_to_int_arr(buff) == NULL){
        parent = -1;
    }
    else{
        parent = column_to_int_arr(buff)[0];
    }
    return parent;
}

int* find_children(char* line){
    char buff[20];
    get_field(line, buff, 4);
    return column_to_int_arr(buff);
}

void print_arr(int * arr, char* arr_name){
    printf("%s = ", arr_name);
    if(arr == NULL){
        printf("none ");
        return;
    }
    int len = arr_len(arr);
    int i = 0;
    for(i=0;i<len;i++){
        printf("%d, ", arr[i]);
    }
}
void print_node(node* n){
    print_arr(n->siblings, "siblings");
    print_arr(n->children, "children");
    printf("parent = %d ", n->parent);
    printf("id = %d\n", n->id);
    
}
node* load_node(int pid)
{
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    node* my_node = malloc(sizeof(node));
    fp = fopen("node_hierarchy", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {
        int id = find_id(line);
        if(pid == id){
            int* children = find_children(line);
            int parent = find_parent(line);
            int * siblings = find_siblings(line);
            my_node->id = id;
            my_node->parent = parent;
            my_node->children = children;
            my_node->siblings = siblings;
            my_node->children_length = arr_len(children);
            my_node->siblings_length = arr_len(siblings);

        }
    }

    fclose(fp);
    if (line)
        free(line);
    return my_node;
}
node_state* init_node(int node_id){
    node* my_node = load_node(node_id);
    print_node(my_node);
    node_state* my_node_state = malloc(sizeof(node_state));
    my_node_state->node_data=my_node;
    my_node_state->resource_owner=my_node->parent;
    my_node_state->resource_request_fifo = malloc(sizeof(fifo));
    sleep(1); //just for print
    return my_node_state;
}
