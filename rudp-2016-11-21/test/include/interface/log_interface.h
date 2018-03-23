/** Copyright (c) 2008-2009
 * All rights reserved.
 * 
 * 文件名称:	log_interface.h  
 * 摘	 要:	log接口基类 及 相关包装类 及 日志宏
 * 
 * 当前版本:	1.0
 * 作	 者:	范涛涛(FTT)
 * 操	 作:	新建
 * 完成日期:	2009年7月19日
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
	/**   日志掩码:
	 *  
	 *    07 06 05 04 03 02 01 00 07 06 05 04 03 02 01 00 07 06 05 04 03 02 01 00 07 06 05 04 03 02 01 00
	 *   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	 *   |         字节          |         字节          |         字节          |         字节          |
	 *   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	 *   |      日志类型:                                         |TR|DU|DE|ER|BU|         日志等级      |
	 *   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	 */
	/* ---------------- log level bit mask ---------------- */
	AOT_LM_CRASH	= 0x01,
	/// 程序bug ---与AOT_BUG配合使用
	AOT_LM_BUG		= 0x02,
	/// 
	AOT_LM_PRINT	= 0x08,
	/// 紧急事件信息
	AOT_LM_EMERG	= 0x09,
	/// 警惕信息
	AOT_LM_ALERT	= 0x0A,
	/// 临界条件信息
	AOT_LM_CRIT		= 0x0B,
	/// 错误信息 ---与AOT_ERROR配合使用
	AOT_LM_ERROR	= 0x0C,
	/// 警告信息
	AOT_LM_WARN		= 0x0D,
	/// 非错误信息, 而是可能需要特别处理的信息
	AOT_LM_NOTICE	= 0x0E,
	/// 提示信息
	AOT_LM_INFO		= 0x0F,
	/// 详细的调试信息
	AOT_LM_DEBUG	= 0x10,
	/// 函数调用入/出口跟踪信息
	AOT_LM_TRACE	= 0x11,
	/// don't use __AOT_LOG_MAX_VALUE__
	__AOT_LM_MAX_VALUE__	= 0xEF,

	/* ---------------- log type bit mask ---------------- */

	/// 程序bug类日志 (建议使用级别范围: AOT_LM_BUG ---> AOT_LM_CRASH)
	AOT_BUG		= 0x00000100,
	/// 错误类日志 (建议使用级别范围: AOT_LM_ERROR ---> AOT_LM_EMERG)
	AOT_ERROR	= 0x00000200,
	/// 诊断类日志 (建议使用级别范围: AOT_LM_DEBUG ---> AOT_LM_WARN)
	AOT_DEBUG	= 0x00000400,
	/// 打印数据   (建议使用级别范围: AOT_LM_DEBUG ---> AOT_LM_CRASH)
	AOT_DUMP    = 0x00000800,
	/// 函数调用入/出口跟踪 (建议使用级别范围: AOT_LM_TRACE ---> AOT_LM_CRASH)
	AOT_TRACE	= 0x00001000,
	/// 内存
	AOT_MEM		= 0x00002000,
	/// 程序正常的打印信息
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
		/// rule: 日志生成规则: enum elog_file_rule
		int rule;	
		/// file_path: 日志保存路径
		const char* file_path;
		/// prefix_fn: 日志文件名前缀
		const char* prefix_fn;
		/// output_flag: 日志输出方式  enum log_type
		int output_flag;
		/**
		 * @format: -p: 线程id;	
		 *			-t: 时间;	
		 *			-f: 文件名,行
		 *			-l: 日志类别及等级
		 */
		const char* format;
		/// level: 日志级别
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
		/// 只销毁日志对象,不必销毁log_helper的单例对象
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

/// 定义一些日志宏, 简化写日志调用

/** aot_log_init
  *	初始化日志
  *	@x:		aot::tt::ilog指针
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
  *	配置日志选项
  *	@cfg:	aot::tt::ilog::log_config_t 结构指针
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
 *	自己指定类型及级别写日志
 *	@level:	int, 日志级别
 *	@fmt:	const char*, 格式化参数
 */
#	define aot_log2(type_and_level, fmt, ...)		\
	do{ \
	aot::tt::log_helper::instance()->log2( type_and_level, __FILE__, __LINE__, fmt, __VA_ARGS__ );	\
	}while(0)	\

/** aot_log_debug
 *	调试类日志(AOT_DEBUG)
 *	@level:	int, 日志级别
 *	@fmt:	const char*, 格式化参数
 */
#	define aot_log_debug(level, fmt, ...)		\
	do{ \
	aot::tt::log_helper::instance()->log2( AOT_DEBUG | level, __FILE__, __LINE__, fmt, __VA_ARGS__ );	\
	}while(0)	\

/** aot_log_hex_dump
 *	打印16进制数据类日志(AOT_DUMP)
 *	@level:	int, 日志级别
 *	@prompt_msg:	const char*, 在打印数据之前,需要显示的前缀信息(可为""或NULL)
 *	@data:	void*, 数据指针
 *	@len:	int, 数据长度
 */
#	define aot_log_hex_dump(level, prompt_msg, data, len)		\
	do{ \
	aot::tt::log_helper::instance()->hex_dump( AOT_DUMP | level, __FILE__, __LINE__, prompt_msg, data, len);	\
	}while(0)	\

/** aot_log_error
 *	错误类日志(AOT_ERROR)
 *	@level:	int, 日志级别
 *	@fmt:	const char*, 格式化参数
 */
#	define aot_log_error(level, fmt, ...)		\
	do{ \
	aot::tt::log_helper::instance()->log2( AOT_ERROR | level, __FILE__, __LINE__, fmt, __VA_ARGS__ );	\
	}while(0)	\

/** aot_log_bug
 *	BUG类日志(AOT_BUG)
 *	@level:	int, 日志级别
 *	@fmt:	const char*, 格式化参数
 */
#	define aot_log_bug(level, fmt, ...)		\
	do{ \
	aot::tt::log_helper::instance()->log2( AOT_BUG | level, __FILE__, __LINE__, fmt, __VA_ARGS__ );	\
	}while(0)	\

/** aot_log_bug
 *	MEM内存类日志(AOT_BUG)
 *	@level:	int, 日志级别
 *	@fmt:	const char*, 格式化参数
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
 *	跟踪函数调用次序类日志
 *	@msg:	const char*, 提示信息
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


