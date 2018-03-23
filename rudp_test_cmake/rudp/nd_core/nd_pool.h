/** Copyright (c) 2016-2017
 * All rights reserved.
 * 
 * �ļ�����:	nd_fix_pool.h 
 * ժ	 Ҫ:	�ڴ��
 * 
 * ��ǰ�汾:	1.0
 * ��	 ��:	������
 * ��	 ��:	�½�
 * �������:	2016��08��02��
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
	nd_fix_pool_large_t		*large_curr;			/// ��ǰ����ʹ�õ�large��
	nd_fix_pool_large_t		*lg_lst;				/// ��ʹ�õ�large��. ע��: pool����Ҳ��һ��large��, ��û�зŵ������lst
	nd_uint32_t				lg_curr_pos;
	nd_uint32_t				lg_curr_size;
	nd_uint32_t				large_page_size;	/// ����һ���µ�large��ʱ��ָ���Ĵ�С
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
	nd_uint32_t		curr_num;			/// ��ǰ��ʹ�õ�chunk������
	nd_uint8_t		idx;
	pool_data_t		*free;
};

struct pool_chunk_s
{
	nd_uint8_t				idx;			/// ��Ӧ��@pool->chunks���±�
	nd_uint32_t				chunk_size;		/// ÿ��chunk�Ĵ�С
	nd_uint32_t				curr_num;		/// ��ǰ��ʹ�õ�chunk������
	/// 1. large��used��Ϊfreeʱ, ���ǲ���@q_lg_freeĩβ
	/// 2. �·����large, ���ǲ���@q_lg_free����
	pool_large_t			q_lg_used;		/// �ѱ�ʹ�����large
	pool_large_t			q_lg_free;		/// ��free chunk��large, ������ʾ�п��ÿռ�,������ʾ����large���ǿ��õ�
	nd_mutex_t				*mutex;
	/// ���½�����Ϊͳ���ֶ�
	nd_uint32_t				STATS_q_lg_used_num;	/// �ѱ�ʹ�����large��. ������@q_lg_used������
	nd_uint32_t				STATS_q_lg_free_num;	/// ��free chunk��large��, ������@q_lg_free������
	nd_uint32_t				STATS_chunk_free_num;	/// free chunk ����
};


typedef struct
{
	nd_uint32_t		idx_num;				/// ��������(@idx)�ĳ���
	nd_uint32_t		chunks_num;				/// @chunks����Ĵ�С
	pool_chunk_t	*chunks;					/// ָ��chunk�鳤�ȵ�����
	nd_uint8_t		*idx;					/// ��������
	pool_large_t	q_lg_free;				/// δʹ�õ�large�б�(����large���ǿ��õ�)
	nd_uint32_t		large_page_size;		/// ��ʼ��ʱ,ָ��һ��large�Ĵ�С
	nd_mutex_t		*q_lg_mutex;				/// lock : @q_lg_free, @mem_total_size
	nd_uint64_t		low_water;				/// ��ʼ�ڴ�ش�С
	nd_uint64_t		high_water;				/// �����ͷ��ڴ�ʱ, ���@mem_total_size > @high_water, �򽫸��ڴ��ֱ���ͷ�,�������ڳ���(@q_lg_free)
	nd_uint64_t		up_limit;				/// ����ʹ�õ��ڴ�����, ��ʹ high_water > up_limit, Ҳ��up_limitΪ����
	nd_uint64_t		mem_total_size;			/// ��ǰ�ѷ����ڴ�(��������δʹ�õĿ�), ����ǰ�ڴ���ܵĴ�С. 
	/// ���½�����Ϊͳ���ֶ�
	nd_uint32_t		STATS_q_lg_free_num;	/// ���п���large����, ������@q_lg_free������
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
