/** Copyright (c) 2016-2017
 * All rights reserved.
 * 
 * 文件名称:	nd_rudp_packet.h   
 * 摘	 要:	
 * 
 * 当前版本：	1.0
 * 作	 者:	范涛涛(FTT)
 * 操	 作:	新建
 * 完成日期:	2016年08月02日
 */
#ifndef __ND_RUDP_PACKET_H_20160802__
#define __ND_RUDP_PACKET_H_20160802__

#include "nd_core/nd_core.h"


#ifdef __cplusplus
extern "C" {
#endif

#define ND_RUDP_HDR_FLAG_ACK			0x0001
#define ND_RUDP_HDR_FLAG_SHAKEHAND		0x0002
#define ND_RUDP_HDR_FLAG_PROBE			0x0004		/// 如果data携带probe标志,则必须同时携带push标志
#define ND_RUDP_HDR_FLAG_STAGED_ACK		0x0008
#define ND_RUDP_HDR_FLAG_DATA			0x0010
#define ND_RUDP_HDR_FLAG_BYE			0x0020
#define ND_RUDP_HDR_FLAG_PUSH			0x0040
#define ND_RUDP_HDR_FLAG_PPP			0x0080
#define ND_RUDP_HDR_FLAG_REPEAT_WARN	0x0100
#define ND_RUDP_HDR_FLAG_BLOCK			0x0200
#define ND_RUDP_HDR_FLAG_DUP			0x0400
#define ND_RUDP_HDR_FLAG_BW_RATE		0x0800

#define ND_RUDP_HDR_LEN					20


static ND_INLINE nd_uint32_t nd_seq_inc(nd_uint32_t seq){
	return ++seq;
}
static ND_INLINE nd_uint32_t nd_seq_dec(nd_uint32_t seq){
	return --seq;
}
static ND_INLINE nd_int32_t nd_seq_cmp(nd_uint32_t seq1, nd_uint32_t seq2){
	return (nd_int32_t)(seq1 - seq2);
}
static ND_INLINE nd_int32_t nd_seq_diff(nd_uint32_t seq1, nd_uint32_t seq2){
	return (nd_int32_t)(seq1 - seq2);
}
static ND_INLINE nd_uint32_t nd_seq_add(nd_uint32_t seq, nd_uint32_t n){
	return seq + n;
}

/// [begin, begin + range)
static ND_INLINE nd_bool_t nd_seq_in_range(nd_uint32_t seq, nd_uint32_t begin, nd_int32_t range)
{
	if ((nd_int32_t)(seq - begin) < 0)
		return 0;
	return (nd_int32_t)(seq - begin) < range ? 1 : 0;
}

/// [begin, end)
static ND_INLINE nd_bool_t nd_seq_between(nd_uint32_t seq, nd_uint32_t begin, nd_int32_t end)
{
	if (nd_seq_cmp(seq, begin) < 0 ||
		nd_seq_cmp(seq, end) >= 0)
		return 0;
	else
		return 1;
}


/**   通讯协议数据包头定义:
*  
*    00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
*   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*   |        unused         |        exhd_len       |                     token                     |
*   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*   |                                              seq                                              |
*   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*   |                                           timestamp                                           |
*   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*   |                      wnd                      |                      flag                     |
*   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*   |                  vir_seq                      |                   unused2                     |
*   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*   |                                                                                               |
*   ~                                             ex_hdr                                            ~
*   |                                                                                               |
*   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*/

typedef struct nd_rudp_packet_s			nd_rudp_packet_t;

typedef struct 
{
	nd_uint8_t			unused;
	nd_uint16_t			exhdr_len;
	nd_uint16_t			token;
	/// @seq: 如果是data, 则表示该数据包的序号
	///       如果是ctrl, 则表示累积确认序号
	nd_uint32_t			seq;
	nd_uint32_t			ts;
	nd_uint16_t			wnd;
	nd_uint16_t			flag;
	nd_uint16_t			vir_seq;
	nd_uint16_t			unused2;
	/// 
	char				*exhdr;
	char				*body;
	nd_uint16_t			body_len;
}nd_rudp_header_t;


#define ND_RUDP_BLOCK_CDR_LEN			12

typedef struct 
{
	nd_uint8_t			flag;
	nd_uint8_t			unused;
	nd_uint16_t			org_cnt;	/// 该block包含的原生分组数(包括携带info的分组本身)
	nd_uint16_t			fec_cnt;	/// 该block包含的fec冗余分组数量
	nd_uint16_t			delay;		/// 最大延迟(仅针对不可靠传输)
	nd_uint16_t			idx;		/// 该分组在block块内的idx, 从0开始计数
	nd_uint16_t			unused2;
}nd_rudp_block_info_t;


static ND_INLINE nd_uint32_t nd_rudp_block_cdr_read(char *buf, nd_uint32_t len, nd_rudp_block_info_t *blk)
{
	if (len < ND_RUDP_BLOCK_CDR_LEN) return 0;
	blk->flag = buf[0];
	blk->unused = buf[1];
	blk->org_cnt = ntohs(((nd_uint16_t*)buf)[1]);
	blk->fec_cnt = ntohs(((nd_uint16_t*)buf)[2]);
	blk->delay = ntohs(((nd_uint16_t*)buf)[3]);
	blk->idx = ntohs(((nd_uint16_t*)buf)[4]);
	blk->unused2 = ntohs(((nd_uint16_t*)buf)[5]);
	return ND_RUDP_BLOCK_CDR_LEN;
}

static ND_INLINE nd_uint32_t nd_rudp_block_cdr_write(char *buf, nd_uint32_t len, nd_rudp_block_info_t *blk)
{
	if (len < ND_RUDP_BLOCK_CDR_LEN) return 0;
	buf[0] = blk->flag;
	buf[1] = blk->unused;
	((nd_uint16_t*)buf)[1] = htons(blk->org_cnt);
	((nd_uint16_t*)buf)[2] = htons(blk->fec_cnt);
	((nd_uint16_t*)buf)[3] = htons(blk->delay);
	((nd_uint16_t*)buf)[4] = htons(blk->idx);
	((nd_uint16_t*)buf)[5] = htons(blk->unused2);
	return ND_RUDP_BLOCK_CDR_LEN;
}

struct nd_rudp_packet_s
{
	__ND_QUEUE_DECALRE__(nd_rudp_packet_t);
	nd_rudp_packet_t	*link;
	nd_rbtree_node_t	node_in_loss;
	struct sockaddr		so_addr;
	nd_rudp_header_t	hdr;
	char				*data;
	nd_uint_t			size;
	nd_uint_t			rd_pos;
	nd_uint_t			wr_pos;
	nd_uint64_t			expect_snd_tm;
	nd_uint64_t			snd_tm;
	nd_uint64_t			loss_tmo;
	nd_uint64_t			rcv_tm;
	nd_uint64_t			org_tm;

	nd_int_t			idx;
	nd_int_t			loss_cnt;
	nd_int_t			tm_loss_cnt;

	nd_uint32_t			affter_seq_when_retrans;

	unsigned			repeat:5;
	unsigned			in_loss_lst:1;
	unsigned			fast_restrans:1;
	unsigned			retrans_cnt:5;
};

static ND_INLINE nd_uint32_t nd_rudp_packet_space(nd_rudp_packet_t *b){ return b->size - b->wr_pos;}
static ND_INLINE nd_uint32_t nd_rudp_packet_length(nd_rudp_packet_t *b){ return b->wr_pos - b->rd_pos;}
static ND_INLINE char* nd_rudp_packet_rd_ptr(nd_rudp_packet_t *b){ return (char*)(b->data) + b->rd_pos;}
static ND_INLINE char* nd_rudp_packet_wr_ptr(nd_rudp_packet_t *b){ return (char*)(b->data) + b->wr_pos;}
static ND_INLINE void nd_rudp_packet_add_rd_pos(nd_rudp_packet_t *b, nd_uint_t offset){ b->rd_pos += offset;}
static ND_INLINE void nd_rudp_packet_add_wr_pos(nd_rudp_packet_t *b, nd_uint_t offset){ b->wr_pos += offset;}
static ND_INLINE void nd_rudp_packet_dec_rd_pos(nd_rudp_packet_t *b, nd_uint_t offset){ b->rd_pos -= offset;}
static ND_INLINE void nd_rudp_packet_dec_wr_pos(nd_rudp_packet_t *b, nd_uint_t offset){ b->wr_pos -= offset;}
static ND_INLINE void nd_rudp_packet_recycle(nd_rudp_packet_t *b){ b->wr_pos=0; b->rd_pos=0;}

nd_rudp_packet_t*
nd_rudp_packet_create(nd_uint_t size);

void 
nd_rudp_packet_destroy(nd_rudp_packet_t *pkt);

nd_err_t 
nd_rudp_packet_parse(nd_rudp_packet_t *pkt);

nd_err_t
nd_rudp_packet_write(nd_rudp_packet_t *pkt, int only_hdr);

typedef struct
{
	nd_mutex_t			mutex;
	nd_cond_t			cdt;
	nd_rudp_packet_t	que;
	volatile nd_int_t	size;
}nd_rudp_rque_t;

static ND_INLINE nd_int_t nd_rudp_rque_size(nd_rudp_rque_t *q)
{
	return q->size;
}

nd_err_t 
nd_rudp_rque_init(nd_rudp_rque_t *q);

void 
nd_rudp_rque_destroy(nd_rudp_rque_t *q);

static ND_INLINE nd_err_t nd_rudp_rque_enqueue(nd_rudp_rque_t *q, nd_rudp_packet_t *pkt)
{
	nd_mutex_lock(&q->mutex);
	nd_queue_insert_tail(&q->que, pkt);
	q->size++;
	nd_cond_signal(&q->cdt);
	nd_mutex_unlock(&q->mutex);
	return ND_RET_OK;
}

static ND_INLINE nd_err_t nd_rudp_rque_enqueue_lst(nd_rudp_rque_t *q, nd_rudp_packet_t *lst)
{
	nd_mutex_lock(&q->mutex);
	while (lst) {
		nd_queue_insert_tail(&q->que, lst);
		q->size++;
		lst = lst->link;
	}
	nd_cond_signal(&q->cdt);
	nd_mutex_unlock(&q->mutex);
	return ND_RET_OK;
}

static ND_INLINE void nd_rudp_rque_broadcast(nd_rudp_rque_t *q)
{
	nd_mutex_lock(&q->mutex);
	nd_cond_broadcast(&q->cdt);
	nd_mutex_unlock(&q->mutex);
}


typedef struct
{
	nd_mutex_t			mutex;
	nd_cond_t			cdt;
	nd_rudp_packet_t	que;
	volatile nd_uint_t	size;
	volatile nd_uint_t	free_size;
	volatile nd_uint_t	max_free_size;
	volatile nd_uint_t	last_blk_token;
}nd_rudp_sque_t;

nd_err_t 
nd_rudp_sque_init(nd_rudp_sque_t *q);

void 
nd_rudp_sque_destroy(nd_rudp_sque_t *q);

static ND_INLINE void nd_rudp_sque_broadcast(nd_rudp_sque_t *q)
{
	nd_mutex_lock(&q->mutex);
	nd_cond_broadcast(&q->cdt);
	nd_mutex_unlock(&q->mutex);
}


#ifdef __cplusplus
}
#endif

#endif /// __ND_RUDP_PACKET_H_20160802__