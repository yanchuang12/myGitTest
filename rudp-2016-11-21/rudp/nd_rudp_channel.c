#include "nd_rudp_channel.h"

void 
nd_rudp_accept_que_destroy(nd_rudp_accept_que_t *q)
{
	nd_dlist_node_t *dn;

	nd_mutex_lock(&q->mutex);
	while (dn = nd_dlist_remove_head(&q->que)) {
		/// ep 不要释放, 因为这里只是一个副本
	}
	nd_mutex_unlock(&q->mutex);

	nd_cond_destroy(&q->cdt);
	nd_mutex_destroy(&q->mutex);
}

void 
nd_rudp_accept_que_remove(nd_rudp_accept_que_t *q, nd_rudp_endpoint_t *ep)
{
	nd_mutex_lock(&q->mutex);
	if (ep->in_accept) {
		ep->in_accept = 0;
		nd_dlist_remove_node(&q->que, &ep->node_in_accept);
	}
	nd_mutex_unlock(&q->mutex);
}

nd_err_t 
nd_rudp_accept_que_enqueue(nd_rudp_accept_que_t *q, nd_rudp_endpoint_t *ep)
{
	nd_mutex_lock(&q->mutex);
	if (q->que.size >= q->max_size) {
		nd_mutex_unlock(&q->mutex);
		return ND_RET_ERROR;
	}
	else {
		ep->in_accept = 1;
		nd_dlist_insert_tail(&q->que, &ep->node_in_accept);
		nd_cond_signal(&q->cdt);
	}
	nd_mutex_unlock(&q->mutex);
	return ND_RET_OK;
}

nd_rudp_endpoint_t* 
nd_rudp_accept_que_dequeue(nd_rudp_channel_t *ch, nd_milli_sec_t msec)
{
	nd_dlist_node_t *dn;
	nd_rudp_endpoint_t *ep;
	ep = NULL;

	nd_mutex_lock(&ch->accept_lst.mutex);
	while (!ch->listen_close && 
		ch->accept_lst.que.size == 0 && 
		ND_RET_OK == nd_cond_time_wait(&ch->accept_lst.cdt, msec)) {
	}
	if (!ch->listen_close) {
		dn = nd_dlist_remove_head(&ch->accept_lst.que);
		if (dn) {
			ep = dn->data;
			ep->in_accept = 0;
		}
	}
	nd_mutex_unlock(&ch->accept_lst.mutex);
	return ep;
}

nd_err_t
nd_rudp_channel_set_nonblocking(nd_rudp_channel_t *ch, int b)
{
#ifdef __ND_WIN32__
	unsigned long val;
	val = b;
	if (0 != ioctlsocket(ch->s, FIONBIO, &val))
		return ND_RET_ERROR;
	else
		return ND_RET_OK;
#else
	if (-1 == fcntl(ch->s, F_SETFL, b ? O_NONBLOCK : 0))
		return ND_RET_ERROR;
	else
		return ND_RET_OK;
#endif
}

