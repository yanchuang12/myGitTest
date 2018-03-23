/** Copyright (c) 2008-2010
 * All rights reserved.
 * 
 * �ļ�����:	aot_sc_interface.h 
 * ժ	 Ҫ:	�ӿ���
 * 
 * ��ǰ�汾:	1.0
 * ��	 ��:	������
 * ��	 ��:	�½�
 * �������:	2010��5��26��
 */
#ifndef __AOT_SC_INTERFACE_20100526_H__
#define __AOT_SC_INTERFACE_20100526_H__

#include "interface_base.h"
#include "aot_inet_interface.h"

namespace aot{ namespace sc{

class isc_event_handler;
struct sc_init_param_t
{
	AIO_POOL_HANDLE		pool;
	isc_event_handler*	ev;
	aot_uint32_t		app_svc_id;			/// Ӧ�÷�������id, ����ֶο���ʹ��ר�ŵĽӿڽ�������
};

class isc_event_handler : public aot::tt::interface_base
{
protected:
	virtual ~isc_event_handler(){;}
public:
	virtual bool query_interface (void** out, const char* key) {return false;}
	virtual const char* interface_name () {return "isc_event_handler";}
public:
	/// �յ���һ��Ŀ������������������ݰ�
	virtual void on_incoming_pkt(aot_buf_t* pkt) = 0;
	/// �յ���һ��Ŀ�꼯Ⱥ���Ǳ��������ڼ�Ⱥ�����ݰ�, ����������ݰ���Ŀ�����������Լ�,
	/// ����Ҫת����Ŀ������
	virtual void on_incoming_a_roam_pkt(aot_buf_t* pkt) = 0;
};

class isc : public aot::tt::interface_base
{
protected:
	virtual ~isc (){;}
public:
	virtual bool query_interface (void** out, const char* key) {return false;}
	virtual const char* interface_name () {return "isc";}
public:
	virtual bool init(sc_init_param_t* p) = 0;
	/// ���ǵ�Ӧ�ò��app_svc_id���������������������з����,���Ϊ�˼�Ӧ�ò�ĳ�ʼ��ʱ��(���Է�Ϊ�����׶ν���
	/// ��ʼ��), �������һ��ר�ŵĽӿ�������, ��û�������������ʱ, Ӧ�ò㴫���������ݰ����ᶪ��
	virtual void set_app_svc_id(aot_uint32_t id) = 0;
	virtual bool start() = 0;
	virtual void stop() = 0;
	/// ��connect֮ǰ,app_svc_id�����Ѿ�����
	virtual bool connect(aot_inet_addr_t* addr) = 0;

	/// @src_id Դ������ID(��Ⱥ����ID); @dest_id Ŀ�������ID(��Ⱥ����ID)
	/// ���۳ɹ���ʧ��, @pkt���ɽӿ��Զ��ͷ�
	virtual bool send_to(aot_buf_t* pkt, aot_uint32_t src_host_id, aot_uint32_t dest_host_id) = 0;

	/// ǳ�㸴��һ��@pkt,��·��.  ǳ�㸴��һ�����ݰ���·����ҪһЩ����Ĳ���, �����ṩ�ýӿ�
	/// ���۳ɹ���ʧ��, @pkt���ɵ������Լ��ͷ�
	virtual bool duplicate_packet_and_send_to(aot_buf_t* pkt, aot_uint32_t src_host_id, aot_uint32_t dest_host_id) = 0;
};

}}

#endif /// __AOT_SC_INTERFACE_20100526_H__



