/** Copyright (c) 2008-2010
 * All rights reserved.
 * 
 * 文件名称:	aot_sc_interface.h 
 * 摘	 要:	接口类
 * 
 * 当前版本:	1.0
 * 作	 者:	范涛涛
 * 操	 作:	新建
 * 完成日期:	2010年5月26日
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
	aot_uint32_t		app_svc_id;			/// 应用服务器的id, 这个字段可以使用专门的接口进行设置
};

class isc_event_handler : public aot::tt::interface_base
{
protected:
	virtual ~isc_event_handler(){;}
public:
	virtual bool query_interface (void** out, const char* key) {return false;}
	virtual const char* interface_name () {return "isc_event_handler";}
public:
	/// 收到了一个目标主机就是自身的数据包
	virtual void on_incoming_pkt(aot_buf_t* pkt) = 0;
	/// 收到了一个目标集群就是本主机所在集群的数据包, 但是这个数据包的目标主机不是自己,
	/// 还需要转发给目标主机
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
	/// 考虑到应用层的app_svc_id可能是由其他服务器进行分配的,因此为了简化应用层的初始化时机(可以分为几个阶段进行
	/// 初始化), 因此增加一个专门的接口来设置, 在没有设置这个参数时, 应用层传下来的数据包将会丢弃
	virtual void set_app_svc_id(aot_uint32_t id) = 0;
	virtual bool start() = 0;
	virtual void stop() = 0;
	/// 在connect之前,app_svc_id必须已经设置
	virtual bool connect(aot_inet_addr_t* addr) = 0;

	/// @src_id 源端主机ID(集群主机ID); @dest_id 目标端主机ID(集群主机ID)
	/// 无论成功或失败, @pkt都由接口自动释放
	virtual bool send_to(aot_buf_t* pkt, aot_uint32_t src_host_id, aot_uint32_t dest_host_id) = 0;

	/// 浅层复制一份@pkt,并路由.  浅层复制一个数据包并路由需要一些特殊的操作, 所以提供该接口
	/// 无论成功或失败, @pkt都由调用者自己释放
	virtual bool duplicate_packet_and_send_to(aot_buf_t* pkt, aot_uint32_t src_host_id, aot_uint32_t dest_host_id) = 0;
};

}}

#endif /// __AOT_SC_INTERFACE_20100526_H__



