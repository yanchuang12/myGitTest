#include "nd_core.h"
#include <stdlib.h>

//#define __UNUSE_POOL__
#define __ND_POOL_HAS_STATS__


#define ND_LARGE_PAGE_DEFAULT_SIZE		(1024*1024)

#define ND_POOL_INVALID_MARK		(NULL)

#define __pool_get_mark(p)			*((void**)((nd_uint8_t*)(p) - ND_MEM_ALIGN))

static ND_INLINE
void* __pool_set_mark(void *p, void *mark)	
{
	 *(void**)p = mark;
	 return (unsigned char*)p + ND_MEM_ALIGN;
}


void* 
nd_default_pool_malloc(size_t size)
{
	unsigned char* p = (unsigned char*)__nd_sys_malloc(size + ND_MEM_ALIGN);
	if (NULL == p) {
		return NULL;
	}

	return __pool_set_mark(p, ND_POOL_INVALID_MARK);
}

void* 
nd_default_pool_cmalloc(size_t size)
{
	unsigned char* p = (unsigned char*)__nd_sys_malloc(size + ND_MEM_ALIGN);
	if (NULL == p) {
		return NULL;
	}

	memset(p, 0, size);
	return __pool_set_mark(p, ND_POOL_INVALID_MARK);
}

void* 
nd_default_pool_malloc_and_mark(size_t size, void* mark)
{
	unsigned char* p = (unsigned char*)__nd_sys_malloc(size + ND_MEM_ALIGN);
	if (NULL == p) {
		return NULL;
	}

	return __pool_set_mark(p, mark);
}

void 
nd_default_pool_free(void* p)
{
	unsigned char* r;
	if (NULL == p) return;

	r = (unsigned char*)p - ND_MEM_ALIGN;
	__nd_sys_free(r);
}

nd_fix_pool_t*
nd_create_fix_pool(nd_uint32_t chunk_size, nd_uint8_t idx, nd_uint32_t low_water, nd_uint32_t high_water, nd_uint32_t large_page_size, nd_bool_t lock)
{
	nd_uint32_t n;
	nd_fix_pool_t* p;

	if (low_water < 1) {
		low_water = 1;
	}
	if (high_water == 0) {
		high_water = 0x7fffffff;
	}

	n = sizeof(nd_fix_pool_t) + sizeof(nd_fix_pool_large_t) + low_water * (chunk_size + ND_MEM_ALIGN);
	if (n < ND_LARGE_PAGE_DEFAULT_SIZE) {
		n = ND_LARGE_PAGE_DEFAULT_SIZE;
	}

	p = (nd_fix_pool_t*)__nd_sys_malloc(n);
	if (NULL == p) {
		return NULL;
	}

	/// zero
	memset(p, 0, sizeof(nd_fix_pool_t) + sizeof(nd_fix_pool_large_t));

	p->idx = idx;
	p->curr_num = p->low_water = low_water;
	p->chunk_size = chunk_size;
	p->high_water = high_water;
	
	/// large
	if (large_page_size < sizeof(nd_fix_pool_large_t) + chunk_size + ND_MEM_ALIGN){
		large_page_size = sizeof(nd_fix_pool_large_t) + chunk_size + ND_MEM_ALIGN;
	}

	p->large_page_size = (large_page_size >= 1024*4 ?  large_page_size : ND_LARGE_PAGE_DEFAULT_SIZE);
	p->large_curr = (nd_fix_pool_large_t*)((char*)p + sizeof(nd_fix_pool_t));
	p->lg_curr_pos = sizeof(nd_fix_pool_large_t);
	p->lg_curr_size = low_water * (chunk_size + ND_MEM_ALIGN);

	if (lock) {
		p->mutex = __nd_sys_malloc(sizeof(nd_mutex_t));
		if (p->mutex) {
			nd_mutex_init(p->mutex);
		}
	}

	return p;
};

