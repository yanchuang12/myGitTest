
#include "nd_core.h"

nd_int_t 
nd_mutex_init(nd_mutex_t *m)
{
	if (m) {
#ifdef __ND_WIN32__
		InitializeCriticalSection(m);
#else
		pthread_mutex_init (m, NULL);
#endif
	}
	return ND_RET_OK;
}

void
nd_mutex_destroy(nd_mutex_t *m)
{
	if (m) {
#ifdef __ND_WIN32__
		DeleteCriticalSection(m);
#else
		pthread_mutex_destroy(m);
#endif
	}
}

/* - * - * - * - * - */

nd_int_t 
nd_sem_init(nd_sem_t *s, nd_long_t init, nd_long_t max)
{
#ifdef __ND_WIN32__
	*s = CreateSemaphore(NULL, init, max, NULL);
	return (*s != NULL) ? ND_RET_OK : ND_RET_ERROR;
#else
	return sem_init(s, 0, init) == 0 ? ND_RET_OK : ND_RET_ERROR;
#endif
}

void
nd_sem_destroy(nd_sem_t *s)
{
#ifdef __ND_WIN32__
	if (s && *s) {
		CloseHandle(*s);
		*s = NULL;
	}
#else
	if (s) {
		nd_size_t retry = 0;
		while ((sem_destroy(s) == -1) && retry < 3) {
			if (errno == EBUSY) {
				retry ++;
				sem_post(s);
				sched_yield();
			}
			else{
				break;
			}
		}
	}
#endif
}

void
nd_sem_post(nd_sem_t *s)
{
#ifdef __ND_WIN32__
	if (s)
		ReleaseSemaphore(*s, 1, NULL);
#else
	if (s)
		sem_post(s);
#endif
}

void
nd_sem_post_n(nd_sem_t *s, nd_long_t n)
{
#ifdef __ND_WIN32__
	if (s)
		ReleaseSemaphore(*s, n, NULL);
#else
	if (s) {
		while (n-- > 0) {
			sem_post(s);
		}
	}
#endif
}

nd_int_t
nd_sem_wait(nd_sem_t *s)
{
#ifdef __ND_WIN32__
	if (s) {
		switch (WaitForSingleObject(*s, INFINITE)) {
		case WAIT_OBJECT_0:
		case WAIT_ABANDONED:
			return ND_RET_OK;
		default:
			return ND_RET_ERROR;
		}
	}
	return ND_RET_ERROR;
#else
	int ret = -1;
	if (s) {
		do{
			ret = sem_wait(s);
		}while (ret == -1 && errno == EINTR);
	}
	return ret == -1 ? ND_RET_ERROR : ND_RET_OK;
#endif
}

nd_int_t
nd_sem_time_wait(nd_sem_t *s, nd_milli_sec_t msec)
{
#ifdef __ND_WIN32__
	if (s) {
		switch (WaitForSingleObject(*s, msec)) {
		case WAIT_OBJECT_0:
		case WAIT_ABANDONED:
			return ND_RET_OK;
		case WAIT_TIMEOUT:
			return ND_RET_AGAIN;
		default:
			return ND_RET_ERROR;
		}
	}
	return ND_RET_ERROR;
#else
	if (s) {
		struct timeval now;
		struct timespec ts;
		int ret;
		
		if (msec == -1) {
			return nd_sem_wait(s);
		}

		gettimeofday(&now, NULL);

		now.tv_sec += msec / 1000;
		now.tv_usec += (msec % 1000) * 1000;

		if (now.tv_usec >= 1000000) {
			now.tv_sec += now.tv_usec / 1000000;
			now.tv_usec = now.tv_usec % 1000000;
		}

		ts.tv_sec = now.tv_sec;
		ts.tv_nsec = now.tv_usec * 1000;
		
		ret = -1;

		do{
			ret = sem_timedwait(s, &ts);
		}while (ret == -1 && errno == EINTR);

		if (ret == 0) return ND_RET_OK;
		else if(ret == -1 && errno == ETIMEDOUT) return ND_RET_AGAIN;
		else return ND_RET_ERROR;
	}
	return ND_RET_ERROR;
#endif
}

/* - * - * - * - * - */

nd_int_t
nd_cond_init(nd_cond_t *c, nd_mutex_t *mu)
{
#ifdef __ND_WIN32__
	if (!c) {
		return ND_RET_ERROR;
	}

	c->mutex = mu;
	c->waiters = 0;
	c->was_broadcast = 0;

	if (ND_RET_OK != nd_sem_init(&c->sem, 0, 0x7fffffff))
		return ND_RET_ERROR;

	if (ND_RET_OK != nd_mutex_init(&c->waiters_mutex))
		return ND_RET_ERROR;

	if (NULL == (c->waiters_done = CreateEvent(NULL, FALSE, FALSE, NULL)))
		return ND_RET_ERROR;

	return ND_RET_OK;
#else
	if (!c) {
		return ND_RET_ERROR;
	}
	c->mutex = mu;

	pthread_cond_init(&c->cond, NULL);
#endif
}

