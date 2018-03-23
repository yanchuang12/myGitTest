#include "nd_rudp_endpoint.h"
#include <math.h>

static void
__handle_next_send_timer_out(nd_tq_node_t *node);

static void
__handle_idle_timer_out(nd_tq_node_t *node);

nd_rudp_endpoint_t*
nd_rudp_endpoint_create()
{
	nd_rudp_endpoint_t *ep;

	ep = malloc(sizeof(nd_rudp_endpoint_t));
	if (!ep) {
		return NULL;
	}

	memset(ep, 0, sizeof(nd_rudp_endpoint_t));

	ep->type = ND_RUDP_EP_TYPE_DUMMY;

	return ep;
}

nd_err_t
nd_rudp_endpoint_init(nd_rudp_endpoint_t *ep)
{
	nd_int_t i;
	nd_uint64_t curr_tm;

	curr_tm = nd_get_ts();
	
	ep->node_in_ch.data = ep;
	ep->node_in_accept.data = ep;

	ep->ling_tmo = (nd_uint64_t)-1;

	ep->max_staged_pkt_cnt = 16;
	ep->mtu = ND_RUDP_MTU;
	ep->mss = ND_RUDP_MTU - ND_RUDP_HDR_LEN;
	ep->rtt = 50*1000;
	ep->rtt_val = (ep->rtt >> 1);
	ep->rto = ep->rtt + (ep->rtt_val << 2);
	ep->max_syn_seq_range = 8192;
	ep->heartbeat_tmo = 15 * 1000 * 1000;
	
	ep->last_snd_staged_ack_tm = curr_tm;

	ep->isyn_seq = (nd_uint32_t)nd_get_ts();
	if (ep->isyn_seq == 0)
		ep->isyn_seq = 1;

	ep->s_next_ack_seq = ep->isyn_seq;
	ep->s_next_seq = ep->isyn_seq;
	ep->s_max_seq = ep->s_next_seq + ep->max_syn_seq_range;

	nd_rudp_endpoint_init_cc(ep);
	__reset_ep_trace_info(&ep->trace_info);
	
	ep->close_wait_tm = 1000*1000*5;

	ep->s_buf_lst = malloc(ep->max_syn_seq_range * sizeof(nd_rudp_packet_t*));
	if (!ep->s_buf_lst) {
		return ND_RET_ERROR;
	}
	for (i = 0; i < ep->max_syn_seq_range; i++)
		ep->s_buf_lst[i] = NULL;

	nd_rbtree_init(&ep->s_loss_lst, &ep->s_loss_lst_sentinel, nd_rbtree_insert_seq_value);

	nd_queue_init(&ep->s_ctrl_lst);

	nd_rudp_sque_init(&ep->s_que);
	ep->s_que.free_size = ep->max_syn_seq_range;
	ep->s_que.max_free_size = ep->max_syn_seq_range;
	
	ep->s_next_tm_node.node.data = &ep->s_next_tm_node;
	ep->s_next_tm_node.node.id = -1;
	ep->s_next_tm_node.handle_timer_out = &__handle_next_send_timer_out;
	ep->s_next_tm_node.data = ep;
	ep->s_next_tm_node.inter_usec = 10000;

	ep->idle_tm_node.node.data = &ep->idle_tm_node;
	ep->idle_tm_node.node.id = -1;
	ep->idle_tm_node.handle_timer_out = &__handle_idle_timer_out;
	ep->idle_tm_node.data = ep;
	ep->idle_tm_node.inter_usec = 2000;

	ep->max_staged_ack_range = 256;
	ep->r_buf_lst = malloc(ep->max_syn_seq_range * sizeof(nd_rudp_packet_t*));
	if (!ep->r_buf_lst) {
		free(ep->s_buf_lst);
		ep->s_buf_lst = NULL;
		return ND_RET_ERROR;
	}
	for (i = 0; i < ep->max_syn_seq_range; i++)
		ep->r_buf_lst[i] = NULL;

	nd_rudp_rque_init(&ep->r_que);

	ep->status = ND_RUDP_EP_STATUS_INIT;

	return ND_RET_OK;
}

void 
nd_rudp_endpoint_destroy(nd_rudp_endpoint_t *ep)
{
	nd_rudp_packet_t *pkt;
	nd_int_t i;
	if (!ep) {
		return;
	}

	if (!ep->ch || (ep->ch->svr && ep->ch->self == ep)) {
		/// dummy, 未初始化的ep
		free(ep);
		return;
	}

	nd_timer_que_remove(ep->ch->rudp->evld->timer_que, &ep->s_next_tm_node);
	nd_timer_que_remove(ep->ch->rudp->evld->timer_que, &ep->idle_tm_node);

	/// s
	if (ep->s_buf_lst) {
		for (i = 0; i < ep->max_syn_seq_range; i++)
		{
			pkt = ep->s_buf_lst[i];
			if (pkt) {
				if (pkt == ep->s_curr_pkt) {
					ep->s_curr_pkt = NULL;
				}
				nd_rudp_packet_destroy(pkt);
			}
		}
		free(ep->s_buf_lst);
	}

	while (!nd_queue_is_empty(&ep->s_ctrl_lst)) {
		pkt = nd_queue_head(&ep->s_ctrl_lst);
		nd_queue_remove(pkt);
		if (pkt == ep->s_curr_pkt) {
			ep->s_curr_pkt = NULL;
		}
		nd_rudp_packet_destroy(pkt);
	}

	if (ep->s_curr_pkt) {
		nd_rudp_packet_destroy(ep->s_curr_pkt);
	}

	nd_rudp_sque_destroy(&ep->s_que);

	/// r
	if (ep->r_buf_lst) {
		for (i = 0; i < ep->max_syn_seq_range; i++) {
			pkt = ep->r_buf_lst[i];
			if (pkt) {
				nd_rudp_packet_destroy(pkt);
			}
		}
		free(ep->r_buf_lst);
	}

	nd_rudp_rque_destroy(&ep->r_que);

	free(ep);
}

void 
nd_rudp_endpoint_post_destroy(nd_rudp_endpoint_t *ep)
{
	if (!ep->post_destroy) {
		ep->post_destroy = 1;
		nd_rudp_evld_locked_add_event(ep->ch->rudp->evld, ND_RUDP_LOCKED_MSG_DESTROY_EP, ep);
	}
}