void
nd_destroy_fix_pool(nd_fix_pool_t *pool)
{
	nd_fix_pool_large_t *d, *n;

	if (pool) {
		for (d = pool->lg_lst; d; /* void */) {
			n = d->next;
			__nd_sys_free(d);
			d = n;
		}
		if (pool->mutex) {
			nd_mutex_destroy(pool->mutex);
			__nd_sys_free(pool->mutex);
		}
		__nd_sys_free(pool);
	}
}

static ND_INLINE void __nd_pool_check_large(nd_fix_pool_t *pool)
{
	if(pool->large_curr && 
		pool->lg_curr_pos + pool->chunk_size + ND_MEM_ALIGN > pool->lg_curr_size) {
		/// no free space

		/// 判断 pool->large_curr 是否 就是 pool本身指向的内存块
		if ((char*)pool + sizeof(nd_fix_pool_t) != (char*)pool->large_curr) {
			/// 不是pool本身, 则放到@pool->lg_lst的head
			pool->large_curr->next = pool->lg_lst;
			pool->lg_lst = pool->large_curr;
		}

		pool->large_curr = NULL;
		pool->lg_curr_pos = 0;
		pool->lg_curr_size = 0;
	}
}

void*
nd_fix_pool_alloc(nd_fix_pool_t *pool)
{
	void *r;

#ifdef __UNUSE_POOL__
	return nd_default_pool_malloc(pool->chunk_size);
#endif

	nd_mutex_lock(pool->mutex);

	if (pool->free) {
		r = pool->free;
		pool->free = pool->free->next;
		nd_mutex_unlock(pool->mutex);
		return r;
	}

	if(pool->large_curr) {
		if(pool->lg_curr_size > pool->lg_curr_pos  && 
			pool->lg_curr_pos + pool->chunk_size + ND_MEM_ALIGN <= pool->lg_curr_size) {
			r = __pool_set_mark((unsigned char*)pool->large_curr + pool->lg_curr_pos, pool);
			pool->lg_curr_pos += pool->chunk_size + ND_MEM_ALIGN;

			__nd_pool_check_large(pool);

			nd_mutex_unlock(pool->mutex);
			return r;
		}
		else {
			__nd_pool_check_large(pool);
		}
	}

	/// 没有空闲内存, 则再分配
	nd_assert(pool->large_curr == NULL);

	if (pool->large_page_size < pool->chunk_size + ND_MEM_ALIGN + sizeof(nd_fix_pool_large_t)) { 
		nd_mutex_unlock(pool->mutex);
		return nd_default_pool_malloc(pool->chunk_size);
	}
	
	pool->large_curr = (nd_fix_pool_large_t*)__nd_sys_malloc(pool->large_page_size);
	if (NULL == pool->large_curr) {
		nd_mutex_unlock(pool->mutex);
		return nd_default_pool_malloc(pool->chunk_size);
	}

	memset(pool->large_curr, 0, sizeof(nd_fix_pool_large_t));

	pool->lg_curr_pos = sizeof(nd_fix_pool_large_t);
	pool->lg_curr_size = pool->large_page_size - sizeof(nd_fix_pool_large_t);
	pool->curr_num += (pool->large_page_size - sizeof(nd_fix_pool_large_t)) / pool->chunk_size + ND_MEM_ALIGN;

	r = __pool_set_mark((unsigned char*)pool->large_curr + pool->lg_curr_pos, pool);
	pool->lg_curr_pos += pool->chunk_size + ND_MEM_ALIGN;

	nd_mutex_unlock(pool->mutex);
	return r;
}

void*
nd_fix_pool_calloc(nd_fix_pool_t* pool)
{
	void* r;

	r = nd_fix_pool_alloc(pool);
	if (r) {
		memset(r, 0, pool->chunk_size);
	}
	return r;
}

void
nd_fix_pool_free(nd_fix_pool_t* pool, void* p)
{
	if (NULL == p) {
		return;
	}

#ifdef __UNUSE_POOL__
	nd_default_pool_free(p);
	return;
#endif

	if(pool != __pool_get_mark(p)) {
		nd_default_pool_free(p);
		return;
	}

	nd_mutex_lock(pool->mutex);
	
	((nd_fix_pool_data_t*)p)->next = pool->free;
	pool->free = (nd_fix_pool_data_t*)p;

	nd_mutex_unlock(pool->mutex);
}

