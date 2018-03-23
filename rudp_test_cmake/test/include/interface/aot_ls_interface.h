/** Copyright (c) 2011-2012
 * All rights reserved.
 * 
 * �ļ�����:	aot_ls_interface.h 
 * ժ	 Ҫ:	�ӿ���
 * 
 * ��ǰ�汾:	1.0
 * ��	 ��:	������
 * ��	 ��:	�½�
 * �������:	2011��4��01��
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
	unsigned char crc[9];		/// �ļ�CRC
	unsigned char key[9];		/// ����key
	char encry_type;			/// ��������
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
	/// ��ѡ���ò���,��δ����,���Զ���ini�ļ���ȡ��ز���
	virtual void set_url(const char* p) = 0;
	virtual void set_sn(const char* p) = 0;

	/// Ĭ��ΪBIT_MODE_AUTO
	virtual void set_login_mode(int m) = 0;
public:
	/// ʵʱ��ȡ
	virtual bool read_data(const char* name, aot::tt::istr* val) = 0;
	virtual bool read_feature(aot_uint32_t id, aot_uint32_t* val) = 0;
public:
	/// һ���԰�data iem����ȫ������ @m
	virtual bool read_all_data(aot::tt::istr2str_map* m) = 0;
	/// ��ȡsn�����Ϣ
	virtual bool get_session_info(int type, aot::tt::istr* val) = 0;
public:
	/// ��ȡ������뼰������Ϣ,�������еĴ���һ�����ж�Ӧ�Ĵ����뼰����,���Ըýӿ�Ӧ�ý�������һЩ���������ĳ���
	/// ���ش�����, @val: ������Ϣ
	virtual int get_last_error(aot::tt::istr* val) = 0;
	virtual const char* get_sn() = 0;
	virtual bool update_online() = 0;
	virtual bool update_offline() = 0;
};

}}

#endif /// __AOT_LS_INTERFACE_20110401_H__



