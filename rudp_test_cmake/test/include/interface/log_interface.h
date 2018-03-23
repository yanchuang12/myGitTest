/** Copyright (c) 2008-2009
 * All rights reserved.
 * 
 * �ļ�����:	log_interface.h  
 * ժ	 Ҫ:	log�ӿڻ��� �� ��ذ�װ�� �� ��־��
 * 
 * ��ǰ�汾:	1.0
 * ��	 ��:	������(FTT)
 * ��	 ��:	�½�
 * �������:	2009��7��19��
 */
#ifndef __LOG_INTERFACE_H__
#define __LOG_INTERFACE_H__

#include <stdio.h>
#include "interface_base.h"
#include "../sys/sys_singleton.h"
#include "../sys/sys_asy.h"

#pragma warning(disable:4996)

#define __USING_AOT_LOG__

enum log_level 
{
	/**   ��־����:
	 *  
	 *    07 06 05 04 03 02 01 00 07 06 05 04 03 02 01 00 07 06 05 04 03 02 01 00 07 06 05 04 03 02 01 00
	 *   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	 *   |         �ֽ�          |         �ֽ�          |         �ֽ�          |         �ֽ�          |
	 *   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	 *   |      ��־����:                                         |TR|DU|DE|ER|BU|         ��־�ȼ�      |
	 *   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	 */
	/* ---------------- log level bit mask ---------------- */
	AOT_LM_CRASH	= 0x01,
	/// ����bug ---��AOT_BUG���ʹ��
	AOT_LM_BUG		= 0x02,
	/// 
	AOT_LM_PRINT	= 0x08,
	/// �����¼���Ϣ
	AOT_LM_EMERG	= 0x09,
	/// ������Ϣ
	AOT_LM_ALERT	= 0x0A,
	/// �ٽ�������Ϣ
	AOT_LM_CRIT		= 0x0B,
	/// ������Ϣ ---��AOT_ERROR���ʹ��
	AOT_LM_ERROR	= 0x0C,
	/// ������Ϣ
	AOT_LM_WARN		= 0x0D,
	/// �Ǵ�����Ϣ, ���ǿ�����Ҫ�ر������Ϣ
	AOT_LM_NOTICE	= 0x0E,
	/// ��ʾ��Ϣ
	AOT_LM_INFO		= 0x0F,
	/// ��ϸ�ĵ�����Ϣ
	AOT_LM_DEBUG	= 0x10,
	/// ����������/���ڸ�����Ϣ
	AOT_LM_TRACE	= 0x11,
	/// don't use __AOT_LOG_MAX_VALUE__
	__AOT_LM_MAX_VALUE__	= 0xEF,

	/* ---------------- log type bit mask ---------------- */

	/// ����bug����־ (����ʹ�ü���Χ: AOT_LM_BUG ---> AOT_LM_CRASH)
	AOT_BUG		= 0x00000100,
	/// ��������־ (����ʹ�ü���Χ: AOT_LM_ERROR ---> AOT_LM_EMERG)
	AOT_ERROR	= 0x00000200,
	/// �������־ (����ʹ�ü���Χ: AOT_LM_DEBUG ---> AOT_LM_WARN)
	AOT_DEBUG	= 0x00000400,
	/// ��ӡ����   (����ʹ�ü���Χ: AOT_LM_DEBUG ---> AOT_LM_CRASH)
	AOT_DUMP    = 0x00000800,
	/// ����������/���ڸ��� (����ʹ�ü���Χ: AOT_LM_TRACE ---> AOT_LM_CRASH)
	AOT_TRACE	= 0x00001000,
	/// �ڴ�
	AOT_MEM		= 0x00002000,
	/// ���������Ĵ�ӡ��Ϣ
	AOT_PRINT	= 0x00004000,
};

namespace aot{ namespace tt{

class ilog : public virtual interface_base
{
public:
	enum elog_file_rule{e_log_file_rule_day = 0, e_log_file_rule_month, e_log_file_rule_year};
	enum elog_output_flag {e_log_output_flag_none = 0, e_log_output_flag_console = 0x01, e_log_output_flag_file = 0x02};