/* - * - * - * - * - */

nd_pool_t*
nd_create_pool(nd_pool_init_param_t* param)
{
	nd_pool_t* p;
	nd_uint32_t i, j;

	p = __nd_sys_malloc(sizeof(nd_pool_t));
	if(NULL == p) {
		return NULL;
	}
	memset(p, 0, sizeof(nd_pool_t));

	p->idx_num = param->idx_num;
	p->idx = param->idx;
	p->chunks_num = param->chunk_size_lst_num;
	p->high_water = (param->high_water ? param->high_water : 1024*1024*1024);
	p->low_water = (param->low_water ? param->low_water : 1024*1024*32);
	p->large_page_size = (param->large_page_size >= 1024*4 ?  param->large_page_size : ND_LARGE_PAGE_DEFAULT_SIZE);
	p->up_limit = (param->up_limit > 1024*1024*100  ? param->up_limit : 0x7fffffffffffffffLL);

	nd_queue_init(&p->q_lg_free);

#ifndef __UNUSE_POOL__
	if (p->low_water) {
		nd_size_t i, n;
		n = (nd_size_t)(p->low_water / p->large_page_size);
		for (i = 0; i < n; i++) {
			pool_large_t* lg = __nd_sys_malloc(p->large_page_size);
			if (!lg) {
				nd_destroy_pool(p);
				return NULL;
			}
			memset(lg, 0, sizeof(pool_large_t));
			lg->pos = nd_mem_align_ptr(lg + sizeof(pool_large_t));
			lg->idx = 0xff;	/// 未确定分配给哪个chunks使用, 先设置一个
			p->mem_total_size += p->large_page_size;

			nd_queue_insert_head(&p->q_lg_free, lg);
		}
	}
#endif
	if (param->lock) {
		p->q_lg_mutex = __nd_sys_malloc(sizeof(nd_mutex_t));
		if (p->q_lg_mutex) {
			nd_mutex_init(p->q_lg_mutex);
		}
	}

	p->chunks = __nd_sys_malloc(p->chunks_num * sizeof(pool_chunk_t));
	if (NULL == p->chunks) {
		nd_destroy_pool(p);
		return NULL;
	}
	memset(p->chunks, 0, p->chunks_num * sizeof(pool_chunk_t));

	for (i = 0; i < p->chunks_num; ++i) {
		if (p->large_page_size < param->chunk_size_lst[i] + 2 * ND_MEM_ALIGN + sizeof(pool_large_t)) {
			nd_assert(0);
			nd_destroy_pool(p);
			return NULL;
		}

		p->chunks[i].chunk_size = param->chunk_size_lst[i];
		p->chunks[i].idx = i;
		nd_queue_init(&p->chunks[i].q_lg_free);
		nd_queue_init(&p->chunks[i].q_lg_used);

		if (param->lock) {
			p->chunks[i].mutex = __nd_sys_malloc(sizeof(nd_mutex_t));
			if (p->chunks[i].mutex) {
				nd_mutex_init(p->chunks[i].mutex);
			}
		}
	}

	/// end pos is idx[idx_num + 1]
	for (j = 0; j <= p->idx_num; j++) {
		if (j > p->chunks[p->idx[j]].chunk_size) {
			nd_assert(0);
			nd_destroy_pool(p);
			return NULL;
		}
	}

	return p;
}

