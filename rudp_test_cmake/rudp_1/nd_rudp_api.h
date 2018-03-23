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
	ND_RUDP_OPT_NM_SO_SNDBUF = 0,		/// int
	ND_RUDP_OPT_NM_SO_RCVBUF,		/// int
};

/// 0: ok; -1: error; -2: time out

/*
  ����: ��ʼ��rudp
  ����: ��
  ����ֵ: 
		0: ok; -1: error
*/
int ndrudp_init();


/*
  ����: ��ʼ��rudp_log
  ����: 
		@level: ���ȼ�
		@opt: 0x01: ��ӡ����Ļ��; 0x02: ��ӡ���ļ�
		@fn: log�ļ���
  ����ֵ: 
		0: ok; -1: error
*/
int ndrudp_init_log(int level, int opt, const char *fn);


/*
  ����: ����ʼ��rudp
  ����: ��
  ����ֵ: ��
*/
void ndrudp_uninit();

/*
  ����: ����rudp socket
  ����: 
		@local_ip: ip��ַ
		@local_port: �˿ں�
		@fd: socket������
		@svr: 1: server; 0: not server
  ����ֵ: 
		!NULL: ok; NULL: error
*/
ND_RUDP_SOCKET ndrudp_socket(long local_ip, int local_port, int fd, int svr);


/*
  ����: ���Ͷ�������rudp����
  ����: 
		@s: ����rudp socket
		@ip: �����ip
		@port: �����port
		@msec: ����
  ����ֵ: 
		0: ok; -1: error; -2: ��ʱ
*/
int ndrudp_connect(ND_RUDP_SOCKET s, long ip, int port, int msec);


/*
  ����: ����˼���rudp����
  ����: 
		@s: ����rudp socket
  ����ֵ: 
		0: ok; -1: error
*/
int ndrudp_listen(ND_RUDP_SOCKET s);

/*
  ����: ����˽���һ��rudp����
  ����: 
		@s: ����rudp socket
		@out: ���ص�����rudp socket
		@msec: acceptʱ��
  ����ֵ: 
		0: ok; -1: error
*/
int ndrudp_accept(ND_RUDP_SOCKET s, ND_RUDP_SOCKET *out, int msec);


/*
  ����: ��������
  ����: 
		@s: ���ص�����rudp socket
		@buf: �洢������
		@len: ���ݳ���
		@msec: ��������ʱ��
  ����ֵ: 
		����0: ʵ�ʽ��뷢�����ݳ���; -1: error; -2: ��ʱ
*/
int ndrudp_send(ND_RUDP_SOCKET s, void *buf, int len, int msec);


/*
  ����: ��������
  ����: 
		@s: ���ص�����rudp socket
		@buf: �洢������
		@len: ���ݳ���
		@msec: ��������ʱ��
  ����ֵ: 
		����0: ʵ�ʽ������ݳ���; -1: error; -2: ��ʱ
*/
int ndrudp_recv(ND_RUDP_SOCKET s, void *buf, int len, int msec);


/*
  ����: �ر�rudp socket
  ����: 
		@s: rudp socket
  ����ֵ: ��
*/
void ndrudp_close(ND_RUDP_SOCKET s);


int ndrudp_set_opt(ND_RUDP_SOCKET s, int level, int opt_name, void *opt_val, int opt_len);

#ifdef __cplusplus
}
#endif

#endif /// __NDRUDP_API_H_20160802__