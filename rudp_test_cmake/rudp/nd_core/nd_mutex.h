/** Copyright (c) 2016-2017
 * All rights reserved.
 * 
 * 文件名称:	nd_mutex.h   
 * 摘	 要:	锁
 * 
 * 当前版本：	1.0
 * 作	 者:	范涛涛(FTT)
 * 操	 作:	新建
 * 完成日期:	2016年08月02日
 */
#ifndef __ND_MUTEX_H_20160802__
#define __ND_MUTEX_H_20160802__

#include "nd_core.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __ND_WIN32__
	typedef HANDLE					nd_sem_t;
	typedef CRITICAL_SECTION		nd_mutex_t;
#else
	typedef sem_t					nd_sem_t;
	typedef pthread_mutex_t			nd_mutex_t;
#endif

nd_int_t 
nd_mutex_init(nd_mutex_t *m);

void
nd_mutex_destroy(nd_mutex_t *m);

static ND_INLINE nd_int_t
nd_mutex_lock(nd_mutex_t *m)
{
	if (m) {
#ifdef __ND_WIN32__
		EnterCriticalSection(m);
#else
		pthread_mutex_lock(m);
#endif
	}
	return ND_RET_OK;
}

static ND_INLINE void
nd_mutex_unlock(nd_mutex_t * m)
{
	if (m) {
#ifdef __ND_WIN32__
		LeaveCriticalSection(m);
#else
		pthread_mutex_unlock(m);
#endif
	}
}

/* - * - * - * - * - */

nd_int_t 
nd_sem_init(nd_sem_t *s, nd_long_t init, nd_long_t max);

void
nd_sem_destroy(nd_sem_t *s);

void
nd_sem_post(nd_sem_t *s);

void
nd_sem_post_n(nd_sem_t *s, nd_long_t n);

nd_int_t
nd_sem_wait(nd_sem_t *s);

nd_int_t
nd_sem_time_wait(nd_sem_t *s, nd_milli_sec_t msec);

/* - * - * - * - * - */

#ifdef __ND_WIN32__
	typedef struct 
	{
		nd_long_t		waiters;
		nd_mutex_t		waiters_mutex;
		nd_sem_t		sem;
		HANDLE			waiters_done;
		size_t			was_broadcast;
		int				xx;
		nd_mutex_t		*mutex;
	}nd_cond_t;
#else
	typedef struct 
	{
		nd_mutex_t		*mutex;
		pthread_cond_t	cond;
	}nd_cond_t;
#endif

nd_int_t
nd_cond_init(nd_cond_t *c, nd_mutex_t *mu);

void
nd_cond_destroy(nd_cond_t *c);

nd_int_t
nd_cond_wait(nd_cond_t *c);

nd_int_t
nd_cond_time_wait(nd_cond_t *c, nd_milli_sec_t msec);

void
nd_cond_signal(nd_cond_t *c);

nd_int_t 
nd_cond_broadcast(nd_cond_t *c);

#ifdef __cplusplus
}
#endif

#endif /// __ND_MUTEX_H_20160802__
