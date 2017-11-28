#ifndef __EX_RBTREE_H_
#define __EX_RBTREE_H_

#include <stdio.h>
#include <stdlib.h>
#include "rbtree.h"

struct data_type{
    int id;
    ///other type you need to define
    ///...
};


//节点类型
struct node_type {
    struct rb_node  my_node;
    struct data_type *data_content;
};


struct rb_root rbtree_init();
struct node_type *rbtree_search(struct rb_root *root, struct data_type *data_content);
int rbtree_insert(struct rb_root *root, struct data_type *data_content);
void rbtree_delete(struct rb_root *root,struct data_type *data_content);
void print_rbtree(struct rb_root *tree);



#endif // __EX_RBTREE_H_