void 
nd_rudp_endpoint_close(nd_rudp_endpoint_t *ep)
{
	if (ep->status != ND_RUDP_EP_STATUS_CLOSE_END &&
		ep->status != ND_RUDP_EP_STATUS_CLOSE_WAIT) {

		if (ep->status != ND_RUDP_EP_STATUS_CLOSE_END) {
			ep->status = ND_RUDP_EP_STATUS_CLOSE_WAIT;
		}

		switch (ep->type) {
		case ND_RUDP_EP_TYPE_LISTEN:
			ep->ch->listen_close = 1;
			/// 唤醒accept调用
			nd_rudp_accept_que_broadcast(&ep->ch->accept_lst);
			break;
		case ND_RUDP_EP_TYPE_ACCEPT:
		case ND_RUDP_EP_TYPE_CONNECT:
			/// 唤醒send, recv调用
			nd_rudp_sque_broadcast(&ep->s_que);
			nd_rudp_rque_broadcast(&ep->r_que);
			break;
		case ND_RUDP_EP_TYPE_DUMMY:
		default:
			break;
		}
	}
}

static nd_rudp_packet_t*
__get_next_send_packet(nd_rudp_endpoint_t *ep, nd_int_t cycle_flush)
{
	nd_rudp_packet_t *pkt;
	nd_rbtree_node_t *rbn;
	nd_int32_t wnd;
	nd_uint64_t curr;

	curr = nd_get_ts();

	pkt = NULL;

	if (ep->s_curr_pkt) {
		/*if (0 == (ep->s_curr_pkt->hdr.flag & ND_RUDP_HDR_FLAG_DATA) ||
			ep->s_curr_pkt->loss ||
			ep->s_loss_lst.root != &ep->s_loss_lst_sentinel ||
			cycle_flush)*/ {
			ep->s_curr_pkt->expect_snd_tm = 0;
		}

		pkt = ep->s_curr_pkt;
		ep->s_curr_pkt = NULL;
		return pkt;
	}

	/// ctrl
	if (!nd_queue_is_empty(&ep->s_ctrl_lst)) {
		pkt = nd_queue_head(&ep->s_ctrl_lst);
		nd_queue_remove(pkt);
		/// 控制包总是立即发送
		pkt->expect_snd_tm = 0;
		return pkt;
	}

	///  loss
	if (ep->s_loss_lst.root != &ep->s_loss_lst_sentinel) {
		rbn = nd_rbtree_min(ep->s_loss_lst.root, &ep->s_loss_lst_sentinel);
		if (rbn) {
			nd_uint32_t seq = rbn->key;
			pkt = ep->s_buf_lst[seq % ep->max_syn_seq_range];
			nd_rbtree_delete(&ep->s_loss_lst, rbn);
			pkt->expect_snd_tm = 0;
			pkt->loss = 1;
			pkt->hdr.flag = ND_RUDP_HDR_FLAG_DATA;
			ep->cc.last_snd_tm = nd_get_ts();
			return pkt;
		}
	}

	/// process data
	wnd = nd_rudp_endpoint_get_snd_wnd(ep);

	if (wnd > 0 && ep->cc.left_wnd < ep->cc.cwnd) {

		nd_mutex_lock(&ep->s_que.mutex);

		if (!nd_queue_is_empty(&ep->s_que.que)) {

			pkt = nd_queue_head(&ep->s_que.que);
			nd_queue_remove(pkt);
			ep->s_que.size--;

			if (1) {
				nd_rudp_packet_t *test = ep->s_buf_lst[ep->s_next_seq % ep->max_syn_seq_range];
				if (test) {
					nd_mutex_unlock(&ep->s_que.mutex);
					nd_assert(0);
					nd_rudp_log(ND_RUDP_LOG_LV_BUG, "s_buf_lst bug");
					nd_rudp_endpoint_close(ep);
					return NULL;
				}
			}

			pkt->hdr.seq = ep->s_next_seq;
			ep->s_next_seq = nd_seq_inc(ep->s_next_seq);
			pkt->hdr.token = ep->token;
			pkt->hdr.flag |= ND_RUDP_HDR_FLAG_DATA;

			if (ep->s_ppp_flag || cycle_flush) {
				if (ep->s_ppp_flag)
					ep->s_ppp_flag = 0;

				pkt->expect_snd_tm = 0;
				ep->cc.last_snd_tm = curr;
			}
			else {
				ep->cc.last_snd_tm += ep->cc.inter_usec;
				pkt->expect_snd_tm = ep->cc.last_snd_tm;
			}

			/// 确保包对能够连续发送
			if (0 == (pkt->hdr.seq & 0x0000000F) && 
				wnd > 1 && 
				ep->cc.left_wnd + 1 <= ep->cc.cwnd && 
				ep->s_que.size > 0) {

				pkt->hdr.flag |= ND_RUDP_HDR_FLAG_PPP;
				ep->s_ppp_flag = 1;
			}

			if (0x10 == (pkt->hdr.seq & 0x0000001F)) {
				/// 每32个packets强制对端返回ack
				pkt->hdr.flag |= ND_RUDP_HDR_FLAG_PUSH;
			}
		}

		nd_mutex_unlock(&ep->s_que.mutex);
	}

	return pkt;
}

void
nd_rudp_endpoint_restore_next_send_packet(nd_rudp_endpoint_t *ep, nd_rudp_packet_t *pkt)
{
	if (!ep->s_curr_pkt) {
		ep->s_curr_pkt = pkt;
	}
	else {
		nd_assert(0);
	}
}

void
nd_rudp_endpoint_update_snd_timer(nd_rudp_endpoint_t *ep, nd_uint64_t expect_tm)
{
	nd_timer_que_update_schedule(ep->ch->rudp->evld->timer_que, &ep->s_next_tm_node, expect_tm);
}

