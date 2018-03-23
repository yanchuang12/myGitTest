
#ifndef __ND_RBTREE_H_20160802__
#define __ND_RBTREE_H_20160802__


#include "nd_core.h"


typedef nd_uint_t  nd_rbtree_key_t;
typedef nd_int_t   nd_rbtree_key_int_t;


typedef struct nd_rbtree_node_s  nd_rbtree_node_t;

struct nd_rbtree_node_s {
    nd_rbtree_key_t       key;
    nd_rbtree_node_t     *left;
    nd_rbtree_node_t     *right;
    nd_rbtree_node_t     *parent;
    u_char                 color;
    u_char                 data;
};


typedef struct nd_rbtree_s  nd_rbtree_t;

typedef void (*nd_rbtree_insert_pt) (nd_rbtree_node_t *root,
    nd_rbtree_node_t *node, nd_rbtree_node_t *sentinel);

struct nd_rbtree_s {
    nd_rbtree_node_t     *root;
    nd_rbtree_node_t     *sentinel;
    nd_rbtree_insert_pt   insert;
	nd_int_t			  size;
};


#define nd_rbtree_init(tree, s, i)                                           \
    nd_rbtree_sentinel_init(s);                                              \
    (tree)->root = s;                                                         \
    (tree)->sentinel = s;                                                     \
	(tree)->size = 0;														\
    (tree)->insert = i


void nd_rbtree_insert(nd_thread_volatile nd_rbtree_t *tree,
    nd_rbtree_node_t *node);
void nd_rbtree_delete(nd_thread_volatile nd_rbtree_t *tree,
    nd_rbtree_node_t *node);
void nd_rbtree_insert_value(nd_rbtree_node_t *root, nd_rbtree_node_t *node,
    nd_rbtree_node_t *sentinel);
void nd_rbtree_insert_seq_value(nd_rbtree_node_t *root,
    nd_rbtree_node_t *node, nd_rbtree_node_t *sentinel);


#define nd_rbt_red(node)               ((node)->color = 1)
#define nd_rbt_black(node)             ((node)->color = 0)
#define nd_rbt_is_red(node)            ((node)->color)
#define nd_rbt_is_black(node)          (!nd_rbt_is_red(node))
#define nd_rbt_copy_color(n1, n2)      (n1->color = n2->color)


/* a sentinel must be black */

#define nd_rbtree_sentinel_init(node)  nd_rbt_black(node)


static ND_INLINE nd_rbtree_node_t *
nd_rbtree_min(nd_rbtree_node_t *node, nd_rbtree_node_t *sentinel)
{
    while (node->left != sentinel) {
        node = node->left;
    }

    return node;
}


#endif /// __ND_RBTREE_H_20160802__
