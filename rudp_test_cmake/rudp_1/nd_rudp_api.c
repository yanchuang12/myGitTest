#include "nd_rudp_api.h"
#include "nd_rudp.h"

int 
ndrudp_init()
{
#ifdef __ND_WIN32__
	WSADATA wd;
	if (NO_ERROR != WSAStartup(MAKEWORD(2,2), &wd)) {
		return -1;
	}
#endif
	if (g_global_mgr)
		return 0;

	g_global_mgr = malloc(sizeof(nd_global_mgr_t));
	if (!g_global_mgr)
		return -1;

	memset(g_global_mgr, 0, sizeof(nd_global_mgr_t));

	nd_queue_init(&g_global_mgr->rudp_lst);
	nd_mutex_init(&g_global_mgr->rl_mutex);
	return 0;
}

int ndrudp_init_log(int level, int opt, const char *fn)
{
	if (ND_RET_OK == nd_rudp_log_init(level, opt, fn))
		return 0;
	else
		return -1;
}

void 
ndrudp_uninit()
{
	nd_rudp_t * rudp;

	if (!g_global_mgr) {
		goto do_end; 
	}

	nd_mutex_lock(&g_global_mgr->rl_mutex);

	nd_queue_for_each(rudp, &g_global_mgr->rudp_lst) {
		nd_thread_shut_down(&rudp->th);
	}

	while (!nd_queue_is_empty(&g_global_mgr->rudp_lst)) {
		nd_mutex_unlock(&g_global_mgr->rl_mutex);
		nd_sleep(1000);
		nd_mutex_lock(&g_global_mgr->rl_mutex);
	}

	nd_mutex_unlock(&g_global_mgr->rl_mutex);

	nd_mutex_destroy(&g_global_mgr->rl_mutex);

	nd_rudp_log_un_init();

	free(g_global_mgr);
	g_global_mgr = NULL;

do_end:
#ifdef __ND_WIN32__
	WSACleanup();
#endif
	return;
}

ND_RUDP_SOCKET 
ndrudp_socket(long local_ip, int local_port, int fd, int svr)
{
	struct sockaddr_in so_addr;
	nd_rudp_t *rudp;

	memset(&so_addr, 0, sizeof(so_addr));
	so_addr.sin_family = AF_INET;
	so_addr.sin_addr.s_addr = htonl(local_ip);
	so_addr.sin_port = htons(local_port);

	rudp = nd_rudp_create((struct sockaddr*)&so_addr, sizeof(so_addr), fd, svr);
	if (!rudp) {
		return NULL;
	}

	return rudp->ch->self;
}

void 
ndrudp_close(ND_RUDP_SOCKET s)
{
	nd_rudp_endpoint_t *ep;

	ep = (nd_rudp_endpoint_t*)s;

	if (!ep)
		return;

	nd_rudp_evld_locked_add_event(ep->ch->rudp->evld, ND_RUDP_LOCKED_MSG_CLOSE_EP, ep);
}


int 
ndrudp_listen(ND_RUDP_SOCKET s)
{
	nd_rudp_endpoint_t *ep;

	ep = (nd_rudp_endpoint_t*)s;
	if (!ep || !ep->ch->svr) {
		nd_assert(0);
		return -1;
	}

	nd_rudp_evld_locked_add_event(ep->ch->rudp->evld, ND_RUDP_LOCKED_MSG_LISTEN, ep);

	return 0;
}

int 
ndrudp_connect(ND_RUDP_SOCKET s, long ip, int port, int msec)
{
	nd_rudp_endpoint_t *ep;
	nd_uint64_t tm;

	ep = (nd_rudp_endpoint_t*)s;
	if (!ep || 
		ep->ch->svr ||
		ep->status >= ND_RUDP_EP_STATUS_CONNECTED) {
		nd_assert(0);
		return -1;
	}

	ep->status = ND_RUDP_EP_STATUS_SHAKEHAND;

	if (msec != -1) {
		msec *= 1000;
	}

	ep->remote_addr.ip = ip;
	ep->remote_addr.port = port;

	nd_rudp_evld_locked_add_event(ep->ch->rudp->evld, ND_RUDP_LOCKED_MSG_CONNECT, ep);

	tm = nd_get_ts();
	while(ep->status < ND_RUDP_EP_STATUS_CONNECTED) {
		if (msec != -1 && nd_get_ts() - tm >= msec) {
			return -2;
		}
		else
			nd_sleep(1);
	}

	return (ep->status == ND_RUDP_EP_STATUS_CONNECTED) ? 0 : -1;
}