void
nd_rudp_endpoint_do_send(nd_rudp_endpoint_t *ep, nd_tq_node_t *tq_node, nd_int_t cycle_flush)
{
	nd_rudp_packet_t *pkt;
	nd_err_t r;
	nd_int_t i;
	nd_uint64_t curr;

	//nd_rudp_log(ND_RUDP_LOG_LV_TRACE, "nd_rudp_endpoint_do_send");

	if (ep->ch->closed/* || ep->dead || ep->status > ND_RUDP_EP_STATUS_CONNECTED*/) {
		goto do_no_packet;
	}

	if (ep->ch->wr_pending) {
		nd_rudp_evld_add_wait_write(ep->ch);
		return;
	}

	i = 0;

	while (pkt = __get_next_send_packet(ep, cycle_flush)) {
		curr = nd_get_ts();

		pkt->loss_tmo = 0;
		/// 可以立即发送
		if (pkt->expect_snd_tm == 0 || 
			nd_tm_u64_cmp(curr, pkt->expect_snd_tm) > 0) {

				if ((pkt->hdr.flag & ND_RUDP_HDR_FLAG_DATA)) {
					/// data 包的ts总是更新为当前时间
					//pkt->hdr.flag|= ND_RUDP_HDR_FLAG_PUSH;
					pkt->hdr.ts = (nd_uint32_t)nd_get_ts();
				}

				if ((pkt->hdr.flag & ND_RUDP_HDR_FLAG_PROBE) &&
					(pkt->hdr.flag & ND_RUDP_HDR_FLAG_ACK)) {
					/// 用于rtt测量的应答包
					if (pkt->org_tm)
						pkt->hdr.ts += (nd_int32_t)(curr - pkt->org_tm);
				}

				nd_rudp_packet_recycle(pkt);
				nd_rudp_packet_write(pkt, 0);

				/// 发送时总是使用@ep->remote_addr
				pkt->addr = ep->remote_addr;

				r = nd_rudp_channel_send_to(ep->ch, pkt, &pkt->addr);

				switch (r) {
				case ND_RET_OK:
					if (0 == (pkt->hdr.flag & ND_RUDP_HDR_FLAG_DATA)) {

						if ((pkt->hdr.flag & ND_RUDP_HDR_FLAG_PROBE) && 
							!(pkt->hdr.flag & ND_RUDP_HDR_FLAG_ACK)) {
							ep->last_probe_tm = nd_get_ts();
							ep->probe_ack_tmo = ep->last_probe_tm + ep->rto;
						}

						if (pkt->hdr.flag & ND_RUDP_HDR_FLAG_STAGED_ACK) {
							/// 更新staged ack发送时间
							ep->last_snd_staged_ack_tm = curr;
							ep->r_staged_pkt_cnt = 0;
						}
						/// ctrl packet 需要释放
						nd_rudp_packet_destroy(pkt);
					}
					else {
						nd_rudp_endpoint_on_snd_packet(ep, pkt);

						nd_rudp_endpoint_set_pkt_rto(ep, pkt, curr);

						if (NULL == ep->s_buf_lst[pkt->hdr.seq % ep->max_syn_seq_range]) {
							ep->s_buf_lst[pkt->hdr.seq % ep->max_syn_seq_range] = pkt;
							ep->s_buf_lst_size++;
						}
						else {
							nd_assert(ep->s_buf_lst[pkt->hdr.seq % ep->max_syn_seq_range] == pkt);
						}
						
						if (pkt->loss) {
							/// 清除loss标志
							pkt->loss = 0;
							/// 不要清除dup标志
						}
						else {
							ep->cc.left_wnd ++;
							i++;

							/// 确保包对总是连续发送, 这一点很关键!
							if ((pkt->hdr.flag & ND_RUDP_HDR_FLAG_PPP) && i == __ONE_CYCLE_MAX_SND_PKT_CNT__) {
								i--;
							}

							if (i >= __ONE_CYCLE_MAX_SND_PKT_CNT__) {
								nd_rudp_evld_add_posted_write(ep->ch);
								return;
							}
						}
					}
					
					break;
				default:
					/// 发送失败, 则保存等待重发
					nd_rudp_endpoint_restore_next_send_packet(ep, pkt);
					nd_rudp_evld_add_wait_write(ep->ch);
					return;
				}
		}
		else {
			nd_rudp_endpoint_restore_next_send_packet(ep, pkt);
			if (cycle_flush == 0) {
				/// 如果不能立即发送, 则设置发送定时器的到期时间
				nd_rudp_endpoint_update_snd_timer(ep, pkt->expect_snd_tm);
			}
			return;
		}
	}

	/// 没有数据包可发送, 移除发送定时器
do_no_packet:
	if (0 == nd_rudp_endpoint_get_snd_wnd(ep)) {
		/// 发送窗口为0
		ep->s_zero_wnd = 1;
	}
	
	nd_timer_que_remove(ep->ch->rudp->evld->timer_que, &ep->s_next_tm_node);
}

static void
__handle_next_send_timer_out(nd_tq_node_t *node)
{
	nd_rudp_endpoint_t *ep;

	ep = node->data;
	nd_rudp_endpoint_do_send(ep, node, 0);
}

