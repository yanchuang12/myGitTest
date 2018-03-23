#include "nd_rudp.h"
#include <stdio.h>
#include <stdarg.h>

nd_global_mgr_t* g_global_mgr = NULL;

nd_err_t 
nd_rudp_log_init(int level, int opt, const char *fn)
{
	if (g_global_mgr) {
		g_global_mgr->log.opt = opt;
		g_global_mgr->log.level = level;
		if (level > 0 && opt) {
			if (!g_global_mgr->log.f && (opt & ND_RUDP_LOG_OPT_FILE))
				g_global_mgr->log.f = fopen(fn, "ab+");
			nd_rudp_log(ND_RUDP_LOG_LV_INFO, "\r\nlog init ok ...\r\n");
		}
	}
	return ND_RET_OK;
}

void 
nd_rudp_log_un_init()
{
	if (g_global_mgr && g_global_mgr->log.f) {
		//fflush(g_global_mgr->log.f);
		fclose(g_global_mgr->log.f);
		g_global_mgr->log.f = NULL;
	}
}

void 
nd_rudp_log(int level, const char *fmt, ...)
{
	if (g_global_mgr && (level & g_global_mgr->log.level) && g_global_mgr->log.opt) {
		
		char buf[1024*4];
		int pos;
		va_list vl;
#ifdef __ND_WIN32__
		SYSTEMTIME tm;
		GetLocalTime(&tm);

		pos = _snprintf(buf, sizeof(buf)-1, "<%02d-%02d %02d:%02d:%02d.%03d>", 
						tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond, tm.wMilliseconds);
#else
		time_t timep;
		struct tm *tp;
		time(&timep);
		tp = localtime(&timep);

		pos = snprintf(buf, sizeof(buf)-1, "<%02d-%02d %02d:%02d:%02d.%03d>", 
			tp->tm_mon + 1, tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec, 0);
#endif
		va_start( vl, fmt );
		vsnprintf(buf + pos, sizeof(buf)-pos-1, fmt, vl);
		va_end(vl);
		buf[sizeof(buf)-1] = 0;

		if (g_global_mgr->log.opt & ND_RUDP_LOG_OPT_CONSOLE)
			printf("%s\r\n", buf);

		if (g_global_mgr->log.opt & ND_RUDP_LOG_OPT_FILE) {
			if (g_global_mgr->log.f) {
				fprintf(g_global_mgr->log.f, "%s\r\n", buf);
				//fflush(g_global_mgr->log.f);
			}
		}
	}
}

static int ND_CALL_PRO __event_thread(void *p);
static void	 __nd_rudp_handle_read(nd_rudp_event_t *ev);
static void	 __nd_rudp_handle_write(nd_rudp_event_t *ev);
static void	 __nd_rudp_handle_locked_posted_event(nd_rudp_locked_posted_event_t *lev);

nd_rudp_t*
nd_rudp_create(struct sockaddr *local_addr, int addr_len, int fd, int svr)
{
	nd_rudp_t * rudp;

	rudp = malloc(sizeof(nd_rudp_t));
	if (!rudp) {
		return NULL;
	}

	memset(rudp, 0, sizeof(nd_rudp_t));

	rudp->evh.handle_read = &__nd_rudp_handle_read;
	rudp->evh.handle_write = &__nd_rudp_handle_write;
	rudp->evh.handle_locked_posted_event = &__nd_rudp_handle_locked_posted_event;

	rudp->evld = nd_rudp_evld_create(&rudp->evh);
	if (!rudp->evld) {
		nd_rudp_evld_destroy(rudp->evld);
		free(rudp);
		return NULL;
	}

	rudp->th.arg = rudp;
	rudp->th.fun_ptr = &__event_thread;
	nd_thread_create(&rudp->th);

	while (!rudp->th.active) {
		nd_sleep(1);
	}

	nd_mutex_lock(&g_global_mgr->rl_mutex);
	nd_queue_insert_tail(&g_global_mgr->rudp_lst, rudp);
	nd_mutex_unlock(&g_global_mgr->rl_mutex);

	rudp->ch = nd_rudp_channel_create(rudp, local_addr, addr_len, fd, svr);
	if (!rudp->ch) {
		nd_rudp_evld_shut_down(rudp->evld);
		return NULL;
	}

	return rudp;
}

void
nd_rudp_close(nd_rudp_t *rudp)
{
	if (!rudp)
		return;

	nd_rudp_evld_shut_down(rudp->evld);
	nd_thread_shut_down(&rudp->th);
}

void
nd_rudp_destroy(nd_rudp_t *rudp)
{
	if (!rudp)
		return;

	nd_rudp_evld_shut_down(rudp->evld);
	nd_rudp_evld_close_wait(rudp->evld);
	
	nd_rudp_channel_destroy(rudp->ch);
	nd_rudp_evld_destroy(rudp->evld);

	nd_mutex_lock(&g_global_mgr->rl_mutex);
	nd_queue_remove(rudp);
	nd_mutex_unlock(&g_global_mgr->rl_mutex);

	free(rudp);
}

static int ND_CALL_PRO __event_thread(void *p)
{
	nd_rudp_t *rudp = (nd_rudp_t*)p;
	while (!rudp->th.shut_down) {
		nd_rudp_event_run(rudp->evld, 2000);
	}

	nd_thread_exit(&rudp->th);
	nd_rudp_destroy(rudp);
	return 0;
}