	struct log_config_t
	{
		log_config_t()
		{
			rule = e_log_file_rule_day;
			file_path = "";
			prefix_fn = "";
			output_flag = e_log_output_flag_none;
			format = "";
			level = 0;
			max_file_size = 100*1024; /*100MB*/
		}
		/// rule: ��־���ɹ���: enum elog_file_rule
		int rule;	
		/// file_path: ��־����·��
		const char* file_path;
		/// prefix_fn: ��־�ļ���ǰ׺
		const char* prefix_fn;
		/// output_flag: ��־�����ʽ  enum log_type
		int output_flag;
		/**
		 * @format: -p: �߳�id;	
		 *			-t: ʱ��;	
		 *			-f: �ļ���,��
		 *			-l: ��־��𼰵ȼ�
		 */
		const char* format;
		/// level: ��־����
		int	level;
		/// KB
		unsigned int max_file_size;
	};
protected:
	virtual ~ilog(){;}
public:
	virtual bool query_interface(void** out, const char* key){return false;}
	virtual const char* interface_name(){return "ilog";}
public:
	virtual bool config(const log_config_t* cfg) = 0;
	virtual bool write_log(int level, const char* data, size_t len) = 0;
	virtual bool write_log2 (int level, const char* fn, int line, const char* data, size_t len) = 0;
	virtual bool is_enable_log(int level) {return false;}
	virtual bool global_config(const log_config_t* cfg, const char* def_cfg_fn = NULL) = 0;
	virtual bool config_from_file (const char* node_name, const char* fn = NULL) = 0;
	virtual bool global_config_from_file(const char* node_name, const char* fn = NULL) = 0;
};

class log_helper
{
	enum { max_buf_size = 30*1024 };
public:
	log_helper ()
	{
		this->need_destroy_ = false;
		this->log_obj_ = NULL;
	}

	~log_helper ()
	{
		if( this->need_destroy_ && this->log_obj_ )
			this->log_obj_->destroy();
	}

	static log_helper*& instance()
	{
		return xy::singleton<log_helper, xy::thread_mutex>::instance();
	}

	void destroy()
	{
		/// ֻ������־����,��������log_helper�ĵ�������
		if( this->need_destroy_ && this->log_obj_ )
			this->log_obj_->destroy();

		this->need_destroy_ = false;
		this->log_obj_ = NULL;
	}

	void attach (ilog* p)
	{
		if( this->need_destroy_ && this->log_obj_ )
			this->log_obj_->destroy();

		this->need_destroy_ = true;
		this->log_obj_ = p;
	}

	ilog* detach ()
	{
		ilog* p = this->log_obj_;
		this->need_destroy_ = false;
		return p;
	}

	void reset (ilog* p)
	{
		if( this->need_destroy_ && this->log_obj_ )
			this->log_obj_->destroy();

		this->log_obj_ = p;
		this->need_destroy_ = false;
	}

	ilog* get() { return this->log_obj_; }

	void log(int level, const char* fmt, ...)
	{
		if( is_enable_log(level) ) {

			char buf[max_buf_size];
			int len = 0;

			va_list vl;
			va_start( vl, fmt );
			len = ::_vsnprintf( buf, sizeof(buf)-1, fmt, vl );
			va_end(vl);

			buf[sizeof(buf)-1] = 0;

			if( len < 1 || len > sizeof(buf)-1 )
				return;

			this->log_obj_->write_log(level, buf, len);
		}
	}

	void log2(int level, const char* fn, int line, const char* fmt, ...)
	{
		if( is_enable_log(level) ) {

			char buf[max_buf_size];
			int len = 0;

			va_list vl;
			va_start( vl, fmt );
			len = ::_vsnprintf( buf, sizeof(buf)-1, fmt, vl );
			va_end(vl);

			buf[sizeof(buf)-1] = 0;

			if( len < 1 || len > sizeof(buf)-1 )
				return;

			this->log_obj_->write_log2(level, fn, line, buf, len);
		}
	}

