#include "nd_rudp_event.h"

static void 
__handle_locked_post_event(nd_rudp_locked_posted_event_t *ev);

static void
__handle_read(nd_rudp_event_t *ev);

static void
__handle_write(nd_rudp_event_t *ev);


/// queue (wait or posted)

static ND_INLINE nd_bool_t 
nd_rudp_ev_que_insert_tail(nd_rudp_evld_t *evld, nd_dlist_t *q, nd_rudp_event_t *ev)
{
	nd_dlist_node_t* n;
	n = malloc(sizeof(nd_dlist_node_t));
	if (!n) return 0;
	n->data = ev;
	ev->node = n;
	return nd_dlist_insert_tail(q, n);
}

static ND_INLINE nd_rudp_event_t* 
nd_rudp_ev_que_remove_header(nd_rudp_evld_t* evld, nd_dlist_t* q)
{
	nd_dlist_node_t* n;
	nd_rudp_event_t* ev;

	n = nd_dlist_remove_head(q);

	if (!n) {
		return NULL;
	}
	else {
		ev = (nd_rudp_event_t*)n->data;
		free(n);
		return ev;
	}	
}

static ND_INLINE void
nd_rudp_ev_que_remove(nd_rudp_evld_t* evld, nd_dlist_t* q, nd_rudp_event_t *ev)
{
	nd_dlist_remove_node(q, ev->node);
	free(ev->node);
	ev->node = NULL;
}


static ND_INLINE nd_bool_t
nd_rudp_ev_que_is_empty(nd_dlist_t* q)
{
	return nd_dlist_is_empty(q);
}

/// lock queue

static ND_INLINE void 
nd_rudp_locked_ev_que_insert_tail(locked_posted_event_queue_t *q, nd_rudp_locked_posted_event_t *lev)
{
	nd_queue_insert_tail(&q->queue, lev);
	q->size++;
}

static ND_INLINE nd_rudp_locked_posted_event_t* 
nd_rudp_locked_ev_que_remove_header(locked_posted_event_queue_t *q)
{
	if (nd_queue_is_empty(&q->queue)) {
		return NULL;
	}
	else {
		nd_rudp_locked_posted_event_t* ev = nd_queue_head(&q->queue);
		nd_queue_remove(ev);
		q->size--;
		return ev;
	}
}

/// event loop driver

nd_rudp_evld_t*
nd_rudp_evld_create(nd_rudp_event_handle_t *evh)
{
	nd_rudp_evld_t *evld;

	evld = malloc(sizeof(nd_rudp_evld_t));
	if (!evld) {
		return NULL;
	}

	memset(evld, 0, sizeof(nd_rudp_evld_t));

	evld->timer_que = nd_timer_que_create(256);

	evld->posted_ev_que = malloc(sizeof(nd_dlist_t));
	evld->posted_ev_que2 = malloc(sizeof(nd_dlist_t));
	evld->wait_ev_que = malloc(sizeof(nd_dlist_t));
	evld->wait_ev_que2 = malloc(sizeof(nd_dlist_t));

	nd_dlist_init(evld->posted_ev_que);
	nd_dlist_init(evld->posted_ev_que2);
	nd_dlist_init(evld->wait_ev_que);
	nd_dlist_init(evld->wait_ev_que2);

	evld->locked_posted_ev_que = malloc(sizeof(locked_posted_event_queue_t));
	evld->locked_posted_ev_que2 = malloc(sizeof(locked_posted_event_queue_t));
	nd_queue_init(&evld->locked_posted_ev_que->queue);
	nd_queue_init(&evld->locked_posted_ev_que2->queue);

	nd_mutex_init(&evld->locked_que_mutex);

	evld->evh = evh;

	return evld;
}

void
nd_rudp_evld_destroy(nd_rudp_evld_t *evld)
{
	if (evld->timer_que) {
		nd_timer_que_destroy(evld->timer_que);
	}

	if (evld->wait_ev_que) {
		free(evld->wait_ev_que);
	}
	if (evld->wait_ev_que2) {
		free(evld->wait_ev_que2);
	}

	if (evld->posted_ev_que) {
		free(evld->posted_ev_que);
	}
	if (evld->posted_ev_que2) {
		free(evld->posted_ev_que2);
	}

	nd_mutex_lock(&evld->locked_que_mutex);
	if (evld->locked_posted_ev_que) {
		free(evld->locked_posted_ev_que);
	}
	if (evld->locked_posted_ev_que2) {
		free(evld->locked_posted_ev_que2);
	}
	nd_mutex_unlock(&evld->locked_que_mutex);

	nd_mutex_destroy(&evld->locked_que_mutex);

	free(evld);
}