static void	 
__nd_rudp_process_packet(nd_rudp_event_t *ev, nd_rudp_packet_t *pkt)
{
	nd_rudp_endpoint_t *ep;
	ep = NULL;

	if (ev->ch->svr) {
		if (pkt->hdr.token) {
			ep = nd_rudp_channel_find_ep(ev->ch, pkt->hdr.token);
		}
		else {
			ep = nd_rudp_channel_find_ep2(ev->ch, &pkt->addr);
			if (!ep && (pkt->hdr.flag & ND_RUDP_HDR_FLAG_SHAKEHAND) && !(pkt->hdr.flag & ND_RUDP_HDR_FLAG_ACK)) {
				/// 初始化endpoint
				ep = nd_rudp_endpoint_create();
				if (!ep || ND_RET_OK != nd_rudp_endpoint_init(ep)) {
					if (ep) {
						nd_rudp_endpoint_destroy(ep);
					}
					nd_rudp_packet_destroy(pkt);
					return;
				}

				ep->type = ND_RUDP_EP_TYPE_ACCEPT;
				ep->ch = ev->ch;
				ep->remote_addr = pkt->addr;
				/// 分配token
				ep->token = nd_rudp_channel_next_token(ev->ch);
				if (0 == ep->token) {
					/// 达到channel允许的最大连接数
					nd_rudp_endpoint_destroy(ep);
					nd_rudp_packet_destroy(pkt);
					return;
				}

				/* 改为在 nd_rudp_process_ctrl()函数 内进行操作
				/// 插入accept队列
				if (ND_RET_OK != nd_rudp_accept_que_enqueue(&ev->ch->accept_lst, ep)) {
					/// 超出backlog上限
					nd_rudp_endpoint_destroy(ep);
					nd_rudp_packet_destroy(pkt);
					return;
				}
				*/

				/// 插入channel管理的client列表
				nd_rudp_channel_insert_ep(ev->ch, ep);
				/// 启动idle定时器
				ep->idle_tm_node.node.key = nd_get_ts() + ep->idle_tm_node.inter_usec;
				nd_timer_que_schedule(ev->ch->rudp->evld->timer_que, &ep->idle_tm_node);
			}
		}
	}
	else {
		/// connect ep
		ep = ev->ch->self;
		if (ep->status >= ND_RUDP_EP_STATUS_CONNECTED && 
			ep->token != pkt->hdr.token) {
				/// 无效包
				nd_rudp_packet_destroy(pkt);
				return;
		}
	}

	if (!ep) {
		nd_rudp_packet_destroy(pkt);
		return;
	}

	/// ip地址变了? 也存在这种可能
	if (ep->remote_addr.ip != pkt->addr.ip || 
		ep->remote_addr.port != pkt->addr.port) {
			/// 重新设置对端地址
			ep->remote_addr = pkt->addr;
	}

	ep->last_rcv_tm = nd_get_ts();

	if (pkt->hdr.flag & ND_RUDP_HDR_FLAG_DATA) {
		if (ep->dead) {
			/// 应用层已经调用了rudp_close(), 则不再处理data分组
			nd_rudp_packet_destroy(pkt);
			nd_rudp_send_ctrl(ep, 0, ND_RUDP_HDR_FLAG_BYE, 0);
			return;
		}
		/// 处理data
		nd_rudp_process_data(ep, pkt);
	}
	else {
		/// 处理ctrl
		nd_rudp_process_ctrl(ep, pkt);
		/// ctrl packet 在这里统一释放
		nd_rudp_packet_destroy(pkt);
	}
}

static void	 
__nd_rudp_handle_read(nd_rudp_event_t *ev)
{
	nd_err_t r;
	nd_rudp_packet_t *pkt;
	nd_int_t i;

	for (i = 0; i < __ONE_CYCLE_MAX_RCV_PKT_CNT__; i++) {

		if (ev->ch->closed) {
			break;
		}

		pkt = nd_rudp_packet_create(ND_RUDP_MTU);
		if (!pkt) {
			break;
		}

		r = nd_rudp_channel_recv_from(ev->ch, pkt, &pkt->addr);
		if (r == ND_RET_ERROR) {
			nd_rudp_packet_destroy(pkt);
			nd_rudp_channel_close(ev->ch);
			return;
		}
		else if (r == ND_RET_AGAIN) {
			nd_rudp_packet_destroy(pkt);
			nd_rudp_evld_add_wait_read(ev->ch);
			return;
		}

		if (ND_RET_OK != nd_rudp_packet_parse(pkt)) {
			nd_rudp_packet_destroy(pkt);
			continue;
		}

		__nd_rudp_process_packet(ev, pkt);
	}

	if (!ev->ch->closed)
		nd_rudp_evld_add_posted_read(ev->ch);
}

