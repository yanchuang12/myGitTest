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

typedef struct {
	nd_uint32_t mtu;
	nd_uint32_t rate;
}mtu_rate_pair_t;

static mtu_rate_pair_t s_mtu_rate_lst[] = { 
	{520, 1024*1024}, 
	{1018, 1024*1024*20}, 
	{1442, (1024*1024*1024)}
};

nd_err_t
nd_rudp_endpoint_init(nd_rudp_endpoint_t *ep)
{
	nd_int_t i;
	nd_uint64_t curr_tm;

	curr_tm = nd_get_ts();
	
	ep->node_in_ch.data = ep;
	ep->node_in_accept.data = ep;

	ep->ling_tmo = (nd_uint64_t)2000*1000;

	ep->max_staged_pkt_cnt = 32;
	ep->max_staged_ack_cnt1 = 32;
	ep->max_staged_ack_cnt2 = 32;

	ep->mtu_monitor.idx = 1;

	ep->pmtu = 1;
	ep->mtu = s_mtu_rate_lst[ep->mtu_monitor.idx].mtu;

	ep->mss = ep->mtu - ND_RUDP_HDR_LEN;
	ep->rtt = 10*1000;
	ep->rtt_val = (ep->rtt >> 1);
	ep->rto = ep->rtt + (ep->rtt_val << 2);
	ep->rto_min = 50000;
	ep->rto_max = 5000000;
	ep->max_syn_seq_range = 816;
	ep->heartbeat_tmo = 30 * 1000 * 1000;

	ep->losschk_threshold.tmo_cnt_limit = 2;
	ep->losschk_threshold.rack_cnt_limit = 2;
	
	ep->last_snd_staged_ack_tm = curr_tm;
	ep->last_ack_bw_rate_tm = curr_tm;

	ep->delay_level = ND_RUDP_DELAY_LV_DEFAULT;
	ep->loss_threshold = 0.01;
	ep->loss_rate = 0;

	ep->isyn_seq = (nd_uint32_t)nd_get_ts();
	if (ep->isyn_seq == 0)
		ep->isyn_seq = 1;

	ep->s_next_ack_seq = ep->isyn_seq;
	ep->s_max_ack_seq = nd_seq_dec(ep->isyn_seq);
	ep->s_next_seq = ep->isyn_seq;
	ep->s_next_vir_seq = 1;
	ep->s_max_seq = ep->s_next_seq + ep->max_syn_seq_range;

	nd_rudp_endpoint_init_cc(ep);
	__reset_ep_trace_info(&ep->trace_info);
	
	ep->dead_wait_tmo = 1000*1000*5;
	ep->close_wait_tmo = 1000*1000*2;

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
	ep->idle_tm_node.inter_usec = 1000;

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

		if (ep->status != ND_RUDP_EP_STATUS_CLOSE_END &&
			ep->status != ND_RUDP_EP_STATUS_LING_WAIT) {
			ep->status = ND_RUDP_EP_STATUS_CLOSE_WAIT;
		}

		switch (ep->type) {
		case ND_RUDP_EP_TYPE_LISTEN:
			ep->ch->listen_close = 1;
			ep->status = ND_RUDP_EP_STATUS_CLOSE_END;
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
	double inter_usec;

	curr = nd_get_ts();

	if (ep->cc_mode == ND_RUDP_CC_MODE_FIXED_RATE)
		inter_usec = ep->cc.inter_usec;
	else {
		if (ep->delay_level == ND_RUDP_DELAY_LV_DEFAULT)
			inter_usec = ep->cc.inter_usec;
		else
			inter_usec = nd_min(ep->cc.inter_usec, ep->cc.burst_inter_usec); /// 如果是时延优先模式, 则采用burst send, 但cwnd仍然根据实际发送间隔计算
	}

	pkt = NULL;

	if (ep->s_curr_pkt) {
		if (ep->s_curr_pkt->hdr.flag & ND_RUDP_HDR_FLAG_DATA) {
			/// @s_curr_pkt 是上次本来要发送, 但因为时间未到或者IO阻塞而没有发成功的分组, 
			/// 所以, 这里不能改变ep->s_curr_pkt->expect_snd_tm 和 ep->cc.last_expect_snd_tm的值
			ep->cc.last_expect_snd_tm += 5;
			ep->s_curr_pkt->hdr.flag &= (ND_RUDP_HDR_FLAG_DATA | ND_RUDP_HDR_FLAG_DUP);
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
			pkt->in_loss_lst = 0;
			pkt->fast_restrans = 0;
			pkt->hdr.flag = (ND_RUDP_HDR_FLAG_DATA | ND_RUDP_HDR_FLAG_DUP);
			ep->cc.last_expect_snd_tm += inter_usec;
			pkt->expect_snd_tm = ep->cc.last_expect_snd_tm;

			pkt->hdr.vir_seq = ep->s_next_vir_seq;
			ep->s_next_vir_seq++;

			return pkt;
		}
	}

	/// process data
	wnd = nd_rudp_endpoint_get_snd_wnd(ep);

	if ((!ep->net_block || ep->s_buf_lst_size < 8) 
		&& wnd > 0 
		&& ep->cc.left_wnd < ep->cc.cwnd) {

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

			if (ep->s_ppp_flag || cycle_flush) {
				if (ep->s_ppp_flag)
					ep->s_ppp_flag = 0;

				ep->cc.last_expect_snd_tm += inter_usec;
				pkt->expect_snd_tm = 0;
			}
			else {
				ep->cc.last_expect_snd_tm += inter_usec;
				pkt->expect_snd_tm = ep->cc.last_expect_snd_tm;
			}

			if (ep->cc.monitor.no_data) {
				nd_int64_t x;
				ep->cc.monitor.no_data = 0;
				x = nd_tm_u64_diff(curr, ep->cc.last_expect_snd_tm);
				if (x >= 0)
					ep->cc.last_expect_snd_tm = curr;
			}

			pkt->hdr.seq = ep->s_next_seq;
			ep->s_next_seq = nd_seq_inc(ep->s_next_seq);

			pkt->hdr.vir_seq = ep->s_next_vir_seq;
			ep->s_next_vir_seq++;

			pkt->hdr.token = ep->token;
			pkt->hdr.flag |= ND_RUDP_HDR_FLAG_DATA;

			/// 确保包对能够连续发送
			if ((ep->cc_mode != ND_RUDP_CC_MODE_FIXED_RATE)&&
				0 == (pkt->hdr.seq & 0x0000000F) && 
				wnd > 1 && 
				/*ep->cc.left_wnd + 1 <= ep->cc.cwnd && */
				ep->s_que.size > 0) {

				pkt->hdr.flag |= ND_RUDP_HDR_FLAG_PPP;
				ep->s_ppp_flag = 1;
			}

			if (/*0 == (pkt->hdr.seq % 32) || */ep->s_que.size == 0 || (!ep->s_ppp_flag && ep->cc.left_wnd + 1 > ep->cc.cwnd)) {
				/// 每32个packets, 或者发送队列即将为空, 强制对端返回ack
				pkt->hdr.flag |= ND_RUDP_HDR_FLAG_PUSH;
				if (nd_tm_u64_cmp(curr, ep->last_probe_tm + ep->cc.cycle_tm) > 0)
					pkt->hdr.flag |= ND_RUDP_HDR_FLAG_PROBE;
			}

			if (nd_tm_u64_cmp(curr, ep->last_probe_tm + ep->cc.cycle_tm) > 0)
				pkt->hdr.flag |= ND_RUDP_HDR_FLAG_PROBE;
		}
		else {
			/// sque is empty
			ep->cc.monitor.last_snd_tm = ep->cc.last_expect_snd_tm;
			ep->cc.monitor.no_data = 1;
			ep->cc.monitor.last_nodata_tm = curr;

			ep->cc.last_expect_snd_tm = curr;
		}

		nd_mutex_unlock(&ep->s_que.mutex);
	}

	if (!pkt) {
		ep->cc.monitor.busy = 0;
	}

	return pkt;
}

