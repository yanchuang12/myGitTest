#include "nd_rudp_packet.h"

nd_rudp_packet_t*
nd_rudp_packet_create(nd_uint_t size)
{
	nd_rudp_packet_t *pkt;
	pkt = malloc(sizeof(nd_rudp_packet_t) + size);
	if (!pkt)
		return NULL;

	memset(pkt, 0, sizeof(nd_rudp_packet_t));

	pkt->data = (char*)pkt + sizeof(nd_rudp_packet_t);
	pkt->size = size;
	pkt->rd_pos = pkt->wr_pos = 0;

	return pkt;
}

void 
nd_rudp_packet_destroy(nd_rudp_packet_t *pkt)
{
	if (pkt) {
		free(pkt);
	}
}

nd_err_t 
nd_rudp_packet_parse(nd_rudp_packet_t *pkt)
{
	nd_uint8_t *data;

	if (nd_rudp_packet_length(pkt) < ND_RUDP_HDR_LEN)
		return ND_RET_ERROR;

	data = (nd_uint8_t*)nd_rudp_packet_rd_ptr(pkt);

	pkt->hdr.flag = data[0];
	pkt->hdr.exhdr_len = data[1] << 2;
	pkt->hdr.token = ntohs(((nd_uint16_t*)data)[1]);
	pkt->hdr.seq = ntohl(((nd_uint32_t*)data)[1]);
	pkt->hdr.ts = ntohl(((nd_uint32_t*)data)[2]);
	pkt->hdr.wnd = ntohs(((nd_uint16_t*)data)[6]);
	pkt->hdr.unused = ntohs(((nd_uint16_t*)data)[7]);
	
	if (nd_rudp_packet_length(pkt) < (nd_uint32_t)pkt->hdr.exhdr_len + ND_RUDP_HDR_LEN) {
		return ND_RET_ERROR;
	}

	if (pkt->hdr.exhdr_len == 0) {
		pkt->hdr.exhdr = NULL;
	} else {
		pkt->hdr.exhdr = pkt->data + ND_RUDP_HDR_LEN;
	}

	pkt->hdr.body_len = nd_rudp_packet_length(pkt) - ND_RUDP_HDR_LEN - pkt->hdr.exhdr_len;
	if (0 == pkt->hdr.body_len)
		pkt->hdr.body = NULL;
	else
		pkt->hdr.body = data + ND_RUDP_HDR_LEN + pkt->hdr.exhdr_len;

	return ND_RET_OK;
}

nd_err_t
nd_rudp_packet_write(nd_rudp_packet_t *pkt, int only_hdr)
{
	nd_uint8_t *data;

	if (pkt->hdr.exhdr_len > (0x00ff << 2) || 
		(ND_RUDP_HDR_LEN + (nd_uint32_t)pkt->hdr.exhdr_len + (nd_uint32_t)pkt->hdr.body_len) > nd_rudp_packet_space(pkt)) {
		nd_assert(0);
		return ND_RET_ERROR;
	}

	data = (nd_uint8_t*)pkt->data;

	data[0] = pkt->hdr.flag;
	data[1] = pkt->hdr.exhdr_len >> 2;
	((nd_uint16_t*)data)[1] = htons(pkt->hdr.token);
	((nd_uint32_t*)data)[1] = htonl(pkt->hdr.seq);
	((nd_uint32_t*)data)[2] = htonl(pkt->hdr.ts);
	((nd_uint16_t*)data)[6] = htons(pkt->hdr.wnd);
	((nd_uint16_t*)data)[7] = htons(pkt->hdr.unused);
	
	if (only_hdr) {
		nd_rudp_packet_add_wr_pos(pkt, ND_RUDP_HDR_LEN);
	}
	else{
		if (pkt->hdr.exhdr_len && pkt->hdr.exhdr) {
			memcpy(data + ND_RUDP_HDR_LEN, pkt->hdr.exhdr, pkt->hdr.exhdr_len);
		}
		if (pkt->hdr.body_len && pkt->hdr.body) {
			memcpy(data + ND_RUDP_HDR_LEN + pkt->hdr.exhdr_len, pkt->hdr.body, pkt->hdr.body_len);
		}

		nd_rudp_packet_add_wr_pos(pkt, ND_RUDP_HDR_LEN + pkt->hdr.exhdr_len + pkt->hdr.body_len);
	}
	
	return ND_RET_OK;
}

/// recv queue

nd_err_t nd_rudp_rque_init(nd_rudp_rque_t *q)
{
	nd_queue_init(&q->que);
	nd_mutex_init(&q->mutex);
	nd_cond_init(&q->cdt, &q->mutex);
	q->size = 0;
	return ND_RET_OK;
}

void nd_rudp_rque_destroy(nd_rudp_rque_t *q)
{
	nd_rudp_packet_t *pkt;

	nd_mutex_lock(&q->mutex);
	while (!nd_queue_is_empty(&q->que)) {
		pkt = nd_queue_head(&q->que);
		nd_queue_remove(pkt);
		nd_rudp_packet_destroy(pkt);
	} 
	nd_mutex_unlock(&q->mutex);

	nd_cond_destroy(&q->cdt);
	nd_mutex_destroy(&q->mutex);
}

/// send queue

nd_err_t nd_rudp_sque_init(nd_rudp_sque_t *q)
{
	nd_queue_init(&q->que);
	nd_mutex_init(&q->mutex);
	nd_cond_init(&q->cdt, &q->mutex);
	q->size = 0;
	q->free_size = 0;
	q->max_free_size = 0;
	return ND_RET_OK;
}

void nd_rudp_sque_destroy(nd_rudp_sque_t *q)
{
	nd_rudp_packet_t *pkt;

	nd_mutex_lock(&q->mutex);
	while (!nd_queue_is_empty(&q->que)) {
		pkt = nd_queue_head(&q->que);
		nd_queue_remove(pkt);
		nd_rudp_packet_destroy(pkt);
	} 
	nd_mutex_unlock(&q->mutex);

	nd_cond_destroy(&q->cdt);
	nd_mutex_destroy(&q->mutex);
}