int 
ndrudp_accept(ND_RUDP_SOCKET s, ND_RUDP_SOCKET *out, int msec)
{
	nd_rudp_endpoint_t *ep, *ac_ep;

	*out = NULL;

	ep = (nd_rudp_endpoint_t*)s;
	if (!ep || !ep->ch->svr || ep->ch->listen_close) {
		return -1;
	}

	ac_ep = nd_rudp_accept_que_dequeue(ep->ch, msec);
	if (!ac_ep) {
		return ep->ch->listen_close ? -1 : -2;
	}
	*out = ac_ep;
	return 0;
}

int 
ndrudp_send(ND_RUDP_SOCKET s, void *buf, int len, int msec)
{
	nd_rudp_endpoint_t *ep;
	nd_rudp_packet_t *pkt, *tail;
	nd_int_t r, snd, total_snd;
	nd_int_t wait_tm;
	nd_uint64_t last_tm;

	ep = (nd_rudp_endpoint_t*)s;
	if (!ep || ep->status != ND_RUDP_EP_STATUS_CONNECTED) {
		return -1;
	}

	if (len <= 0) {
		return 0;
	}

	r = ND_RET_OK;
	total_snd = 0;
	last_tm = nd_get_ts();
	wait_tm = msec;

	for(;;) {

		nd_mutex_lock(&ep->s_que.mutex);

		while (ep->s_que.free_size == 0 && r == ND_RET_OK && ep->status == ND_RUDP_EP_STATUS_CONNECTED) {
			r = nd_cond_time_wait(&ep->s_que.cdt, wait_tm);
		}

		if (ep->status != ND_RUDP_EP_STATUS_CONNECTED) {
			nd_mutex_unlock(&ep->s_que.mutex);
			return -1;
		}

		if (r != ND_RET_OK) {
			/// time out
			nd_mutex_unlock(&ep->s_que.mutex);
			return total_snd ? total_snd : -2;
		}

		tail = NULL;

		if (!nd_queue_is_empty(&ep->s_que.que)) {
			if (tail = nd_queue_tail(&ep->s_que.que)) {
				int space = tail->size - ND_RUDP_HDR_LEN - tail->hdr.body_len;
				if (space > 0) {
					snd = space > (len - total_snd) ? (len - total_snd) : space;
					memcpy(tail->data + ND_RUDP_HDR_LEN + tail->hdr.body_len, (char*)buf + total_snd, snd);
					tail->hdr.body_len += snd;
					total_snd += snd;
				}
			}
		}

		if (total_snd < len) {
			nd_int_t n = (ep->mss - ND_RUDP_HDR_LEN) * ep->s_que.free_size + total_snd;
			if (n > len)
				n = len;

			while (total_snd != n) {
				pkt = nd_rudp_packet_create(ep->mss + ND_RUDP_HDR_LEN);
				if (!pkt)
					break;

				snd = (n - total_snd > ep->mss ? ep->mss : n - total_snd);
				memcpy(pkt->data + ND_RUDP_HDR_LEN, (char*)buf + total_snd,  snd);
				pkt->hdr.body_len = snd;
				total_snd += snd;

				nd_queue_insert_tail(&ep->s_que.que, pkt);
				ep->s_que.size++;
				ep->s_que.free_size--;
			}	
		}

		if (total_snd == len) {
			if (!tail) {
				nd_rudp_evld_locked_add_event(ep->ch->rudp->evld, ND_RUDP_LOCKED_MSG_ADD_WRITE, ep);
			}
			nd_mutex_unlock(&ep->s_que.mutex);
			return total_snd;
		}

		if (msec != -1) {
			nd_uint64_t curr = nd_get_ts();
			if ((nd_int_t)((curr - last_tm)/1000) >= msec) {
				/// time out
				if (!tail) {
					nd_rudp_evld_locked_add_event(ep->ch->rudp->evld, ND_RUDP_LOCKED_MSG_ADD_WRITE, ep);
				}
				nd_mutex_unlock(&ep->s_que.mutex);
				return total_snd ? total_snd : -2;
			}
			else {
				wait_tm = msec - (nd_int_t)((curr - last_tm)/1000);
			}
		}

		nd_mutex_unlock(&ep->s_que.mutex);
	}

	return total_snd;
}

