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
	ND_RUDP_OPT_NM_SO_SNDBUF = 0,		/// int
	ND_RUDP_OPT_NM_SO_RCVBUF,		/// int
};

/// 0: ok; -1: error; -2: time out

/*
  功能: 初始化rudp
  参数: 无
  返回值: 
		0: ok; -1: error
*/
int ndrudp_init();


/*
  功能: 初始化rudp_log
  参数: 
		@level: 优先级
		@opt: 0x01: 打印到屏幕上; 0x02: 打印到文件
		@fn: log文件名
  返回值: 
		0: ok; -1: error
*/
int ndrudp_init_log(int level, int opt, const char *fn);


/*
  功能: 反初始化rudp
  参数: 无
  返回值: 无
*/
void ndrudp_uninit();

/*
  功能: 建立rudp socket
  参数: 
		@local_ip: ip地址
		@local_port: 端口号
		@fd: socket描述符
		@svr: 1: server; 0: not server
  返回值: 
		!NULL: ok; NULL: error
*/
ND_RUDP_SOCKET ndrudp_socket(long local_ip, int local_port, int fd, int svr);


/*
  功能: 发送端请求建立rudp连接
  参数: 
		@s: 本地rudp socket
		@ip: 服务端ip
		@port: 服务端port
		@msec: 毫秒
  返回值: 
		0: ok; -1: error; -2: 超时
*/
int ndrudp_connect(ND_RUDP_SOCKET s, long ip, int port, int msec);


/*
  功能: 服务端监听rudp连接
  参数: 
		@s: 监听rudp socket
  返回值: 
		0: ok; -1: error
*/
int ndrudp_listen(ND_RUDP_SOCKET s);

/*
  功能: 服务端接受一个rudp连接
  参数: 
		@s: 监听rudp socket
		@out: 返回的连接rudp socket
		@msec: accept时间
  返回值: 
		0: ok; -1: error
*/
int ndrudp_accept(ND_RUDP_SOCKET s, ND_RUDP_SOCKET *out, int msec);


/*
  功能: 发送数据
  参数: 
		@s: 返回的连接rudp socket
		@buf: 存储的数据
		@len: 数据长度
		@msec: 发送数据时间
  返回值: 
		大于0: 实际进入发送数据长度; -1: error; -2: 超时
*/
int ndrudp_send(ND_RUDP_SOCKET s, void *buf, int len, int msec);


/*
  功能: 接收数据
  参数: 
		@s: 返回的连接rudp socket
		@buf: 存储的数据
		@len: 数据长度
		@msec: 接收数据时间
  返回值: 
		大于0: 实际接收数据长度; -1: error; -2: 超时
*/
int ndrudp_recv(ND_RUDP_SOCKET s, void *buf, int len, int msec);


/*
  功能: 关闭rudp socket
  参数: 
		@s: rudp socket
  返回值: 无
*/
void ndrudp_close(ND_RUDP_SOCKET s);


int ndrudp_set_opt(ND_RUDP_SOCKET s, int level, int opt_name, void *opt_val, int opt_len);

#ifdef __cplusplus
}
#endif

#endif /// __NDRUDP_API_H_20160802__