void 
nd_rudp_process_ctrl(nd_rudp_endpoint_t *ep, nd_rudp_packet_t *pkt)
{
	nd_int_t exhd_pos;
	exhd_pos = 0;
	/// shake hand
	if (pkt->hdr.flag & ND_RUDP_HDR_FLAG_SHAKEHAND) {
		nd_uint32_t isyn_seq;
		nd_uint16_t token, mss;

		if (pkt->hdr.exhdr_len != 8) {
			return;
		}

		if (ep->status > ND_RUDP_EP_STATUS_CONNECTED) {
			/// 连接已经关闭
			nd_rudp_send_ctrl(ep, 0, ND_RUDP_HDR_FLAG_BYE, 0);
			return;
		}

		isyn_seq = ntohl(*(nd_uint32_t*)(pkt->hdr.exhdr));
		token = ntohs(*(nd_uint16_t*)(pkt->hdr.exhdr + 4));
		mss = ntohs(*(nd_uint16_t*)(pkt->hdr.exhdr + 6));

		if (ep->type == ND_RUDP_EP_TYPE_ACCEPT) {
			if (ep->status < ND_RUDP_EP_STATUS_CONNECTED) {
				nd_rudp_endpoint_init_with_peer(ep, isyn_seq);
				ep->status = ND_RUDP_EP_STATUS_CONNECTED;

				/// 插入accept队列
				if (ND_RET_OK != nd_rudp_accept_que_enqueue(&ep->ch->accept_lst, ep)) {
					/// 超出backlog上限
					nd_rudp_channel_remove_ep(ep->ch, ep);
					nd_rudp_endpoint_destroy(ep);
					return;
				}
			}
			/*else if (ep->status == ND_RUDP_EP_STATUS_CONNECTED) {
				/// 已经处于连接状态, 则检查初始序号是否匹配
				if (isyn_seq != ep->peer_isyn_seq) {
					nd_rudp_send_ctrl(ep, 0, ND_RUDP_HDR_FLAG_BYE, 0);
					return;
				}
			}*/
			nd_rudp_send_ctrl(ep, (nd_uint32_t)nd_get_ts(), ND_RUDP_HDR_FLAG_SHAKEHAND | ND_RUDP_HDR_FLAG_ACK, 0);
		}
		else {
			if (ep->status < ND_RUDP_EP_STATUS_CONNECTED) {
				ep->token = token;
				nd_rudp_endpoint_init_with_peer(ep, isyn_seq);
				ep->status = ND_RUDP_EP_STATUS_CONNECTED;
			}
			//nd_rudp_endpoint_update_rtt(ep, (nd_uint32_t)nd_get_ts(), pkt->hdr.ts);
		}

		return;
	}

	if (pkt->hdr.flag & ND_RUDP_HDR_FLAG_BYE) {
		/// 对端close了, 则本端也直接close, 跳过 ND_RUDP_EP_STATUS_LING_WAIT 状态
		nd_rudp_endpoint_close(ep);
		ep->status = ND_RUDP_EP_STATUS_CLOSE_END;
		if (!(pkt->hdr.flag & ND_RUDP_HDR_FLAG_ACK) && ep->token) {
			nd_rudp_send_ctrl(ep, (nd_uint32_t)nd_get_ts(), ND_RUDP_HDR_FLAG_BYE | ND_RUDP_HDR_FLAG_ACK, 0);
		}
		return;
	}

	/// 先更新rtt, 速率
	if (pkt->hdr.flag & ND_RUDP_HDR_FLAG_PROBE) {
		/// todo: 检查该包是否是延后到达
		if (pkt->hdr.flag & ND_RUDP_HDR_FLAG_ACK) {	
			ep->exception_cnt = 0;
			ep->probe_ack_tmo = 0;
			nd_rudp_endpoint_update_rtt(ep, (nd_uint32_t)nd_get_ts(), pkt->hdr.ts);
		}
		else {
			if (pkt->hdr.exhdr_len >= 8) {
				nd_uint32_t rate, bw;
				rate = ntohl(*(nd_uint32_t*)(pkt->hdr.exhdr));
				if (rate > 0)
					nd_rudp_endpoint_update_rate(ep, rate);

				bw = ntohl(*(nd_uint32_t*)(pkt->hdr.exhdr + 4));
				if (bw > 0)
					nd_rudp_endpoint_update_bw(ep, bw);

				exhd_pos = 8;

				//nd_rudp_log(ND_RUDP_LOG_LV_TRACE, "-------peer notify: rate=%u, bw=%u", rate, bw);
			}
		}
	}

	/// 再处理staged_ack
	if (pkt->hdr.flag & (ND_RUDP_HDR_FLAG_STAGED_ACK)) {
		if (ND_RET_OK != nd_rudp_process_staged_ack(ep, pkt, pkt->hdr.exhdr + exhd_pos, pkt->hdr.exhdr_len - exhd_pos)) {
			return;
		}
	}

	/// 最后, 确认是否需要应答
	if ((pkt->hdr.flag & ND_RUDP_HDR_FLAG_PROBE) &&
		!(pkt->hdr.flag & ND_RUDP_HDR_FLAG_ACK)) {
		/// 探测包, 应答之
		nd_rudp_send_ctrl(ep, pkt->hdr.ts, ND_RUDP_HDR_FLAG_ACK|ND_RUDP_HDR_FLAG_PROBE|ND_RUDP_HDR_FLAG_STAGED_ACK, pkt->rcv_tm);
	}
}

