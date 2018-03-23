/** Copyright (c) 2011-2012
 * All rights reserved.
 * 
 * 文件名称:	aot_ls_interface.h 
 * 摘	 要:	接口类
 * 
 * 当前版本:	1.0
 * 作	 者:	范涛涛
 * 操	 作:	新建
 * 完成日期:	2011年4月01日
 */
#ifndef __AOT_LS_INTERFACE_20110401_H__
#define __AOT_LS_INTERFACE_20110401_H__

#include "interface_base.h"
#include "out_param_interface.h"
#include <commondef/aot_typedef.h>

namespace aot{ namespace ls{

enum
{ 
	e_encrypt = 1, 
	e_nonencrypt = -1, 
	e_encrypt_error = -2,
};

struct file_encrypt_info_t
{
	unsigned char crc[9];		/// 文件CRC
	unsigned char key[9];		/// 加密key
	char encry_type;			/// 加密类型
};

class ils : public aot::tt::interface_base
{
protected:
	virtual ~ils(){;}
public:
	virtual bool query_interface (void** out, const char* key) {return false;}
	virtual const char* interface_name () {return "ils";}
public:
	virtual bool encrypt(const char* src_fn, const char* dest_fn, char* key) = 0;
	virtual bool decrypt(const char* src_fn, const char* dest_fn, char* key) = 0;
	virtual bool encrypt_ex(const char* src_fn, const char* dest_fn) = 0;
	virtual bool decrypt_ex(const char* src_fn, const char* dest_fn) = 0;
	virtual void free_buf(void* p) = 0;
	virtual bool decrypt_to_mem(const char* src_fn, void** buf, int* buf_len, char* key) = 0;
	virtual bool decrypt_to_mem_ex(const char* src_fn, void** buf, int* buf_len) = 0;
	virtual bool get_file_encrypt_info(const char *fn, aot::ls::file_encrypt_info_t* info) = 0;
	virtual const char* get_hd_serial() = 0;
};

class ibit_auth : public aot::tt::interface_base
{
protected:
	virtual ~ibit_auth(){;}
public:
	virtual bool query_interface(void** out, const char* key) {return false;}
	virtual const char* interface_name() {return "ibit_auth";}
public:
	virtual bool login() = 0;
	virtual void logout() = 0;
public:
	/// 可选设置参数,如未设置,则自动从ini文件读取相关参数
	virtual void set_url(const char* p) = 0;
	virtual void set_sn(const char* p) = 0;

	/// 默认为BIT_MODE_AUTO
	virtual void set_login_mode(int m) = 0;
public:
	/// 实时读取
	virtual bool read_data(const char* name, aot::tt::istr* val) = 0;
	virtual bool read_feature(aot_uint32_t id, aot_uint32_t* val) = 0;
public:
	/// 一次性把data iem数据全部读到 @m
	virtual bool read_all_data(aot::tt::istr2str_map* m) = 0;
	/// 获取sn相关信息
	virtual bool get_session_info(int type, aot::tt::istr* val) = 0;
public:
	/// 获取错误代码及错误信息,并非所有的错误都一定会有对应的错误码及描述,所以该接口应该仅仅用于一些辅助诊断类的场合
	/// 返回错误码, @val: 错误信息
	virtual int get_last_error(aot::tt::istr* val) = 0;
	virtual const char* get_sn() = 0;
	virtual bool update_online() = 0;
	virtual bool update_offline() = 0;
};

}}

#endif /// __AOT_LS_INTERFACE_20110401_H__



