/** Copyright (c) 2016-2017
 * All rights reserved.
 * 
 * 文件名称:	nd_dlist.h   
 * 摘	 要:	数据块的容器
 * 
 * 当前版本：	1.0
 * 作	 者:	范涛涛(FTT)
 * 操	 作:	新建
 * 完成日期:	2016年08月02日
 */
#ifndef __ND_DLIST_H_20160802__
#define __ND_DLIST_H_20160802__

#include "nd_core.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nd_dlist_node_s nd_dlist_node_t;

struct nd_dlist_node_s
{
	__ND_QUEUE_DECALRE__(nd_dlist_node_t);
	void	*data;
};

typedef struct 
{
	nd_uint32_t		size;
	nd_dlist_node_t	header;				
}nd_dlist_t;


static ND_INLINE void
nd_dlist_init(nd_dlist_t *lst)
{
	lst->size = 0;
	nd_queue_init(&lst->header);
}

static ND_INLINE nd_bool_t
nd_dlist_is_empty(nd_dlist_t *lst)
{
	return (nd_queue_is_empty(&lst->header)) ? 1 : 0;
}

static ND_INLINE nd_dlist_node_t*
nd_dlist_begin(nd_dlist_t *lst)
{
	if (nd_queue_is_empty(&lst->header))
		return NULL;
	else
		return nd_queue_head(&lst->header);
}

static ND_INLINE nd_dlist_node_t* 
nd_dlist_next(nd_dlist_t *lst, nd_dlist_node_t *curr)
{
	if (!curr) 
		return NULL;
	else
		return curr->next;
}

static ND_INLINE nd_bool_t
nd_dlist_is_end(nd_dlist_t *lst, nd_dlist_node_t *curr)
{
	return (NULL == curr) || (curr == &lst->header);
}

static ND_INLINE void 
nd_dlist_remove_node(nd_dlist_t *lst, nd_dlist_node_t *curr)
{	
	if (!nd_dlist_is_end(lst, curr)){
		nd_queue_remove(curr);
		--lst->size;
	}
}

static ND_INLINE void 
nd_dlist_move_node_to_end(nd_dlist_t *lst, nd_dlist_node_t *curr)
{
	if (!nd_dlist_is_end(lst, curr)) {
		nd_queue_remove(curr);
		curr->next = NULL;
		curr->prev = NULL;
		nd_queue_insert_tail(&lst->header, curr);
	}
}

static ND_INLINE nd_uint32_t 
nd_dlist_size(nd_dlist_t *lst)
{
	return lst->size;
}

static ND_INLINE nd_bool_t 
nd_dlist_insert_tail(nd_dlist_t *lst, nd_dlist_node_t *curr)
{
	nd_queue_insert_tail(&lst->header, curr);
	++lst->size;
	return 1;
}
static ND_INLINE nd_bool_t 
nd_dlist_insert_head(nd_dlist_t *lst, nd_dlist_node_t *curr)
{
	nd_queue_insert_head(&lst->header, curr);
	++lst->size;
	return 1;
}

static ND_INLINE nd_dlist_node_t* 
nd_dlist_head(nd_dlist_t *lst)
{
	if (nd_queue_is_empty(&lst->header))
		return NULL;
	else
		return nd_queue_head(&lst->header);
}

static ND_INLINE nd_dlist_node_t*
nd_dlist_remove_head(nd_dlist_t *lst)
{
	if (nd_queue_is_empty(&lst->header))
		return NULL;
	else {
		nd_dlist_node_t *n;
		n = nd_queue_head(&lst->header);
		nd_queue_remove(n);
		--lst->size;
		return n;
	}
}

static ND_INLINE nd_dlist_node_t* 
nd_dlist_tail(nd_dlist_t *lst)
{
	if (nd_queue_is_empty(&lst->header))
		return NULL;
	else
		return nd_queue_tail(&lst->header);
}

static ND_INLINE nd_dlist_node_t*
nd_dlist_remove_tail(nd_dlist_t *lst)
{
	if (nd_queue_is_empty(&lst->header)) {
		return NULL;
	}
	else {
		nd_dlist_node_t *n;
		n = nd_queue_tail(&lst->header);
		nd_queue_remove(n);
		--lst->size;
		return n;
	}
}

static ND_INLINE nd_dlist_node_t*
nd_dlist_find(nd_dlist_t *lst, void *data)
{
	nd_dlist_node_t* n;
	nd_queue_for_each(n, &lst->header)
	{
		if (n->data == data) {
			return n;
		}
	}
	return NULL;
}

#ifdef __cplusplus
}
#endif

#endif /// __ND_DLIST_H_20160802__