static void	 
__nd_rudp_handle_write(nd_rudp_event_t *ev)
{
	nd_dlist_node_t *nd;

	if (ev->ch->closed) {
		return;
	}

	if (ev->ch->svr) {
		nd_queue_for_each(nd, &ev->ch->eps.header) {
			nd_rudp_endpoint_t *ep = nd->data;
			nd_rudp_endpoint_do_send(ep, NULL, 0);
			if (ev->ch->wr_pending) {
				break;
			}
		}
	}
	else {
		nd_rudp_endpoint_do_send(ev->ch->self, NULL, 0);
	}
}

static void	 
__nd_rudp_handle_locked_posted_event(nd_rudp_locked_posted_event_t *lev)
{
	/// locked msg 主要用于应用层与协议栈之间的消息交互
	switch( lev->msg) {
	case ND_RUDP_LOCKED_MSG_LISTEN:
	case ND_RUDP_LOCKED_MSG_ADD_READ:
		{
			/// IO read 事件只针对channel, 不细分至endpoint
			nd_rudp_endpoint_t *ep = (nd_rudp_endpoint_t*)lev->param;
			if (ep->ch->rd_ev.wait_pending == 0 &&
				ep->ch->rd_ev.posted_pending == 0) {
				nd_rudp_evld_add_posted_read(ep->ch);
			}
		}
		break;
	case ND_RUDP_LOCKED_MSG_ADD_WRITE: {
			/// IO write 可以精确匹配到endpoint
			nd_rudp_endpoint_t *ep = (nd_rudp_endpoint_t*)lev->param;
			nd_rudp_endpoint_do_send(ep, NULL, 0);
		}
		break;
	case ND_RUDP_LOCKED_MSG_DESTROY_EP: {
			nd_rudp_endpoint_t *ep = (nd_rudp_endpoint_t*)lev->param;
			switch (ep->type) {
			case ND_RUDP_EP_TYPE_LISTEN:
				break;
			case ND_RUDP_EP_TYPE_ACCEPT: {
					nd_rudp_channel_t *ch = ep->ch;
					nd_rudp_accept_que_remove(&ep->ch->accept_lst, ep);
					nd_rudp_channel_remove_ep(ep->ch, ep);	
					nd_rudp_endpoint_destroy(ep);
					if (ch->listen_close && nd_dlist_is_empty(&ch->eps)) {
						nd_rudp_close(ch->rudp);
					}
				}
				break;
			case ND_RUDP_EP_TYPE_CONNECT:
				nd_rudp_close(ep->ch->rudp);
				break;
			case ND_RUDP_EP_TYPE_DUMMY:
			default:
				break;
			}
		}
		break;
	case ND_RUDP_LOCKED_MSG_CONNECT: {
			nd_rudp_endpoint_t *ep = (nd_rudp_endpoint_t*)lev->param;
			ep->last_snd_shk_tm = nd_get_ts();
			nd_rudp_send_ctrl(ep, (nd_uint32_t)nd_get_ts(), ND_RUDP_HDR_FLAG_SHAKEHAND, 0);
			if (ep->ch->rd_ev.wait_pending == 0 &&
				ep->ch->rd_ev.posted_pending == 0) {
				nd_rudp_evld_add_posted_read(ep->ch);
			}
		}
		break;
	case ND_RUDP_LOCKED_MSG_CLOSE_EP: {
			nd_rudp_endpoint_t *ep = (nd_rudp_endpoint_t*)lev->param;
			ep->dead = 1;
			nd_rudp_endpoint_on_dead(ep);
		}
		break;
	default: {
		nd_assert(0);
		}
		break;
	}
}

