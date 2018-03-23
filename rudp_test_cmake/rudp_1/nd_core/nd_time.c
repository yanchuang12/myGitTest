#include "nd_core.h"


nd_uint64_t nd_get_timeofday()
{
#ifdef __ND_WIN32__
	FILETIME filetime;
	nd_uint64_t time;
	GetSystemTimeAsFileTime(&filetime);

	time |= filetime.dwHighDateTime;
	time <<= 32;
	time |= filetime.dwLowDateTime;

	time /= 10;
	time -= 11644473600000000LL;
	/// time /= 1000;

	return time;
#else
	struct timeval t;
	gettimeofday(&t, 0);
	return t.tv_sec * 1000000ULL + t.tv_usec;
#endif
}

#ifdef __ND_WIN32__
static nd_uint64_t __nd_tm_freq = 0;

nd_uint64_t __nd_get_cpu_freq()
{
	if (__nd_tm_freq == 0) {
		nd_int64_t ccf;
		if (QueryPerformanceFrequency((LARGE_INTEGER *)&ccf)) {
			__nd_tm_freq = ccf;
		}
		else
			__nd_tm_freq = 0;
	}
	return __nd_tm_freq;
}
#endif

nd_uint64_t nd_get_ts()
{
#ifdef WIN32
	HANDLE hCurThread;
	DWORD_PTR dwOldMask;
	nd_uint64_t freq;

	hCurThread = GetCurrentThread(); 
	dwOldMask = SetThreadAffinityMask(hCurThread, 1);
	freq = __nd_get_cpu_freq();

	if (freq)
	{
		LARGE_INTEGER cc;
		if (QueryPerformanceCounter(&cc)) {
			SetThreadAffinityMask(hCurThread, dwOldMask); 
			return (cc.QuadPart * 1000000ULL / freq);
		}
	}

	SetThreadAffinityMask(hCurThread, dwOldMask); 
	return nd_get_timeofday();
#else	
	return nd_get_timeofday();
#endif
}

nd_timer_que_t*
nd_timer_que_create(nd_uint_t max_size)
{
	nd_timer_que_t *q;
	q = malloc(sizeof(nd_timer_que_t));
	if (!q)
		return NULL;

	memset(q, 0, sizeof(nd_timer_que_t));

	q->heap = nd_min_heap_create(max_size);
	if (!q->heap) {
		free(q);
		return NULL;
	}

	return q;
}

void
nd_timer_que_remove_all(nd_timer_que_t *tq)
{
	if (tq) {
		nd_heap_node_t *n;
		while (n = nd_min_heap_remove_first(tq->heap)) 
		{}
	}
}

void
nd_timer_que_destroy(nd_timer_que_t *tq)
{
	if (!tq)
		return;

	nd_min_heap_destroy(tq->heap);
}

nd_tq_node_t*
nd_timer_que_first_node(nd_timer_que_t *tq)
{
	nd_heap_node_t *node;
	node = nd_min_heap_first(tq->heap);
	
	return node ? (nd_tq_node_t*)node->data : NULL;
}

void 
nd_timer_que_dispatch(nd_timer_que_t *tq)
{
	nd_uint64_t curr_tm;
	nd_heap_node_t *node;

	tq->dispatch = 1;
	tq->wait_lst = NULL;

	curr_tm = nd_get_ts();

	node = nd_min_heap_first(tq->heap);
	while (node) {
		if (nd_tm_u64_diff((nd_uint64_t)node->key, curr_tm) < 1) {
			nd_tq_node_t *tmp;
			node = nd_min_heap_remove_first(tq->heap);
			node->id = -1;
			tmp = node->data;
			tmp->handle_timer_out(tmp);
			node = nd_min_heap_first(tq->heap);
		}
		else {
			break;
		}
		curr_tm = nd_get_ts();
	}

	if (tq->wait_lst) {
		while (tq->wait_lst) {
			nd_tq_node_t *next = tq->wait_lst->link;
			tq->wait_lst->link = NULL;
			tq->wait_lst->in_wait_lst = 0;
			nd_min_heap_insert(tq->heap, &tq->wait_lst->node);
			tq->wait_lst = next;
		}
	}

	tq->dispatch = 0;
}

nd_err_t 
nd_timer_que_schedule(nd_timer_que_t *tq, nd_tq_node_t *node)
{
	if (node->in_wait_lst || node->node.id != -1)
		return ND_RET_OK;

	if (tq->dispatch) {
		node->in_wait_lst = 1;
		node->link = tq->wait_lst;
		tq->wait_lst = node;
		return ND_RET_OK;
	}
	else {
		return nd_min_heap_insert(tq->heap, &node->node);
	}
}

nd_err_t 
nd_timer_que_update_schedule(nd_timer_que_t *tq, nd_tq_node_t *node, nd_uint64_t new_tm)
{
	nd_uint64_t old = node->node.key;

	node->node.key = new_tm;

	if (node->in_wait_lst)
		return ND_RET_OK;

	if (tq->dispatch) {
		node->in_wait_lst = 1;
		node->link = tq->wait_lst;
		tq->wait_lst = node;
		return ND_RET_OK;
	}
	else {
		if (node->node.id != -1) {
			if (new_tm != old) {
				nd_min_heap_remove(tq->heap, &node->node);
				return nd_min_heap_insert(tq->heap, &node->node);
			}
			else
				return ND_RET_OK;
		}
		else
			return nd_min_heap_insert(tq->heap, &node->node);
	}

	return ND_RET_ERROR;
}

void
nd_timer_que_remove(nd_timer_que_t *tq, nd_tq_node_t *node)
{
	if (!node)
		return;

	if (node->in_wait_lst) {
		nd_tq_node_t *find;

		if (tq->wait_lst == node) {
			tq->wait_lst = tq->wait_lst->link;
			node->in_wait_lst = 0;
			return;
		}

		for (find = tq->wait_lst; find; find = find->link) {
			if (find->link == node) {
				find->link = node->link;
				node->link = NULL;
				node->in_wait_lst = 0;
				return;
			}
		}
	}
	else {
		if (node->node.id != -1)
			nd_min_heap_remove(tq->heap, &node->node);
	}
}