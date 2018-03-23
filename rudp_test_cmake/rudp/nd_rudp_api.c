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
	g_global_mgr->poll_wait_msec = 10;

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
ndrudp_shutdown(ND_RUDP_SOCKET s)
{
	nd_rudp_endpoint_t *ep;

	ep = nd_rudp_sock2ep(s);
	if (!ep)
		return;

	nd_rudp_evld_locked_add_event(ep->ch->rudp->evld, ND_RUDP_LOCKED_MSG_SHUTDOWN_EP, ep);
}

void 
ndrudp_close(ND_RUDP_SOCKET s)
{
	nd_rudp_endpoint_t *ep;

	ep = nd_rudp_sock2ep(s);

	if (!ep)
		return;

	nd_rudp_evld_locked_add_event(ep->ch->rudp->evld, ND_RUDP_LOCKED_MSG_CLOSE_EP, ep);
}

int 
ndrudp_listen(ND_RUDP_SOCKET s)
{
	nd_rudp_endpoint_t *ep;

	ep = nd_rudp_sock2ep(s);
	if (!ep || !ep->ch->svr) {
		nd_assert(0);
		return -1;
	}

	ep->status = ND_RUDP_EP_STATUS_LISTEN;

	nd_rudp_evld_locked_add_event(ep->ch->rudp->evld, ND_RUDP_LOCKED_MSG_LISTEN, ep);

	return 0;
}