	void hex_dump(int level, const char* fn, int line, const char* prompt_msg, const void* data, int len)
	{
		/// hex code table
		static const char hex_code[] = "0123456789ABCDEF";
		/// default prompt message
		static const char dump_msg[] = "hex dump: ";
		
		if( is_enable_log(level) )
		{
			char buf[max_buf_size];
			int i = 0, n = 0;

			/// the prompt_msg is valid ?
			if( prompt_msg && prompt_msg[0] )
			{
				/// if prompt_msg >= 64, cut it !
				for( i = 0; i < 64; i++) {
					if( 0 == (buf[i] = prompt_msg[i]) ) {
						n = i; break;
					}
				}

				if( buf[i] ) {
					/// in this case, prompt_msg is too long
					n = i;
					buf[n++] = ':';
					buf[n++] = ' ';
				}
			}
			else {
				/// used default prompt message
				strcpy(buf, dump_msg);
				n = sizeof(dump_msg) - 1;
			}			

			for( i = 0; i < len && n < max_buf_size - 4; ++i) {

				buf[ n++ ] = hex_code[ ((unsigned char*)data)[i] & 0x0f ];
				buf[ n++ ] = hex_code[ (((unsigned char*)data)[i] & 0xf0) >> 4 ];
				buf[ n++ ] = ' ';
			}

			buf[n] = 0;
			this->log_obj_->write_log2(level, fn, line, buf, n);
		}
	}

	void config(const ilog::log_config_t* cfg)
	{
		if( this->log_obj_  && cfg )
		{
			this->log_obj_->config(cfg);
		}
	}

	bool is_enable_log( int level)
	{
		if( this->log_obj_ )
		{
			return this->log_obj_->is_enable_log(level);
		}
		return false;
	}

	void global_config( const aot::tt::ilog::log_config_t* cfg, const char* def_cfg_fn = NULL)
	{
		if( this->log_obj_ )
		{
			this->log_obj_->global_config(cfg, def_cfg_fn);
		}
	}

	void config_from_file ( const char* node_name, const char* fn = NULL)
	{
		if( this->log_obj_ )
		{
			this->log_obj_->config_from_file(node_name, fn);
		}
	}

	void global_config_from_file(const char* node_name, const char* fn = NULL)
	{
		if( this->log_obj_ )
		{
			this->log_obj_->global_config_from_file(node_name, fn);
		}
	}
private:
	ilog* log_obj_;
	bool need_destroy_;
};

}} /// end namespace aot/tt

#	ifdef __USING_AOT_LOG__

/// ����һЩ��־��, ��д��־����

/** aot_log_init
  *	��ʼ����־
  *	@x:		aot::tt::ilogָ��
  */
#	define aot_log_init(x)		\
	do{ \
	aot::tt::log_helper::instance()->attach(x); \
	}while(0)	\

#	define aot_log_destroy()		\
	do{ \
	aot::tt::log_helper::instance()->destroy(); \
	}while(0)	\

/** aot_log_cfg
  *	������־ѡ��
  *	@cfg:	aot::tt::ilog::log_config_t �ṹָ��
  */
#	define aot_log_cfg(cfg)		\
	do{ \
	aot::tt::log_helper::instance()->config(cfg);	\
	}while(0)	\

#	define aot_log_cfg_from_file(node_name, fn)		\
	do{ \
	aot::tt::log_helper::instance()->config_from_file(node_name, fn);	\
	}while(0)	\

/** aot_log
 *	�Լ�ָ�����ͼ�����д��־
 *	@level:	int, ��־����
 *	@fmt:	const char*, ��ʽ������
 */
#	define aot_log2(type_and_level, fmt, ...)		\
	do{ \
	aot::tt::log_helper::instance()->log2( type_and_level, __FILE__, __LINE__, fmt, __VA_ARGS__ );	\
	}while(0)	\

