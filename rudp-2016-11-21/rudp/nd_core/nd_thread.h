/** Copyright (c) 2016-2017
 * All rights reserved.
 * 
 * 文件名称:	nd_thread.h   
 * 摘	 要:	线程
 * 
 * 当前版本：	1.0
 * 作	 者:	范涛涛(FTT)
 * 操	 作:	新建
 * 完成日期:	2016年08月02日
 */
#ifndef __ND_THREAD_H_20160802__
#define __ND_THREAD_H_20160802__

#include "nd_core.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nd_thread_s nd_thread_t;
typedef nd_int_t (ND_CALL_PRO  *thread_fun_ptr)(void*);

struct nd_thread_s
{
	/* -------------------------------------- */
	/* 必须填充@fun_ptr, @arg, 这两个字段 */
	thread_fun_ptr fun_ptr;
	void		*arg;
	/* -------------------------------------- */

#ifdef __ND_WIN32__
	DWORD		tid;
	HANDLE		h;
#else
	pthread_t	tid;
	pthread_t	h;
#endif

	nd_cond_t	cond;
	nd_mutex_t	mutex;

	volatile nd_int_t	shut_down;
	volatile nd_int_t	active;
	volatile nd_int_t	thr_cnt;
};

/* - * - * - * - * - */

nd_int_t
nd_thread_create(nd_thread_t *t);

void 
nd_thread_exit(nd_thread_t *t);

void
nd_thread_shut_down(nd_thread_t *t);

nd_int_t
nd_thread_wait(nd_thread_t *t);

nd_int_t
nd_thread_time_wait(nd_thread_t *t, nd_milli_sec_t msec);

void
nd_thread_destroy(nd_thread_t *t);

/* - * - * - * - * - */


#ifdef __cplusplus
}
#endif

#endif /// __ND_THREAD_H_20160802__
