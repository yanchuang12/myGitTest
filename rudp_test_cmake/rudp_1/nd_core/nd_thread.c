
#include "nd_core.h"

/* - * - * - * - * - */
#ifdef __ND_WIN32__
static DWORD ND_CALL_PRO 
#else
static void* ND_CALL_PRO
#endif
__thread_fun(void *arg)
{
	nd_thread_t *t;
	nd_int_t r;

	t = (nd_thread_t*)arg;
	t->active = 1;

	r = t->fun_ptr(t->arg);

#ifdef __ND_WIN32__
	return r;
#else
	return 0;
#endif
}

void 
nd_thread_exit(nd_thread_t *t)
{
	nd_mutex_lock(&t->mutex);

	t->thr_cnt = 0;
	t->shut_down = 1;
	nd_cond_broadcast(&t->cond);

	nd_mutex_unlock(&t->mutex);

	nd_thread_destroy(t);
}

nd_int_t
nd_thread_create(nd_thread_t *t)
{
	if (NULL == t->fun_ptr) {
		return ND_RET_ERROR;
	}

	t->shut_down = 0;
	t->thr_cnt = 0;

	nd_mutex_init(&t->mutex);
	nd_cond_init(&t->cond, &t->mutex);

#ifdef __ND_WIN32__
	t->h = CreateThread(NULL, 0, &__thread_fun, (void*)t, 0, &t->tid);
	if (NULL == t->h) {
		nd_cond_destroy(&t->cond);
		nd_mutex_destroy(&t->mutex);
		t->shut_down = 1;
		return ND_RET_ERROR;
	}
	t->thr_cnt = 1;
	return ND_RET_OK;
#else
	if (1) {
		pthread_attr_t attr;

		if (pthread_attr_init(&attr) != 0) return ND_RET_ERROR;

		if (pthread_create(&t->tid, &attr, &__thread_fun, t) == 0) {
			t->h = t->tid;
			t->thr_cnt = 1;
			pthread_detach(t->tid);
		}
		else {
			pthread_attr_destroy(&attr);
			nd_cond_destroy(&t->cond);
			nd_mutex_destroy(&t->mutex);
			return ND_RET_ERROR;
		}

		pthread_attr_destroy(&attr);
	}
	return ND_RET_OK;
#endif
}

void
nd_thread_shut_down(nd_thread_t *t)
{
	t->shut_down = 1;
}

nd_int_t
nd_thread_wait(nd_thread_t *t)
{
	nd_int_t r;
	r = ND_RET_OK;

#ifdef __ND_WIN32__
	if (t->h != NULL) {
		CloseHandle(t->h);
	}
#endif

	nd_mutex_lock(&t->mutex);

	if (t->thr_cnt) {
		r = nd_cond_wait(&t->cond);
	}
	if (t->thr_cnt == 0) {
		r = ND_RET_OK;
	}

	nd_mutex_unlock(&t->mutex);

	return r;
}

nd_int_t
nd_thread_time_wait(nd_thread_t *t, nd_milli_sec_t msec)
{
	nd_int_t r;
	r = ND_RET_OK;

#ifdef __ND_WIN32__
	if (t->h != NULL) {
		CloseHandle(t->h);
		t->h = NULL;
	}
#endif

	nd_mutex_lock(&t->mutex);

	if (t->thr_cnt) {
		r = nd_cond_time_wait(&t->cond, msec);
	}
	if (t->thr_cnt == 0) {
		r = ND_RET_OK;
	}

	nd_mutex_unlock(&t->mutex);

	return r;
}

void
nd_thread_destroy(nd_thread_t *t)
{
#ifdef __ND_WIN32__
	if (t->h != NULL) {
		CloseHandle(t->h);
		t->h = NULL;
	}
#endif

	nd_cond_destroy(&t->cond);
	nd_mutex_destroy(&t->mutex);
}