void 
nd_rudp_process_data(nd_rudp_endpoint_t *ep, nd_rudp_packet_t *pkt)
{
	nd_rudp_packet_t *find, *lst, **p, *tmp_pkt;
	nd_int_t pos, loss, size;
	nd_uint8_t pkt_flag;

	/// 确保分组序号在有效区间
	if (0 == nd_rudp_chk_dateseq(ep, pkt->hdr.seq)) {
		//nd_rudp_log(ND_RUDP_LOG_LV_DEBUG, "data packet<sn=%u> out of seq range", pkt->hdr.seq);
		nd_rudp_packet_destroy(pkt);
		return;
	}

	size = nd_rudp_rque_size(&ep->r_que) + ep->r_buf_lst_size;
	if (size >= ep->max_syn_seq_range) {
		/// 超出了当前允许的最大发送序号: sendctrl cwnd = 0
		nd_rudp_send_ctrl(ep, pkt->hdr.ts, ND_RUDP_HDR_FLAG_STAGED_ACK, pkt->rcv_tm);
		//if (ep->r_buf_lst_size == ep->max_syn_seq_range || 
		//	size >= ep->max_syn_seq_range + ep->max_syn_seq_range >> 1) {
			nd_rudp_packet_destroy(pkt);
			return;
		//}
	}

	pos = pkt->hdr.seq % ep->max_syn_seq_range;
	find = ep->r_buf_lst[pos];
	if (find) {
		/// 重复收到了该分组
		if (find->repeat < 3) {
			find->repeat++;
		}
		nd_rudp_packet_destroy(pkt);
		nd_rudp_log(ND_RUDP_LOG_LV_DEBUG, "recv repeat packet: repeat=%d, sn=%u", (int)find->repeat, find->hdr.seq);
		return;
	}

	/// 更新最近收到的序号
	ep->r_last_seq = pkt->hdr.seq;

	/// 更新最近收到的最大序号
	if (nd_seq_cmp(ep->r_last_max_seq, pkt->hdr.seq) < 0) {
		ep->r_last_max_seq = pkt->hdr.seq;
	}

	/// 缓存收到的分组
	ep->r_buf_lst[pos] = pkt;
	ep->r_buf_lst_size++;

	/// 更新速率预估相关参数
	nd_rudp_endpoint_on_rcv_packet(ep, pkt);

	lst = NULL;
	p = &lst;
	tmp_pkt = pkt;
	pkt_flag = pkt->hdr.flag;

	/// 检查是否有数据可以提交给上层, 为减小锁粒度, 这里先缓存到lst
	while (tmp_pkt && tmp_pkt->hdr.seq == ep->r_next_seq) {

		if (ep->status != ND_RUDP_EP_STATUS_CONNECTED) {
			nd_rudp_packet_destroy(tmp_pkt);
		}
		else {
			*p = tmp_pkt;
			p = &((*p)->link);
		}
		
		ep->r_buf_lst[pos % ep->max_syn_seq_range] = NULL;
		ep->r_buf_lst_size--;

		++pos;

		tmp_pkt = ep->r_buf_lst[pos % ep->max_syn_seq_range];
		/// 递增 下一个期望序号
		ep->r_next_seq = nd_seq_inc(ep->r_next_seq);
	}

	*p = NULL;

	/// PUSH标志将触发立即返回ack, 注意: 这里要先等待@ep->r_next_seq 更新后再ACK
	if (ep->r_staged_pkt_cnt++ >= ep->max_staged_pkt_cnt ||
		(pkt_flag & ND_RUDP_HDR_FLAG_PUSH)) {
			/// nd_rudp_send_ctrl() 会将@r_staged_pkt_cnt 置为0
			ep->r_staged_pkt_cnt = 0;
			nd_rudp_send_ctrl(ep, pkt->hdr.ts, ND_RUDP_HDR_FLAG_STAGED_ACK, pkt->rcv_tm);
	}

	if (lst) {
		loss = 0;
		nd_rudp_rque_enqueue_lst(&ep->r_que, lst);
	}
	else {
		loss = 1;
	}

	/// 检测接收窗口是否为0
	if (0 == nd_rudp_endpoint_get_rcv_wnd(ep)) {
		ep->r_zero_wnd = 1;
	}
	else {
		ep->r_zero_wnd = 0;
	}
}

