/** Copyright (c) 2010-2011
 * All rights reserved.
 * 
 * 文件名称:	aot_nmctrl_interface.h 
 * 摘	 要:	接口类
 * 
 * 当前版本:	1.0
 * 作	 者:	范涛涛
 * 操	 作:	新建
 * 完成日期:	2011年1月14日
 */
#ifndef __AOT_NMCTRL_INTERFACE_H__
#define __AOT_NMCTRL_INTERFACE_H__

#include "interface_base.h"
#include <commondef/aot_typedef.h>

namespace aot{ namespace nmctrl{

class inmctrl_event_handler : public aot::tt::interface_base
{
public:
	enum{iid = 0};
protected:
	virtual ~inmctrl_event_handler(){;}
public:
	virtual bool query_interface (void** out, const char* key) {return false;}
	virtual const char* interface_name () {return "inmctrl_event_handler";}
public:
	virtual void on_nmctrl_reg_ret(aot_int32_t ret) = 0;
	virtual void on_nmctrl_disconnect() = 0;
};

class inmctrl_op : public aot::tt::interface_base
{
public:
	enum{iid = 0};
protected:
	virtual ~inmctrl_op (){;}
public:
	virtual bool query_interface (void** out, const char* key) {return false;}
	virtual const char* interface_name () {return "inmctrl_op";}
public:
	virtual bool regist_to_nms(const char* ip, aot_uint32_t port, const char* user, const char* pwd, aot_uint32_t svc_id, aot_int32_t wait_sec) = 0;
	virtual void unregist_to_nms() = 0;
	virtual bool send_sys_ebm_msg(aot_uint32_t ent_id, aot_uint32_t dep_id, aot_uint32_t serv_id, void* msg, aot_uint32_t msg_len) = 0;
	virtual bool send_sys_tbm_msg(aot_uint32_t tribe_id, aot_uint32_t serv_id, void* msg, aot_uint32_t msg_len) = 0;
	virtual bool send_sys_multicast_msg(const char* to_tata_lst, void* msg, aot_uint32_t msg_len) = 0;
	virtual bool send_sys_broadcast_msg(void* msg, aot_uint32_t msg_len) = 0;
	virtual bool send_sys_broadcast_msg_unknow(void* msg, aot_uint32_t msg_len) = 0;
	virtual bool is_regist() = 0;
	virtual void set_event_handler(inmctrl_event_handler* ev) = 0;
};

}} /// end aot/nmctrl

#endif /// __AOT_NMCTRL_INTERFACE_H__