void 
nd_rudp_evld_shut_down(nd_rudp_evld_t *evld)
{
	evld->shut_down = 1;
}

void
nd_rudp_evld_close_wait(nd_rudp_evld_t *evld)
{
	if (evld->timer_que) {
		nd_timer_que_remove_all(evld->timer_que);
	}

	if (evld->wait_ev_que) {
		while (nd_rudp_ev_que_remove_header(evld, evld->wait_ev_que)) 
		{}
	}

	if (evld->wait_ev_que2) {
		while (nd_rudp_ev_que_remove_header(evld, evld->wait_ev_que2)) 
		{}
	}

	if (evld->posted_ev_que) {
		while (nd_rudp_ev_que_remove_header(evld, evld->posted_ev_que)) 
		{}
	}
	if (evld->posted_ev_que2) {
		while (nd_rudp_ev_que_remove_header(evld, evld->posted_ev_que2)) 
		{}
	}

	nd_mutex_lock(&evld->locked_que_mutex);

	if (evld->locked_posted_ev_que) {
		nd_rudp_locked_posted_event_t *lev;
		while (lev = nd_rudp_locked_ev_que_remove_header(evld->locked_posted_ev_que)) 
		{
			free(lev);
		}
	}
	if (evld->locked_posted_ev_que2) {
		nd_rudp_locked_posted_event_t *lev;
		while (lev = nd_rudp_locked_ev_que_remove_header(evld->locked_posted_ev_que2)) 
		{
			free(lev);
		}
	}

	nd_mutex_unlock(&evld->locked_que_mutex);
}

/// wait event
nd_err_t
nd_rudp_evld_add_wait_read(nd_rudp_channel_t *ch)
{
	if (ch->closed){
		return ND_RET_ERROR;
	}
	else if (ch->rd_ev.wait_pending == 0 && ch->rd_ev.posted_pending == 0) {
		ch->rd_ev.handle_event = &__handle_read;
		nd_rudp_ev_que_insert_tail(ch->rudp->evld, ch->rudp->evld->wait_ev_que, &ch->rd_ev);
		ch->rd_ev.wait_pending = 1;
	}
	return ND_RET_OK;
}

nd_err_t
nd_rudp_evld_add_wait_write(nd_rudp_channel_t *ch)
{
	if (ch->closed){
		return ND_RET_ERROR;
	}
	else if (ch->wr_ev.wait_pending == 0 && ch->wr_ev.posted_pending == 0) {
		ch->wr_ev.handle_event = &__handle_write;
		nd_rudp_ev_que_insert_tail(ch->rudp->evld, ch->rudp->evld->wait_ev_que, &ch->wr_ev);
		ch->wr_ev.wait_pending = 1;
	}
	return ND_RET_OK;
}

nd_err_t
nd_rudp_evld_del_wait_read(nd_rudp_channel_t *ch)
{
	if (ch->rd_ev.wait_pending == 1) {
		nd_rudp_ev_que_remove(ch->rudp->evld, ch->rudp->evld->wait_ev_que, &ch->rd_ev);
		ch->rd_ev.wait_pending = 0;
	}
	return ND_RET_OK;
} 

nd_err_t
nd_rudp_evld_del_wait_write(nd_rudp_channel_t *ch)
{
	if (ch->wr_ev.wait_pending == 1) {
		nd_rudp_ev_que_remove(ch->rudp->evld, ch->rudp->evld->wait_ev_que, &ch->wr_ev);
		ch->wr_ev.wait_pending = 0;
	}
	return ND_RET_OK;
}

/// posted event