void
nd_rudp_endpoint_restore_next_send_packet(nd_rudp_endpoint_t *ep, nd_rudp_packet_t *pkt)
{
	if (!ep->s_curr_pkt) {
		ep->s_curr_pkt = pkt;
		pkt->loss_tmo = 0;
		pkt->loss_cnt = 0;
		pkt->tm_loss_cnt = 0;
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
nd_rudp_endpoint_do_send(nd_rudp_endpoint_t *ep, nd_tq_node_t *tq_node, nd_int_t cycle_flush, nd_int_t max_cnt)
{
	nd_rudp_packet_t *pkt;
	nd_err_t r;
	nd_uint64_t curr, begin_tm;
	nd_int_t i;
	double inter_usec;

	if (ep->ch->closed) {
		goto do_no_packet;
	}

	if (ep->ch->wr_pending) {
		nd_rudp_evld_add_wait_write(ep->ch);
		return;
	}

	i = 0;

	if (max_cnt < 1)
		max_cnt = __ONE_CYCLE_MAX_SND_PKT_CNT__;

	/*if (1) {
		int m = __ONE_CYCLE_MAX_SND_PKT_CNT__ / (max(1, ep->ch->eps.size));
		if (m < 1) m = 1;
		if (max_cnt > m)
			max_cnt = m;
	}*/

	begin_tm = nd_get_ts();

	while (pkt = __get_next_send_packet(ep, cycle_flush)) {
		curr = nd_get_ts();
		pkt->loss_tmo = 0;
		pkt->loss_cnt = 0;
		pkt->tm_loss_cnt = 0;

		/// 可以立即发送
		if (nd_tm_u64_diff(curr, begin_tm) < 5000 && 
			(pkt->expect_snd_tm == 0 || nd_tm_u64_cmp(curr + 100, pkt->expect_snd_tm) > 0)) {

				if ((pkt->hdr.flag & ND_RUDP_HDR_FLAG_DATA)) {
					/// data 包的ts总是更新为当前时间
					pkt->hdr.ts = (nd_uint32_t)nd_get_ts();

					/*  对于以下情况, 总是强制对端立即立即返回ACK
						1. 对于慢启动, 
						2. 如果发送间隔小于rto
						3. 发送缓冲区已满
					*/
					/*if ((ep->cc.slow_start && 0 == (pkt->hdr.seq % 32)) || 
						(ep->rtt > ep->cc.cycle_tm && (inter_usec + inter_usec) > ep->rtt)) {
						pkt->hdr.flag|= ND_RUDP_HDR_FLAG_PUSH;
					}*/

					if (pkt->hdr.flag & ND_RUDP_HDR_FLAG_DUP) {
						pkt->affter_seq_when_retrans = ep->s_next_seq;
					}
				}

				if (pkt->hdr.flag & ND_RUDP_HDR_FLAG_PROBE) { 
					if (pkt->hdr.flag & ND_RUDP_HDR_FLAG_ACK) {
						/// 用于rtt测量的应答包
						if (pkt->org_tm)
							pkt->hdr.ts += (nd_int32_t)(curr - pkt->org_tm);
					} 
					else {
						pkt->hdr.ts = curr;
					}
				}

				nd_rudp_packet_recycle(pkt);
				nd_rudp_packet_write(pkt, 0);

				/// 发送时总是使用@ep->remote_addr
				nd_rudp_ip_copy(&pkt->so_addr, &ep->so_remote_addr, ep->ch->ip_version);

				r = nd_rudp_channel_send_to(ep->ch, pkt, &pkt->so_addr);

				switch (r) {
				case ND_RET_OK:
					if ((pkt->hdr.flag & ND_RUDP_HDR_FLAG_PROBE) && 
						!(pkt->hdr.flag & ND_RUDP_HDR_FLAG_ACK)) {
							ep->last_probe_tm = nd_get_ts();
							ep->probe_ack_tmo = ep->last_probe_tm + ep->rto;
					}

					if (0 == (pkt->hdr.flag & ND_RUDP_HDR_FLAG_DATA)) {
						if (pkt->hdr.flag & ND_RUDP_HDR_FLAG_STAGED_ACK) {
							/// 更新staged ack发送时间
							ep->last_snd_staged_ack_tm = curr;
							ep->r_staged_pkt_cnt = 0;
						}
						/// ctrl packet 需要释放
						nd_rudp_packet_destroy(pkt);
					}
					else {
						ep->last_snd_data_tm = curr;
						nd_rudp_endpoint_on_snd_packet(ep, pkt);
						nd_rudp_endpoint_set_pkt_rto(ep, pkt, curr);

						if (NULL == ep->s_buf_lst[pkt->hdr.seq % ep->max_syn_seq_range]) {
							ep->s_buf_lst[pkt->hdr.seq % ep->max_syn_seq_range] = pkt;
							ep->s_buf_lst_size++;
						}
						else {
							nd_assert(ep->s_buf_lst[pkt->hdr.seq % ep->max_syn_seq_range] == pkt);
						}
						
						ep->cc.left_wnd += 1;
						i++;

						/// 确保包对总是连续发送, 这一点很关键!
						if (pkt->hdr.flag & ND_RUDP_HDR_FLAG_PPP) {
							if (i == max_cnt)
								i--;
							if (ep->cc.left_wnd >= ep->cc.cwnd)
								ep->cc.left_wnd -= 1;
						}

						if (i >= max_cnt) {
							nd_rudp_evld_add_posted_write(ep->ch);
							return;
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

	//nd_rudp_endpoint_update_snd_timer(ep, ep->cc.last_expect_snd_tm + ep->cc.inter_usec);
	nd_timer_que_remove(ep->ch->rudp->evld->timer_que, &ep->s_next_tm_node);
}

static void
__handle_next_send_timer_out(nd_tq_node_t *node)
{
	nd_rudp_endpoint_t *ep;

	ep = node->data;
	nd_rudp_endpoint_do_send(ep, node, 0, -1);
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

			if (0 == has_data || (ep->ling_tmo > 0 && nd_tm_u64_diff(curr, ep->ling_wait_tm) >= ep->ling_tmo)) {
				/// 残留数据已经发完并被确认
				ep->status = ND_RUDP_EP_STATUS_CLOSE_WAIT;
				if (ep->dead) {
					ep->dead_tm = curr;
				}
			}
		}

		if (ep->status != ND_RUDP_EP_STATUS_LING_WAIT){
			if (ep->dead && nd_tm_u64_diff(curr, ep->dead_tm) >= ep->dead_wait_tmo) {
				nd_rudp_endpoint_post_destroy(ep);
				return;
			}

			if (ep->status == ND_RUDP_EP_STATUS_CLOSE_WAIT) {
				if (ep->close_wait_tm == 0) {
					ep->close_wait_tm = curr;
				}
				if (nd_tm_u64_diff(curr, ep->close_wait_tm) > ep->close_wait_tmo) {
					ep->status = ND_RUDP_EP_STATUS_CLOSE_END;
				}
				else if (nd_tm_u64_diff(curr, ep->last_snd_bye_tm) > 1000*500)  {
					ep->last_snd_bye_tm = curr;
					nd_rudp_send_ctrl(ep, (nd_uint32_t)nd_get_ts(), ND_RUDP_HDR_FLAG_BYE, 0);
				}
			}

			if (ep->status == ND_RUDP_EP_STATUS_CLOSE_END) {
				nd_timer_que_remove(ep->ch->rudp->evld->timer_que, &ep->s_next_tm_node);
				if (ep->type == ND_RUDP_EP_TYPE_CONNECT) {
					nd_rudp_channel_close(ep->ch);
				}
			}
		}
		
		return;
	}

	if (ep->type == ND_RUDP_EP_TYPE_CONNECT) {
		if (ep->status == ND_RUDP_EP_STATUS_SHAKEHAND) {
			if (ep->first_snd_shk_tm && nd_tm_u64_diff(curr, ep->first_snd_shk_tm) > 35*1000*1000) {
				nd_rudp_endpoint_close(ep);
				return;
			}
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

		if (0 && nd_tm_u64_diff(curr, ep->trace_info.last_tm) > 1000*1000) {
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
			else if (ep->s_zero_wnd && nd_tm_u64_diff(curr, ep->last_probe_tm) > nd_min(ep->rtt, 200000)) {
				/// 发送窗口为0, 每2个RTT,或者200msec探测一次,这个时间以及探测是否合理,必要???
				flag |= ND_RUDP_HDR_FLAG_PROBE;
			}
			else {
				if (ep->s_buf_lst_size > 0 || ep->s_que.size > 0) {
					if (nd_tm_u64_cmp(curr, ep->last_probe_tm + (ep->cc.cycle_tm << 1)) > 0)
						flag |= ND_RUDP_HDR_FLAG_PROBE;
				}
				else {
					if (nd_tm_u64_cmp(curr, ep->last_probe_tm + ep->heartbeat_tmo / 3) > 0)
						flag |= ND_RUDP_HDR_FLAG_PROBE;
				}
			}

			if (ep->r_staged_pkt_cnt > 0 || ep->r_buf_lst_size > 0) {
				/// 定时返回staged ack
				if (nd_tm_u64_diff(curr, ep->last_snd_staged_ack_tm + ep->cc.cycle_tm) > 0) {
					ep->r_staged_pkt_cnt = 0;
					flag |= ND_RUDP_HDR_FLAG_STAGED_ACK;
				}
			}

			if (flag) {
				nd_rudp_send_ctrl(ep, (nd_uint32_t)nd_get_ts(), flag, 0);
			}
			
			/// 探测应答超时, 减速
			if (ep->probe_ack_tmo && nd_tm_u64_cmp(curr, ep->probe_ack_tmo) > 0) {
					ep->exception_cnt++;
					ep->probe_ack_tmo = 0;

					if (0 == (ep->exception_cnt % 3)){
						ep->cc.inter_usec = ep->cc.inter_usec * 2;
						ep->cc.last_dec_inter_usec = ep->cc.inter_usec;
						nd_rudp_log(ND_RUDP_LOG_LV_TRACE, "probe timeout !");
					}

					if (0 == (ep->exception_cnt % 9)) {
						ep->net_block = 1;
					}

					if (ep->cc_mode == ND_RUDP_CC_MODE_FIXED_RATE) {
						ep->cc.inter_usec = ep->cc.fixed_inter_usec;
					}
			}
		}
	}

	/// 定时检测分组loss
	nd_rudp_endpoint_chk_loss(ep, NULL, 0, curr);

	nd_rudp_endpoint_on_staged_ack(ep, 0, 0);
	/**/
}

void 
nd_rudp_endpoint_on_dead(nd_rudp_endpoint_t *ep)
{
	if (ep->dead && ep->dead_tm == 0) {
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

		ep->ling_wait_tm = nd_get_ts();
		ep->status = ND_RUDP_EP_STATUS_LING_WAIT;
	}

	nd_rudp_endpoint_close(ep);
}

void 
nd_rudp_endpoint_add_loss_pkt(nd_rudp_endpoint_t *ep, nd_rudp_packet_t *pkt) 
{
	pkt->node_in_loss.key = pkt->hdr.seq;
	pkt->loss_cnt = 0;
	pkt->tm_loss_cnt = 0;
	pkt->expect_snd_tm = 0;
	pkt->hdr.flag |= ND_RUDP_HDR_FLAG_DUP;
	if (pkt->retrans_cnt < 6)
		pkt->retrans_cnt ++;
	
	if (pkt->in_loss_lst == 0) {
		nd_rbtree_insert(&ep->s_loss_lst, &pkt->node_in_loss);
		pkt->in_loss_lst = 1;
	}
	else {
		nd_rudp_log(ND_RUDP_LOG_LV_BUG, "loss list bug: nd_rudp_endpoint_add_loss_pkt");
		nd_assert(0);
	}

	nd_rudp_on_loss(ep, pkt);
}

void
nd_rudp_endpoint_chk_loss(nd_rudp_endpoint_t *ep, nd_rudp_packet_t *pkt, nd_int_t flag, nd_uint64_t curr_tm)
{
	nd_uint64_t curr;
	nd_int_t loss, wnd, add_loss;
	curr = curr_tm;

	loss = 0; add_loss = 0;
	wnd = nd_rudp_endpoint_get_snd_wnd(ep);

	if (pkt) {
		if (pkt->in_loss_lst || pkt == ep->s_curr_pkt)
			return;

		if (flag == 1) {
			/// staged ack1 区间loss检测
			nd_int16_t vir_diff = ep->s_max_vir_seq - pkt->hdr.vir_seq;
			if (vir_diff > 0) {
				pkt->loss_cnt += vir_diff;
				pkt->hdr.vir_seq = ep->s_max_vir_seq;
			}
			else if (vir_diff == 0)
				pkt->loss_cnt++;
		}
		else if (flag == 2 || flag == 0) { /// 最小序号检测
			nd_int16_t vir_diff = ep->s_max_vir_seq - pkt->hdr.vir_seq;
			if (vir_diff > 0) {
				pkt->loss_cnt += vir_diff;
				pkt->hdr.vir_seq = ep->s_max_vir_seq;
			}
			else if (vir_diff == 0)
				pkt->loss_cnt++;
		}
		
		if (pkt->loss_tmo && nd_tm_u64_diff(curr, pkt->loss_tmo) > 0) {
			pkt->tm_loss_cnt ++;
			nd_rudp_endpoint_set_pkt_rto(ep, pkt, curr);
		}

		if (pkt->loss_cnt > ep->losschk_threshold.rack_cnt_limit) {
			pkt->fast_restrans = 1;
			ep->trace_info.fast_retrans++;
			ep->trace_info.total_fast_retrans++;
		}

		if (pkt->fast_restrans || pkt->tm_loss_cnt > ep->losschk_threshold.tmo_cnt_limit) {
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
		
		while (pkt && i++ != 8) {

			if (!pkt->in_loss_lst && pkt != ep->s_curr_pkt) {

				nd_int16_t vir_diff = ep->s_max_vir_seq - pkt->hdr.vir_seq;
				if (vir_diff > 0) {
					pkt->loss_cnt += vir_diff;
					pkt->hdr.vir_seq = ep->s_max_vir_seq;
				}

				if (pkt->loss_cnt > ep->losschk_threshold.rack_cnt_limit) {
					pkt->fast_restrans = 1;
					ep->trace_info.fast_retrans++;
					ep->trace_info.total_fast_retrans++;
				}

				if (pkt->loss_tmo && nd_tm_u64_diff(curr, pkt->loss_tmo) > 0) {
					pkt->tm_loss_cnt ++;
					nd_rudp_endpoint_set_pkt_rto(ep, pkt, curr);
				}

				if (pkt->tm_loss_cnt > ep->losschk_threshold.tmo_cnt_limit || pkt->fast_restrans) {
					nd_rudp_endpoint_add_loss_pkt(ep, pkt);
					loss = 1;
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
	double d;
	nd_uint64_t curr;
	curr = nd_get_ts();

	ep->cc.loss = 1;

	ep->trace_info.loss++;
	ep->trace_info.loss_bytes += pkt->hdr.body_len;

	ep->trace_info.total_loss++;
	ep->trace_info.total_loss_bytes += pkt->hdr.body_len;

	ep->cc.loss_cnt++;

	if (ep->cc_mode == ND_RUDP_CC_MODE_FIXED_RATE) {
		return;
	}

	if (ep->cc.slow_start && ep->cc.loss_cnt > 2) {
		ep->cc.slow_start = 0;
	}

	if (ep->cc.slow_start) {
		return;
	}

	if (ep->loss_rate < ep->loss_threshold)
		return;

	d = 0;

	if (pkt->hdr.seq > ep->cc.last_loss_seq /*&& ep->cc.begin_tm + ep->cc.cycle_tm < curr*/) {
		++ep->cc.x2;
	//if (ep->cc.begin_tm + ep->cc.cycle_tm < curr) {

		/*if (ep->cc.inter_usec >  ep->cc.last_dec_inter_usec) {
			if (pkt && pkt->fast_restrans)
				ep->cc.inter_usec *= 1.02 + d;
			else
				ep->cc.inter_usec *= 1.04 + d;
		}
		else*/ {
			if (pkt && pkt->fast_restrans)
				ep->cc.inter_usec *= 1.04 + d;
			else
				ep->cc.inter_usec *= 1.06 + d;
		}

		ep->cc.last_dec_inter_usec = ep->cc.inter_usec;
		

		ep->cc.x_loss_avg = ceil(ep->cc.x_loss_avg * 0.875 + ep->cc.x_loss_cnt * 0.125);

		ep->cc.x_loss_cnt = 1;
		ep->cc.x_loss_dec_cnt = 1;

		ep->cc.last_loss_seq = nd_seq_dec(ep->s_next_seq);

		srand(ep->cc.last_loss_seq);
		ep->cc.x_loss_dec_random = ceil(ep->cc.x_loss_avg * (((double)rand()) / 0x7fff));
		if (ep->cc.x_loss_dec_random < 1)
			ep->cc.x_loss_dec_random = 1;
	}
	else if (ep->cc.x_loss_dec_cnt++ < 5 && 0 == (++ep->cc.x_loss_cnt % ep->cc.x_loss_dec_random)){
		++ep->cc.x2;
		if (pkt && pkt->fast_restrans)
			ep->cc.inter_usec *= (1.04 + d);
		else
			ep->cc.inter_usec *= 1.06 + d;

		ep->cc.last_loss_seq = nd_seq_dec(ep->s_next_seq);
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

	if (pkt->hdr.flag & ND_RUDP_HDR_FLAG_DUP) {
		ep->trace_info.retrans++;
		ep->trace_info.retrans_bytes += pkt->hdr.body_len;

		ep->trace_info.total_retrans++;
		ep->trace_info.total_retrans_bytes += pkt->hdr.body_len;
	}
}

static double 
nd_rudp_endpoint_ccc_2(nd_rudp_endpoint_t *ep, nd_uint64_t real_cycle_tm)
{
	double x = 0;
	if (ep->cc.slow_start || ep->cc.loss)
		return x;
}

static double 
nd_rudp_endpoint_ccc_1(nd_rudp_endpoint_t *ep, nd_uint64_t real_cycle_tm)
{
	double x = 0;

	if (ep->cc.slow_start) {
		return x;
	}

	double wnd = (int)(real_cycle_tm / ep->cc.inter_usec);
	double min_inc = nd_min(ep->cc.inter_usec * 0.01 * 0.000001, 0.0002);

	if (!ep->cc.loss) {
		if (ep->cc.monitor.unloss_cyc_cnt && ep->cc.monitor.unloss_cyc_cnt % 3 == 0 && ep->cc.last_dec_inter_usec > 0 && (ep->cc.inter_usec > ep->cc.last_dec_inter_usec) &&
			(ep->cc.inter_usec - ep->cc.last_dec_inter_usec) > nd_max(2, ep->cc.inter_usec * 0.01)) {
			x += -((ep->cc.inter_usec - ep->cc.last_dec_inter_usec) / 2 + (ep->cc.inter_usec - ep->cc.last_dec_inter_usec) / 64);
		}
		else {
			if (ep->cc.monitor.unloss_cyc_cnt > 1) {
				if (ep->cc.monitor.busy && ep->cc.snd_cnt > wnd * 0.9) {
					double max_iu = 1000000.0 / (double)ep->cc.bw;

					++ep->cc.monitor.inc_cnt;
					++ep->cc.monitor.inc_cnt2;
					if ((ep->cc.inter_usec - max_iu) > nd_max(16, ep->cc.inter_usec * 0.1) && max_iu / ep->cc.inter_usec < 0.8) {
						ep->cc.x3++;
						x += -((ep->cc.inter_usec - max_iu) / 16 + (ep->cc.inter_usec - max_iu) / 64);
					}
					else if (ep->cc.monitor.inc_cnt > 20) {
						double z = nd_min(ep->cc.inter_usec * 0.01 * 0.001, 0.05);
						x += ep->cc.inter_usec * -(0.05 + z);
						ep->cc.monitor.inc_cnt = 0; ++ep->cc.x1;
					}
					else
						x += ep->cc.inter_usec * -(0.00088 + min_inc); //(log(0.98) / (double)log(100.0));
				}
				else if (ep->cc.snd_cnt && ep->cc.bw > 16 && (1000000.0 / ep->cc.inter_usec < ep->cc.bw * 0.875)){
					double min_inc2 = 0;
					ep->cc.monitor.inc_with_rate++;

					if (ep->cc.monitor.inc_with_rate > 3 && ep->cc.inter_usec > 100) {
						min_inc2 = 0.0003 * (double)ep->cc.snd_cnt / wnd;
						if (min_inc2 > 0.0003) min_inc2 = 0.0003;

						x += ep->cc.inter_usec * -(0.00082 + min_inc + min_inc2);
					}
					else
						x += ep->cc.inter_usec * -0.000018;
				}
				else if (ep->cc.snd_cnt) {
					x += ep->cc.inter_usec * -0.000011;
				}
			}
		}
	}

	if (ep->cc.monitor.busy && ep->delay_level == ND_RUDP_DELAY_LV_DEFAULT) {

		if (wnd < ep->cc.cwnd)
			wnd = ep->cc.cwnd;

		if (!ep->cc.loss && ep->cc.snd_cnt < wnd * 0.75) {
			++ep->cc.x2;
			x = ep->cc.inter_usec * 0.0099;
			//ep->cc.last_dec_inter_usec = ep->cc.inter_usec;
		}

		if (ep->cc.snd_cnt < wnd * 0.9) {
			ep->cc.monitor.inc_cnt = -3;
			if (ep->cc.monitor.inc_cnt < 0)
				ep->cc.monitor.inc_cnt = 0;
		}
	}

	return x;
}

static ND_INLINE
void __cal_cycle_loss_rate(nd_rudp_endpoint_t *ep)
{
	/// cycle 统计cycle丢失率并清零
	if (ep->cc.snd_cnt) {
		ep->cc.last_loss_rate = ep->cc.last_loss_rate * 0.875 + ((double)ep->cc.loss_cnt / (double)ep->cc.snd_cnt) * 0.125;
	}
	else if (ep->cc.last_loss_rate > 0.00001){
		/// 如果一个cycle内没有发送任何分组, 则将@last_loss_rate 逐步减小. 
		/// 这里的意义在于: 如果在某个时刻loss率很高, 但是应用程序此后长时间
		/// 没有发送任何数据, 则之前统计的loss率应该不再具有参考意义, 需要重新统计, 
		/// 因为链路状况可能发生了变化
		ep->cc.last_loss_rate = ep->cc.last_loss_rate * 0.975;
	}

	if (ep->cc.last_loss_rate > 1)
		ep->cc.last_loss_rate = 1;
	else if (ep->cc.last_loss_rate < 0)
		ep->cc.last_loss_rate = 0;
}


void
nd_rudp_endpoint_on_staged_ack(nd_rudp_endpoint_t *ep, nd_int_t cnt, int flag)
{
	double x;
	nd_uint64_t curr, real_cycle_tm, last_rcv_staged_ack_tm;
	x = 0.0;
	curr = nd_get_ts();

	ep->cc.ack_cnt += cnt;
	ep->trace_info.ack += cnt;
	ep->trace_info.total_ack += cnt;

	last_rcv_staged_ack_tm = ep->last_rcv_staged_ack_tm;
	ep->last_rcv_staged_ack_tm = curr;

	/// 
	if (nd_tm_u64_diff(curr, ep->cc.begin_tm) < (nd_int64_t)ep->cc.cycle_tm) {
		if (ep->cc_mode == ND_RUDP_CC_MODE_FIXED_RATE) {
			;
		}
		else if (ep->cc_mode == ND_RUDP_CC_MODE_AUTO_RATE) {
			if (ep->cc.slow_start) {
				ep->cc.cwnd += cnt;
			}
			else if (flag){
				/*double add = 0;
				if (ep->s_buf_lst_size == 0) {
					add = max(cnt, 0.5);
				}

				ep->cc.cwnd += add;
				if (ep->cc.cwnd > ((double)ep->cc.cycle_tm) / ep->cc.inter_usec)
					ep->cc.cwnd = ((double)ep->cc.cycle_tm) / ep->cc.inter_usec;*/
			}
			nd_rudp_endpoint_do_send(ep, NULL, 0, -1);
		}
		else if (ep->cc_mode == ND_RUDP_CC_MODE_MIX_WND || ep->cc_mode == ND_RUDP_CC_MODE_FAST_WND) {
			if (ep->cc.slow_start) {
				ep->cc.cwnd += cnt;
			}
			else if (flag){
				
				if (ep->delay_level == ND_RUDP_DELAY_LV_DEFAULT) {
					double add = 0;
					if (ep->s_buf_lst_size == 0) {
						add = nd_max(cnt, 0.5);
					}

					ep->cc.cwnd += add;
					if (ep->cc.cwnd > ((double)ep->cc.cycle_tm) / ep->cc.inter_usec)
						ep->cc.cwnd = ((double)ep->cc.cycle_tm) / ep->cc.inter_usec;
				}
				else {
					double add = 0;
					add = nd_max(cnt, 0.5);

					ep->cc.cwnd += add;
					if (ep->cc.cwnd > ((double)ep->cc.cycle_tm) / ep->cc.inter_usec){
						if (ep->loss_rate < ep->loss_threshold)
							ep->cc.cwnd += 0.5;
					}
				}
			}
			nd_rudp_endpoint_do_send(ep, NULL, 0, nd_max(1, cnt));
		}
		return;
	}

	real_cycle_tm = ep->cc.cycle_tm;//nd_tm_u64_diff(curr, ep->cc.begin_tm);

	if (ep->cc.monitor.r_repeat_cnt > 4){
		ep->cc.monitor.r_repeat_cnt = 0;
		nd_rudp_send_ctrl(ep, 0, ND_RUDP_HDR_FLAG_REPEAT_WARN, 0);
	}

	__cal_cycle_loss_rate(ep);

	if (nd_tm_u64_diff(curr, ep->trace_info.last_tm) > 1000*1000) {
		nd_rudp_dump_trace_info(ep);
		__reset_ep_trace_info(&ep->trace_info);
	}

	curr = nd_get_ts();
	ep->cc.begin_tm = curr;

	if (ep->cc_mode == ND_RUDP_CC_MODE_FIXED_RATE) {
		ep->cc.inter_usec = ep->cc.fixed_inter_usec;
		ep->cc.cwnd = ep->cc.cycle_tm / ep->cc.inter_usec;
	}
	else {
		if (ep->cc.slow_start) {
			if (ep->cc.inter_usec < 20) {
				ep->cc.slow_start = 0;
			}
			else if (ep->cc.ack_cnt && ep->cc.snd_cnt && ep->cc.loss_cnt == 0 && ++ep->cc.cycle_cnt > 10){
				ep->cc.cycle_cnt = 0;
				ep->cc.inter_usec *= 0.88;
			}
		}
		else if (ep->cc_mode == ND_RUDP_CC_MODE_AUTO_RATE) {
			ep->cc.cwnd = (double)(ep->cc.rate) / 1000000.0 * (ep->cc.cycle_tm + ep->rtt / 2) + 16;

			if (!ep->cc.loss && ep->cc.snd_cnt && ep->cc.ack_cnt) {
				const double min_inc = 0.04;
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
			}
		}
		else {

			x = nd_rudp_endpoint_ccc_1(ep, real_cycle_tm);

			if (ep->cc.inter_usec < ep->cc.inter_usec_min)
				ep->cc.inter_usec = ep->cc.inter_usec_min;
			if (ep->cc.inter_usec > ep->cc.inter_usec_max)
				ep->cc.inter_usec = ep->cc.inter_usec_max;


			ep->cc.cwnd = ceil(ep->cc.cycle_tm / ep->cc.inter_usec);

			if (ep->s_buf_lst_size > 16) {
				double w = nd_min(ep->max_syn_seq_range, ep->cc.cwnd);
				ep->cc.cwnd *= (w + nd_min(ep->max_syn_seq_range - ep->s_buf_lst_size, nd_rudp_endpoint_get_snd_wnd(ep))) / (w + ep->max_syn_seq_range);
			}

			if (ep->cc.cwnd < 1.0)
				ep->cc.cwnd = 1.0;
		}
	}

	if (ep->cc.slow_start) {
		goto do_end;
	}

	if (ep->cc.loss) {
		ep->cc.monitor.unloss_cyc_cnt = 0;
		ep->cc.monitor.inc_with_rate = 0;

		ep->cc.monitor.inc_cnt -= 3;
		if (ep->cc.monitor.inc_cnt < 0)
			ep->cc.monitor.inc_cnt = 0;

		ep->cc.monitor.inc_cnt2 = 0;

		ep->cc.loss = 0;
		ep->cc.last_loss_tm = curr;
		goto do_end;
	}
	else {
		ep->cc.monitor.unloss_cyc_cnt++;
	}

do_end:
	
/*ep->cc.x1=ep->cc.x2=ep->cc.x3=0;*/ ep->ch->rudp->evld->cnt = 0;
	ep->cc.loss = 0;
	ep->cc.update_bw = 1;
	ep->cc.loss_cnt = 0;
	ep->cc.snd_cnt = 0;
	ep->cc.ack_cnt = 0;

	ep->cc.left_wnd = 0;

	ep->cc.part.wnd = 0;
	ep->cc.part.tm = curr;

	ep->cc.last_expect_snd_tm = curr;
	ep->cc.monitor.no_data = 0;
	ep->cc.monitor.last_nodata_tm = curr;
	ep->cc.monitor.last_snd_tm = curr;
	ep->cc.monitor.busy = 1;

	ep->cc.monitor.r_pkt_cnt = 0;
	ep->cc.monitor.r_repeat_cnt = 0;

	if (ep->cc.monitor.s_repeat_keep > 0)
		ep->cc.monitor.s_repeat_keep--;

	if (ep->cc_mode == ND_RUDP_CC_MODE_FIXED_RATE) {
		ep->cc.inter_usec = ep->cc.fixed_inter_usec;
		ep->cc.cwnd = ep->cc.cycle_tm / ep->cc.inter_usec;
	}
	else {
		ep->cc.inter_usec += x;

		if (ep->cc.inter_usec < ep->cc.inter_usec_min)
			ep->cc.inter_usec = ep->cc.inter_usec_min;
		if (ep->cc.inter_usec > ep->cc.inter_usec_max)
			ep->cc.inter_usec = ep->cc.inter_usec_max;

		if (!ep->cc.slow_start) {
			ep->cc.cwnd += ep->cc.cwnd_mantissa;
			ep->cc.cwnd_mantissa = ep->cc.cwnd - (int)ep->cc.cwnd;
		}
	}
	/// cycle 重新开始时, 更新发送定时器时间为当前时间
	nd_rudp_endpoint_update_snd_timer(ep, curr + nd_min(ep->cc.inter_usec, 100));
}

void 
nd_rudp_endpoint_on_rcv_packet(nd_rudp_endpoint_t *ep, nd_rudp_packet_t *pkt, nd_uint64_t curr_tm)
{
	nd_rudp_rcv_pkt_tm_wnd_t *ptw;
	nd_rudp_ppp_t *p;
	nd_uint64_t curr;
	p = &ep->r_ppp;

	curr = curr_tm;

	ep->trace_info.rcv++;
	ep->trace_info.rcv_bytes += pkt->hdr.body_len;
	ep->trace_info.total_rcv++;
	ep->trace_info.total_rcv_bytes += pkt->hdr.body_len;

	ep->cc.monitor.r_pkt_cnt++;

	if (pkt->hdr.flag & ND_RUDP_HDR_FLAG_PPP) {
		p->ts0 = curr;
		p->seq0 = pkt->hdr.seq;
		p->s_ts0 = pkt->hdr.ts;
	}
	else if (p->ts0 && (nd_seq_cmp(pkt->hdr.seq, p->seq0) > 0)) {
			
			nd_int64_t r_x = curr - p->ts0 - p->ts_dev;
			nd_int64_t s_x = pkt->hdr.ts - p->s_ts0;

			if (r_x < 10)
				r_x = 10;

			if (r_x < s_x)
				r_x = s_x;

			p->lst[p->pos] = r_x;

			if (p->lst[p->pos] > 1000*1000)
				p->lst[p->pos] = 1000*1000;

			p->ts0 = 0;

			p->pos++;

			if (p->pos == ND_RUDP_PPP_SIZE)
				p->pos = 0;

			if (p->size < ND_RUDP_PPP_SIZE)
				p->size++;
	}

	ptw = &ep->r_pkt_tm_wnd;
	if (ptw->ts0 == 0) {
		ptw->ts0 = curr;
		ptw->seq0 = pkt->hdr.seq;
		ptw->s_ts0 = pkt->hdr.ts;
	}
	else if (ptw->ts0 && (nd_seq_cmp(pkt->hdr.seq, ptw->seq0) > 0)) {

		nd_int64_t r_x = curr - ptw->ts0;// - ptw->ts_dev;
		nd_int64_t s_x = pkt->hdr.ts - ptw->s_ts0;

		if (r_x < 10)
			r_x = 10;

		if (0) {
			//ptw->ts0 = 0;
		}
		else {
			ptw->lst[ptw->pos] = r_x;

			if (ptw->lst[ptw->pos] > 1000*1000)
				ptw->lst[ptw->pos] = 1000*1000;

			ptw->ts0 = curr;

			ptw->pos++;

			if (ptw->pos == ND_RUDP_PPP_SIZE)
				ptw->pos = 0;

			if (ptw->size < ND_RUDP_PPP_SIZE)
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
			/*if (*pk > upper)
				*pk = upper;
			if (*pk < lower)
				*pk = lower;*/

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
			/*if (*pk > upper)
				*pk = upper;
			if (*pk < lower)
				*pk = lower;*/

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

void
nd_rudp_dump_trace_info(nd_rudp_endpoint_t *ep)
{
	nd_uint64_t curr;
	nd_rudp_ep_trace_info_t *ti;
	
	ti = &ep->trace_info;
	curr = nd_get_ts();

	if (ti->snd)
		ep->loss_rate = ep->loss_rate * 0.125 + ((double)ti->loss / (double)ti->snd) * 0.875;
	else
		ep->loss_rate = ep->loss_rate * 0.125;

	if (ep->loss_rate < 0.0001)
		ep->loss_rate = 0;
		

	ti->snd_rate = ((double)(ti->snd_bytes) / (double)(curr - ti->last_tm)) * 1000000.0;
	ti->rcv_rate = ((double)(ti->rcv_bytes) / (double)(curr - ti->last_tm)) * 1000000.0;

	if (ep->pmtu && ep->mtu_monitor.idx < (sizeof(s_mtu_rate_lst) / sizeof(s_mtu_rate_lst[0]) - 1)) {
		if (ti->snd_rate > s_mtu_rate_lst[ep->mtu_monitor.idx].rate) {
			if (++ep->mtu_monitor.cnt > 2) {
				ep->mtu_monitor.cnt = 0;
				ep->mtu_monitor.idx++;
				ep->mtu = s_mtu_rate_lst[ep->mtu_monitor.idx].mtu;
				ep->mss = ep->mtu - ND_RUDP_HDR_LEN;
			}
		}
		else {
			ep->mtu_monitor.cnt = 0;
		}
	}

	if (!nd_rudp_log_enable(ND_RUDP_LOG_LV_DUMP)) {
		return;
	}

	nd_rudp_log(ND_RUDP_LOG_LV_DUMP, 
		"dump trace: "
		"snd_rate=%llu(Bytes/S), rcv_rate=%llu(Bytes/S), "
		"total_snd=%llu, total_rcv=%llu, total_ack_pkt_cnt=%llu, "
		"total_loss=%llu, total_retrans=%llu, total_fast_retrans=%llu, "
		"total_out_of_range=%llu, total_repeat_warn=%llu, "
		"total_ack=%llu, "
		"loss=%llu, snd=%llu, "
		"rcv=%llu, retrans=%llu, fast_retrans=%llu, "
		"out_of_range=%llu, repeat_warn=%llu, "
		"ack=%llu",
		ti->snd_rate, ti->rcv_rate,
		ti->total_snd, ti->total_rcv, ti->total_ack_pkt_cnt,
		ti->total_loss, ti->total_retrans, ti->total_fast_retrans,
		ti->total_out_of_range, ti->total_repeat_warn,
		ti->total_ack,
		ti->loss, ti->snd,
		ti->rcv, ti->retrans, ti->fast_retrans,
		ti->out_of_range, ti->repeat_warn,
		ti->ack);

	nd_rudp_log(ND_RUDP_LOG_LV_DUMP, 
		"dump trace: slow_start=%d, "
		"rtt=%u, rto=%u, mtu=%u, cycle_tm=%llu, cwnd=%.2f, cc.left_wnd=%.2f, inter_usec=%.2f, cc.rate=%u, cc.bw=%u, cc.last_loss_rate=%.4f, cc.snd_cnt=%llu, +x1=%d, -x2=%d, x3=%d, evld->cnt=%llu, "
		"s_next_seq=%u, s_next_ack_seq=%u, s_max_seq=%u, "
		"r_next_seq=%u, r_last_seq=%u, r_last_max_seq=%u, r_next_staged_ack_seq=%u, "
		"s_que.size=%u, r_que.size=%u, s_buf_lst_size=%u, r_buf_lst_size=%d, last_dec_inter_usec=%.2f, max_bw=%u, s_max_vir_seq=%u, s_que.free_size=%d, "
		"\r\n",
		ep->cc.slow_start ? 1 : 0, ep->rtt, ep->rto, ep->mtu, ep->cc.cycle_tm, ep->cc.cwnd, ep->cc.left_wnd, ep->cc.inter_usec, ep->cc.rate, ep->cc.bw, ep->cc.last_loss_rate, ep->cc.snd_cnt, ep->cc.x1, ep->cc.x2, ep->cc.x3, ep->ch->rudp->evld->cnt, 
		ep->s_next_seq, ep->s_next_ack_seq, ep->s_max_seq,
		ep->r_next_seq, ep->r_last_seq, ep->r_last_max_seq, ep->r_next_staged_ack_seq,
		ep->s_que.size, ep->r_que.size, ep->s_buf_lst_size, ep->r_buf_lst_size, ep->cc.last_dec_inter_usec, ep->trace_info.max_bw, (nd_int32_t)ep->s_max_vir_seq, ep->s_que.free_size);

}
