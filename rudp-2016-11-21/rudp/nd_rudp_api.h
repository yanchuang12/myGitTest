/** Copyright (c) 2016-2017
 * All rights reserved.
 * 
 * �ļ�����:	nd_rudp_api.h   
 * ժ	 Ҫ:	
 * 
 * ��ǰ�汾��	1.0
 * ��	 ��:	������(FTT)
 * ��	 ��:	�½�
 * �������:	2016��08��02��
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
	ND_RUDP_OPT_NM_SO_HEARTBEAT_TMO,		/// int sec: ����������, û���յ��κ�����, ��Ͽ�����
	ND_RUDP_OPT_NM_SO_MAX_STAGED_PKT_CNT,	/// int: ���ն�ÿ�յ����ٸ����鷵��staged ack
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