void 
nd_destroy_pool(nd_pool_t *pool)
{
	if (NULL == pool) {return;}
	if (pool->chunks) {
		nd_uint32_t i;
		for (i = 0; i < pool->chunks_num; i++)
		{
			while (!nd_queue_is_empty(&pool->chunks[i].q_lg_free)) {
				pool_large_t* lg = nd_queue_head(&pool->chunks[i].q_lg_free);
				nd_queue_remove(lg);
				__nd_sys_free(lg);
			}
			while (!nd_queue_is_empty(&pool->chunks[i].q_lg_used)) {
				pool_large_t* lg = nd_queue_head(&pool->chunks[i].q_lg_used);
				nd_queue_remove(lg);
				__nd_sys_free(lg);
			}
			if (pool->chunks[i].mutex) {
				nd_mutex_destroy(pool->chunks[i].mutex);
				__nd_sys_free(pool->chunks[i].mutex);
			}
		}
		__nd_sys_free(pool->chunks);
	}
	while (!nd_queue_is_empty(&pool->q_lg_free)) {
		pool_large_t* lg = nd_queue_head(&pool->q_lg_free);
		nd_queue_remove(lg);
		__nd_sys_free(lg);
	}
	if (pool->q_lg_mutex) {
		nd_mutex_destroy(pool->q_lg_mutex);
		__nd_sys_free(pool->q_lg_mutex);
	}
	__nd_sys_free(pool);
}

void* 
nd_pool_alloc(nd_pool_t *pool, size_t size)
{
	pool_chunk_t *ch;
	pool_large_t *lg;
	void* r;

	if (size == 0) {
		nd_assert(0);
		return NULL;
	}
#ifdef __UNUSE_POOL__
	return nd_default_pool_malloc(size);
#endif
	if (size > pool->idx_num) {
		return nd_default_pool_malloc(size);
	}

	nd_assert(pool->idx[size] < pool->chunks_num);

	r = NULL;
	ch = &(pool->chunks[pool->idx[size]]);

	nd_mutex_lock(ch->mutex);

	nd_assert( size <= ch->chunk_size );
	/// 1. 先从chunk本身的@q_lg_free中分配
	while (!nd_queue_is_empty(&ch->q_lg_free)) {
		lg = nd_queue_head(&ch->q_lg_free);
		if(lg->free) {
			r = lg->free;
			lg->free = lg->free->next;
			lg->curr_num++;
			ch->curr_num++;
#ifdef __ND_POOL_HAS_STATS__
			ch->STATS_chunk_free_num--;
#endif
			break;
		}
		else {
			if (lg->pos + ch->chunk_size + ND_MEM_ALIGN <= (nd_uint8_t*)lg + pool->large_page_size) {
				r = __pool_set_mark(lg->pos, lg);
				lg->pos += ch->chunk_size + ND_MEM_ALIGN;
				lg->curr_num++;
				ch->curr_num++;
#ifdef __ND_POOL_HAS_STATS__
				ch->STATS_chunk_free_num--;
#endif
				break;
			}
			else {
				nd_queue_remove(lg);
				nd_queue_insert_tail(&ch->q_lg_used, lg);
#ifdef __ND_POOL_HAS_STATS__
				ch->STATS_q_lg_used_num++;
				ch->STATS_q_lg_free_num--;
#endif
			}
		}
	}
	nd_mutex_unlock(ch->mutex);

	if (r) { 
		return r;
	}

	/// 2. 再从pool的@q_lg_free中分配
	lg = NULL;

	nd_mutex_lock(pool->q_lg_mutex);
	if (!nd_queue_is_empty(&pool->q_lg_free)) {
		lg = nd_queue_head(&pool->q_lg_free);
		nd_queue_remove(lg);
#ifdef __ND_POOL_HAS_STATS__
		pool->STATS_q_lg_free_num--;
#endif
	}
	else {
		if (pool->mem_total_size < pool->up_limit) {
			if(NULL != (lg = __nd_sys_malloc(pool->large_page_size)))
			{
				pool->mem_total_size += pool->large_page_size;
			}
		}
	}
	nd_mutex_unlock(pool->q_lg_mutex);

	if (NULL == lg) {
		return NULL;
	}

	memset(lg, 0, sizeof(pool_large_t));
	lg->pos = (nd_uint8_t*)lg + sizeof(pool_large_t);
	lg->idx = pool->idx[size];
	r = __pool_set_mark(lg->pos, lg);
	lg->pos += ch->chunk_size + ND_MEM_ALIGN;
	lg->curr_num = 1;

	nd_mutex_lock(ch->mutex);
	/// 新分配的large, 总是插在@q_lg_free队首
	nd_queue_insert_head(&ch->q_lg_free, lg);
	ch->curr_num++;
#ifdef __ND_POOL_HAS_STATS__
	ch->STATS_q_lg_free_num++;
	ch->STATS_chunk_free_num += (pool->large_page_size - sizeof(pool_large_t)) / ch->chunk_size - 1;
#endif
	nd_mutex_unlock(ch->mutex);

	return r;
}