nd_err_t
nd_rudp_evld_add_posted_read(nd_rudp_channel_t *ch)
{
	if (ch->closed){
		return ND_RET_ERROR;
	}
	else if (ch->rd_ev.posted_pending == 0 && ch->rd_ev.wait_pending == 0) {
		ch->rd_ev.handle_event = &__handle_read;
		nd_rudp_ev_que_insert_tail(ch->rudp->evld, ch->rudp->evld->posted_ev_que, &ch->rd_ev);
		ch->rd_ev.posted_pending = 1;
	}
	return ND_RET_OK;
}

nd_err_t
nd_rudp_evld_add_posted_write(nd_rudp_channel_t *ch)
{
	if (ch->closed){
		return ND_RET_ERROR;
	}
	else if (ch->wr_ev.posted_pending == 0 && ch->wr_ev.wait_pending == 0) {
		ch->wr_ev.handle_event = &__handle_write;
		nd_rudp_ev_que_insert_tail(ch->rudp->evld, ch->rudp->evld->posted_ev_que, &ch->wr_ev);
		ch->wr_ev.posted_pending = 1;
	}
	return ND_RET_OK;
}

/// locked posted event

nd_err_t
nd_rudp_evld_locked_add_event(nd_rudp_evld_t *evld, nd_ulong_t msg, void *param)
{
	nd_rudp_locked_posted_event_t *ev;

	ev = malloc(sizeof(nd_rudp_locked_posted_event_t));
	if (!ev)
		return ND_RET_ERROR;

	ev->evld = evld;
	ev->msg = msg;
	ev->param = param;
	ev->handle_event = &__handle_locked_post_event;

	nd_mutex_lock(&evld->locked_que_mutex);
	nd_rudp_locked_ev_que_insert_tail(evld->locked_posted_ev_que, ev);
	nd_mutex_unlock(&evld->locked_que_mutex);

	return ND_RET_OK;
}


/// event loop

static nd_err_t
nd_rudp_event_wait(nd_rudp_evld_t *evld, nd_uint64_t usec)
{
	fd_set rd_set, wr_set;
	nd_rudp_event_t *ev;
	nd_dlist_node_t *nd;
	nd_dlist_t *wait_que;
	struct timeval tv, *tp;
	int ready, valid, m, width;

	m = -1;
	memset(&rd_set, 0, sizeof(rd_set));
	memset(&wr_set, 0, sizeof(wr_set));

	if (usec == -1) {
		tp = NULL;
	} else {
		tv.tv_sec = (long)(usec / 1000000);
		tv.tv_usec = (long)(usec % 1000000);
		tp = &tv;
	}

	valid = 0;

	wait_que = evld->wait_ev_que;
	//evld->wait_ev_que = evld->wait_ev_que2;
	//evld->wait_ev_que2 = wait_que;

	//for (nd = (&wait_que->header)->next; (nd) != (&wait_que->header); nd = nd->next) {
	nd_queue_for_each(nd, &wait_que->header) {
		ev = nd->data;
		if (ev->ch->s == ND_INVALID_SOCKET) {
			continue;
		}

		if (ev == &ev->ch->rd_ev) {
			FD_SET (ev->ch->s, &rd_set);
			valid = 1;
#ifndef __ND_WIN32__
			if (ev->ch->s > m) {
				m = ev->ch->s;
			}
#endif
		} 
		else if (ev == &ev->ch->wr_ev){
			FD_SET (ev->ch->s, &wr_set);
			valid = 1;
#ifndef __ND_WIN32__
			if (ev->ch->s > m) {
				m = ev->ch->s;
			}
#endif
		}
	}

	if (!valid) {
		if (usec) {
			nd_sleep(usec / 1000);
		}
		return ND_RET_AGAIN;
	}

#ifdef __ND_WIN32__
	width = 0;
#else
	if (m == -1) {
		if (usec)
			nd_sleep(usec / 1000);
		return ND_RET_AGAIN;
	}
	width = m + 1;
#endif

	//timeBeginPeriod(1);
	ready = select(width, &rd_set, &wr_set, NULL, tp);
	//timeEndPeriod(1);

	if (ready == -1) {
		return ND_RET_ERROR;
	} 
	else if (ready == 0) {
		return ND_RET_AGAIN;
	}
	else {
		//while (ev = nd_rudp_ev_que_remove_header(evld, wait_que)) {
		for (nd = (&wait_que->header)->next; nd != (&wait_que->header); ) {
			ev = nd->data;		
			nd = nd->next;

			if (ev == &ev->ch->rd_ev) {
				if (FD_ISSET(ev->ch->s, &rd_set)) {
					ev->ch->rd_pending = 0;
					ev->wait_pending = 0;
					nd_rudp_ev_que_remove(evld, wait_que, ev);
					nd_rudp_evld_add_posted_read(ev->ch);				
				}
				else {
					//nd_rudp_evld_add_wait_read(ev->ch);
					ev->wait_pending = 1;
				}
			} else if (ev == &ev->ch->wr_ev) {
				if (FD_ISSET(ev->ch->s, &wr_set)) {
					ev->ch->wr_pending = 0;
					ev->wait_pending = 0;
					nd_rudp_ev_que_remove(evld, wait_que, ev);
					nd_rudp_evld_add_posted_write(ev->ch);				
				}
				else {
					//nd_rudp_evld_add_wait_write(ev->ch);
					ev->wait_pending = 1;
				}
			}
		}
	}

	return ND_RET_OK;
}

