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
#ifndef __NDRUDP_API_H_20160802__
#define __NDRUDP_API_H_20160802__

#ifdef __cplusplus
extern "C" {
#endif

typedef void*		ND_RUDP_SOCKET;
#define ND_RUDP_INVALID_SOCKET		(0)

/// opt level
enum 
{
	ND_RUDP_OPT_LV_SO = 0,
};

/// opt name
enum 
{
	ND_RUDP_OPT_NM_SO_SNDBUF = 0,			/// int
	ND_RUDP_OPT_NM_SO_RCVBUF,				/// int
	ND_RUDP_OPT_NM_SO_HEARTBEAT_TMO,		/// int sec: 连续多少秒, 没有收到任何数据, 则断开连接
	ND_RUDP_OPT_NM_SO_MAX_STAGED_PKT_CNT,	/// int: 接收端每收到多少个分组返回staged ack
};

/// 0: ok; -1: error; -2: time out

int ndrudp_init();

int ndrudp_init_log(int level, int opt, const char *fn);

void ndrudp_uninit();

ND_RUDP_SOCKET ndrudp_socket(long local_ip, int local_port, int fd, int svr);

int ndrudp_connect(ND_RUDP_SOCKET s, long ip, int port, int msec);

int ndrudp_listen(ND_RUDP_SOCKET s);

int ndrudp_accept(ND_RUDP_SOCKET s, ND_RUDP_SOCKET *out, int msec);

int ndrudp_send(ND_RUDP_SOCKET s, void *buf, int len, int msec);

int ndrudp_recv(ND_RUDP_SOCKET s, void *buf, int len, int msec);

void ndrudp_close(ND_RUDP_SOCKET s);

int ndrudp_set_opt(ND_RUDP_SOCKET s, int level, int opt_name, void *opt_val, int opt_len);

#ifdef __cplusplus
}
#endif

#endif /// __NDRUDP_API_H_20160802__