/** Copyright (c) 2016-2017
 * All rights reserved.
 * 
 * 文件名称:	nd_rudp_channel.h   
 * 摘	 要:	
 * 
 * 当前版本：	1.0
 * 作	 者:	范涛涛(FTT)
 * 操	 作:	新建
 * 完成日期:	2016年08月02日
 */
#ifndef __ND_RUDP_CHANNEL_H_20160802__
#define __ND_RUDP_CHANNEL_H_20160802__

#include "nd_rudp.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	nd_mutex_t			mutex;
	nd_cond_t			cdt;
	nd_dlist_t			que;
	nd_uint_t			max_size;
}nd_rudp_accept_que_t;

static ND_INLINE void nd_rudp_accept_que_init(nd_rudp_accept_que_t *q)
{
	q->max_size = 10;
	nd_dlist_init(&q->que);
	nd_mutex_init(&q->mutex);
	nd_cond_init(&q->cdt, &q->mutex);
}
void nd_rudp_accept_que_destroy(nd_rudp_accept_que_t *q);

void nd_rudp_accept_que_remove(nd_rudp_accept_que_t *q, nd_rudp_endpoint_t *ep);

nd_err_t nd_rudp_accept_que_enqueue(nd_rudp_accept_que_t *q, nd_rudp_endpoint_t *ep);

nd_rudp_endpoint_t* nd_rudp_accept_que_dequeue(nd_rudp_channel_t *ch, nd_milli_sec_t msec);

static ND_INLINE void nd_rudp_accept_que_broadcast(nd_rudp_accept_que_t *q)
{
	nd_cond_broadcast(&q->cdt);
}

struct nd_rudp_channel_s
{
	nd_uint32_t			token_next;
	nd_dlist_t			eps;
	nd_rudp_accept_que_t	accept_lst;
	ND_SOCKET			s;
	nd_rudp_t			*rudp;
	nd_rudp_endpoint_t	*self;			/// 

	nd_rudp_event_t		rd_ev;
	nd_rudp_event_t		wr_ev;
	nd_rudp_event_t		cl_ev;

	volatile nd_int_t	listen_close;
	/// 作为server使用
	volatile nd_int_t	svr;
	
	unsigned			closed:1;
	unsigned			rd_pending:1;
	unsigned			wr_pending:1;

};

void nd_rudp_channel_insert_ep(nd_rudp_channel_t *ch, nd_rudp_endpoint_t *ep);

void nd_rudp_channel_remove_ep(nd_rudp_channel_t *ch, nd_rudp_endpoint_t *ep);

nd_rudp_channel_t*
nd_rudp_channel_create(nd_rudp_t *rudp, struct sockaddr *local_addr, int addr_len, int fd, int svr);

void
nd_rudp_channel_destroy(nd_rudp_channel_t *ch);

void
nd_rudp_channel_close(nd_rudp_channel_t *ch);

nd_err_t 
nd_rudp_channel_send_to(nd_rudp_channel_t *ch, nd_rudp_packet_t *pkt, nd_inet_addr_t *remote_addr);

nd_err_t
nd_rudp_channel_recv_from(nd_rudp_channel_t *ch, nd_rudp_packet_t *pkt, nd_inet_addr_t *remote_addr);

nd_uint32_t
nd_rudp_channel_next_token(nd_rudp_channel_t *ch);

nd_rudp_endpoint_t*
nd_rudp_channel_find_ep(nd_rudp_channel_t *ch, nd_uint32_t token);

nd_rudp_endpoint_t*
nd_rudp_channel_find_ep2(nd_rudp_channel_t *ch, nd_inet_addr_t *addr);

#ifdef __cplusplus
}
#endif

#endif /// __ND_RUDP_CHANNEL_H_20160802__