/** aot_log_debug
 *	��������־(AOT_DEBUG)
 *	@level:	int, ��־����
 *	@fmt:	const char*, ��ʽ������
 */
#	define aot_log_debug(level, fmt, ...)		\
	do{ \
	aot::tt::log_helper::instance()->log2( AOT_DEBUG | level, __FILE__, __LINE__, fmt, __VA_ARGS__ );	\
	}while(0)	\

/** aot_log_hex_dump
 *	��ӡ16������������־(AOT_DUMP)
 *	@level:	int, ��־����
 *	@prompt_msg:	const char*, �ڴ�ӡ����֮ǰ,��Ҫ��ʾ��ǰ׺��Ϣ(��Ϊ""��NULL)
 *	@data:	void*, ����ָ��
 *	@len:	int, ���ݳ���
 */
#	define aot_log_hex_dump(level, prompt_msg, data, len)		\
	do{ \
	aot::tt::log_helper::instance()->hex_dump( AOT_DUMP | level, __FILE__, __LINE__, prompt_msg, data, len);	\
	}while(0)	\

/** aot_log_error
 *	��������־(AOT_ERROR)
 *	@level:	int, ��־����
 *	@fmt:	const char*, ��ʽ������
 */
#	define aot_log_error(level, fmt, ...)		\
	do{ \
	aot::tt::log_helper::instance()->log2( AOT_ERROR | level, __FILE__, __LINE__, fmt, __VA_ARGS__ );	\
	}while(0)	\

/** aot_log_bug
 *	BUG����־(AOT_BUG)
 *	@level:	int, ��־����
 *	@fmt:	const char*, ��ʽ������
 */
#	define aot_log_bug(level, fmt, ...)		\
	do{ \
	aot::tt::log_helper::instance()->log2( AOT_BUG | level, __FILE__, __LINE__, fmt, __VA_ARGS__ );	\
	}while(0)	\

/** aot_log_bug
 *	MEM�ڴ�����־(AOT_BUG)
 *	@level:	int, ��־����
 *	@fmt:	const char*, ��ʽ������
 */
#	define aot_log_mem(level, fmt, ...)		\
	do{ \
	aot::tt::log_helper::instance()->log2( AOT_MEM | level, __FILE__, __LINE__, fmt, __VA_ARGS__ );	\
	}while(0)	\

#	define aot_log_print(fmt, ...)		\
	do{ \
	aot::tt::log_helper::instance()->log2( AOT_PRINT | AOT_LM_PRINT, __FILE__, __LINE__, fmt, __VA_ARGS__ );	\
	}while(0)	\

#include <string>

class log_trace_helper
{
public:
	log_trace_helper(const char* msg, int level = AOT_LM_TRACE)
	{
		this->msg_ = msg;
		this->level_ = level;
		aot_log2(AOT_TRACE | this->level_, "enter into %s", msg);
	}

	~log_trace_helper()
	{
		aot_log2(AOT_TRACE | this->level_, "leaving %s", this->msg_ ? this->msg_ : ".");
	}
private:
	const char* msg_;
	int	level_;
};

/** aot_log_trace
 *	���ٺ������ô�������־
 *	@msg:	const char*, ��ʾ��Ϣ
 */
#	define aot_log_trace(level, msg)		log_trace_helper	__tmp_log_trace_helper_obj__(msg, level)

#else

#	define	aot_log		__noop
#	define	aot_log_init(x, cfg)			((void)0)
#	define	aot_log_debug(level, fmt, ...)	((void)0)
#	define	aot_log_error(level, fmt, ...)	((void)0)
#	define	aot_log_bug(fmt, ...)			((void)0)
#	define	aot_log_trace(msg)				((void)0)
#	define	aot_log_hex_dump(level, prompt_msg, data, len)		((void)0)
#	define	aot_log_print(fmt, ...)			((void)0)

#endif /// #define __USING_AOT_LOG__

#pragma warning(default:4996)

#endif /// __LOG_INTERFACE_H__