nd_rudp_channel_t*
nd_rudp_channel_create(nd_rudp_t *rudp, struct sockaddr *local_addr, int addr_len, int fd, int svr)
{
	nd_rudp_channel_t *ch;
	nd_rudp_endpoint_t *ep;
	int opt;

	ch = malloc(sizeof(*ch));
	if (!ch) {
		return NULL;
	}
	memset(ch, 0, sizeof(*ch));

	ch->rudp = rudp;
	ch->token_next = (nd_uint16_t)nd_get_ts();
	ch->svr = svr;

	ch->rd_ev.ch = ch;
	ch->wr_ev.ch = ch;
	ch->cl_ev.ch = ch;

	nd_dlist_init(&ch->eps);
	nd_rudp_accept_que_init(&ch->accept_lst);

	if (fd == ND_INVALID_SOCKET) {
		ch->s = socket(AF_INET, SOCK_DGRAM, 0);
		if (ch->s == ND_INVALID_SOCKET) {
			free(ch);
			return NULL;
		}

	#ifdef __ND_WIN32__
		#define ND_RUDP_SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR,12)
		if (1) {
			DWORD udp_opt = 0, udp_btr = 0;
			WSAIoctl(ch->s, ND_RUDP_SIO_UDP_CONNRESET, &udp_opt, sizeof(udp_opt), NULL, 0, &udp_btr, NULL, NULL);
		}
	#endif

		if (local_addr) {
			if (0 != bind(ch->s, local_addr, addr_len)) {
				nd_rudp_channel_destroy(ch);
				return NULL;
			}
		}
	}
	else {
		ch->s = fd;
	}

#if defined(BSD) || defined(OSX)
	opt = 64000;
	setsockopt(ch->s, SOL_SOCKET, SO_RCVBUF, (char*)&opt, sizeof(opt));
	opt = 64000;
	setsockopt(ch->s, SOL_SOCKET, SO_SNDBUF, (char*)&opt, sizeof(opt));
#else
	opt = 512 * 1024;
	setsockopt(ch->s, SOL_SOCKET, SO_RCVBUF, (char*)&opt, sizeof(opt));
	opt = 512 * 1024;
	setsockopt(ch->s, SOL_SOCKET, SO_SNDBUF, (char*)&opt, sizeof(opt));
#endif

	/// 设为非阻塞模式
	if (ND_RET_OK != nd_rudp_channel_set_nonblocking(ch, 1)) {
		nd_rudp_channel_destroy(ch);
		return NULL;
	}

	ep = nd_rudp_endpoint_create();
	if (!ep) {
		nd_rudp_channel_destroy(ch);
		return NULL;
	}

	ch->self = ep;
	ep->ch = ch;

	if (!ch->svr) {
		if (ND_RET_OK != nd_rudp_endpoint_init(ep)) {
			nd_rudp_channel_destroy(ch);
			return NULL;
		}
		ep->type = ND_RUDP_EP_TYPE_CONNECT;
		ep->idle_tm_node.node.key = nd_get_ts() + ep->idle_tm_node.inter_usec;
		nd_timer_que_schedule(ch->rudp->evld->timer_que, &ep->idle_tm_node);
	}
	else {
		ep->type = ND_RUDP_EP_TYPE_LISTEN;
	}

	return ch;
}

void
nd_rudp_channel_destroy(nd_rudp_channel_t *ch)
{
	nd_dlist_node_t *dn;

	if (!ch)
		return;

	nd_rudp_channel_close(ch);

	if (ch->self) {
		nd_rudp_endpoint_destroy(ch->self);
	}

	nd_rudp_accept_que_destroy(&ch->accept_lst);

	while (dn = nd_dlist_remove_head(&ch->eps)) {
		nd_rudp_endpoint_destroy(dn->data);
	}

	free(ch);
}

void
nd_rudp_channel_close(nd_rudp_channel_t *ch)
{
	ch->closed = 1;

	if (ch->s != -1) {
#ifdef __ND_WIN32__
		closesocket(ch->s);
#else
		close(ch->s);
#endif
		ch->s = -1;
	}
}


nd_err_t 
nd_rudp_channel_send_to(nd_rudp_channel_t *ch, nd_rudp_packet_t *pkt, nd_inet_addr_t *remote_addr)
{
	struct sockaddr_in so_addr;
	nd_int_t err;

	if (ch->closed) {
		return ND_RET_ERROR;
	}

	memset(&so_addr, 0, sizeof(so_addr));
	so_addr.sin_family = AF_INET;
	so_addr.sin_addr.s_addr = htonl(remote_addr->ip);
	so_addr.sin_port = htons(remote_addr->port);

	err = sendto(ch->s, nd_rudp_packet_rd_ptr(pkt), nd_rudp_packet_length(pkt), 0, (struct sockaddr*)&so_addr, sizeof(so_addr));

#ifdef __ND_WIN32__
	if (err == SOCKET_ERROR) {
		if (GetLastError() == WSAEWOULDBLOCK) {
			ch->wr_pending = 1;
			return ND_RET_AGAIN;
		}
		else
			return ND_RET_ERROR;
	}
	else {
		ch->wr_pending = 0;
	}
#else
	if (err == -1) {
		if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR) {
			ch->wr_pending = 1;
			return ND_RET_AGAIN;
		}
		else
			return ND_RET_ERROR;
	}
	else {
		ch->wr_pending = 0;
	}
