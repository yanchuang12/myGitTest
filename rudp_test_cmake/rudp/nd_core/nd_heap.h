/** Copyright (c) 2016-2017
 * All rights reserved.
 * 
 * 文件名称:	nd_heap.h   
 * 摘	 要:	堆
 * 
 * 当前版本：	1.0
 * 作	 者:	范涛涛(FTT)
 * 操	 作:	新建
 * 完成日期:	2016年08月02日
 */
#ifndef __ND_HEAP_H_20160802__
#define __ND_HEAP_H_20160802__

#include "nd_core.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef nd_uint64_t						nd_heap_key_t;
typedef struct nd_heap_node_s			nd_heap_node_t;
struct nd_heap_node_s
{
	nd_heap_key_t		key;
	void				*data;
	nd_int_t			id;
};

typedef struct
{
	nd_uint_t			max_size;
	nd_uint_t			cur_size;
	nd_uint_t			ids_curr;
	nd_uint_t			ids_min_free;
	nd_int_t			*ids;
	nd_heap_node_t		**heap;
}nd_min_heap_t;

nd_min_heap_t*
nd_min_heap_create(nd_uint_t max_size);

void
nd_min_heap_destroy(nd_min_heap_t *q);

nd_err_t
nd_min_heap_insert(nd_min_heap_t *q, nd_heap_node_t *node);

nd_err_t 
nd_min_heap_remove(nd_min_heap_t *q, nd_heap_node_t *node);

nd_heap_node_t*
nd_min_heap_remove2(nd_min_heap_t *q, nd_int_t id);

static ND_INLINE nd_heap_node_t* 
nd_min_heap_first(nd_min_heap_t *q)
{
	if (q->cur_size)
		return q->heap[0];
	else
		return NULL;
}

nd_heap_node_t*
nd_min_heap_remove_first(nd_min_heap_t *q);


#ifdef __cplusplus
}
#endif

#endif /// __ND_HEAP_H_20160802__