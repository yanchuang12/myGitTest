/** Copyright (c) 2016-2017
 * All rights reserved.
 * 
 * 文件名称:	nd_fix_pool.h 
 * 摘	 要:	内存池
 * 
 * 当前版本:	1.0
 * 作	 者:	范涛涛
 * 操	 作:	新建
 * 完成日期:	2016年08月02日
 */
#ifndef __ND_FIX_POOL_H_20160802__
#define __ND_FIX_POOL_H_20160802__

#include "nd_core.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nd_fix_pool_data_s	nd_fix_pool_data_t;
typedef struct nd_fix_pool_s		nd_fix_pool_t;
typedef struct nd_fix_pool_large_s	nd_fix_pool_large_t;

struct nd_fix_pool_data_s
{
	nd_fix_pool_data_t *next;
};

struct nd_fix_pool_large_s
{
	nd_fix_pool_large_t *next;
};

/*
 ---------------------------------------------------------
| nd_fix_pool_t | nd_fix_pool_large_t | chunks ...        |
 ---------------------------------------------------------
| nd_fix_pool_large_t | chunks ...                        |
 ---------------------------------------------------------
| nd_fix_pool_large_t | chunks ...                        |
 ---------------------------------------------------------
*/
struct nd_fix_pool_s
{
	nd_uint8_t				idx;
	nd_uint32_t				chunk_size;
	nd_uint32_t				low_water;
	nd_uint32_t				high_water;
	nd_uint32_t				curr_num;
	nd_fix_pool_data_t		*free;				/// free list
	nd_mutex_t				*mutex;
	/// 
	nd_fix_pool_large_t		*large_curr;			/// 当前正在使用的large块
	nd_fix_pool_large_t		*lg_lst;				/// 已使用的large块. 注意: pool本身也是一个large块, 但没有放到这里的lst
	nd_uint32_t				lg_curr_pos;
	nd_uint32_t				lg_curr_size;
	nd_uint32_t				large_page_size;	/// 分配一个新的large块时，指定的大小
};

void* 
nd_default_pool_malloc(size_t size);

void* 
nd_default_pool_cmalloc(size_t size);

void* 
nd_default_pool_malloc_and_mark(size_t size, void *mark);

void 
nd_default_pool_free(void *p);

nd_fix_pool_t*
nd_create_fix_pool(nd_uint32_t chunk_size, nd_uint8_t idx, nd_uint32_t low_water, nd_uint32_t high_water, nd_uint32_t large_page_size, nd_bool_t lock);

void
nd_destroy_fix_pool(nd_fix_pool_t *pool);

void*
nd_fix_pool_alloc(nd_fix_pool_t *pool);

void*
nd_fix_pool_calloc(nd_fix_pool_t *pool);

void
nd_fix_pool_free(nd_fix_pool_t *pool, void *p);

/* - * - * - * - * - */

typedef struct pool_chunk_s		pool_chunk_t;
typedef struct pool_data_s		pool_data_t;
typedef struct pool_large_s		pool_large_t;

struct pool_data_s
{
	pool_data_t*	next;
};

struct pool_large_s
{
	__ND_QUEUE_DECALRE__(pool_large_t);
	nd_uint8_t		*pos;
	nd_uint32_t		curr_num;			/// 当前已使用的chunk的数量
	nd_uint8_t		idx;
	pool_data_t		*free;
};

struct pool_chunk_s
{
	nd_uint8_t				idx;			/// 对应的@pool->chunks的下标
	nd_uint32_t				chunk_size;		/// 每个chunk的大小
	nd_uint32_t				curr_num;		/// 当前已使用的chunk的数量
	/// 1. large从used变为free时, 总是插在@q_lg_free末尾
	/// 2. 新分配的large, 总是插在@q_lg_free队首
	pool_large_t			q_lg_used;		/// 已被使用完的large
	pool_large_t			q_lg_free;		/// 有free chunk的large, 仅仅表示有可用空间,并不表示整个large都是可用的
	nd_mutex_t				*mutex;
	/// 以下仅仅作为统计字段
	nd_uint32_t				STATS_q_lg_used_num;	/// 已被使用完的large数. 即队列@q_lg_used的数量
	nd_uint32_t				STATS_q_lg_free_num;	/// 有free chunk的large数, 即队列@q_lg_free的数量
	nd_uint32_t				STATS_chunk_free_num;	/// free chunk 总数
};


typedef struct
{
	nd_uint32_t		idx_num;				/// 索引数组(@idx)的长度
	nd_uint32_t		chunks_num;				/// @chunks数组的大小
	pool_chunk_t	*chunks;					/// 指定chunk块长度的数组
	nd_uint8_t		*idx;					/// 索引数组
	pool_large_t	q_lg_free;				/// 未使用的large列表(整个large都是可用的)
	nd_uint32_t		large_page_size;		/// 初始化时,指定一个large的大小
	nd_mutex_t		*q_lg_mutex;				/// lock : @q_lg_free, @mem_total_size
	nd_uint64_t		low_water;				/// 初始内存池大小
	nd_uint64_t		high_water;				/// 仅当释放内存时, 如果@mem_total_size > @high_water, 则将该内存块直接释放,不保留在池中(@q_lg_free)
	nd_uint64_t		up_limit;				/// 允许使用的内存上限, 即使 high_water > up_limit, 也以up_limit为上限
	nd_uint64_t		mem_total_size;			/// 当前已分配内存(包括池中未使用的块), 即当前内存池总的大小. 
	/// 以下仅仅作为统计字段
	nd_uint32_t		STATS_q_lg_free_num;	/// 池中空余large数量, 即队列@q_lg_free的数量
}nd_pool_t;



nd_pool_t*
nd_create_pool(nd_pool_init_param_t *param);

void
nd_destroy_pool(nd_pool_t *pool);

void* 
nd_pool_alloc(nd_pool_t *pool, size_t size);

void* 
nd_pool_calloc(nd_pool_t *pool, size_t size);

void 
nd_pool_free(nd_pool_t *pool, void* p);


/* - * - * - * - * - */

#ifdef __cplusplus
}
#endif

#endif /// __ND_FIX_POOL_H_20160802__
