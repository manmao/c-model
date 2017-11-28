#ifndef __RADIX_TREE_H_
#define __RADIX_TREE_H_

#include <malloc.h>
#include <stdio.h>

#define ARRARY_SIZE(a) ((sizeof(a)) /(sizeof((a)[0])))

#if 0
#define RADIX_TREE_MAP_SHIFT 6  /*值为6时，表示每个结点有2^6＝64个slot，值为4时，表示有2^4=16个slot*/
#define RADIX_TREE_MAP_SIZE (1 <<RADIX_TREE_MAP_SHIFT )
#define RADIX_TREE_MAP_MASK (RADIX_TREE_MAP_SIZE - 1)
#define RADIX_TREE_INDEX_BITS (8*sizeof(unsigned int))
#define RADIX_TREE_MAX_PATH (RADIX_TREE_INDEX_BITS / TADIX_TREE_MAP_SHIFT + 1)
#endif
#define RADIX_TREE_MAP_SHIFT 3 
#define RADIX_TREE_MAP_SIZE (1 << RADIX_TREE_MAP_SHIFT) /*表示1个叶子结点可映射的页数，如：1<<6=64，表示可映射64个slot映射64页*/
#define RADIX_TREE_MAP_MASK (RADIX_TREE_MAP_SIZE - 1)
#define RADIX_TREE_INDEX_BITS (8 * sizeof(unsigned int))
#define RADIX_TREE_MAX_PATH ( 4 )

typedef struct radix_tree_node
{
    unsigned int count;
    void *slots[RADIX_TREE_MAP_SIZE];
}radix_tree_node;

//typedef radix_tree_node * Pradix_tree_node;
//typedef Pradix_tree_node * PPradix_tree_node;

struct radix_tree_head
{
    unsigned int height;
    struct radix_tree_node *rnode;
    unsigned int height_to_maxindex[RADIX_TREE_MAX_PATH];
};
typedef struct radix_tree_head  radix_tree_head;


struct radix_tree_path
{
    struct radix_tree_node *node;
    struct radix_tree_node *slot;
};
typedef struct radix_tree_path radix_tree_path;


#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

extern radix_tree_head* radix_tree_head_new(void);
extern void radix_tree_initial(radix_tree_head* thiz);
extern int radix_tree_destroy(radix_tree_head *thiz);
extern int radix_tree_insert(radix_tree_head *thiz, unsigned int index, void *item);


extern void *radix_tree_delete(radix_tree_head *thiz,unsigned index);

extern void *radix_tree_lookup(radix_tree_head *thiz,unsigned index);
extern unsigned int radix_tree_gang_lookup(radix_tree_head *thiz,
                                           void ** results,
                                           unsigned int firs_index,
                                           unsigned int max_items);

#ifdef __cplusplus
}
#endif

#endif // __RADIX_TREE_H_