static void
__handle_idle_timer_out(nd_tq_node_t *node)
{
	nd_rudp_endpoint_t *ep;
	nd_uint64_t curr;
	nd_int_t staged_acked;

	staged_acked = 0;
	ep = node->data;

	//nd_rudp_log(ND_RUDP_LOG_LV_TRACE, "__handle_idle_timer_out");

	curr = nd_get_ts();
	nd_timer_que_update_schedule(ep->ch->rudp->evld->timer_que, node, curr + node->inter_usec);

	if ((ep->status > ND_RUDP_EP_STATUS_CONNECTED || ep->dead) && ep->token) {

		if (ep->status == ND_RUDP_EP_STATUS_LING_WAIT) {

			int has_data = 0;
			if (ep->s_buf_lst_size ||
				(ep->s_curr_pkt && (ep->s_curr_pkt->hdr.flag & ND_RUDP_HDR_FLAG_DATA)) ||
				ep->s_loss_lst.root != &ep->s_loss_lst_sentinel ||
				!nd_queue_is_empty(&ep->s_que.que)) {  /// 外部调用了close, 此时访问@ep->s_que是安全的
				has_data = 1;
			}

			if (0 == has_data) {
				/// 残留数据已经发完并被确认
				ep->status = ND_RUDP_EP_STATUS_CLOSE_WAIT;
				if (ep->dead) {
					ep->dead_tm = curr;
				}
			}
		}

		if (ep->status != ND_RUDP_EP_STATUS_LING_WAIT){
			if (ep->dead && nd_tm_u64_diff(curr, ep->dead_tm) >= ep->close_wait_tm) {
				nd_rudp_endpoint_post_destroy(ep);
				return;
			}

			if (ep->status == ND_RUDP_EP_STATUS_CLOSE_WAIT &&
				nd_tm_u64_diff(curr, ep->last_snd_bye_tm) > 1000*20) {

				ep->last_snd_bye_tm = curr;
				nd_rudp_send_ctrl(ep, (nd_uint32_t)nd_get_ts(), ND_RUDP_HDR_FLAG_BYE, 0);
			}
		}
		
		return;
	}

	if (ep->type == ND_RUDP_EP_TYPE_CONNECT) {
		if (ep->status < ND_RUDP_EP_STATUS_CONNECTED) {
			if (nd_tm_u64_diff(curr ,ep->last_snd_shk_tm) > 100*1000) {
				/// 100msec, 定时发送握手分组. todo: 可以考虑采用指数退避方式来定时
				ep->last_snd_shk_tm = curr;
				nd_rudp_send_ctrl(ep, (nd_uint32_t)nd_get_ts(), ND_RUDP_HDR_FLAG_SHAKEHAND, 0);
			}
			return;
		}
	}

	if (ep->status == ND_RUDP_EP_STATUS_CONNECTED) {

		if (ep->heartbeat_tmo > 0 && 
			ep->last_rcv_tm && 
			nd_tm_u64_diff(curr, ep->last_rcv_tm) > ep->heartbeat_tmo) {

			nd_rudp_send_ctrl(ep, (nd_uint32_t)nd_get_ts(), ND_RUDP_HDR_FLAG_BYE, 0);
			nd_rudp_endpoint_close(ep);
			return;
		}

		if (/*ep->type == ND_RUDP_EP_TYPE_ACCEPT &&*/ nd_tm_u64_diff(curr, ep->trace_info.last_tm) > 1000*1000) {
			nd_rudp_dump_trace_info(ep);
			__reset_ep_trace_info(&ep->trace_info);
		}

		if (1) {
			int flag;
			nd_uint64_t cycle_tm;
			flag = 0;
			cycle_tm = 10 * 1000;

			if (ep->r_zero_wnd && 0 != nd_rudp_endpoint_get_rcv_wnd(ep)) {
				/// 接收端接收窗口由0变为非0, 主动发起窗口变化通知, 如果该通知丢失, 对端也会定时探测
				ep->r_zero_wnd = 0;
				flag |= ND_RUDP_HDR_FLAG_PROBE;
			}
			else if (ep->s_zero_wnd && nd_tm_u64_diff(curr, ep->last_probe_tm) > nd_min(cycle_tm, 200000)) {
				/// 发送窗口为0, 每2个RTT,或者200msec探测一次,这个时间以及探测是否合理,必要???
				flag |= ND_RUDP_HDR_FLAG_PROBE;
			}
			else if (nd_tm_u64_cmp(curr, ep->last_probe_tm + cycle_tm) > 0) {
				/// 定时探测
				flag |= ND_RUDP_HDR_FLAG_PROBE;
			}

			if (nd_tm_u64_diff(curr, ep->last_snd_staged_ack_tm + cycle_tm) > 0 ) {
				/// 定时返回staged ack
				flag |= ND_RUDP_HDR_FLAG_STAGED_ACK;
			}

			if (flag) {
				nd_rudp_send_ctrl(ep, (nd_uint32_t)nd_get_ts(), flag, 0);
			}
			
			/// 探测应答超时, 减速
			if (ep->probe_ack_tmo && nd_tm_u64_cmp(curr, ep->probe_ack_tmo) > 0) {
					ep->exception_cnt++;
					ep->probe_ack_tmo = 0;

					if (ep->cc.slow_start) {
						ep->cc.slow_start = 0;
						if (ep->cc.rate > 0)
							ep->cc.inter_usec = 1000000 / ep->cc.rate;
						else
							ep->cc.inter_usec = (ep->rtt + ep->cc.cycle_tm) / ep->cc.cwnd;

						if (ep->cc.inter_usec < 1)
							ep->cc.inter_usec = 1;
					}
					else if (0 == (ep->exception_cnt % 3)){
						ep->cc.last_dec_inter_usec = ep->cc.inter_usec;
						ep->cc.inter_usec = ceil(ep->cc.inter_usec * 2);
						//nd_rudp_log(ND_RUDP_LOG_LV_TRACE, "\r\n----------inter_usec / 2\r\n");
					}
			}
		}
	}

	/// 定时检测分组loss
	nd_rudp_endpoint_chk_loss(ep, NULL, 0);
	/**/
}

void 
nd_rudp_endpoint_on_dead(nd_rudp_endpoint_t *ep)
{
	if (ep->dead_tm == 0) {
		ep->dead_tm = nd_get_ts();
	}

	if (ep->status < ND_RUDP_EP_STATUS_CONNECTED) {
		nd_rudp_endpoint_close(ep);
		return;
	}
	if (ep->status > ND_RUDP_EP_STATUS_CONNECTED) {
		/// 已经被协议栈close过了
		return;
	}

	if (ep->ling_tmo == 0) {
		/// 不需要优雅的关闭, 则不处理协议栈遗留数据
		nd_rudp_endpoint_close(ep);
		return;
	}

	/// 如果连接被上层close, 但是发送缓冲区还有数据, 
	if (ep->s_buf_lst_size ||
		(ep->s_curr_pkt && (ep->s_curr_pkt->hdr.flag & ND_RUDP_HDR_FLAG_DATA)) ||
		ep->s_loss_lst.root != &ep->s_loss_lst_sentinel ||
		!nd_queue_is_empty(&ep->s_que.que)) {  /// 外部调用了close, 此时访问@ep->s_que是安全的

		ep->status = ND_RUDP_EP_STATUS_LING_WAIT;
		return;
	}

	nd_rudp_endpoint_close(ep);
}

void 
nd_rudp_endpoint_add_loss_pkt(nd_rudp_endpoint_t *ep, nd_rudp_packet_t *pkt) 
{
	pkt->node_in_loss.key = pkt->hdr.seq;
	pkt->loss_cnt = 0;
	pkt->loss = 1;
	pkt->expect_snd_tm = 0;

	nd_rbtree_insert(&ep->s_loss_lst, &pkt->node_in_loss);

	nd_rudp_on_loss(ep, pkt);
}

