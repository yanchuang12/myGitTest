/** Copyright (c) 2016-2017
 * All rights reserved.
 * 
 * 文件名称:	nd_rudp_event.h   
 * 摘	 要:	
 * 
 * 当前版本：	1.0
 * 作	 者:	范涛涛(FTT)
 * 操	 作:	新建
 * 完成日期:	2016年08月02日
 */
#ifndef __ND_RUDP_EVENT_H_20160802__
#define __ND_RUDP_EVENT_H_20160802__

#include "nd_rudp.h"


#ifdef __cplusplus
extern "C" {
#endif

struct nd_rudp_locked_posted_event_s
{
	__ND_QUEUE_DECALRE__(nd_rudp_locked_posted_event_t);
	nd_ulong_t			msg;
	void				*param;
	nd_rudp_evld_t		*evld;
	void (*handle_event)(nd_rudp_locked_posted_event_t *lev);
};

typedef struct
{
	nd_rudp_locked_posted_event_t		queue;
	nd_uint32_t			size;
}locked_posted_event_queue_t;

struct nd_rudp_evld_s
{
	nd_dlist_t			*posted_ev_que;
	nd_dlist_t			*posted_ev_que2;
	nd_dlist_t			*wait_ev_que;
	nd_dlist_t			*wait_ev_que2;

	locked_posted_event_queue_t	*locked_posted_ev_que;
	locked_posted_event_queue_t	*locked_posted_ev_que2;
	nd_mutex_t			locked_que_mutex;

	/// 定时器队列
	nd_timer_que_t		*timer_que;
	nd_rudp_event_handle_t	*evh;

	volatile nd_int_t	shut_down;
};

nd_rudp_evld_t*
nd_rudp_evld_create(nd_rudp_event_handle_t *ev);

void
nd_rudp_evld_destroy(nd_rudp_evld_t *evld);

void 
nd_rudp_evld_shut_down(nd_rudp_evld_t *evld);

void
nd_rudp_evld_close_wait(nd_rudp_evld_t *evld);

/// wait event

nd_err_t
nd_rudp_evld_add_wait_read(nd_rudp_channel_t *ch);

nd_err_t
nd_rudp_evld_add_wait_write(nd_rudp_channel_t *ch);

nd_err_t
nd_rudp_evld_del_wait_read(nd_rudp_channel_t *ch);

nd_err_t
nd_rudp_evld_del_wait_write(nd_rudp_channel_t *ch);

/// posted event

nd_err_t
nd_rudp_evld_add_posted_read(nd_rudp_channel_t *ch);

nd_err_t
nd_rudp_evld_add_posted_write(nd_rudp_channel_t *ch);

/// locked posted event

#define ND_RUDP_LOCKED_MSG_ADD_READ			1
#define ND_RUDP_LOCKED_MSG_ADD_WRITE		2
#define ND_RUDP_LOCKED_MSG_CLOSE_EP			3
#define ND_RUDP_LOCKED_MSG_DESTROY_EP		4
#define ND_RUDP_LOCKED_MSG_CONNECT			5
#define ND_RUDP_LOCKED_MSG_LISTEN			6

nd_err_t
nd_rudp_evld_locked_add_event(nd_rudp_evld_t *evld, nd_ulong_t msg, void *param);

/// event loop
nd_err_t
nd_rudp_event_run(nd_rudp_evld_t *evld, nd_milli_sec_t msec);

#ifdef __cplusplus
}
#endif

#endif /// __ND_RUDP_EVENT_H_20160802__