void
nd_cond_destroy(nd_cond_t *c)
{
#ifdef __ND_WIN32__
	if (NULL == c) {
		return;
	}

	if (c->waiters_done) {
		CloseHandle(c->waiters_done);
		c->waiters_done = NULL;
	}

	c->waiters = 0;
	c->was_broadcast = 0;

	nd_mutex_destroy(&c->waiters_mutex);
	nd_sem_destroy(&c->sem);
#else
	int retry = 0;
	if (NULL == c) {
		return;
	}

	while ((pthread_cond_destroy (&c->cond) == EBUSY) && retry++ < 3){
		pthread_cond_broadcast(&c->cond);
		sched_yield();
	}
#endif
}

nd_int_t
nd_cond_wait(nd_cond_t *c)
{
#ifdef __ND_WIN32__
	nd_int_t r;
	nd_int_t last_waiter;

	if (NULL == c) {
		return ND_RET_ERROR;
	}

	nd_mutex_lock(&c->waiters_mutex);
	c->waiters++;
	nd_mutex_unlock(&c->waiters_mutex);

	nd_mutex_unlock(c->mutex);

	r = nd_sem_wait(&c->sem);

	nd_mutex_lock(&c->waiters_mutex);
	c->waiters--;
	last_waiter = (c->was_broadcast && c->waiters == 0);
	nd_mutex_unlock(&c->waiters_mutex);

	if (ND_RET_OK == r && last_waiter) {	
		SetEvent(c->waiters_done);	
	}

	nd_mutex_lock(c->mutex);
	return r;
#else
	nd_int_t r;

	if (NULL == c) {
		return ND_RET_ERROR;
	}

	r = pthread_cond_wait(&c->cond, c->mutex);
	return (r == 0 ? ND_RET_OK : ND_RET_ERROR);
#endif
}

nd_int_t
nd_cond_time_wait(nd_cond_t *c, nd_milli_sec_t msec)
{
#ifdef __ND_WIN32__
	nd_int_t r;
	nd_int_t last_waiter;

	if (NULL == c) {
		return ND_RET_ERROR;
	}

	nd_mutex_lock(&c->waiters_mutex);
	c->waiters++;
	nd_mutex_unlock(&c->waiters_mutex);

	nd_mutex_unlock(c->mutex);

	r = nd_sem_time_wait(&c->sem, msec);

	nd_mutex_lock(&c->waiters_mutex);
	c->waiters--;
	last_waiter = (c->was_broadcast && c->waiters == 0);
	nd_mutex_unlock(&c->waiters_mutex);

	if (ND_RET_OK == r && last_waiter) {	
		SetEvent(c->waiters_done);	
	}

	nd_mutex_lock(c->mutex);
	return r;
#else
	struct timeval now;
	struct timespec ts;
	int r;

	if (NULL == c) {
		return ND_RET_ERROR;
	}

	if (msec == -1) {
		return nd_cond_wait(c);
	}

	gettimeofday(&now, NULL);

	now.tv_sec += msec / 1000;
	now.tv_usec += (msec % 1000) * 1000;

	if (now.tv_usec >= 1000000) {
		now.tv_sec += now.tv_usec / 1000000;
		now.tv_usec = now.tv_usec % 1000000;
	}

	ts.tv_sec = now.tv_sec;
	ts.tv_nsec = now.tv_usec * 1000;

	r = pthread_cond_timedwait(&c->cond, c->mutex, &ts);
	if (r == 0) return ND_RET_OK;
	else if (r == ETIMEDOUT || (r == -1 && errno == ETIMEDOUT)) return ND_RET_AGAIN;
	else return ND_RET_ERROR;
#endif
}

void
nd_cond_signal(nd_cond_t *c)
{	
#ifdef __ND_WIN32__
	nd_long_t waiters;

	if (NULL == c) {
		return;
	}

	nd_mutex_lock(&c->waiters_mutex);
	waiters = c->waiters;
	nd_mutex_unlock(&c->waiters_mutex);

	if (waiters > 0) {
		nd_sem_post(&c->sem);
	}
#else
	if (NULL == c) {
		return;
	}

	pthread_cond_signal(&c->cond);
#endif
}

nd_int_t 
nd_cond_broadcast(nd_cond_t *c)
{
#ifdef __ND_WIN32__
	nd_int_t r;
	nd_int_t have_waiters;

	have_waiters = 0;

	if (NULL == c) {
		return ND_RET_ERROR;
	}

	nd_mutex_lock(&c->waiters_mutex);

	if (c->waiters > 0) {
		c->was_broadcast = 1;
		have_waiters = 1;
	}
	nd_mutex_unlock(&c->waiters_mutex);

	if (have_waiters) {
		nd_sem_post_n(&c->sem, c->waiters);

		switch (WaitForSingleObject( c->waiters_done, INFINITE)) {
		case WAIT_OBJECT_0:
			r = ND_RET_OK;
			break;
		default:
			r = ND_RET_ERROR;
			break;
		}

		c->was_broadcast = 0;
		return r;
	}

	return ND_RET_OK;
#else
	if (NULL == c) {
		return ND_RET_ERROR;
	}

	pthread_cond_broadcast(&c->cond);
#endif
}