void
nd_rudp_endpoint_chk_loss(nd_rudp_endpoint_t *ep, nd_rudp_packet_t *pkt, nd_int_t flag)
{
	nd_uint64_t curr;
	nd_int_t loss, wnd;
	curr = nd_get_ts();

	loss = 0;
	wnd = nd_rudp_endpoint_get_snd_wnd(ep);

	if (pkt) {
		if (pkt->loss)
			return;

		if (flag == 1) {
			/// 连续收到了三个大于该序号的后续分组
			
			/// 1. 如果没有待发送分组, 则立即重传该分组
			/*if (nd_queue_is_empty(&ep->s_que.que)) {
				nd_rudp_endpoint_add_loss_pkt(ep, pkt);
				loss = 1;
				goto end;
			}

			/// 2. 如果还有分组待发送, 并且剩余发送窗口不足1/8, 则立即重传该分组
			if (wnd < (ep->max_syn_seq_range >> 3)) {
				nd_rudp_endpoint_add_loss_pkt(ep, pkt);
				loss = 1;
				goto end;
			}*/

			/// 
			pkt->loss_cnt += 3;

			if (pkt->loss_tmo && nd_tm_u64_diff(curr, pkt->loss_tmo) > 0) {
				pkt->loss_cnt ++;
				nd_rudp_endpoint_set_pkt_rto(ep, pkt, curr);
			}
		}
		else {
			if (pkt->loss_tmo && nd_tm_u64_diff(curr, pkt->loss_tmo) > 0) {
				pkt->loss_cnt ++;
				nd_rudp_endpoint_set_pkt_rto(ep, pkt, curr);
			}
		}

		if (pkt->loss_cnt > 2) {
			nd_rudp_endpoint_add_loss_pkt(ep, pkt);
			loss = 1;
			goto end;
		}	
	}
	else {
		
		nd_uint32_t seq, i;
		i = 0;
		seq = ep->s_next_ack_seq;
		pkt = ep->s_buf_lst[seq % ep->max_syn_seq_range];
		
		while (pkt && i++ != 64) {
			if (pkt->loss_tmo && nd_tm_u64_cmp(curr, pkt->loss_tmo) < 0) {
				//break;
			}

			if (!pkt->loss) {

				/// 1. 如果没有待发送分组, 则立即重传该分组
				/*if (nd_queue_is_empty(&ep->s_que.que)) {
					nd_rudp_endpoint_add_loss_pkt(ep, pkt);
					loss = 1;
				}
				else if (wnd < (ep->max_syn_seq_range >> 3)) {
					nd_rudp_endpoint_add_loss_pkt(ep, pkt);
					loss = 1;
				}
				else*/ {
					if (pkt->loss_tmo && nd_tm_u64_diff(curr, pkt->loss_tmo) > 0) {
						pkt->loss_cnt ++;
						nd_rudp_endpoint_set_pkt_rto(ep, pkt, curr);
					}

					if (pkt->loss_cnt > 2) {
						nd_rudp_endpoint_add_loss_pkt(ep, pkt);
						loss = 1;
					}	
				}				
			}

			seq = nd_seq_inc(seq);
			if (nd_seq_cmp(seq, ep->s_next_seq) < 0) {
				pkt = ep->s_buf_lst[seq % ep->max_syn_seq_range];
			}
			else
				break;
		}
	}

end:
	if (loss) {
		nd_rudp_endpoint_update_snd_timer(ep, 0);
	}
}

void
nd_rudp_on_loss(nd_rudp_endpoint_t *ep, nd_rudp_packet_t *pkt)
{
	nd_uint64_t curr;
	curr = nd_get_ts();

	ep->cc.loss = 1;

	ep->trace_info.loss++;
	ep->trace_info.loss_bytes += pkt->hdr.body_len;

	ep->trace_info.total_loss++;
	ep->trace_info.total_loss_bytes += pkt->hdr.body_len;

	ep->cc.loss_cnt++;

	if (ep->cc.slow_start) {
		ep->cc.slow_start = 0;
		if (ep->cc.rate > 0)
			ep->cc.inter_usec = 1000000.0 / ep->cc.rate;
		else
			ep->cc.inter_usec = ((double)(ep->cc.cycle_tm)) / ep->cc.cwnd;

		if (ep->cc.inter_usec < 1)
			ep->cc.inter_usec = 1;
	}

	if (ep->cc.last_loss_tm + ep->cc.cycle_tm < curr) {
		double factor = 1.008;
		if (ep->cc.inter_usec > 1000) {
			factor = 1.02;
		}
		else {
			factor += ep->cc.last_loss_rate * 0.08;
		}

		ep->cc.last_loss_tm = curr;

		ep->cc.last_dec_inter_usec = ep->cc.inter_usec;
		ep->cc.inter_usec = ceil(ep->cc.inter_usec * factor);
		//////////////////////////////////////////////////////////////////////////
		//ep->cc.cwnd = ep->cc.cwnd * 0.9;
	}
}

void
nd_rudp_endpoint_on_snd_packet(nd_rudp_endpoint_t *ep, nd_rudp_packet_t *pkt)
{
	ep->trace_info.snd++;
	ep->trace_info.snd_bytes += pkt->hdr.body_len;

	ep->trace_info.total_snd++;
	ep->trace_info.total_snd_bytes += pkt->hdr.body_len;

	ep->cc.snd_cnt++;

	if (pkt->loss) {
		ep->trace_info.retrans++;
		ep->trace_info.retrans_bytes += pkt->hdr.body_len;

		ep->trace_info.total_retrans++;
		ep->trace_info.total_retrans_bytes += pkt->hdr.body_len;
	}
}

