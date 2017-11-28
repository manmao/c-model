#include "ex-rbtree.h"


struct rb_root rbtree_init()
{
   struct rb_root mytree = RB_ROOT;
   return mytree;
}

//Êı¾İ·ÖÅä
struct node_type *rbtree_search(struct rb_root *root, struct data_type *data_content)
{
    struct rb_node *node = root->rb_node;
    while (node) {

        struct node_type *data = container_of(node, struct node_type, my_node);

        if (data_content->id <  data->data_content->id)
            node = node->rb_left;

        else if (data_content->id > data->data_content->id)
            node = node->rb_right;

        else
            return data;
    }
    return NULL;
}

int rbtree_insert(struct rb_root *root, struct data_type *data_content)
{
    struct node_type *data=(struct node_type *)calloc(1,sizeof(struct node_type));
    data->data_content=data_content;

    struct rb_node **tmp = &(root->rb_node), *parent = NULL;

    /* Figure out where to put new node */
    while (*tmp) {
        struct node_type *thiz = container_of(*tmp, struct node_type, my_node);

        parent = *tmp;

        if ((data->data_content->id)  <  (thiz->data_content->id))
            tmp = &((*tmp)->rb_left);
        else if ((data->data_content->id)  > (thiz->data_content->id))
            tmp = &((*tmp)->rb_right);
        else
            return -1;
    }
    /* Add new node and rebalance tree. */
    rb_link_node(&data->my_node, parent, tmp);
    rb_insert_color(&data->my_node, root);

    return 0;
}

void rbtree_delete(struct rb_root *root, struct data_type *data_content)
{
    struct node_type *data = rbtree_search(root, data_content);

    if (!data) {
        fprintf(stderr, "Not found %d.\n", data_content->id);
	    return;
    }

    rb_erase(&data->my_node, root);

    free(data->data_content);
    free(data);
}

void print_rbtree(struct rb_root *tree)
{
    struct rb_node *node;

    for (node = rb_first(tree); node; node = rb_next(node))
	printf("%d ", rb_entry(node, struct node_type, my_node)->data_content->id);

    printf("\n");
}