void 
nd_rudp_send_ctrl(nd_rudp_endpoint_t *ep, nd_uint32_t ts, nd_uint8_t flag, nd_uint64_t org_tm)
{
	nd_rudp_packet_t *pkt;
	nd_uint8_t *org_ptr;
	nd_int_t wnd;
	nd_uint32_t staged_seq;

	pkt = nd_rudp_packet_create(ND_RUDP_MTU);
	if (!pkt) {
		nd_rudp_packet_destroy(pkt);
		return;
	}
	pkt->org_tm = org_tm;

	org_ptr = (nd_uint8_t*)nd_rudp_packet_wr_ptr(pkt);

	/// build header
	/// cal wnd, 这个wnd并非直接代表接收窗口
	wnd = (nd_int_t)ep->max_syn_seq_range - (nd_int_t)nd_rudp_rque_size(&ep->r_que);
	if (wnd > 0) {
		pkt->hdr.wnd = wnd;
	}
	else {
		pkt->hdr.wnd = 0;
	}

	if (0 == nd_rudp_endpoint_get_rcv_wnd(ep)) {
		ep->r_zero_wnd = 1;
	}
	else {
		ep->r_zero_wnd = 0;
	}

	pkt->hdr.seq = ep->r_next_seq;
	pkt->hdr.flag = flag;
	pkt->hdr.token = ep->token;
	pkt->hdr.ts = ts;
	pkt->addr = ep->remote_addr;

	if (flag & ND_RUDP_HDR_FLAG_BYE) {
		/// 没有获取到token
		if (ep->type == ND_RUDP_EP_TYPE_CONNECT && ep->token == 0) {
			nd_rudp_packet_destroy(pkt);
			return;
		}
		/// bye 没有扩展头
		goto do_build;
	}

	if (flag & ND_RUDP_HDR_FLAG_SHAKEHAND) {
		/// shakehand ack 也同样处理
		nd_uint8_t *ptr = org_ptr + ND_RUDP_HDR_LEN + pkt->hdr.exhdr_len;
		*(nd_uint32_t*)ptr = htonl(ep->isyn_seq);
		*(nd_uint16_t*)(ptr + 4) = htons(ep->token);
		*(nd_uint16_t*)(ptr + 6) = htons(ep->mss);

		pkt->hdr.exhdr_len += 8;
		goto do_build;
	}

	if ((flag & ND_RUDP_HDR_FLAG_PROBE) && !(flag & ND_RUDP_HDR_FLAG_ACK)) {
		nd_uint32_t rate, bw;
		nd_uint8_t *ptr = org_ptr + ND_RUDP_HDR_LEN + pkt->hdr.exhdr_len;
		/// 更新探测时间
		ep->last_probe_tm = nd_get_ts();
		/// 这里暂不设置probe ack超时时间, 在发送成功后再设置
		//ep->probe_ack_tmo = ep->last_probe_tm + ep->rto;
		/// 携带速率
		rate = 0; bw = 0;
#ifdef __RATE_BW_METHOD__
	#if (__RATE_BW_METHOD__ == 1)
			rate = nd_rudp_endpoint_cal_rate_1(ep);
			bw = nd_rudp_endpoint_cal_bw_1(ep);
	#elif (__RATE_BW_METHOD__ == 2)
			rate = nd_rudp_endpoint_cal_rate_2(ep);
			bw = nd_rudp_endpoint_cal_bw_2(ep);
	#elif (__RATE_BW_METHOD__ == 3)
			rate = nd_rudp_endpoint_cal_rate_3(ep);
			bw = nd_rudp_endpoint_cal_bw_3(ep);
	#else
			rate = nd_rudp_endpoint_cal_rate_1(ep);
			bw = nd_rudp_endpoint_cal_bw_1(ep);
	#endif
#else
			rate = nd_rudp_endpoint_cal_rate_1(ep);
			bw = nd_rudp_endpoint_cal_bw_1(ep);
#endif

		*(nd_uint32_t*)ptr = htonl(rate);
		*(nd_uint32_t*)(ptr + 4) = htonl(bw);
		pkt->hdr.exhdr_len += 8;
	}

	if (flag & ND_RUDP_HDR_FLAG_STAGED_ACK) 
	{
		nd_int_t cnt1, cnt2, pos, n, i;
		nd_uint8_t *ptr, *cnt_ptr;

		cnt_ptr = org_ptr + ND_RUDP_HDR_LEN + pkt->hdr.exhdr_len;
		ptr = cnt_ptr + 4;

		/* exhdr: 
					b0: cnt1, b1: unused, b2-b3: cnt2
					b0-b3: bitmap, ep->r_next_seq ---> ep->r_next_seq + cnt1
					b0-b3: ep->r_next_staged_ack_seq
					b0-bn: bitmap, ep->r_next_staged_ack_seq ---> ep->r_next_staged_ack_seq + cnt2
		*/
		
		/// ep->r_next_seq ---> ep->r_next_seq + n
		nd_assert(ep->r_next_seq != ep->r_last_max_seq);
		n = nd_seq_diff(ep->r_last_max_seq, ep->r_next_seq) + 1;
		cnt1 = 0;

		if (n > 1) {
			if (n > 32) 
				n = 32;

			pos = ep->r_next_seq % ep->max_syn_seq_range;
			/// 实际上, bitmap的第一个bit总为0, 即ep->r_next_seq 总是未收到
			for (cnt1 = 0; cnt1 < n; ) {
				*ptr = 0;
				for (i = 0; i < 8 && cnt1 < n; i++) {
					if (ep->r_buf_lst[pos++ % ep->max_syn_seq_range]) {
						(*ptr) |= 0x01 << i;
					}
					cnt1++;
				}
				ptr++;
			}
		}

		*cnt_ptr = cnt1;
		
		/// ep->r_next_staged_ack_seq ---> ep->r_next_staged_ack_seq + n
		staged_seq = nd_seq_add(ep->r_next_seq, cnt1);
		if (nd_seq_cmp(ep->r_next_staged_ack_seq, staged_seq) < 0) {
			ep->r_next_staged_ack_seq = staged_seq;
		}
		
		cnt2 = 0;
		n = nd_seq_diff(ep->r_last_max_seq, ep->r_next_staged_ack_seq) + 1;

		if (n > 0) {
			if (n > (nd_int32_t)ep->max_staged_ack_range)
				n = (nd_int32_t)ep->max_staged_ack_range;

			ptr = cnt_ptr + 8;
			*(nd_uint32_t*)ptr = htonl(ep->r_next_staged_ack_seq);
			ptr += 4;

			pos = ep->r_next_staged_ack_seq % ep->max_syn_seq_range;
			for (; cnt2 < n; ) {
				*ptr = 0;
				for (i = 0; i < 8 && cnt2 < n; i++) {
					if (ep->r_buf_lst[pos++ % ep->max_syn_seq_range]) {
						(*ptr) |= 0x01 << i;
					}
					cnt2++;
				}
				ptr++;
			}
		}
		
		*(nd_uint16_t*)(cnt_ptr + 2) = htons(cnt2);

		ep->r_next_staged_ack_seq = nd_seq_add(ep->r_next_staged_ack_seq, cnt2);
		/// 更新staged ack发送时间
		ep->last_snd_staged_ack_tm = nd_get_ts();
		ep->r_staged_pkt_cnt = 0;
		
		if (cnt1 == 0) {
			pkt->hdr.exhdr_len += 4;
		}
		else{
			if (cnt2)
				pkt->hdr.exhdr_len += 12 + (cnt2 >> 3) + (cnt2 & 0x00000007) ? 1 : 0; /// cnt2 / 8 + (cnt2 % 8) ? 1 : 0
			else
				pkt->hdr.exhdr_len += 8;
		}
	} /// end if (flag & ND_RUDP_HDR_FLAG_STAGED_ACK) 
	
do_build:
	if (ND_RET_OK != nd_rudp_packet_write(pkt, 1)) {
		nd_rudp_packet_destroy(pkt);
		return;
	}

	nd_rudp_packet_add_wr_pos(pkt, pkt->hdr.exhdr_len);

	/*if ((flag & ND_RUDP_HDR_FLAG_BYE) ||
		(flag & ND_RUDP_HDR_FLAG_SHAKEHAND))*/ {
			/// bye 和 shakehand直接发送, 不进入队列
			nd_rudp_channel_send_to(ep->ch, pkt, &pkt->addr);
			nd_rudp_packet_destroy(pkt);
			return;
	}

	nd_queue_insert_tail(&ep->s_ctrl_lst, pkt);	
	/// 更新定时器, 立即发送
	nd_rudp_endpoint_update_snd_timer(ep, nd_get_ts());
}