int 
ndrudp_recv(ND_RUDP_SOCKET s, void *buf, int len, int msec)
{
	nd_rudp_endpoint_t *ep;
	nd_rudp_packet_t *pkt;
	nd_int_t r, total_rcv, n, k;
	nd_int_t wait_tm;
	nd_uint64_t last_tm;

	ep = (nd_rudp_endpoint_t*)s;
	if (!ep || ep->status != ND_RUDP_EP_STATUS_CONNECTED) {
		return -1;
	}

	if (len <= 0) {
		return 0;
	}

	wait_tm = msec;
	total_rcv = 0;
	r = ND_RET_OK;
	last_tm = nd_get_ts();

	for (;;) {
		nd_mutex_lock(&ep->r_que.mutex);

		while (nd_queue_is_empty(&ep->r_que.que) && r == ND_RET_OK && ep->status == ND_RUDP_EP_STATUS_CONNECTED) {
			r = nd_cond_time_wait(&ep->r_que.cdt, wait_tm);
		}

		if (ep->status != ND_RUDP_EP_STATUS_CONNECTED) {
			nd_mutex_unlock(&ep->r_que.mutex);
			/// 如果收到了有效数据, 即使连接被关闭, 也仍然递交给上层
			return total_rcv ? total_rcv : -1;
		}

		if (r != ND_RET_OK) {
			/// time out
			nd_mutex_unlock(&ep->r_que.mutex);
			return total_rcv ? total_rcv : -2;
		}

		while (!nd_queue_is_empty(&ep->r_que.que) && total_rcv != len) {

			pkt = nd_queue_head(&ep->r_que.que);
			n = pkt->hdr.body_len;
			k = (len - total_rcv > n ? n : len - total_rcv);
			memcpy((char*)buf + total_rcv, pkt->hdr.body, k);

			if (n == k) {
				nd_queue_remove(pkt);
				nd_rudp_packet_destroy(pkt);
				ep->r_que.size--;
				/// 接收窗口发生了变化, ep的idle_tm_node定时器会定时检测该窗口的变化(由0变为非0)
			}
			else {
				pkt->hdr.body_len -= (nd_uint16_t)k;
				pkt->hdr.body += k;
			}

			total_rcv += k;
		}

		nd_mutex_unlock(&ep->r_que.mutex);

		if (total_rcv == len) {
			return total_rcv;
		}

		if (msec != -1) {
			nd_uint64_t curr = nd_get_ts();
			if ((nd_int_t)((curr - last_tm)/1000) >= msec) {
				/// time out
				return total_rcv ? total_rcv : -2;
			}
			else {
				wait_tm = msec - (nd_int_t)((curr - last_tm)/1000);
			}
		}
	}

	nd_assert(0);
	return total_rcv;
}

int 
ndrudp_set_opt(ND_RUDP_SOCKET s, int level, int opt_name, void *opt_val, int opt_len)
{
	if (level == ND_RUDP_OPT_LV_SO) {

		nd_rudp_endpoint_t *ep;
		ep = (nd_rudp_endpoint_t*)s;
		if (!ep) {
			return -1;
		}

		switch (opt_name) 
		{
		case ND_RUDP_OPT_NM_SO_SNDBUF:
			{
				int val = *((int*)opt_val);
				if (val < 32 || val > 8192)
					return -1;

				if (ep->type == ND_RUDP_EP_TYPE_CONNECT && 
					ep->status < ND_RUDP_EP_STATUS_CONNECTED) {
					nd_mutex_lock(&ep->s_que.mutex);
					ep->s_que.free_size = val;
					nd_mutex_unlock(&ep->s_que.mutex);
					return 0;
				}
			}
			break;
		case ND_RUDP_OPT_NM_SO_RCVBUF:
			{
				;
			}
			break;
		default:
			break;
		}
	}

	return -1;
}