void
nd_rudp_endpoint_on_staged_ack(nd_rudp_endpoint_t *ep, nd_int_t cnt)
{
	nd_uint64_t curr;
	curr = nd_get_ts();

	ep->trace_info.ack += cnt;
	ep->trace_info.total_ack += cnt;

	/// 
	if (nd_tm_u64_diff(curr, ep->cc.last_rcv_staged_ack_tm) < (nd_int64_t)ep->cc.cycle_tm)
		return;

	/// 先将上一个cycle未发完的包尽量发完
	/*if (ep->cc.inter_usec < 1000)*/ {
		nd_rudp_endpoint_do_send(ep, NULL, 1);
	}

	ep->cc.last_rcv_staged_ack_tm = curr;
	ep->cc.begin_tm = curr;
	ep->cc.last_snd_tm = curr;
	ep->cc.left_wnd = 0;

	ep->cc.part.wnd = 0;
	ep->cc.part.tm = curr;

	/// cycle 统计cycle丢失率并清零
	if (ep->cc.snd_cnt) {
		ep->cc.last_loss_rate = ep->cc.last_loss_rate * 0.125 + ((double)ep->cc.loss_cnt / (double)ep->cc.snd_cnt) * 0.875;
	}
	else if (ep->cc.last_loss_rate > 0.00001){
		/// 如果一个cycle内没有发送任何分组, 则将@last_loss_rate 逐步减小. 
		/// 这里的意义在于: 如果在某个时刻loss率很高, 但是应用程序此后长时间
		/// 没有发送任何数据, 则之前统计的loss率应该不再具有参考意义, 需要重新统计, 
		/// 因为链路状况可能发送了变化
		ep->cc.last_loss_rate = ep->cc.last_loss_rate * 0.875;
	}

	if (ep->cc.last_loss_rate > 1)
		ep->cc.last_loss_rate = 1;
	else if (ep->cc.last_loss_rate < 0)
		ep->cc.last_loss_rate = 0;

	ep->cc.loss_cnt = 0;
	ep->cc.snd_cnt = 0;

	if (ep->cc.slow_start) {
		ep->cc.cwnd += cnt;
		//nd_rudp_log(ND_RUDP_LOG_LV_TRACE, "----on_staged_ack: cc.cwnd=%.2f, cnt=%d", ep->cc.cwnd, cnt);
		if (ep->cc.cwnd > 102) {
			ep->cc.slow_start = 0;
			if (ep->cc.rate > 0)
				ep->cc.inter_usec = 1000000 / ep->cc.rate;
			else
				ep->cc.inter_usec = (ep->cc.cycle_tm) / ep->cc.cwnd;
		}
	}
	else {
		double old_wnd = ep->cc.cwnd;
		//if (ep->cc.inter_usec < 1000)
			//ep->cc.cycle_tm = 10 * 1000;
		/*else*/ {
			if (ep->cc.cycle_tm < ep->cc.inter_usec * 5)
				ep->cc.cycle_tm = ep->cc.inter_usec * 5;
			if (ep->cc.cycle_tm < 8 * 1000)
				ep->cc.cycle_tm = 8 * 1000;
			//ep->cc.cycle_tm = 50 * 1000;
		}

		ep->cc.cwnd = (double)ep->cc.rate / 1000000.0 * (/*ep->rtt +*/ ep->cc.cycle_tm) + cnt;
		/*if (ep->cc.cwnd > old_wnd)
			ep->cc.cwnd = ep->cc.cwnd * 1.06;
		else
			ep->cc.cwnd = ep->cc.cwnd * 0.94;*/
		//ep->cc.inter_usec = (ep->rtt + ep->cc.cycle_tm) / ep->cc.cwnd;
	}

	if (ep->cc.inter_usec < 1)
		ep->cc.inter_usec = 1;

	if (ep->cc.slow_start) {
		goto do_end;
	}
	if (ep->cc.loss) {
		ep->cc.loss = 0;
		ep->cc.last_loss_tm = curr;
		goto do_end;
	}

	if (1) {
		const double min_inc = 0.006;
		double inc;
		nd_int64_t B;
		B = (nd_int64_t)(ep->cc.bw - 1000000.0 / ep->cc.inter_usec);
		if ((ep->cc.inter_usec > ep->cc.last_dec_inter_usec) && ((ep->cc.bw / 9) < B))
			B = ep->cc.bw / 9;

		if (B <= 0)
			inc = min_inc;
		else
		{
			inc = pow(10.0, ceil(log10(B * ep->mss * 8.0))) * 0.0000015 / ep->mss;

			if (inc < min_inc)
				inc = min_inc;
		}

		ep->cc.inter_usec = (ep->cc.inter_usec * ep->cc.cycle_tm) / (ep->cc.inter_usec * inc + ep->cc.cycle_tm);

		if (ep->cc.inter_usec < 1)
			ep->cc.inter_usec = 1;
	}

do_end:
	nd_rudp_endpoint_update_snd_timer(ep, curr + ep->cc.inter_usec);
}

void 
nd_rudp_endpoint_on_rcv_packet(nd_rudp_endpoint_t *ep, nd_rudp_packet_t *pkt)
{
	nd_rudp_rcv_pkt_tm_wnd_t *ptw;
	nd_rudp_ppp_t *p;
	nd_uint64_t curr;
	p = &ep->r_ppp;

	curr = nd_get_ts();

	ep->trace_info.rcv++;
	ep->trace_info.rcv_bytes += pkt->hdr.body_len;
	ep->trace_info.total_rcv++;
	ep->trace_info.total_rcv_bytes += pkt->hdr.body_len;

	if (pkt->hdr.flag & ND_RUDP_HDR_FLAG_PPP) {
		p->ts0 = curr;
	}
	else if (p->ts0 && (pkt->hdr.seq & 0x0000000F) == 0x00000001) {
			p->lst[p->pos] = (curr - p->ts0);

			if (p->lst[p->pos] > 1000*1000*10)
				p->lst[p->pos] = 1000*1000*10;

			p->ts0 = 0;

			p->pos++;

			if (p->pos == ND_RUDP_PPP_SIZE)
				p->pos = 0;

			if (p->size < ND_RUDP_PPP_SIZE) {
				p->size++;
			}
	}

	ptw = &ep->r_pkt_tm_wnd;
	if (ptw->ts0 == 0) {
		ptw->ts0 = curr;
	}
	else {
		ptw->lst[ptw->pos] = (nd_int64_t)(curr - ptw->ts0);

		if (ptw->lst[ptw->pos] > 1000*1000*10)
			ptw->lst[ptw->pos] = 1000*1000*10;

		ptw->ts0 = curr;

		ptw->pos++;

		if (ptw->pos == ND_RUDP_PPP_SIZE)
			ptw->pos = 0;

		if (ptw->size < ND_RUDP_PPP_SIZE) {
			ptw->size++;
		}
	}
}