static void
__nd_rudp_check_sendlst(nd_rudp_endpoint_t *ep, nd_uint32_t seq, nd_int_t num, nd_uint8_t *bitmap, nd_uint32_t range_begin_seq, nd_uint32_t *chk_cnt, nd_int_t chk_loss)
{
#define SEQ_LST_SIZE  3
	nd_int_t i, j, k, n, pos, cnt;
	nd_uint8_t *p;
	nd_rudp_packet_t *pkt;
	nd_uint64_t tm;
	nd_uint32_t seq_lst[SEQ_LST_SIZE];
	nd_int_t seq_lst_pos, seq_lst_cnt;

	seq_lst_pos = 0;
	seq_lst_cnt = 0;

	tm = nd_get_ts();

	/// 检查seq是否在发送端有效数据区间
	if (0 == nd_seq_between(seq, range_begin_seq, ep->s_next_seq) ||
		0 == nd_seq_in_range(seq, range_begin_seq, ep->max_syn_seq_range)) {
		return;
	}

	/// 确保 [seq, seq + cnt] 在发送端有效区间[range_begin_seq, range_begin_seq + max_syn_seq_range)
	n = nd_seq_diff(seq, range_begin_seq);
	if (n + num > ep->max_syn_seq_range) {
		num = ep->max_syn_seq_range - n;
	}

	cnt = num;

	p = bitmap;
	pos = seq % ep->max_syn_seq_range;

	for (i = 0; cnt; i++) {
		k = (cnt > 8 ? 8 : cnt);
		if (p[i]) {
			for (j = 0; j < k; j++) {
				/// 取余防止回绕溢出
				pkt = ep->s_buf_lst[(pos + j) % ep->max_syn_seq_range];
				if (p[i] & (0x01 << j)) {	/// bit == 1		
					if (pkt) {
						(*chk_cnt)++;
						if (pkt->hdr.seq == ep->s_next_ack_seq) {
							/// 被确认的seq 等于 发送端累积确认seq, 则 ++s_next_ack_seq;
							ep->s_next_ack_seq = nd_seq_inc(ep->s_next_ack_seq);
						}
						/// 数据包已被接收端确认, 移除之
						if (pkt->loss) {
							nd_rbtree_delete(&ep->s_loss_lst, &pkt->node_in_loss);
						}
						if (pkt == ep->s_curr_pkt) {
							ep->s_curr_pkt = NULL;
						}

						seq_lst[seq_lst_pos] = pkt->hdr.seq;
						if (++seq_lst_pos == SEQ_LST_SIZE)
							seq_lst_pos = 0;
						if (seq_lst_cnt != SEQ_LST_SIZE)
							seq_lst_cnt++;

						nd_rudp_packet_destroy(pkt);
						ep->s_buf_lst[(pos + j) % ep->max_syn_seq_range] = NULL;
						ep->s_buf_lst_size--;
					}
				}
				else {
					/// 这里依赖一个事实,即: 对端至少收到了 [seq, seq + cnt)区间的数据包, 才会确认cnt个包
					/// 因此, bit == 0的数据包就表示: 对端未收到该包,但肯定收到了大于该序号的后续包
					/// 注: 还存在一种情况, 发送端发送的[ep->s_next_ack_seq, ep->s_next_seq)区间的数据包 对端
					///     一个都没收到, 则此时这里无法检测出是否loss, 因此, 将通过@ep->s_loss_tmo进一步检测
					/*if (chk_loss && pkt && !pkt->loss) {
						nd_rudp_endpoint_chk_loss(ep, pkt, 0);
					}*/
				}
			}
		}
		pos += k;
		cnt -= k;
	}

	/// @seq_lst[seq_lst_pos]: 此时表示seq_lst列表内最小的seq
	if (chk_loss && seq_lst_cnt == SEQ_LST_SIZE) {
		nd_uint32_t s, min_seq;
		min_seq = seq_lst[seq_lst_pos];
		cnt = 0;
		if (nd_seq_cmp(seq, min_seq) < 0) {
			for (s = seq; s != min_seq && cnt < num; s = nd_seq_inc(s), cnt++) {
				pkt = ep->s_buf_lst[s % ep->max_syn_seq_range];
				if (pkt && !pkt->loss) {
					nd_rudp_endpoint_chk_loss(ep, pkt, 1);
				}
			}
		}
	}
}

