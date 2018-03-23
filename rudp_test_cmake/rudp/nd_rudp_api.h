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

#include "nd_rudp_common_def.h"

#ifdef __cplusplus
extern "C" {
#endif


/// 0: ok; -1: error; -2: time out

int ndrudp_init();

int ndrudp_init_log(int level, int opt, const char *fn);

void ndrudp_uninit();

ND_RUDP_SOCKET ndrudp_socket(long local_ip, int local_port, int fd, int svr);

int ndrudp_connect(ND_RUDP_SOCKET s, long ip, int port, int msec);

int ndrudp_listen(ND_RUDP_SOCKET s);

int ndrudp_accept(ND_RUDP_SOCKET s, ND_RUDP_SOCKET *out, int msec);

int ndrudp_send(ND_RUDP_SOCKET s, void *buf, int len, int msec, int flag);

int ndrudp_recv(ND_RUDP_SOCKET s, void *buf, int len, int msec, int flag);

void ndrudp_shutdown(ND_RUDP_SOCKET s);

void ndrudp_close(ND_RUDP_SOCKET s);

int ndrudp_set_opt(void *handle, int level, int opt_name, void *opt_val, int opt_len);

int ndrudp_get_opt(void *handle, int level, int opt_name, void *opt_val, int *opt_len);

typedef struct 
{
	int unreliable;				/// 0: 可靠   1: 不可靠
	int max_delay_tm;			/// 当@unreliable == 1时, 指定该数据块在超出@max_delay_tm时间后, 不再重传
}ndrudp_block_snd_opt_t;

typedef struct 
{
	void *buf;
	int len;
	struct 
	{
		int completed_len;
		int idx;				/// 
		unsigned int token;		/// block标识. 0: 标识一个新的block.
	}op_info;
}ndrudp_block_t;

int ndrudp_send_block(ND_RUDP_SOCKET s, ndrudp_block_t *blk, int msec, ndrudp_block_snd_opt_t *opt);

enum 
{
	ND_RUDP_IO_READ		= 0x01,
	ND_RUDP_IO_WRITE	= 0x02,
};

typedef struct 
{
	ND_RUDP_SOCKET		s;
	int					ev;
}ndrudp_poll_event_t;

#define NDRUDP_POLL_MAX			(512)

typedef struct 
{
	ndrudp_poll_event_t	lst[NDRUDP_POLL_MAX];
	int					cnt;
}ndrudp_poll_t;

int ndrudp_poll_add_event(ndrudp_poll_t *poll, ND_RUDP_SOCKET s, int ev);

int ndrudp_poll_del_event(ndrudp_poll_t *poll, ND_RUDP_SOCKET s, int ev);

void ndrudp_poll_zero(ndrudp_poll_t *poll);

int ndrudp_poll_wait(ndrudp_poll_t *poll_in, ndrudp_poll_t *poll_out, int msec);

#ifdef __cplusplus
}
#endif

#endif /// __NDRUDP_API_H_20160802__