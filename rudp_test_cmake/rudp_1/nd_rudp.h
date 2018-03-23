/** Copyright (c) 2016-2017
 * All rights reserved.
 * 
 * 文件名称:	nd_rudp.h   
 * 摘	 要:	
 * 
 * 当前版本：	1.0
 * 作	 者:	范涛涛(FTT)
 * 操	 作:	新建
 * 完成日期:	2016年08月02日
 */

#ifndef __ND_RUDP_H_20160802__
#define __ND_RUDP_H_20160802__

#include "nd_core/nd_core.h"

#ifdef __cplusplus
extern "C" {
#endif

#define __ONE_CYCLE_MAX_SND_PKT_CNT__		(256)
#define __ONE_CYCLE_MAX_RCV_PKT_CNT__		(256)

#define __RATE_BW_METHOD__					(3)

typedef struct nd_rudp_channel_s		nd_rudp_channel_t;
typedef struct nd_rudp_endpoint_s		nd_rudp_endpoint_t;
typedef struct nd_rudp_s				nd_rudp_t;
typedef struct nd_rudp_event_s			nd_rudp_event_t;
typedef struct nd_rudp_evld_s			nd_rudp_evld_t;

typedef struct nd_rudp_locked_posted_event_s nd_rudp_locked_posted_event_t;

typedef struct 
{
	void				(*handle_read)(nd_rudp_event_t *ev);
	void				(*handle_write)(nd_rudp_event_t *ev);
	void				(*handle_locked_posted_event)(nd_rudp_locked_posted_event_t *lev);
}nd_rudp_event_handle_t;

struct nd_rudp_event_s
{
	void				(*handle_event)(nd_rudp_event_t *ev);
	nd_rudp_channel_t	*ch;
	void				*data;
	nd_dlist_node_t		*node;
	unsigned			wait_pending:1;
	unsigned			posted_pending:1;
};

#include "nd_rudp_packet.h"
#include "nd_rudp_event.h"
#include "nd_rudp_endpoint.h"
#include "nd_rudp_channel.h"

struct nd_rudp_s
{
	__ND_QUEUE_DECALRE__(nd_rudp_t);

	nd_rudp_evld_t		*evld;
	nd_thread_t			th;

	nd_rudp_channel_t	*ch;
	nd_rudp_event_handle_t evh;
};

typedef struct
{
	int			opt;
	int			level;
	FILE		*f;
}nd_rudp_log_info_t;

typedef struct
{
	nd_mutex_t			rl_mutex;
	nd_rudp_t			rudp_lst;
	nd_rudp_log_info_t	log;
	unsigned			active:1;
}nd_global_mgr_t;

extern nd_global_mgr_t* g_global_mgr;

#define ND_RUDP_LOG_OPT_CONSOLE		(0x01)
#define ND_RUDP_LOG_OPT_FILE		(0x02)

#define ND_RUDP_LOG_LV_PRINT		(0x0001 << 2)
#define ND_RUDP_LOG_LV_BUG			(0x0001 << 3)
#define ND_RUDP_LOG_LV_ERROR		(0x0001 << 4)
#define ND_RUDP_LOG_LV_WARN			(0x0001 << 5)
#define ND_RUDP_LOG_LV_INFO			(0x0001 << 6)
#define ND_RUDP_LOG_LV_DEBUG		(0x0001 << 7)
#define ND_RUDP_LOG_LV_DUMP			(0x0001 << 8)
#define ND_RUDP_LOG_LV_TRACE		(0x0001 << 9)

nd_err_t 
nd_rudp_log_init(int level, int opt, const char *fn);

void 
nd_rudp_log_un_init();

void 
nd_rudp_log(int level, const char* fmt, ...);

nd_rudp_t*
nd_rudp_create(struct sockaddr *local_addr, int addr_len, int fd, int svr);

void
nd_rudp_uninit();

void 
nd_rudp_process_data(nd_rudp_endpoint_t *ep, nd_rudp_packet_t *pkt);

void 
nd_rudp_process_ctrl(nd_rudp_endpoint_t *ep, nd_rudp_packet_t *pkt);

void 
nd_rudp_send_ctrl(nd_rudp_endpoint_t *ep, nd_uint32_t ts, nd_uint8_t flag, nd_uint64_t org_tm);

nd_err_t 
nd_rudp_process_staged_ack(nd_rudp_endpoint_t *ep, nd_rudp_packet_t *pkt, nd_uint8_t *ptr, nd_int_t len);


#ifdef __cplusplus
}
#endif

#endif /// __ND_RUDP_H_20160802__