int 
ndrudp_connect(ND_RUDP_SOCKET s, long ip, int port, int msec)
{
	nd_rudp_endpoint_t *ep;
	nd_uint64_t tm;

	ep = nd_rudp_sock2ep(s);
	if (!ep || 
		ep->ch->svr ||
		ep->status >= ND_RUDP_EP_STATUS_CONNECTED) {
		nd_assert(0);
		return -1;
	}

	tm = nd_get_ts();

	ep->status = ND_RUDP_EP_STATUS_SHAKEHAND;
	ep->first_snd_shk_tm = tm;

	if (msec != -1) {
		msec *= 1000;
	}

	ep->so_remote_addr.sa_family = ep->ch->ip_version;
	((struct sockaddr_in*)(&ep->so_remote_addr))->sin_addr.s_addr = htonl(ip);
	((struct sockaddr_in*)(&ep->so_remote_addr))->sin_port = htons(port);

	nd_rudp_evld_locked_add_event(ep->ch->rudp->evld, ND_RUDP_LOCKED_MSG_CONNECT, ep);

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

	ep = nd_rudp_sock2ep(s);
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
ndrudp_send(ND_RUDP_SOCKET s, void *buf, int len, int msec, int flag)
{
	nd_rudp_endpoint_t *ep;
	nd_rudp_packet_t *pkt, *tail;
	nd_int_t r, snd, total_snd;
	nd_int_t wait_tm;
	nd_uint64_t last_tm;

	ep = nd_rudp_sock2ep(s);
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
			if (msec == 0) {
				/// time out
				nd_mutex_unlock(&ep->s_que.mutex);
				return total_snd ? total_snd : -2;
			}
			else
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
				int space = tail->size - ND_RUDP_HDR_LEN - tail->hdr.body_len - ep->pre_snd_header_len;
				if (space > 0) {
					snd = ((space > (len - total_snd)) ? (len - total_snd) : space);
					memcpy(tail->data + ND_RUDP_HDR_LEN + tail->hdr.body_len, (char*)buf + total_snd, snd);
					tail->hdr.body_len += snd;
					total_snd += snd;
				}
			}
		}

		if (total_snd < len) {
			nd_int_t n = (ep->mss - ND_RUDP_HDR_LEN - ep->pre_snd_header_len) * ep->s_que.free_size + total_snd;
			if (n > len)
				n = len;

			while (total_snd != n) {
				pkt = nd_rudp_packet_create(ep->mss + ND_RUDP_HDR_LEN);
				if (!pkt)
					break;

				snd = (n - total_snd > (ep->mss - ep->pre_snd_header_len) ? (ep->mss - ep->pre_snd_header_len) : n - total_snd);
				memcpy(pkt->data + ND_RUDP_HDR_LEN, (char*)buf + total_snd,  snd);
				pkt->hdr.body_len = snd;
				total_snd += snd;

				nd_queue_insert_tail(&ep->s_que.que, pkt);
				ep->s_que.size++;
				ep->s_que.free_size--;
			}	
		}

		/// 如果: 1.数据全部发完 or
		///       2.发送了部分数据 但没有指定ND_RUDP_S_FLAG_SND_N标志
		/// 则返回实际发送数据长度
		if (total_snd == len || (total_snd && !(flag & ND_RUDP_S_FLAG_SND_N))) {
			if (!tail) {
				nd_rudp_evld_locked_add_event(ep->ch->rudp->evld, ND_RUDP_LOCKED_MSG_ADD_WRITE, ep);
			}
			nd_mutex_unlock(&ep->s_que.mutex);
			return total_snd;
		}

		if (msec == 0) {
			nd_mutex_unlock(&ep->s_que.mutex);
			return total_snd ? total_snd : -2;
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
ndrudp_recv(ND_RUDP_SOCKET s, void *buf, int len, int msec, int flag)
{
	nd_rudp_endpoint_t *ep;
	nd_rudp_packet_t *pkt;
	nd_int_t r, total_rcv, n, k;
	nd_int_t wait_tm;
	nd_uint64_t last_tm;

	ep = nd_rudp_sock2ep(s);
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
			if (msec == 0) {
				nd_mutex_unlock(&ep->r_que.mutex);
				return total_rcv ? total_rcv : -2;
			}
			else
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
			k = ((len - total_rcv > n) ? n : (len - total_rcv));
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

		/// 如果只接收了部分数据, 但没有指定ND_RUDP_R_FLAG_RCV_N标志, 则返回实际接收数据长度
		if (total_rcv && !(flag & ND_RUDP_R_FLAG_RCV_N)) {
			return total_rcv;
		}

		if (msec == 0) {
			return total_rcv ? total_rcv : -2;
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
ndrudp_set_opt(void *handle, int level, int opt_name, void *opt_val, int opt_len)
{
	if (level == ND_RUDP_OPT_LV_SO) {

		nd_rudp_endpoint_t *ep;
		ep = nd_rudp_sock2ep(handle);
		if (!ep) {
			return -1;
		}

		switch (opt_name) 
		{
		case ND_RUDP_OPT_NM_SO_SNDBUF:
			{
				int val = *((int*)opt_val);
				if (val < 32 || val > 65535)
					return -1;

				if (ep->type == ND_RUDP_EP_TYPE_CONNECT && 
					ep->status < ND_RUDP_EP_STATUS_CONNECTED) {
					nd_mutex_lock(&ep->s_que.mutex);
					ep->s_que.max_free_size = val;
					nd_mutex_unlock(&ep->s_que.mutex);
					return 0;
				}
			}
			break;
		case ND_RUDP_OPT_NM_SO_RCVBUF:
			{
				/// 暂不支持该选项
			}
			break;
		case ND_RUDP_OPT_NM_SO_MTU:
			{
				int val = *((int*)opt_val);
				if (val < 128 || val > 1472) 
					return -1;

				ep->mtu = val;
				return 0;
			}
			break;
		case ND_RUDP_OPT_NM_SO_HEARTBEAT_TMO:
			{
				int val = *((int*)opt_val);
				if (val < 5 && val != -1)
					return -1;

				ep->heartbeat_tmo = val;
				if (val > 0)
					ep->heartbeat_tmo *= 1000000;
				return 0;
			}
			break;
		case ND_RUDP_OPT_NM_SO_MAX_STAGED_PKT_CNT:
			{
				int val = *((int*)opt_val);
				if (val < 1 || val > 256)
					return -1;

				ep->max_staged_pkt_cnt = val;
				return 0;
			}
			break;
		case ND_RUDP_OPT_NM_SO_CC_MODE:
			{
				int val = *((int*)opt_val);
				ep->cc_mode = val;
				return 0;
			}
			break;
		case ND_RUDP_OPT_NM_SO_TRANS_MODE:
			{
				int val = *((int*)opt_val);
				ep->trans_mode = val;
				return 0;
			}
			break;
		case ND_RUDP_OPT_NM_SO_CC_FIXED_INTER_USEC:
			{
				int val = *((int*)opt_val);
				if (val < 1)
					return -1;

				ep->cc.fixed_inter_usec = val;
				return 0;
			}
			break;
		case ND_RUDP_OPT_NM_SO_LING_TMO:
			{
				int val = *((int*)opt_val);
				if (val < 0)
					ep->ling_tmo = (nd_uint64_t)-1;
				else 
					ep->ling_tmo = val;

				return 0;
			}
			break;
		case ND_RUDP_OPT_NM_SO_USER_DATA:
			{
				if (opt_len == sizeof(void*)) {
					ep->user_data = opt_val;
					return 0;
				}
			}
			break;
		case ND_RUDP_OPT_NM_SO_PRE_SND_HEADER_LEN:
			{
				int val = *((int*)opt_val);
				if (val < 0)
					return -1;

				ep->pre_snd_header_len = val;
				return 0;
			}
			break;
		case ND_RUDP_OPT_NM_SO_RTO_RANGE:
			{
				if (opt_len == sizeof(nd_rudp_rto_range_t))
				{
					nd_rudp_rto_range_t *r = (nd_rudp_rto_range_t*)opt_val;
					ep->rto_min = r->min;
					ep->rto_max = r->max;
					return 0;
				}
			}
			break;
		case ND_RUDP_OPT_NM_SO_INTERUSEC_RANGE:
			{
				if (opt_len == sizeof(nd_rudp_interusec_range_t))
				{
					nd_rudp_interusec_range_t *r = (nd_rudp_interusec_range_t*)opt_val;
					ep->cc.inter_usec_min = r->min;
					ep->cc.inter_usec_max = r->max;
					return 0;
				}
			}
			break;
		case ND_RUDP_OPT_NM_SO_LOSSCHK_THRESHOLD:
			{
				if (opt_len == sizeof(nd_rudp_losschk_threshold_t))
				{
					nd_rudp_losschk_threshold_t *r = (nd_rudp_losschk_threshold_t*)opt_val;
					ep->losschk_threshold.rack_cnt_limit = r->rack_cnt_limit;
					ep->losschk_threshold.tmo_cnt_limit = r->tmo_cnt_limit;
					return 0;
				}
			}
			break;
		case ND_RUDP_OPT_NM_SO_LOSS_THRESHOLD:
			{
				if (opt_len == sizeof(int))
				{
					int val = *((int*)opt_val);
					if (val >= 0 && val <= 100)
					{
						ep->loss_threshold = (double)val / (double)100.0;
						return 0;
					}		
				}
			}
			break;
		case ND_RUDP_OPT_NM_SO_DELAY_LEVEL:
			{
				if (opt_len == sizeof(int)) {
					int val = *((int*)opt_val);
					switch (val)
					{
					case ND_RUDP_DELAY_LV_DEFAULT:
						{
							ep->delay_level = val;
							ep->rto_min = 50000;
							ep->rto_max = 5000000;
							ep->cc.inter_usec_min = ND_TUDP_MIN_INTER_USEC;
							ep->cc.inter_usec_max = ND_TUDP_MAX_INTER_USEC;
							ep->losschk_threshold.rack_cnt_limit = 2;
							ep->losschk_threshold.tmo_cnt_limit = 2;
							ep->loss_threshold = 0.01;
							g_global_mgr->poll_wait_msec = 10;
						}
						return 0;
					case ND_RUDP_DELAY_LV_LOW:
						{
							ep->delay_level = val;
							ep->rto_min = 20000;
							ep->rto_max = 2000000;
							ep->cc.inter_usec_min = ND_TUDP_MIN_INTER_USEC;
							ep->cc.inter_usec_max = ND_TUDP_MAX_INTER_USEC;
							ep->cc.burst_inter_usec = 5000;
							ep->losschk_threshold.rack_cnt_limit = 2;
							ep->losschk_threshold.tmo_cnt_limit = 2;
							ep->loss_threshold = 0.05;
							g_global_mgr->poll_wait_msec = 10;
						}
						return 0;
					case ND_RUDP_DELAY_LV_MID:
						{
							ep->delay_level = val;
							ep->rto_min = 10000;
							ep->rto_max = 1000000;
							ep->cc.inter_usec_min = ND_TUDP_MIN_INTER_USEC;
							ep->cc.inter_usec_max = ND_TUDP_MAX_INTER_USEC;
							ep->cc.burst_inter_usec = 2000;
							ep->losschk_threshold.rack_cnt_limit = 2;
							ep->losschk_threshold.tmo_cnt_limit = 1;
							ep->loss_threshold = 0.1;
							g_global_mgr->poll_wait_msec = 5;
						}
						return 0;
					case ND_RUDP_DELAY_LV_HIGH:
						{
							ep->delay_level = val;
							ep->rto_min = 5000;
							ep->rto_max = 1000000;
							ep->cc.inter_usec_min = ND_TUDP_MIN_INTER_USEC;
							ep->cc.inter_usec_max = ND_TUDP_MAX_INTER_USEC;
							ep->cc.burst_inter_usec = 1000;
							ep->losschk_threshold.rack_cnt_limit = 2;
							ep->losschk_threshold.tmo_cnt_limit = 0;
							ep->loss_threshold = 0.2;
							g_global_mgr->poll_wait_msec = 1;
						}
						return 0;
					default:
						return -1;
					}
				}
			}
			break;
		default:
			break;
		}
	}

	if (level == ND_RUDP_OPT_LV_CH) {

		nd_rudp_channel_t *ch = handle;
		if (!ch) {
			return -1;
		}

		switch (opt_name) 
		{
		case ND_RUDP_OPT_NM_CH_SND_BEFORE_CB:
			{
				if (opt_len == sizeof(void*)) {
					ch->handle_before_send_cb = opt_val;
					return 0;
				}
			}
			break;
		case ND_RUDP_OPT_NM_CH_RCV_AFTER_CB:
			{
				if (opt_len == sizeof(void*)) {
					ch->handle_after_recv_cb = opt_val;
					return 0;
				}
			}
			break;
		case ND_RUDP_OPT_NM_CH_CLOSE_BEFOR_CB:
			{
				if (opt_len == sizeof(void*)) {
					ch->handle_before_close_cb = opt_val;
					return 0;
				}
			}
			break;
		case ND_RUDP_OPT_NM_CH_REFRESH_CB:
			{
				if (opt_len == sizeof(void*)) {
					ch->handle_refresh_cb = opt_val;
					return 0;
				}
			}
			break;
		case ND_RUDP_OPT_NM_CH_USER_DATA:
			{
				if (opt_len == sizeof(void*)) {
					ch->user_data = opt_val;
					return 0;
				}
			}
			break;
		default:
			break;
		}
	}

	if (level == ND_RUDP_OPT_LV_GLOBAL) {

		switch (opt_name)
		{
		case ND_RUDP_OPT_NM_GLOBAL_POLL_WAIT_MSEC:
			{
				if (opt_len == sizeof(int)) {
					int wt = *((int*)opt_val);
					if (wt < 1) wt = 1;
					if (wt > 100) wt = 100;
					g_global_mgr->poll_wait_msec = wt;
					return 0;
				}
			}
			break;
		default:
			break;
		}
	}

	return -1;
}

int 
ndrudp_get_opt(void *handle, int level, int opt_name, void *opt_val, int *opt_len)
{
	if (level == ND_RUDP_OPT_LV_SO) {

		nd_rudp_endpoint_t *ep;
		ep = nd_rudp_sock2ep(handle);
		if (!ep) {
			return -1;
		}

		switch(opt_name)
		{
		case ND_RUDP_OPT_NM_SO_SND_RATE:
			{
				*((int*)opt_val) = ep->trace_info.snd_rate;
				*opt_len = sizeof(int);
				return 0;
			}
			break;
		case ND_RUDP_OPT_NM_SO_RCV_RATE:
			{
				*((int*)opt_val) = ep->trace_info.rcv_rate;
				*opt_len = sizeof(int);
				return 0;
			}
			break;
		case ND_RUDP_OPT_NM_SO_ACK_BYTES:
			{
				if(*opt_len == sizeof(nd_uint64_t)) {
					*((nd_uint64_t*)opt_val) = ep->trace_info.total_ack_bytes;
					return 0;
				}
			}
			break;
		case ND_RUDP_OPT_NM_SO_REMOTE_ADDR:
			{
				struct sockaddr *addr = (struct sockaddr *)opt_val;
				if (*opt_len >= sizeof(struct sockaddr)) {
					*opt_len = sizeof(struct sockaddr);
					nd_rudp_ip_copy(addr, &ep->so_remote_addr, ep->ch->ip_version);
					return 0;
				}
			}
			break;
		case ND_RUDP_OPT_NM_SO_RAW_FD:
			{
				if (*opt_len == sizeof(ND_SOCKET)) {
					*((ND_SOCKET*)opt_val) = ep->ch->s;
					*opt_len = sizeof(ND_SOCKET);
					return 0;
				}
			}
			break;
		case ND_RUDP_OPT_NM_SO_STATUS:
			{
				if (ep->ch->svr) {
					if (ep->ch->listen_close)
						*((int*)opt_val) = -1;
					else 
						*((int*)opt_val) = 0;
				}
				else {
					if (ep->status > ND_RUDP_EP_STATUS_CONNECTED) *((int*)opt_val) = -1;
					else if (ep->status == ND_RUDP_EP_STATUS_CONNECTED) *((int*)opt_val) = 0;
					else *((int*)opt_val) = 1;
				}

				*opt_len = sizeof(int);
				return 0;
			}
			break;
		case ND_RUDP_OPT_NM_SO_USER_DATA:
			{
				if (*opt_len == sizeof(void*)) {
					*((void**)opt_val) = ep->user_data;
					return 0;
				}
			}
			break;
		case ND_RUDP_OPT_NM_SO_RAW_CH:
			{
				if (*opt_len == sizeof(void*)) {
					*((void**)opt_val) = ep->ch;
					return 0;
				}
			}
			break;
		case ND_RUDP_OPT_NM_SO_PEEK_SNDBUF_FREESIZE:
			{
				/// 这里仅做不精确的统计(实际上无法做到精确, 因此也不加锁)
				*((int*)opt_val) = (ep->mss - ND_RUDP_HDR_LEN - ep->pre_snd_header_len) * ep->s_que.free_size;
				*opt_len = sizeof(int);
				return 0;
			}
			break;
		default:
			break;
		}
	}

	if (level == ND_RUDP_OPT_LV_CH) {

		nd_rudp_channel_t *ch = handle;
		if (!ch) {
			return -1;
		}

		switch(opt_name)
		{
		case ND_RUDP_OPT_NM_CH_USER_DATA:
			{
				if (*opt_len == sizeof(void*)) {
					*((void**)opt_val) = ch->user_data;
					return 0;
				}
			}
			break;
		default:
			break;
		}
	}

	*opt_len = 0;
	return -1;
}

int 
ndrudp_poll_add_event(ndrudp_poll_t *poll, ND_RUDP_SOCKET s, int ev)
{
	if (poll->cnt <= NDRUDP_POLL_MAX && s && ev) {

		int i;
		for (i = 0; i < poll->cnt; i++) {
			if (s == poll->lst[i].s) {
				poll->lst[i].ev |= ev;
				return 0;
			}
		}

		if (poll->cnt < NDRUDP_POLL_MAX) {
			poll->lst[poll->cnt].s = s;
			poll->lst[poll->cnt].ev = ev;
			poll->cnt++;

			return 0;
		}
	}
	return -1;
}

int 
ndrudp_poll_del_event(ndrudp_poll_t *poll, ND_RUDP_SOCKET s, int ev)
{
	if (s && ev) {
		int i;

		for (i = 0; i < poll->cnt; i++) {
			if (s == poll->lst[i].s) {
				poll->lst[i].ev &= ~ev;

				if (poll->lst[i].ev == 0) {
					int j;
					for (j = i; j < poll->cnt - 1; j++) {
						poll->lst[j].s = poll->lst[j + 1].s;
						poll->lst[j].ev = poll->lst[j + 1].ev;
					}
					poll->cnt--;
				}
				return 0;
			}
		}
	}
	return -1;
}

void ndrudp_poll_zero(ndrudp_poll_t *poll)
{
	if (poll)
		poll->cnt = 0;
}

int 
ndrudp_poll_wait(ndrudp_poll_t *poll_in, ndrudp_poll_t *poll_out, int msec)
{
	int i, ev;
	nd_uint64_t last_tm;

	if (!poll_in || !poll_out) {
		return 0;
	}

	last_tm = nd_get_ts();

	for (;;) {
		for (i = 0; i < poll_in->cnt; i++) {
			nd_rudp_endpoint_t *ep = nd_rudp_sock2ep(poll_in->lst[i].s);
			if (!ep)
				continue;

			ev = 0;

			if (poll_in->lst[i].ev & ND_RUDP_IO_WRITE) 
			{
				if ((ep->s_que.free_size && ep->status == ND_RUDP_EP_STATUS_CONNECTED) ||
					ep->status > ND_RUDP_EP_STATUS_CONNECTED ||								/// 连接被close, 也返回可写
					(ep->status== ND_RUDP_EP_STATUS_LISTEN && !ep->ch->listen_close && ep->ch->accept_lst.que.size))	/// 有已排队等待被accept的监听ep
				{
					ev |= ND_RUDP_IO_WRITE;
				}
			}

			if (poll_in->lst[i].ev & ND_RUDP_IO_READ) 
			{
				if ((ep->r_que.size && ep->status == ND_RUDP_EP_STATUS_CONNECTED) ||
					ep->status > ND_RUDP_EP_STATUS_CONNECTED ||								/// 连接被close, 也返回可写
					(ep->status== ND_RUDP_EP_STATUS_LISTEN && !ep->ch->listen_close && ep->ch->accept_lst.que.size))	/// 有已排队等待被accept的监听ep
				{
					ev |= ND_RUDP_IO_READ;
				}
			}

			if (ev) {
				if (poll_out->cnt < NDRUDP_POLL_MAX) {
					poll_out->lst[poll_out->cnt].s = poll_in->lst[i].s;
					poll_out->lst[poll_out->cnt].ev = ev;
					poll_out->cnt++;
				}
				else {
					return poll_out->cnt;
				}
			}
		}

		if (poll_out->cnt)
			return poll_out->cnt;

		if (msec == 0) {
			return -2;
		}

		if (msec != -1) {
			nd_uint64_t curr = nd_get_ts();
			if ((nd_int_t)((curr - last_tm)/1000) >= msec) {
				/// time out
				return -2;
			}
			else {
				nd_milli_sec_t wt = g_global_mgr->poll_wait_msec;
				if (wt < 1) wt = 1;
				if (wt > 100) wt = 100;
#ifdef __ND_WIN32__
				timeBeginPeriod(1);
#endif
				nd_sleep(wt);
#ifdef __ND_WIN32__
				timeEndPeriod(1);
#endif
			}
		}
	}

	return 0;
}

int 
ndrudp_send_block(ND_RUDP_SOCKET s, ndrudp_block_t *blk, int msec, ndrudp_block_snd_opt_t *opt)
{
	nd_rudp_endpoint_t *ep;
	nd_rudp_packet_t *pkt, *tail;
	char *buf;
	nd_int_t r, snd, total_snd, len;
	nd_int_t wait_tm;
	nd_uint64_t last_tm;

	ep = nd_rudp_sock2ep(s);
	if (!ep || ep->status != ND_RUDP_EP_STATUS_CONNECTED) {
		return -1;
	}

	buf = (char*)blk->buf + blk->op_info.completed_len;

	len = blk->len - blk->op_info.completed_len;
	if (len <= 0) {
		return 0;
	}

	r = ND_RET_OK;
	total_snd = 0;
	last_tm = nd_get_ts();
	wait_tm = msec;

	/// 基于以下原因, 我们要求上层不能在多个线程中同时进行ndrudp_send_block调用:
	/// 1. 因为协议栈缓冲区大小有限, 我们无法保证不出现短写情况
	/// 2. 当出现短写时, 如果在多个线程中同时调用ndrudp_send_block, block将被打乱.
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
				/// do nothing. 对于块传输, 总是创建新的分片, 而不与tail进行拼接
				/// 这样, 首尾两个分组可能存在冗余空间
			}
		}

		/// 新的block
		if (blk->op_info.token == 0) {
			/// 为新的block分配token进行标识, token不能为0
			if (++ep->s_que.last_blk_token == 0) /// 回绕
				ep->s_que.last_blk_token = 1;

			blk->op_info.token = ep->s_que.last_blk_token;
		}
		else {
			/// 继续发送未发完的block
			if (blk->op_info.token != ep->s_que.last_blk_token) {
				nd_rudp_log(ND_RUDP_LOG_LV_ERROR, "invalid block id(%u)", blk->op_info.token);
				nd_assert(0);
				nd_mutex_unlock(&ep->s_que.mutex);
				return -1;
			}
		}

		if (total_snd < len) {
			nd_int_t n = (ep->mss - ND_RUDP_HDR_LEN - ep->pre_snd_header_len) * ep->s_que.free_size + total_snd;
			if (n > len)
				n = len;

			while (total_snd != n) {
				pkt = nd_rudp_packet_create(ep->mss + ND_RUDP_HDR_LEN);
				if (!pkt)
					break;

				snd = (n - total_snd > (ep->mss - ep->pre_snd_header_len) ? (ep->mss - ep->pre_snd_header_len) : (n - total_snd));
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

		if (msec == 0) {
			nd_mutex_unlock(&ep->s_que.mutex);
			return total_snd ? total_snd : -2;
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