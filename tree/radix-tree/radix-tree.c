#include<malloc.h>
#include<stdio.h>
#include "radix-tree.h"

static radix_tree_node* radix_tree_node_alloc()
{
    radix_tree_node *ret;
    ret=(radix_tree_node *)calloc(1,sizeof(struct radix_tree_node));
    if(!ret){
        printf("calloc radix_tree_node failed\n");
    }

    return ret;
}

static void radix_tree_node_free(radix_tree_node *node)
{
    free(node);
}

static unsigned int radix_tree_maxindex(radix_tree_head *head)
{
    return head->height_to_maxindex[head->height];
}

static unsigned int __maxindex(unsigned int height)
{
    unsigned int tmp =  height * RADIX_TREE_MAP_SHIFT;

    unsigned int index = (~0U >> (RADIX_TREE_INDEX_BITS - tmp -1)) >> 1;

    if(tmp >= RADIX_TREE_INDEX_BITS)
    {
        index = ~0;
    }

    return index;
}

static void radix_tree_init_maxindex(radix_tree_head *head)
{
    unsigned int i;
    for(i=0; i< RADIX_TREE_MAX_PATH;i++)
    {
        head->height_to_maxindex[i]=__maxindex(i);
    }
}

static int radix_tree_extend(radix_tree_head *head,unsigned int index)
{
    radix_tree_node *node;
    unsigned int height;

    height= head->height+1;

    while(index > head->height_to_maxindex[height])
    {
        height++;
        if(height > RADIX_TREE_MAX_PATH)
        {
            printf("index are out of the capacity of radix tree\n");
        }
    }

    if(head->rnode)
    {
        do{
            if(!(node = radix_tree_node_alloc()))
            {
                return 1;
            }

            node->slots[0] = head->rnode;
            node->count=1;
            head->rnode=node;
            head->height++;

        }while(height > head->height);
    }
    else
    {
        head->height = height;
    }

    return 0;
}

int radix_tree_insert(radix_tree_head *head,unsigned int index,void *item)
{
    radix_tree_node *node=NULL,*tmp;
    radix_tree_node **slot;
    unsigned int height,shift;
    int error;

    // Make sure tree is hight enough
    if((index > radix_tree_maxindex(head))
       || (!radix_tree_maxindex(head)))
    {
        error = radix_tree_extend(head,index);
        if(error)
        {
            return error;
        }
    }

    slot = &head->rnode;
    height=head->height;
    shift = (height -1) *RADIX_TREE_MAP_SHIFT;

    while(height > 0){
        if(*slot == NULL)
        {
            //Have to add a child node
            if(!(tmp = radix_tree_node_alloc()))
            {
                return 1;
            }

            *slot = tmp;
            if(node)
            {
                node->count++;
            }
        }
        /*Go a level down*/
        node = *slot;
        slot = (radix_tree_node **)(node->slots+((index >> shift) & RADIX_TREE_MAP_MASK));
        shift -= RADIX_TREE_MAP_SHIFT;
        height--;
    }

    if(*slot != NULL)
    {
        printf("slot have been occupied\n");
        return 1;
    }

    if(node)
    {
        node->count++;
    }

insert_succeed:
    *slot=item;
    return 0;
}


void * radix_tree_lookup(radix_tree_head *head,unsigned int index)
{
    unsigned int height,shift;
    radix_tree_node **slot;

    height=head->height;

    if(index > radix_tree_maxindex(head))
    {
        printf("index are out of range\n");
        return NULL;
    }

    shift = (height -1) * RADIX_TREE_MAP_SHIFT;
    slot= &head->rnode;

    while(height > 0)
    {
        if(*slot == NULL)
        {
            printf("index does not exist,its empty\n");
            return NULL;
        }

        slot = (radix_tree_node **)((*slot)->slots+((index >> shift) & RADIX_TREE_MAP_MASK));
        shift -= RADIX_TREE_MAP_SHIFT;
        height--;
    }

    return (void *)(*slot);
}