nd_uint32_t 
nd_rudp_endpoint_cal_rate_1(nd_rudp_endpoint_t *ep)
{
	nd_rudp_rcv_pkt_tm_wnd_t *p;
	nd_int64_t *pi;
	int i, n, j, m;

	p = &ep->r_pkt_tm_wnd;

	if (p->size != ND_RUDP_PPP_SIZE)
		return 0;

	pi = ep->r_pkt_tm_wnd.lst;
	for (i = 0, n = (ND_RUDP_PPP_SIZE >> 1) + 1; i < n; ++ i)
	{
		nd_int64_t* pj = pi;
		for (j = i, m = ND_RUDP_PPP_SIZE; j < m; ++ j)
		{
			if (*pi > *pj)
			{
				int temp = *pi;
				*pi = *pj;
				*pj = temp;
			}
			++ pj;
		}
		++ pi;
	}

	if (1) {
		int k, l;
		int median = (ep->r_pkt_tm_wnd.lst[(ND_RUDP_PPP_SIZE >> 1) - 1] + ep->r_pkt_tm_wnd.lst[ND_RUDP_PPP_SIZE >> 1]) >> 1;
		int count = 0;
		int sum = 0;
		int upper = median << 3;
		int lower = median >> 3;

		// median filtering
		nd_int64_t* pk = ep->r_pkt_tm_wnd.lst;
		for (k = 0, l = ND_RUDP_PPP_SIZE; k < l; ++ k)
		{
			if ((*pk < upper) && (*pk > lower))
			{
				++ count;
				sum += *pk;
			}
			++ pk;
		}

		if (count >= (ND_RUDP_PPP_SIZE >> 1))
			return (nd_uint32_t)ceil(1000000.0 / (sum / count));
		else
			return 0;
	}
}

nd_uint32_t 
nd_rudp_endpoint_cal_bw_1(nd_rudp_endpoint_t *ep)
{
	nd_rudp_ppp_t *p;
	nd_int64_t *pi;
	int i, n, j, m;

	p = &ep->r_ppp;

	if (p->size != ND_RUDP_PPP_SIZE)
		return 0;

	pi = ep->r_ppp.lst;
	for (i = 0, n = (ND_RUDP_PPP_SIZE >> 1) + 1; i < n; ++ i)
	{
		nd_int64_t* pj = pi;
		for (j = i, m = ND_RUDP_PPP_SIZE; j < m; ++ j)
		{
			if (*pi > *pj)
			{
				int temp = *pi;
				*pi = *pj;
				*pj = temp;
			}
			++ pj;
		}
		++ pi;
	}

	if (1) {
		int k, l;
		int median = (ep->r_ppp.lst[(ND_RUDP_PPP_SIZE >> 1) - 1] + ep->r_ppp.lst[ND_RUDP_PPP_SIZE >> 1]) >> 1;
		int count = 1;
		int sum = median;
		int upper = median << 3;
		int lower = median >> 3;

		// median filtering
		nd_int64_t* pk = ep->r_ppp.lst;
		for (k = 0, l = ND_RUDP_PPP_SIZE; k < l; ++ k)
		{
			if ((*pk < upper) && (*pk > lower))
			{
				++ count;
				sum += *pk;
			}
			++ pk;
		}

		return (nd_uint32_t)ceil(1000000.0 / (sum / count));
	}
}

nd_uint32_t 
nd_rudp_endpoint_cal_rate_2(nd_rudp_endpoint_t *ep)
{
	nd_rudp_rcv_pkt_tm_wnd_t *p;
	p = &ep->r_pkt_tm_wnd;

	if (p->size == ND_RUDP_PPP_SIZE) {
		double inter;
		nd_int_t min_idx, max_idx;
		nd_int_t i, pos;

		min_idx = p->pos;
		max_idx = p->pos;

		pos = p->pos;

		for (i = 0; i < ND_RUDP_PPP_SIZE; i++) {
			if (i && p->lst[pos] < p->lst[min_idx])
				min_idx = pos;
			if (i && p->lst[pos] > p->lst[max_idx])
				max_idx = pos;

			if (++pos == ND_RUDP_PPP_SIZE)
				pos = 0;
		}

		if (min_idx == max_idx) {
			inter = p->lst[0];
		}
		else {
			nd_int_t idx;

			idx = 0;
			inter = 0;
			pos = p->pos;

			for (i = 0; i < ND_RUDP_PPP_SIZE; i++) {
				if (i != min_idx && i != max_idx) {
					inter += (double)(p->lst[i]);
					idx++;
				}
			}
			inter = inter / idx;
		}

		if (inter < 0.001)
			inter = 0.001;
		else if (inter > 1000000)
			inter = 1000000;
		/// 每秒收到多少包
		return (nd_uint32_t)(1000000.0 / inter);
	}

	return 0;
}

nd_uint32_t 
nd_rudp_endpoint_cal_bw_2(nd_rudp_endpoint_t *ep)
{
	nd_rudp_ppp_t *p;
	p = &ep->r_ppp;

	if (p->size == ND_RUDP_PPP_SIZE) {
		double inter;
		nd_int_t min_idx, max_idx;
		nd_int_t i, pos;

		min_idx = p->pos;
		max_idx = p->pos;

		pos = p->pos;

		for (i = 0; i < ND_RUDP_PPP_SIZE; i++) {
			if (i && p->lst[pos] < p->lst[min_idx])
				min_idx = pos;
			if (i && p->lst[pos] > p->lst[max_idx])
				max_idx = pos;

			if (++pos == ND_RUDP_PPP_SIZE)
				pos = 0;
		}

		if (min_idx == max_idx) {
			inter = p->lst[0];
		}
		else {
			nd_int_t idx;

			idx = 0;
			inter = 0;
			pos = p->pos;

			for (i = 0; i < ND_RUDP_PPP_SIZE; i++) {
				if (i != min_idx && i != max_idx) {
					inter += (double)(p->lst[i]);
					idx++;
				}
			}
			inter = inter / idx;
		}

		if (inter < 0.001)
			inter = 0.001;
		else if (inter > 1000000)
			inter = 1000000;
		/// 每秒收到多少包
		return (nd_uint32_t)(1000000.0 / inter);
	}

	return 0;
}

static double __smooth_factor[ND_RUDP_PPP_SIZE - 2] = 
{
	0.1, 0.125, 0.15, 0.175, 0.2, 0.25
};

