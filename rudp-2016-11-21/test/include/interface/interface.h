/** Copyright (c) 2008-2009
 * All rights reserved.
 * 
 * �ļ�����:	interface.h 
 * ժ	 Ҫ:	�ӿ���
 * 
 * ��ǰ�汾:	1.0
 * ��	 ��:	������
 * ��	 ��:	�½�
 * �������:	2009��7��12��
 */
#ifndef __AOT_INTERFACE_H__
#define __AOT_INTERFACE_H__

#include "interface_base.h"

namespace aot{ namespace tt{

struct message_t
{
	int		type;				/// ��Ϣ����
	int		val;				/// ������Ϣֵ
	void*	from;				/// ���ͷ���ݱ�ʶ,��ΪNULL
	void*	to;					/// ���շ���ݱ�ʶ,��ΪNULL
	void*	data;				/// �Զ�������,������ʹ��; ���ȿ���ʹ��user_data[] �� user_data_ptr[]
	void*	unknow;				/// ��ʹ��
	int		user_data[8];		/// �Զ�������
	void*	user_data_ptr[8];	/// �Զ�������

	message_t()
	{
		type = 0;
		val = 0;
		from = NULL;
		to = NULL;
		data = NULL;
		unknow = NULL;
		int i;
		for( i = 0; i < sizeof(user_data) / sizeof(user_data[0]); ++i )
			user_data[i] = 0;
		for( i = 0; i < sizeof(user_data_ptr) / sizeof(user_data_ptr[0]); ++i )
			user_data_ptr[i] = NULL;
	}
};

typedef struct message_t command_t;

struct sys_event_param_t
{
	sys_event_param_t()
	{
		wparam = NULL;
		lparam = NULL;
	}
	aot::tt::interface_base* wparam;
	aot::tt::interface_base* lparam;
};

class imain : public interface_base
{
public:
	enum
	{ 
		e_regist_frame_group = 0, 
		e_regist_frame_top, 
		e_regist_frame_bottom,
		e_regist_trans_msg,
		e_regist_sys_event,
	};

	struct regist_frame_group_param_t
	{
		enum{e_use_as_normal = -1, e_use_as_button};
		regist_frame_group_param_t()
		{
			token = NULL;
			image_id = -1;
			style = e_use_as_normal;
			tip_msg = "";
			label = "";
			img_fn = "";	/// DSkin ��ǩ
			icon = NULL;
		}
		void* token;
		int image_id;
		int style;
		const char*	tip_msg;
		const char*	label;
		const char*	img_fn;
		HICON icon;
		
	};

	struct regist_frame_top_t
	{
		regist_frame_top_t()
		{
			token = NULL;
			min_height = min_width = 0;
		}
		void* token;
		int min_height;
		int min_width;
	};
	
	struct regist_trans_msg_t
	{
		regist_trans_msg_t()
		{
			token = NULL;
			is_trans_msg = false;
		}
		void* token;
		bool is_trans_msg;
	};

	struct regist_frame_bottom_t
	{
		regist_frame_bottom_t()
		{
			token = NULL;
			min_height = min_width = 0;
		}
		void* token;
		int min_height;
		int min_width;
	};

	struct regist_sys_event_t
	{
		enum{ e_type_im = 0, e_type_voip = 1};
		regist_sys_event_t()
		{
			event_type = -1;
			token = NULL;
			event_lst = NULL;
			event_num = 0;
		}
		int event_type;
		void* token;
		int* event_lst;
		int event_num;
	};
protected:
	virtual ~imain () {;}
public:
	virtual bool query_interface (void** out, const char* key){return false;}
	virtual const char* interface_name () {return "imain";}
public:
	virtual bool regist_to (const void* rg_param, int type, result_t* ret = NULL) = 0;
	virtual int putq_to (const char* key, message_t* msg, long tm = -1, result_t* ret = NULL){return -1;}
	virtual int command_to (const char* key, command_t* cmd, result_t* ret = NULL){ return -1;}
	virtual bool query_interface_ex(void** out, const char* dll_name, const char* key){return false;}
	virtual bool query_interface_by_iid_ex(void** out, const char* dll_name, int key){return false;}
	virtual void dispatch_sys_event(int event_type, int ev, WPARAM wparam, LPARAM lparam, sys_event_param_t* sep = NULL){;}
};

class ialloc;
class idll : public interface_base
{
public:
	struct dll_regist_param_t
	{
		imain* mi;
		HWND parent_wnd;
		RECT rc;		
		void* token;
		void* unknow; /// no used
		HWND app_wnd;
		ialloc*	shr_alloc;
	};
protected:
	virtual ~idll () {;}
public:
	virtual bool query_interface (void** out, const char* key){return false;}
	/// �����dll�����ӿ�, �ӿ���Ϊ: "idll" + "." + ģ����(����·��,����׺��)
	virtual const char* interface_name () = 0;
public:
	/// ����ģ����(����·��,����׺��)
	virtual const char* module_name () = 0;
	/// ע��/��ʼ��
	virtual bool on_regist (dll_regist_param_t* p, result_t* ret = NULL) { return true;}
	virtual int on_putq (message_t* msg, long tm = -1, result_t* ret = NULL) {return -1;}
	virtual int on_command (command_t* cmd, result_t* ret = NULL){ return -1;}
	virtual void on_window_size_changed(HWND parent_wnd, const RECT* r){ return;}
	virtual bool on_show_window(HWND parent_wnd, const RECT* r, bool show, result_t* ret = NULL) {return true;}
	virtual BOOL pre_tans_msg(MSG* msg) {return FALSE;}
	/// ����: = 0 : ��ʾ�¼������ַ�. = -1: ��ʾ�¼����������ַ�
	virtual int on_sys_event(int event_type, int ev, WPARAM wparam, LPARAM lparam, sys_event_param_t* sep = NULL){return 0;}
};

}}
#endif /// __AOT_INTERFACE_H__