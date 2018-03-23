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

#define ND_RUDP_HDR_FLAG_ACK			0x01
#define ND_RUDP_HDR_FLAG_SHAKEHAND		0x02
#define ND_RUDP_HDR_FLAG_PROBE			0x04
#define ND_RUDP_HDR_FLAG_STAGED_ACK		0x08
#define ND_RUDP_HDR_FLAG_DATA			0x10
#define ND_RUDP_HDR_FLAG_BYE			0x20
#define ND_RUDP_HDR_FLAG_PUSH			0x40
#define ND_RUDP_HDR_FLAG_PPP			0x80

#define ND_RUDP_HDR_LEN					16

#define ND_RUDP_MTU						1452


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
*   |          flag         |        exhd_len       |                     token                     |
*   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*   |                                              seq                                              |
*   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*   |                                           timestamp                                           |
*   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*   |                      wnd                      |                    unused                     |
*   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*   |                                                                                               |
*   ~                                             ex_hdr                                            ~
*   |                                                                                               |
*   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

	flag :
		00bit: ack
*/

typedef struct nd_rudp_packet_s			nd_rudp_packet_t;

typedef struct 
{
	nd_uint8_t			flag;
	nd_uint16_t			exhdr_len;
	nd_uint16_t			token;
	/// @seq: 如果是data, 则表示该数据包的序号
	///       如果是ctrl, 则表示累积确认序号
	nd_uint32_t			seq;
	nd_uint32_t			ts;
	nd_uint16_t			wnd;
	nd_uint16_t			unused;
	/// 
	char				*exhdr;
	char				*body;
	nd_uint16_t			body_len;
}nd_rudp_header_t;


struct nd_rudp_packet_s
{
	__ND_QUEUE_DECALRE__(nd_rudp_packet_t);
	nd_rudp_packet_t	*link;
	nd_rbtree_node_t	node_in_loss;
	nd_inet_addr_t		addr;
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

	unsigned			repeat:5;
	unsigned			loss:1;

};

static ND_INLINE nd_uint32_t nd_rudp_packet_space(nd_rudp_packet_t* b){ return b->size - b->wr_pos;}
static ND_INLINE nd_uint32_t nd_rudp_packet_length(nd_rudp_packet_t* b){ return b->wr_pos - b->rd_pos;}
static ND_INLINE char* nd_rudp_packet_rd_ptr(nd_rudp_packet_t* b){ return (char*)(b->data) + b->rd_pos;}
static ND_INLINE char* nd_rudp_packet_wr_ptr(nd_rudp_packet_t* b){ return (char*)(b->data) + b->wr_pos;}
static ND_INLINE void nd_rudp_packet_add_rd_pos(nd_rudp_packet_t*b, nd_uint_t offset){ b->rd_pos += offset;}
static ND_INLINE void nd_rudp_packet_add_wr_pos(nd_rudp_packet_t*b, nd_uint_t offset){ b->wr_pos += offset;}
static ND_INLINE void nd_rudp_packet_dec_rd_pos(nd_rudp_packet_t*b, nd_uint_t offset){ b->rd_pos -= offset;}
static ND_INLINE void nd_rudp_packet_dec_wr_pos(nd_rudp_packet_t*b, nd_uint_t offset){ b->wr_pos -= offset;}
static ND_INLINE void nd_rudp_packet_recycle(nd_rudp_packet_t*b){ b->wr_pos=0; b->rd_pos=0;}

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

nd_err_t nd_rudp_rque_init(nd_rudp_rque_t *q);
void nd_rudp_rque_destroy(nd_rudp_rque_t *q);

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
	nd_cond_broadcast(&q->cdt);
}


typedef struct
{
	nd_mutex_t			mutex;
	nd_cond_t			cdt;
	nd_rudp_packet_t	que;
	volatile nd_uint_t	size;
	volatile nd_uint_t	free_size;
	volatile nd_uint_t	max_free_size;
}nd_rudp_sque_t;

nd_err_t nd_rudp_sque_init(nd_rudp_sque_t *q);
void nd_rudp_sque_destroy(nd_rudp_sque_t *q);


static ND_INLINE void nd_rudp_sque_broadcast(nd_rudp_sque_t *q)
{
	nd_cond_broadcast(&q->cdt);
}


#ifdef __cplusplus
}
#endif

#endif /// __ND_RUDP_PACKET_H_20160802__