nd_err_t
nd_rudp_event_run(nd_rudp_evld_t *evld, nd_milli_sec_t msec)
{
	nd_err_t r;
	nd_uint64_t init_tm, curr_tm, tmo;
	nd_dlist_t	*que;
	locked_posted_event_queue_t *locked_que;
	nd_rudp_event_t *ev;
	nd_tq_node_t *tq_node;

	init_tm = nd_get_ts();

	while (!evld->shut_down) 
	{evld->cnt++;
		/// 默认1毫秒
		tmo = 1000;
		curr_tm = nd_get_ts();
		if (msec != -1 && curr_tm - init_tm >= msec * 1000) {
			return ND_RET_AGAIN;
		}

		nd_timer_que_dispatch(evld->timer_que);
		tq_node = nd_timer_que_first_node(evld->timer_que);
		if (tq_node) {
			tmo = tq_node->node.key - nd_get_ts();
		}

#ifdef __ND_WIN32__
		if ((nd_int64_t)tmo < 1)
			tmo = 0;
		else
			tmo = 1;
#else
		if ((nd_int_t)tmo < 1)
			tmo = 0;
		else if ((nd_int_t)tmo > 1000)
			tmo = 1000;
#endif

		if (tmo > 0) {
			if (!nd_rudp_ev_que_is_empty(evld->posted_ev_que) ||
				!nd_queue_is_empty(&evld->locked_posted_ev_que->queue)) {
					tmo = 0;
			}
		}

		r = nd_rudp_event_wait(evld, tmo);

		if (ND_RET_ERROR == r) {
			return ND_RET_ERROR;
		}
	
		que = evld->posted_ev_que;
		evld->posted_ev_que = evld->posted_ev_que2;
		evld->posted_ev_que2 = que;

		while (ev = nd_rudp_ev_que_remove_header(evld, que)) {
			ev->posted_pending = 0;
			if (ev && ev->handle_event) {
				ev->handle_event(ev);
			}
		}

		if (!nd_queue_is_empty(&evld->locked_posted_ev_que->queue)) {
			nd_rudp_locked_posted_event_t *lev;

			nd_mutex_lock(&evld->locked_que_mutex);

			locked_que = evld->locked_posted_ev_que;	
			evld->locked_posted_ev_que = evld->locked_posted_ev_que2;
			evld->locked_posted_ev_que2 = locked_que;

			nd_mutex_unlock(&evld->locked_que_mutex);

			while (lev = nd_rudp_locked_ev_que_remove_header(locked_que)) {
				lev->handle_event(lev);
				/// @lev 需要释放
				free(lev);
			}
		}
	}

	return ND_RET_OK;
}

static void 
__handle_locked_post_event(nd_rudp_locked_posted_event_t *lev)
{
	lev->evld->evh->handle_locked_posted_event(lev);
}

static void
__handle_read(nd_rudp_event_t *ev)
{
	ev->ch->rudp->evld->evh->handle_read(ev);
}

static void
__handle_write(nd_rudp_event_t *ev)
{
	ev->ch->rudp->evld->evh->handle_write(ev);
}