nd_uint32_t 
nd_rudp_endpoint_cal_rate_3(nd_rudp_endpoint_t *ep)
{
	nd_rudp_rcv_pkt_tm_wnd_t *p;
	p = &ep->r_pkt_tm_wnd;

	if (p->size == ND_RUDP_PPP_SIZE) {
		double inter;
		nd_int_t min_idx, max_idx;
		nd_int_t i, pos;

		min_idx = p->pos;
		max_idx = p->pos;

		pos = p->pos;

		for (i = 0; i < ND_RUDP_PPP_SIZE; i++) {
			if (i && p->lst[pos] < p->lst[min_idx])
				min_idx = pos;
			if (i && p->lst[pos] > p->lst[max_idx])
				max_idx = pos;

			if (++pos == ND_RUDP_PPP_SIZE)
				pos = 0;
		}

		if (min_idx == max_idx) {
			inter = p->lst[0];
		}
		else {
			nd_int_t idx;

			idx = 0;
			inter = 0;
			pos = p->pos;

			for (i = 0; i < ND_RUDP_PPP_SIZE; i++) {
				if (i != min_idx && i != max_idx && idx < sizeof(__smooth_factor) / sizeof(__smooth_factor[0])) {
					inter += ((double)(p->lst[pos])) * __smooth_factor[idx];
					idx++;
				}
				if (++pos == ND_RUDP_PPP_SIZE)
					pos = 0;
			}
		}

		if (inter < 0.001)
			inter = 0.001;
		else if (inter > 1000000)
			inter = 1000000;
		/// 每秒收到多少包
		return (nd_uint32_t)(1000000.0 / inter);
	}

	return 0;
}

nd_uint32_t 
nd_rudp_endpoint_cal_bw_3(nd_rudp_endpoint_t *ep)
{
	nd_rudp_ppp_t *p;
	p = &ep->r_ppp;

	if (p->size == ND_RUDP_PPP_SIZE) {
		double inter;
		nd_int_t min_idx, max_idx;
		nd_int_t i, pos;

		min_idx = p->pos;
		max_idx = p->pos;

		pos = p->pos;

		for (i = 0; i < ND_RUDP_PPP_SIZE; i++) {
			if (i && p->lst[pos] < p->lst[min_idx])
				min_idx = pos;
			if (i && p->lst[pos] > p->lst[max_idx])
				max_idx = pos;

			if (++pos == ND_RUDP_PPP_SIZE)
				pos = 0;
		}

		if (min_idx == max_idx) {
			inter = p->lst[0];
		}
		else {
			nd_int_t idx;

			idx = 0;
			inter = 0;
			pos = p->pos;

			for (i = 0; i < ND_RUDP_PPP_SIZE; i++) {
				if (i != min_idx && i != max_idx && idx < sizeof(__smooth_factor) / sizeof(__smooth_factor[0])) {
					inter += ((double)(p->lst[pos])) * __smooth_factor[idx];
					idx++;
				}
				if (++pos == ND_RUDP_PPP_SIZE)
					pos = 0;
			}
		}

		if (inter < 0.001)
			inter = 0.001;
		else if (inter > 1000000)
			inter = 1000000;
		/// 每秒收到多少包
		return (nd_uint32_t)(1000000.0 / inter);
	}

	return 0;
}

void
nd_rudp_dump_trace_info(nd_rudp_endpoint_t *ep)
{
	nd_uint64_t curr;
	nd_rudp_ep_trace_info_t *ti;

	ti = &ep->trace_info;
	curr = nd_get_ts();

	ti->snd_rate = ((double)(ti->snd_bytes) / (double)(curr - ti->last_tm)) * 1000000.0;
	ti->rcv_rate = ((double)(ti->rcv_bytes) / (double)(curr - ti->last_tm)) * 1000000.0;

	if (!nd_rudp_log_enable(ND_RUDP_LOG_LV_DUMP)) {
		return;
	}

	nd_rudp_log(ND_RUDP_LOG_LV_DUMP, 
		"dump trace: "
		"snd_rate=%llu(Bytes/S), rcv_rate=%llu(Bytes/S), "
		"total_snd=%llu, total_snd_bytes=%llu, total_rcv=%llu, total_rcv_bytes=%llu, "
		"total_loss=%llu, total_loss_bytes=%llu, total_retrans=%llu, total_retrans_bytes=%llu, "
		"total_ack=%llu, "
		"loss=%llu, loss_bytes=%llu, snd=%llu, snd_bytes=%llu, "
		"rcv=%llu, rcv_bytes=%llu, retrans=%llu, retrans_bytes=%llu, "
		"ack=%llu",
		ti->snd_rate, ti->rcv_rate,
		ti->total_snd, ti->total_snd_bytes, ti->total_rcv, ti->total_rcv_bytes,
		ti->total_loss, ti->total_loss_bytes, ti->total_retrans, ti->total_retrans_bytes,
		ti->total_ack,
		ti->loss, ti->loss_bytes, ti->snd, ti->snd_bytes,
		ti->rcv, ti->rcv_bytes, ti->retrans, ti->retrans_bytes,
		ti->ack);

	nd_rudp_log(ND_RUDP_LOG_LV_DUMP, 
		"dump trace: "
		"rtt=%u, cycle_tm=%llu, cwnd=%.2f, cc.left_wnd=%d, inter_usec=%.2f, cc.rate=%u, cc.bw=%u, cc.last_loss_rate=%.2f, "
		"s_next_seq=%u, s_next_ack_seq=%u, s_max_seq=%u, "
		"r_next_seq=%u, r_last_seq=%u, r_last_max_seq=%u, r_next_staged_ack_seq=%u, "
		"s_que.size=%u, r_que.size=%u, s_buf_lst_size=%u, r_buf_lst_size=%d\r\n",
		ep->rtt, ep->cc.cycle_tm, ep->cc.cwnd, ep->cc.left_wnd, ep->cc.inter_usec, ep->cc.rate, ep->cc.bw, ep->cc.last_loss_rate,
		ep->s_next_seq, ep->s_next_ack_seq, ep->s_max_seq,
		ep->r_next_seq, ep->r_last_seq, ep->r_last_max_seq, ep->r_next_staged_ack_seq,
		ep->s_que.size, ep->r_que.size, ep->s_buf_lst_size, ep->r_buf_lst_size);
}