static unsigned int __lookup(radix_tree_head *head,
                             void **results,
                             unsigned int first_index,
                             unsigned int max_items,
                             unsigned int *next_index)
{

    unsigned int nr_found=0,index=first_index;
    unsigned int shift;
    unsigned int height = head->height;
    radix_tree_node *slot;

    shift=(height -1 )*RADIX_TREE_MAP_SHIFT;
    slot=head->rnode;

    while(height > 0)
    {
        unsigned int i=(index >> shift) & RADIX_TREE_MAP_MASK;
        height--;
        if(height &&!slot->slots[i])
        {
            printf("bug,please check!!\n");
        }
        if(height == 0)
        {
            unsigned int j = index & RADIX_TREE_MAP_MASK;
            for(;j<RADIX_TREE_MAP_SIZE;j++)
            {
                index++;
                if(slot->slots[j])
                {
                    results[nr_found++] = slot->slots[j];
                    if(nr_found == max_items)
                    {
                        index = 0;
                        goto out;
                    }
                }
            }
        }
        shift -= RADIX_TREE_MAP_SHIFT;
        slot=slot->slots[i];
    }
out:
    *next_index = index;
    return nr_found;

}


/**
 * Perform multiple lookup on a radix tree
 * @head:   radix tree head
 * @results:    where the results of the lookup are placed
 * @first_index: start the lookup from this key
 * @max_items: palce up to this many items at *results
 *
 * Performs an index-ascending scan of the tree for present items. Places
 * them at *@results and returns the number of items which were placed at
 * *@results
 *
 * The implementation is naive
 */
unsigned int radix_tree_gang_lookup(radix_tree_head *head,
                                    void **results,
                                    unsigned int first_index,
                                    unsigned int max_items)
{
    const unsigned int max_index = radix_tree_maxindex(head);
    unsigned int cur_index = first_index;
    unsigned int ret = 0;

    if((head->rnode == NULL) || (max_index == 0))
    {
        goto out;
    }
    while(ret < max_items)
    {
        unsigned int nr_found, next_index;
        if(cur_index > max_index)
        {
            break;
        }
        nr_found = __lookup(head, results + ret, cur_index, max_items - ret, &next_index);
        ret += nr_found;
        if(next_index == 0)
        {
            break;
        }
        cur_index = next_index;
    }
out:
    return ret;
}

/**
**
 * radix_tree_delete    delete an item from a radix tree
 * @head:   radix tree head
 * @index:  index key
 *
 * Remove the item at @index from the radix tree @head
 *
 * Return the address of the deleted item, or NULL if it was not present
 */
void *radix_tree_delete(radix_tree_head *head, unsigned int index)
{
    radix_tree_path path[RADIX_TREE_MAX_PATH];
    radix_tree_path *pathp = path;
    unsigned int height, shift;
    void *ret = NULL;

    height = head->height;
    /* Make sure tree is high enough */
    if(index > radix_tree_maxindex(head))
    {
        goto out;
    }
    shift = (height - 1) * RADIX_TREE_MAP_SHIFT;
    pathp->node = NULL;
    pathp->slot = &head->rnode;

    while(height)
    {
        if(pathp->slot == NULL)
        {
            printf("bug, please check your input\n");
            goto out;
        }
        pathp[1].node = pathp[0].slot;
        pathp[1].slot = (radix_tree_node **) (pathp[1].node->slots + ((index >> shift) & RADIX_TREE_MAP_MASK));

        pathp++;
        shift -= RADIX_TREE_MAP_SHIFT;
        height--;
    }

    ret = pathp[0].slot;
    if(ret == NULL)
    {
        printf("bug, please check your input\n");
        goto out;
    }

    pathp[0].slot = NULL;
    while((pathp[0].node) && (--(pathp[0].node->count) == 0))
    {
        pathp--;
        pathp[0].slot = NULL;
        radix_tree_node_free(pathp[1].node);
    }
    if(head->rnode == NULL)
    {
        printf("radix tree emtpy now\n");
        head->height = 0;
    }
out:
    return ret;
}

radix_tree_head *radix_tree_head_new(void)
{
    radix_tree_head *head = (radix_tree_head*)calloc(1, sizeof(radix_tree_head));
    if(!head)
    {
        printf("calloc radix_tree_node failed\n");
    }
    return head;
}

void radix_tree_initial(radix_tree_head *head)
{
    radix_tree_init_maxindex(head);
}

int radix_tree_destroy(radix_tree_head *head)
{
    printf("why I should write this function?\n, is it useful?!!!!\n");
    return 0;
}