nd_err_t 
nd_rudp_process_staged_ack(nd_rudp_endpoint_t *ep, nd_rudp_packet_t *pkt, nd_uint8_t *ptr, nd_int_t len)
{
	/* exhdr: 
				b0: cnt1, b1: unused, b2-b3: cnt2
				b0-b3: bitmap, ep->r_next_seq ---> ep->r_next_seq + cnt1
				b0-b3: ep->r_next_staged_ack_seq
				b0-bn: bitmap, ep->r_next_staged_ack_seq ---> ep->r_next_staged_ack_seq + cnt2
	*/
	nd_uint32_t ack, old_next_ack_seq, max_seq;
	nd_uint_t cnt1, cnt2;
	nd_uint32_t chk_cnt;
	nd_err_t ret;

	/// 
	ret = ND_RET_OK;
	chk_cnt = 0;

	/// process pkt->hdr.seq: 对端返回的累积确认序号
	old_next_ack_seq = ep->s_next_ack_seq;
	ack = nd_seq_dec(pkt->hdr.seq);

	if (nd_seq_in_range(pkt->hdr.seq, ep->s_next_ack_seq, ep->max_syn_seq_range + 1)) {
		if (nd_seq_between(pkt->hdr.seq, ep->s_next_ack_seq, nd_seq_inc(ep->s_next_seq))) {
			nd_uint32_t s, pos;
			nd_rudp_packet_t *p;
			for (s = ep->s_next_ack_seq; nd_seq_cmp(s, pkt->hdr.seq) < 0; s = nd_seq_inc(s)) {
				pos = s % ep->max_syn_seq_range;
				p = ep->s_buf_lst[pos];
				if (p) {
					if (p->loss) {
						/// 从loss列表移除
						nd_rbtree_delete(&ep->s_loss_lst, &p->node_in_loss);
					}
					/// 是当前正待发送的包(可能是loss列表取出的)
					if (p == ep->s_curr_pkt) {
						ep->s_curr_pkt = NULL;
					}
					nd_rudp_packet_destroy(p);
					ep->s_buf_lst[pos] = NULL;
					ep->s_buf_lst_size--;
					chk_cnt++;
				}
			}
			ep->s_next_ack_seq = pkt->hdr.seq;
		}

		/// 根据wnd计算发送端允许发送的最大序号
		max_seq = pkt->hdr.seq + pkt->hdr.wnd;
		if (nd_seq_cmp(max_seq, ep->s_max_seq) > 0) {
			ep->s_max_seq = max_seq;
		}
	}

	if (len < 4) {
		goto end;
	}

	/// process ack

	cnt1 = ptr[0];
	if (cnt1 == 0) {
		goto end;
	}

	if (cnt1 > 32 || cnt1 < 0) {
		nd_assert(0);
		ret = ND_RET_ERROR;
		goto end;
	}

	if (cnt1 > 0) {
		/// 仅staged ack1 检查loss
		__nd_rudp_check_sendlst(ep, pkt->hdr.seq, cnt1, ptr + 4, old_next_ack_seq, &chk_cnt, 1);
	}
	
	/// process staged ack
	cnt2 = ntohs(*(nd_uint16_t*)(ptr + 2));
	if (cnt2 > ep->max_staged_ack_range || cnt2 < 0) {
		nd_assert(0);
		ret = ND_RET_ERROR;
		goto end;
	}

	if (len < (12 + (cnt2 >> 3) + (cnt2 & 0x00000007) ? 1 : 0)) {
		nd_assert(0);
		ret = ND_RET_ERROR;
		goto end;
	}

	if (cnt2 > 0) {
		nd_uint32_t staged_ack;
		staged_ack = ntohl(*(nd_uint32_t*)(ptr + 8));
		__nd_rudp_check_sendlst(ep, staged_ack, cnt2, ptr + 12, old_next_ack_seq, &chk_cnt, 0);
	}

end:
	if (ret != ND_RET_OK) {
		//nd_rudp_endpoint_close(ep);
		return ret;
	}

	/// cwnd
	//if (chk_cnt) {
		//nd_rudp_log(ND_RUDP_LOG_LV_TRACE, "recv staged_ack, ack cnt=%d", chk_cnt);
		nd_rudp_endpoint_on_staged_ack(ep, chk_cnt + 1);
	//}

	if (old_next_ack_seq != ep->s_next_ack_seq) {
		nd_rudp_packet_t *p = ep->s_buf_lst[ep->s_next_ack_seq % ep->max_syn_seq_range];
		if (p) {
			nd_rudp_endpoint_chk_loss(ep, p, 0);
		}

		/// 2. 新的数据被ack, 唤醒外部send调用
		nd_mutex_lock(&ep->s_que.mutex);
		if (ep->s_que.free_size == 0) {
			ep->s_que.free_size += nd_seq_diff(ep->s_next_ack_seq, old_next_ack_seq);
			nd_cond_signal(&ep->s_que.cdt);
		}
		else {
			ep->s_que.free_size += nd_seq_diff(ep->s_next_ack_seq, old_next_ack_seq);
		}
		nd_mutex_unlock(&ep->s_que.mutex);
	}
	if (ep->s_zero_wnd) {
		if (nd_rudp_endpoint_get_snd_wnd(ep)) {
			ep->s_zero_wnd = 0;
			/// 发送窗口由0变为非0, 设置定时器立即检测
			if (ep->cc.left_wnd < ep->cc.cwnd)
				nd_rudp_endpoint_update_snd_timer(ep, nd_get_ts());
		}
	}
	return ret;
}