void* 
nd_pool_calloc(nd_pool_t *pool, size_t size)
{
	void *r;

	r = nd_pool_alloc(pool, size);
	if (r) {
		memset(r, 0, size);
	}
	return r;
}

void 
nd_pool_free(nd_pool_t *pool, void *p)
{
	pool_large_t *lg;

#ifdef __UNUSE_POOL__
	nd_default_pool_free(p);
	return;
#endif

	if (NULL == p) return;

	lg = __pool_get_mark(p);
	if (lg == ND_POOL_INVALID_MARK) {
		nd_default_pool_free(p);
	}
	else {
		pool_chunk_t* ch;
		pool_data_t* pd;
		int is_at_free_lst;
		
		pd = p;
		ch = &(pool->chunks[lg->idx]);
		nd_assert( lg->idx < pool->chunks_num );

		nd_mutex_lock(ch->mutex);
		ch->curr_num--;
#ifdef __ND_POOL_HAS_STATS__
		ch->STATS_chunk_free_num++;
#endif

		if (lg->free || lg->pos + ch->chunk_size + ND_MEM_ALIGN <= (nd_uint8_t*)lg + pool->large_page_size){
			/// 有free chunk, 说明此时lg在ch->q_lg_free链表中
			is_at_free_lst = 1;
		}
		else{
			/// 在ch->q_lg_used链表中
			is_at_free_lst = 0;
		}
		pd->next = lg->free;
		lg->free = pd;
		
		if (--lg->curr_num == 0) {
			/// lg此时一定是在@ch->q_lg_free 中
			nd_queue_remove(lg);
#ifdef __ND_POOL_HAS_STATS__
			ch->STATS_q_lg_free_num--;
			ch->STATS_chunk_free_num -= (pool->large_page_size - sizeof(pool_large_t)) / ch->chunk_size;
#endif

			if (nd_queue_is_empty(&ch->q_lg_free)) {
				/// 至少预留一块使用	
				nd_queue_insert_head(&ch->q_lg_free, lg);
#ifdef __ND_POOL_HAS_STATS__
				ch->STATS_q_lg_free_num++;
				ch->STATS_chunk_free_num += (pool->large_page_size - sizeof(pool_large_t)) / ch->chunk_size;
#endif
				nd_mutex_unlock(ch->mutex);
				return;
			}
			else {
				nd_mutex_unlock(ch->mutex);

				nd_mutex_lock(pool->q_lg_mutex);
				if (pool->mem_total_size < pool->high_water) {
					nd_queue_insert_head(&pool->q_lg_free, lg);
#ifdef __ND_POOL_HAS_STATS__
					pool->STATS_q_lg_free_num++;
#endif
					nd_mutex_unlock(pool->q_lg_mutex);		
				}
				else {
					pool->mem_total_size -= pool->large_page_size;
					nd_mutex_unlock(pool->q_lg_mutex);
					__nd_sys_free(lg);
				}
				return;
			}
		}
		else {
			if (is_at_free_lst == 0) {
				nd_queue_remove(lg);
				nd_queue_insert_tail(&ch->q_lg_free, lg);
#ifdef __ND_POOL_HAS_STATS__
				ch->STATS_q_lg_free_num++;
				ch->STATS_q_lg_used_num--;
#endif
			}
			nd_mutex_unlock(ch->mutex);
		}
	}
}
