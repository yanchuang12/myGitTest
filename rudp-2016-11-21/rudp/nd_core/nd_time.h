/** Copyright (c) 2016-2017
 * All rights reserved.
 * 
 * 文件名称:	nd_timer.h   
 * 摘	 要:	定时器队列
 * 
 * 当前版本：	1.0
 * 作	 者:	范涛涛(FTT)
 * 操	 作:	新建
 * 完成日期:	2016年08月02日
 */

#ifndef __ND_TIME_H_20160802__
#define __ND_TIME_H_20160802__

#include "nd_core.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nd_tq_node_s		nd_tq_node_t;

struct nd_tq_node_s
{
	nd_tq_node_t		*link;
	void				*data;
	nd_heap_node_t		node;
	void				(*handle_timer_out)(nd_tq_node_t *node);
	nd_uint64_t			inter_usec;
	unsigned			loop:1;
	unsigned			in_wait_lst:1;
};

typedef struct 
{
	nd_min_heap_t		*heap;
	nd_tq_node_t		*wait_lst;
	nd_int_t			dispatch;
}nd_timer_que_t;


nd_timer_que_t*
nd_timer_que_create(nd_uint_t max_size);

void
nd_timer_que_remove_all(nd_timer_que_t *tq);

void
nd_timer_que_destroy(nd_timer_que_t *tq);

nd_tq_node_t*
nd_timer_que_first_node(nd_timer_que_t *tq);

void 
nd_timer_que_dispatch(nd_timer_que_t *tq);

nd_err_t 
nd_timer_que_schedule(nd_timer_que_t *tq, nd_tq_node_t *node);

nd_err_t 
nd_timer_que_update_schedule(nd_timer_que_t *tq, nd_tq_node_t *node, nd_uint64_t new_tm);

void
nd_timer_que_remove(nd_timer_que_t *tq, nd_tq_node_t *node);

/// 返回微秒数
nd_uint64_t nd_get_ts();

static ND_INLINE nd_int64_t nd_tm_u64_cmp(nd_uint64_t t1, nd_uint64_t t2){
	return (nd_int64_t)(t1 - t2);
}
static ND_INLINE nd_int64_t nd_tm_u64_diff(nd_uint64_t t1, nd_uint64_t t2){
	return (nd_int64_t)(t1 - t2);
}


static ND_INLINE void nd_sleep(nd_milli_sec_t msec)
{
#ifdef __ND_WIN32__
	Sleep(msec);
#else
	#if (0)
		usleep(msec * 1000);
	#else
		struct timeval delay;
		delay.tv_sec = msec / 1000;
		delay.tv_usec =(msec % 1000) * 1000;
		select(0, NULL, NULL, NULL, &delay);
	#endif
#endif
}

#ifdef __cplusplus
}
#endif

#endif /// __ND_TIME_H_20160802__
