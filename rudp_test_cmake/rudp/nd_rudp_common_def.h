/** Copyright (c) 2016-2017
 * All rights reserved.
 * 
 * 文件名称:	nd_rudp_api.h   
 * 摘	 要:	
 * 
 * 当前版本：	1.0
 * 作	 者:	范涛涛(FTT)
 * 操	 作:	新建
 * 完成日期:	2016年08月02日
 */
#ifndef __NDRUDP_COMMON_DEF_H_20160802__
#define __NDRUDP_COMMON_DEF_H_20160802__

#ifdef __cplusplus
extern "C" {
#endif

typedef void*		ND_RUDP_SOCKET;
typedef void*		ND_RUDP_CH;
#define ND_RUDP_INVALID_SOCKET		(0)

/// opt level
enum 
{
	ND_RUDP_OPT_LV_SO = 0,
	ND_RUDP_OPT_LV_CH,
	ND_RUDP_OPT_LV_GLOBAL,
};

/// opt name
enum 
{
	ND_RUDP_OPT_NM_SO_SNDBUF = 0,			/// int
	ND_RUDP_OPT_NM_SO_RCVBUF,				/// int
	ND_RUDP_OPT_NM_SO_MTU,					/// int
	ND_RUDP_OPT_NM_SO_HEARTBEAT_TMO,		/// int sec: 连续多少秒, 没有收到任何数据, 则断开连接
	ND_RUDP_OPT_NM_SO_MAX_STAGED_PKT_CNT,	/// int: 接收端每收到多少个分组返回staged ack
	ND_RUDP_OPT_NM_SO_CC_MODE,				/// int, 0: 自动(默认);  1: 固定速率控制
	ND_RUDP_OPT_NM_SO_CC_FIXED_INTER_USEC,	/// int, 当cc mode==1时, 设置固定发送间隔(单位: 微秒)

	ND_RUDP_OPT_NM_SO_SND_RATE,				/// int
	ND_RUDP_OPT_NM_SO_RCV_RATE,				/// int

	ND_RUDP_OPT_NM_SO_LING_TMO,				/// int
	ND_RUDP_OPT_NM_SO_ACK_BYTES,			/// uint64

	ND_RUDP_OPT_NM_SO_TRANS_MODE,			/// int
	ND_RUDP_OPT_NM_SO_REMOTE_ADDR,			/// struct sockaddr*
	ND_RUDP_OPT_NM_SO_LOCAL_ADDR,			/// struct sockaddr*
	ND_RUDP_OPT_NM_SO_RAW_FD,				/// win: SOCKET; other: int
	ND_RUDP_OPT_NM_SO_STATUS,				/// int: -1 :> ND_RUDP_EP_STATUS_CONNECTED;  0: ==ND_RUDP_EP_STATUS_CONNECTED; 1: <ND_RUDP_EP_STATUS_CONNECTED
	ND_RUDP_OPT_NM_SO_USER_DATA,			/// void*
	ND_RUDP_OPT_NM_SO_PRE_SND_HEADER_LEN,	/// int
	ND_RUDP_OPT_NM_SO_RAW_CH,
	ND_RUDP_OPT_NM_SO_PEEK_SNDBUF_FREESIZE,	/// int (byte)
	ND_RUDP_OPT_NM_SO_RTO_RANGE,			/// nd_rudp_rto_range_t*
	ND_RUDP_OPT_NM_SO_INTERUSEC_RANGE,		/// nd_rudp_interusec_range_t*
	ND_RUDP_OPT_NM_SO_LOSSCHK_THRESHOLD,	/// nd_rudp_interusec_range_t*
	ND_RUDP_OPT_NM_SO_DELAY_LEVEL,			/// int 
	ND_RUDP_OPT_NM_SO_LOSS_THRESHOLD,		/// int (x%)

	__ND_RUDP_OPT_NM_CH_begin__ = 128,
	ND_RUDP_OPT_NM_CH_SND_BEFORE_CB,
	ND_RUDP_OPT_NM_CH_RCV_AFTER_CB,
	ND_RUDP_OPT_NM_CH_CLOSE_BEFOR_CB,
	ND_RUDP_OPT_NM_CH_REFRESH_CB,
	ND_RUDP_OPT_NM_CH_USER_DATA,

	__ND_RUDP_OPT_NM_GLOBAL_begin__ = 256,
	ND_RUDP_OPT_NM_GLOBAL_POLL_WAIT_MSEC,	/// int
};

enum 
{
	ND_RUDP_S_FLAG_SND_N = 0x00000001,
};

enum 
{
	ND_RUDP_R_FLAG_RCV_N = 0x00000001,
};

enum
{
	ND_RUDP_DELAY_LV_DEFAULT = 0,
	ND_RUDP_DELAY_LV_LOW,
	ND_RUDP_DELAY_LV_MID,
	ND_RUDP_DELAY_LV_HIGH,
};

typedef struct
{
	void	*buf;
	int		len;

}nd_rudp_iovec_t;

typedef struct
{
	int		min;
	int		max;
}nd_rudp_rto_range_t;

typedef struct
{
	int		min;
	int		max;
}nd_rudp_interusec_range_t;

typedef struct
{
	int		tmo_cnt_limit;
	int		rack_cnt_limit;
}nd_rudp_losschk_threshold_t;

#ifdef __cplusplus
}
#endif

#endif /// __NDRUDP_COMMON_DEF_H_20160802__