#endif

	if (err != nd_rudp_packet_length(pkt)) {
		nd_assert(0);
		return ND_RET_ERROR;
	}

	pkt->snd_tm = nd_get_ts();
	return ND_RET_OK;
}

nd_err_t
nd_rudp_channel_recv_from(nd_rudp_channel_t *ch, nd_rudp_packet_t *pkt, nd_inet_addr_t *remote_addr)
{
	struct sockaddr_in so_addr;
	nd_int_t err, addr_len;

	if (ch->closed) {
		return ND_RET_ERROR;
	}

	addr_len = sizeof(so_addr);
	err = recvfrom(ch->s, nd_rudp_packet_wr_ptr(pkt), nd_rudp_packet_space(pkt), 0, (struct sockaddr*)&so_addr, &addr_len);

#ifdef __ND_WIN32__
	if (err == SOCKET_ERROR) {
		if (GetLastError() == WSAEWOULDBLOCK) {
			ch->rd_pending = 1;
			return ND_RET_AGAIN;
		}
		else
			return ND_RET_ERROR;
	}
	else {
		ch->rd_pending = 0;
	}
#else
	if (err == -1) {
		if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR) {
			ch->rd_pending = 1;
			return ND_RET_AGAIN;
		}
		else
			return ND_RET_ERROR;
	}
	else {
		ch->rd_pending = 0;
	}
#endif

	nd_rudp_packet_add_wr_pos(pkt, err);

	remote_addr->ip = ntohl(so_addr.sin_addr.s_addr);
	remote_addr->port = ntohs(so_addr.sin_port);
	pkt->rcv_tm = nd_get_ts();

	return ND_RET_OK;
}

nd_rudp_endpoint_t*
nd_rudp_channel_find_ep(nd_rudp_channel_t *ch, nd_uint32_t token)
{
	nd_dlist_node_t *n;
	nd_rudp_endpoint_t *ep;
	nd_queue_for_each(n, &ch->eps.header)
	{
		ep = (nd_rudp_endpoint_t*)n->data;
		if (ep->token == token) {
			return ep;
		}
	}
	return NULL;
}

nd_rudp_endpoint_t*
nd_rudp_channel_find_ep2(nd_rudp_channel_t *ch, nd_inet_addr_t *addr)
{
	nd_dlist_node_t *n;
	nd_rudp_endpoint_t *ep;
	nd_queue_for_each(n, &ch->eps.header)
	{
		ep = (nd_rudp_endpoint_t*)n->data;
		if (ep->remote_addr.ip == addr->ip &&
			ep->remote_addr.port == addr->port) {
			return ep;
		}
	}
	return NULL;
}

void nd_rudp_channel_insert_ep(nd_rudp_channel_t *ch, nd_rudp_endpoint_t *ep)
{
	if (!ep->in_ch) {
		ep->in_ch = 1;
		nd_dlist_insert_tail(&ch->eps, &ep->node_in_ch);
	}
}

void nd_rudp_channel_remove_ep(nd_rudp_channel_t *ch, nd_rudp_endpoint_t *ep)
{
	if (ep->in_ch) {
		ep->in_ch = 0;
		nd_dlist_remove_node(&ch->eps, &ep->node_in_ch);
	}
}

nd_uint32_t
nd_rudp_channel_next_token(nd_rudp_channel_t *ch)
{
	nd_uint32_t i;

	for (i = 0; i < 0xffff; i++) {
		if (0 == ch->token_next) {
			ch->token_next++;
		}
		else {
			if (NULL == nd_rudp_channel_find_ep(ch, ch->token_next))
				return ch->token_next++;
			else
				ch->token_next++;
		}
	}

	return 0;
}