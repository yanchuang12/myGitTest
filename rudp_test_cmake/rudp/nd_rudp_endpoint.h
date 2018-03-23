/** Copyright (c) 2016-2017
 * All rights reserved.
 * 
 * 文件名称:	nd_rudp_endpoint.h   
 * 摘	 要:	
 * 
 * 当前版本：	1.0
 * 作	 者:	范涛涛(FTT)
 * 操	 作:	新建
 * 完成日期:	2016年08月02日
 */
#ifndef __ND_RUDP_ENDPOINT_H_20160802__
#define __ND_RUDP_ENDPOINT_H_20160802__

#include "nd_rudp.h"

#ifdef __cplusplus
extern "C" {
#endif

/// status
#define ND_RUDP_EP_STATUS_ZERO			0
#define ND_RUDP_EP_STATUS_INIT			1
#define ND_RUDP_EP_STATUS_SHAKEHAND		2
#define ND_RUDP_EP_STATUS_LISTEN		3
#define ND_RUDP_EP_STATUS_CONNECTED		4
#define ND_RUDP_EP_STATUS_LING_WAIT		5
#define ND_RUDP_EP_STATUS_CLOSE_WAIT	6
#define ND_RUDP_EP_STATUS_CLOSE_END		7

/// type
#define ND_RUDP_EP_TYPE_DUMMY			0
#define ND_RUDP_EP_TYPE_LISTEN			1
#define ND_RUDP_EP_TYPE_ACCEPT			2
#define ND_RUDP_EP_TYPE_CONNECT			3

/// cc mode
#define ND_RUDP_CC_MODE_MIX_WND			0		/// default
#define ND_RUDP_CC_MODE_AUTO_RATE		1
#define ND_RUDP_CC_MODE_FIXED_RATE		2
#define ND_RUDP_CC_MODE_FAST_WND		3

/// data trans mode
#define ND_RUDP_TRANS_MODE_UNINTERRUP	0

/// ppp
#define ND_RUDP_PPP_SIZE				8

#define ND_TUDP_MAX_INTER_USEC			10000
#define ND_TUDP_MIN_INTER_USEC			8

typedef struct
{
	nd_uint64_t			s_ts0;			/// 发送端包对的第一个分组的时间戳
	nd_uint64_t			ts0;			
	nd_uint32_t			seq0;
	nd_int64_t			lst[ND_RUDP_PPP_SIZE];
	nd_int_t			pos;
	nd_int_t			size;
	nd_uint64_t			ts_dev;			/// 偏差
}nd_rudp_rcv_pkt_tm_wnd_t;

typedef struct 
{
	nd_uint64_t			s_ts0;			/// 发送端包对的第一个分组的时间戳
	nd_uint64_t			ts0;
	nd_uint32_t			seq0;
	nd_int64_t			lst[ND_RUDP_PPP_SIZE];
	nd_int_t			pos;
	nd_int_t			size;
	nd_uint64_t			ts_dev;			/// 偏差
}nd_rudp_ppp_t;

typedef struct 
{
	nd_uint64_t			tm;
	double				wnd;
}nd_rudp_cc_part_t;

typedef struct 
{
	double				loss[ND_RUDP_PPP_SIZE];
	nd_int_t			pos;
	nd_int_t			size;
}nd_rudp_loss_t;

typedef struct 
{
	struct {
		nd_uint8_t	no_data;
		nd_uint64_t	last_snd_tm;
		nd_uint64_t	last_nodata_tm;
		nd_int_t	busy;
		nd_int_t	dec_cnt;
		nd_int_t	inc_cnt;
		nd_int_t	inc_cnt2;
		nd_int_t	unloss_cyc_cnt;

		nd_uint_t	r_pkt_cnt;
		nd_uint_t	r_repeat_cnt;

		nd_int_t	s_repeat_keep;
		nd_uint_t	inc_with_rate;
	}monitor;

	nd_uint64_t			cycle_tm;
	nd_uint64_t			begin_tm;
	nd_int_t			cycle_cnt;
	double				last_dec_inter_usec;
	double				inter_usec, burst_inter_usec, inter_usec_min, inter_usec_max;
	double				fixed_inter_usec;
	nd_uint32_t			rate;
	nd_uint32_t			bw;
	nd_int_t			ack_cnt;
	double				cwnd;
	double				left_wnd;
	double				cwnd_mantissa;

	nd_rudp_cc_part_t	part;

	nd_uint64_t			last_expect_snd_tm;
	nd_uint32_t			last_loss_seq;
	nd_uint64_t			last_loss_tm;
	nd_int_t			x_loss_avg;
	nd_int_t			x_loss_cnt;
	nd_int_t			x_loss_dec_cnt;
	nd_int_t			x_loss_dec_random;
	nd_int_t			x1, x2, x3;

	/// cycle内丢失率统计
	nd_uint64_t			loss_cnt;
	nd_uint64_t			snd_cnt;
	nd_uint64_t			last_snd_cnt;
	double				last_loss_rate;
	nd_rudp_loss_t		loss_rate;

	unsigned			slow_start:1;
	unsigned			loss:1;
	unsigned			update_bw:1;
}nd_rudp_cc_t;

typedef struct
{
	nd_uint64_t			total_snd;
	nd_uint64_t			total_snd_bytes;
	nd_uint64_t			total_rcv;
	nd_uint64_t			total_rcv_bytes;
	nd_uint64_t			total_loss;
	nd_uint64_t			total_loss_bytes;
	nd_uint64_t			total_retrans;
	nd_uint64_t			total_retrans_bytes;
	nd_uint64_t			total_fast_retrans;
	nd_uint64_t			total_ack;
	nd_uint64_t			total_ack_bytes;
	nd_uint64_t			total_ack_pkt_cnt;
	nd_uint64_t			total_out_of_range;
	nd_uint64_t			total_repeat_warn;

	nd_uint64_t			last_tm;
	nd_uint64_t			loss;
	nd_uint64_t			loss_bytes;
	nd_uint64_t			snd;
	nd_uint64_t			snd_bytes;
	nd_uint64_t			rcv;
	nd_uint64_t			rcv_bytes;
	nd_uint64_t			retrans;
	nd_uint64_t			retrans_bytes;
	nd_uint64_t			fast_retrans;
	nd_uint64_t			ack;
	nd_uint64_t			out_of_range;
	nd_uint64_t			repeat_warn;
	
	nd_uint64_t			snd_rate;		/// bytes/sec
	nd_uint64_t			rcv_rate;
	nd_uint32_t			max_bw;
}nd_rudp_ep_trace_info_t;

static ND_INLINE void __reset_ep_trace_info(nd_rudp_ep_trace_info_t *p)
{
	p->last_tm = nd_get_ts();
	p->loss = p->loss_bytes = p->snd = p->snd_bytes = p->rcv = p->rcv_bytes = 0;
	p->retrans = p->fast_retrans = p->retrans_bytes = p->ack = p->out_of_range = p->repeat_warn = 0;
	//p->snd_rate = 0; p->rcv_rate = 0;
}

struct nd_rudp_endpoint_s
{
	__ND_QUEUE_DECALRE__(nd_rudp_endpoint_t);
	nd_rudp_channel_t	*ch;
	struct sockaddr		so_remote_addr;
	struct sockaddr		so_local_addr;
	nd_dlist_node_t		node_in_ch;
	nd_dlist_node_t		node_in_accept;
	
	void				*user_data;
	nd_uint32_t			token;						/// @token 标识一个可靠的虚拟连接
	nd_uint64_t			dead_tm;
	nd_uint32_t			dead_wait_tmo;
	nd_uint64_t			close_wait_tm;
	nd_uint32_t			close_wait_tmo;
	
	nd_uint64_t			last_snd_staged_ack_tm;		/// 上次发送staged ack的时间
	nd_uint64_t			last_rcv_staged_ack_tm;
	nd_uint64_t			last_rcv_tm;
	nd_uint64_t			last_snd_bye_tm;
	nd_uint64_t			last_snd_shk_tm;
	nd_uint64_t			last_snd_data_tm;
	nd_uint64_t			first_snd_shk_tm;
	nd_uint64_t			last_ack_bw_rate_tm;
	nd_uint64_t			last_probe_tm;
	nd_uint64_t			probe_ack_tmo;
	nd_uint64_t			ling_tmo;
	nd_uint64_t			ling_wait_tm;
	nd_uint64_t			heartbeat_tmo;				/// 心跳超时, 超出该时间为收到任何数据, 则关闭连接
	
	nd_int_t			exception_cnt;				/// 异常次数, 主要指 probe_ack 连续超时次数

	nd_tq_node_t		idle_tm_node;

	/// common
	nd_int32_t			max_staged_pkt_cnt;			/// 当收到多少个分组时触发一次staged ack
	nd_uint32_t			isyn_seq;
	nd_uint32_t			peer_isyn_seq;
	nd_uint32_t			mtu;
	nd_int32_t			mss;
	nd_int32_t			pre_snd_header_len;
	nd_uint32_t			rtt;
	nd_uint32_t			rtt_val;
	nd_uint32_t			rto;
	nd_uint32_t			rto_min;
	nd_uint32_t			rto_max;
	nd_int_t			max_syn_seq_range;
	nd_uint32_t			max_staged_ack_cnt1;
	nd_uint32_t			max_staged_ack_cnt2;
	double				loss_threshold;
	double				loss_rate;					/// 每秒丢失率

	/// send
	nd_tq_node_t		s_next_tm_node;				/// 发送定时器节点			
	nd_rudp_sque_t		s_que;
	nd_rudp_packet_t	**s_buf_lst;
	nd_int_t			s_buf_lst_size;
	nd_rudp_packet_t	s_ctrl_lst;
	nd_rudp_packet_t	*s_curr_pkt;
	nd_rbtree_t			s_loss_lst;
	nd_rbtree_node_t    s_loss_lst_sentinel;
	nd_uint32_t			s_next_seq;					/// 下一个待发送序号
	nd_uint16_t			s_next_vir_seq;				/// 下一个待发送的虚拟序号
	nd_uint32_t			s_max_seq;					/// 允许发送的最大序号 + 1
	nd_uint32_t			s_next_ack_seq;				/// 下一个累积确认的序号
	nd_uint32_t			s_max_ack_seq;				/// 当前收到的最大确认的序号
	nd_uint16_t			s_max_vir_seq;				/// 已收到的最大虚拟序号

	/// recv
	nd_int_t			r_staged_pkt_cnt;			/// 上次staged ack到现在,收到的数据包数量
	nd_uint32_t			r_next_staged_ack_seq;		/// 下一个staged ack序号
	
	nd_rudp_rque_t		r_que;
	nd_rudp_packet_t	**r_buf_lst;
	nd_int_t			r_buf_lst_size;

	nd_uint32_t			r_next_seq;					/// 下一个期望序号
	nd_uint32_t			r_last_seq;					/// 最近一次收到的序号
	nd_uint32_t			r_last_max_seq;				/// 当前收到的最大序号
	nd_uint16_t			r_last_max_vir_seq;			/// 已收到的最大虚拟序号

	nd_rudp_ppp_t		r_ppp;
	nd_rudp_rcv_pkt_tm_wnd_t	r_pkt_tm_wnd;
	nd_rudp_cc_t		cc;
	nd_rudp_ep_trace_info_t		trace_info;

	struct {
		nd_uint8_t	idx;
		nd_uint32_t cnt;
	}mtu_monitor;

	struct
	{
		nd_int_t		tmo_cnt_limit;
		nd_int_t		rack_cnt_limit;
	}losschk_threshold;
	
	/// 
	volatile nd_int_t	status;
	volatile nd_int_t	dead;
	volatile nd_int_t	post_destroy;
	volatile nd_int_t	in_ch;
	volatile nd_int_t	in_accept;
	volatile nd_int_t	type;
	volatile nd_int_t	delay_level;
	unsigned			s_zero_wnd:1;
	unsigned			r_zero_wnd:1;
	unsigned			s_loss_flag:1;
	unsigned			s_ppp_flag:1;
	unsigned			wait_snd_bye:1;
	unsigned			trans_mode:2;
	unsigned			cc_mode:3;					/// 拥塞控制模式
	unsigned			pmtu:1;
	unsigned			net_block;
};

static ND_INLINE void
nd_rudp_endpoint_init_cc(nd_rudp_endpoint_t *ep)
{
	nd_rudp_cc_t *cc;
	nd_uint64_t curr;
	nd_int_t i;
	cc = &ep->cc;
	curr = nd_get_ts();

	cc->begin_tm = curr;
	cc->last_expect_snd_tm = curr;

	cc->last_loss_tm = curr;

	cc->part.tm = curr;
	cc->part.wnd = 0;
	
	cc->cycle_tm = 10*1000;
	cc->bw = 16;
	cc->rate = 16;
	cc->burst_inter_usec = ND_TUDP_MIN_INTER_USEC;
	cc->inter_usec = 2000;
	cc->inter_usec_min = ND_TUDP_MIN_INTER_USEC;
	cc->inter_usec_max = ND_TUDP_MAX_INTER_USEC;
	cc->last_dec_inter_usec = 2000;
	cc->cwnd = 4;
	cc->left_wnd = 0;
	cc->slow_start = 1;
	cc->last_loss_seq = nd_seq_dec(ep->s_next_ack_seq);

	cc->update_bw = 1;

	cc->last_loss_rate = 0.0;
	for (i = 0; i < ND_RUDP_PPP_SIZE; i++) {
		cc->loss_rate.loss[i] = 0;
	}

	ep->cc.x_loss_dec_random = 1;

	cc->monitor.last_snd_tm = curr;
}

nd_rudp_endpoint_t*
nd_rudp_endpoint_create();

nd_err_t
nd_rudp_endpoint_init(nd_rudp_endpoint_t *e);

static ND_INLINE void
nd_rudp_endpoint_init_with_peer(nd_rudp_endpoint_t *ep, nd_uint32_t peer_isyn_seq)
{
	ep->peer_isyn_seq = peer_isyn_seq;
	ep->r_next_seq = peer_isyn_seq;
	ep->r_next_staged_ack_seq = peer_isyn_seq;
	ep->r_last_max_seq = ep->r_last_seq = nd_seq_dec(peer_isyn_seq);
	ep->last_snd_staged_ack_tm = nd_get_ts();
}

void 
nd_rudp_endpoint_destroy(nd_rudp_endpoint_t *ep);

void 
nd_rudp_endpoint_post_destroy(nd_rudp_endpoint_t *ep);

static ND_INLINE nd_int_t nd_rudp_endpoint_get_snd_wnd(nd_rudp_endpoint_t *ep)
{
	nd_int_t n = nd_seq_diff(ep->s_max_seq, ep->s_next_seq);
	return n > 0 ? n : 0;
}

static ND_INLINE nd_bool_t nd_rudp_chk_dateseq(nd_rudp_endpoint_t *ep, nd_uint32_t seq)
{
	nd_int_t size, n;
	size = nd_rudp_rque_size(&ep->r_que);
	n = ep->max_syn_seq_range - size;
	return n < 0 ? 0 : nd_seq_in_range(seq, ep->r_next_seq, n);
}

static ND_INLINE nd_int_t nd_rudp_endpoint_get_rcv_wnd(nd_rudp_endpoint_t *ep)
{
	nd_int_t size, n;

	size = nd_rudp_rque_size(&ep->r_que);
	n = ep->max_syn_seq_range - size;
	n -= nd_seq_diff(ep->r_last_max_seq, ep->r_next_seq) + 1;
	return n > 0 ? n : 0;
}

void 
nd_rudp_endpoint_close(nd_rudp_endpoint_t *ep);

void
nd_rudp_endpoint_do_send(nd_rudp_endpoint_t *ep, nd_tq_node_t *tq_node, nd_int_t cycle_flush, nd_int_t max_cnt);

void
nd_rudp_endpoint_update_snd_timer(nd_rudp_endpoint_t *ep, nd_uint64_t expect_tm);

nd_uint32_t nd_rudp_endpoint_cal_rate_1(nd_rudp_endpoint_t *ep);
nd_uint32_t nd_rudp_endpoint_cal_bw_1(nd_rudp_endpoint_t *ep);

void
nd_rudp_dump_trace_info(nd_rudp_endpoint_t *ep);

void 
nd_rudp_endpoint_on_dead(nd_rudp_endpoint_t *ep);

void 
nd_rudp_endpoint_on_rcv_packet(nd_rudp_endpoint_t *ep, nd_rudp_packet_t *pkt, nd_uint64_t curr_tm);

void
nd_rudp_endpoint_on_staged_ack(nd_rudp_endpoint_t *ep, nd_int_t cnt, int flag);

void
nd_rudp_endpoint_on_snd_packet(nd_rudp_endpoint_t *ep, nd_rudp_packet_t *pkt);

void
nd_rudp_on_loss(nd_rudp_endpoint_t *ep, nd_rudp_packet_t *pkt);

void 
nd_rudp_endpoint_add_loss_pkt(nd_rudp_endpoint_t *ep, nd_rudp_packet_t *pkt);

void
nd_rudp_endpoint_chk_loss(nd_rudp_endpoint_t *ep, nd_rudp_packet_t *pkt, nd_int_t flag, nd_uint64_t curr_tm);

static ND_INLINE void 
nd_rudp_endpoint_update_rtt(nd_rudp_endpoint_t *ep, nd_uint64_t curr, nd_uint32_t last)
{
	nd_int32_t t = (nd_uint32_t)curr - last;
	if (t < 0 || t > 10*1000*1000)
		return;

	if (t < 1)
		t = 1;

	ep->rtt_val = (ep->rtt_val * 3 + abs(t - ep->rtt)) >> 2;

	ep->rtt = (ep->rtt * 7 + t) >> 3;
	if (ep->rtt < 1)
		ep->rtt = 1;

	ep->rto = ep->rtt + (ep->rtt_val << 2);
	//ep->rto = ep->rtt << 2;
	//ep->rto = ep->rto * 0.875 + (ep->rtt + (ep->rtt_val << 2)) * 0.125;
}

static ND_INLINE void 
nd_rudp_endpoint_update_rate(nd_rudp_endpoint_t *ep, nd_uint32_t rate)
{
	ep->cc.rate = (ep->cc.rate * 7 + rate) >> 3;
}

static ND_INLINE void 
nd_rudp_endpoint_update_bw(nd_rudp_endpoint_t *ep, nd_uint32_t bw)
{
	/*if (ep->cc.update_bw)*/ {
		ep->cc.update_bw = 0;
		ep->cc.bw = (ep->cc.bw * 7 + bw) >> 3;
		if (ep->cc.bw > ep->trace_info.max_bw)
			ep->trace_info.max_bw = ep->cc.bw;
	}
}

static ND_INLINE void
nd_rudp_endpoint_set_pkt_rto(nd_rudp_endpoint_t *ep, nd_rudp_packet_t *pkt, nd_uint64_t curr)
{
	nd_uint64_t rto;
	if (ep->delay_level == ND_RUDP_DELAY_LV_DEFAULT) {
		rto = ep->rto + ep->cc.inter_usec;
		if (rto < ep->rto_min)
			rto = ep->rto_min;
		if (rto > ep->rto_max)
			rto = ep->rto_max;

		pkt->loss_tmo = curr + rto + (ep->cc.monitor.s_repeat_keep ? (rto >> 1) : 0);
	}
	else {
		rto = ep->rto;

		if (ep->loss_rate < ep->loss_threshold) {
			if (rto > ep->rtt + ep->cc.cycle_tm)
				rto = ep->rtt + ep->cc.cycle_tm;
		}
		else {
			if (pkt->retrans_cnt > 1) {
				rto += ep->rto * (pkt->retrans_cnt - 1);
			}
		}

		if (ep->net_block)
			rto = (rto << 1);
		
		if (rto < ep->rto_min)
			rto = ep->rto_min;
		if (rto > nd_max(5000000, ep->rto_max))
			rto = nd_max(5000000, ep->rto_max);

		pkt->loss_tmo = curr + rto;
	}
}

static ND_INLINE
nd_rudp_endpoint_chk_inter_usec(nd_rudp_endpoint_t *ep)
{
	;
}

#ifdef __cplusplus
}
#endif

#endif /// __ND_RUDP_ENDPOINT_H_20160802__
