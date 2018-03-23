/** Copyright (c) 2008-2009
 * All rights reserved.
 * 
 * 文件名称:	buffer_interface.h
 * 摘	 要:	接口类
 * 
 * 当前版本:	1.0
 * 作	 者:	范涛涛
 * 操	 作:	新建
 * 完成日期:	2009年11月17日
 */
#ifndef __BUFFER_INTERFACE_H__
#define __BUFFER_INTERFACE_H__

#include "interface_base.h"

namespace aot{ namespace tt{

class ibuffer : public interface_base
{
protected:
	virtual ~ibuffer (){;}
public:
	virtual bool query_interface (void** out, const char* key) {return false;}
	virtual const char* interface_name () {return "ibuffer";}
public:
	virtual bool create(size_t size) = 0;
	/** enlarge_space()
	 *  增大空闲缓冲区大小, 
	 *  @new_size: 新的空闲缓冲区大小
	 *  1. 如果当前space() >= @new_size, 则不进行任何操作,直接返回
	 *  2. 否则, 新的空闲缓冲区大小 == @new_size, 并且读写指针及数据均原样保留
	 */
	virtual bool enlarge_space(size_t new_size) = 0;
	virtual char* base() = 0;
	virtual size_t size() = 0;
	virtual size_t length() = 0;
	virtual size_t space() = 0;
	virtual void rd_ptr(size_t n) = 0;
	virtual void rd_ptr(char* p) = 0;
	/// 注意: 如果没有分配内存,rd_ptr()可能返回NULL, 使用该接口前,请确定是否需要调用length()来判断是否有数据
	virtual char* rd_ptr() = 0;
	virtual void wr_ptr(size_t n) = 0;
	virtual void wr_ptr(char* p) = 0;
	virtual char* wr_ptr() = 0;
	virtual void recycle() = 0;
	/// copy数据后, 写指针位置自动前进@data_len
	virtual bool copy_from(const char* data, size_t data_len) = 0;
	virtual bool clone(aot::tt::ibuffer** out) = 0;
	virtual void attach(char* buf, size_t buf_len, size_t rd_pos = 0, size_t wr_pos = 0) = 0;
	virtual char* detach() = 0;
	virtual void set_str_eof() = 0;
};

class buffer_impl : public aot::tt::ibuffer
{
public:
	buffer_impl()
	{
		need_destroy_ = true;
		buf_ = NULL;
		size_ = 0;
		rd_pos_ = 0;
		wr_pos_ = 0;
	}
	virtual ~buffer_impl()
	{
		if( buf_ )
		{
			delete buf_;
			buf_ = NULL;
		}
	}
public:
	virtual void destroy ()
	{
		if( need_destroy_ )
		{
			if(0 == dec_ref())
				delete this;
		}
	}
public:
	virtual bool create(size_t size)
	{
		if( buf_ )
		{
			delete buf_;
			buf_ = NULL;
		}

		buf_ = new char[size];
		if( NULL == buf_ )
			return false;

		recycle_i();
		this->size_ = size;
		return true;
	}
	virtual bool enlarge_space(size_t new_size)
	{
		if( new_size <= space() )
			return true;

		char* p = new char[new_size + this->wr_pos_];
		if( NULL == p )
			return false;

		if( this->buf_ )
		{
			if( this->wr_pos_ > 0 )
				memcpy(p, this->buf_, this->wr_pos_);

			delete this->buf_;
		}

		this->size_ = new_size + this->wr_pos_;
		this->buf_ = p;
		return true;
	}
	virtual char* base(){ return this->buf_; }
	virtual size_t size(){ return this->size_; }
	virtual size_t length(){ return this->wr_pos_ - this->rd_pos_; }
	virtual size_t space(){ return this->size_ - this->wr_pos_; }
	virtual void rd_ptr(size_t n)
	{
		this->rd_pos_ += n;
		if( this->rd_pos_ > this->size_ )
		{
			this->rd_pos_ = this->size_;
			//aot_log_error(AOT_LM_ALERT, "buffer_impl::rd_ptr param [n=%d] out of range [buf_size=%d]", n, size_);
		}
	}
	virtual void rd_ptr(char* p)
	{
		if( NULL == p )
		{
			this->rd_pos_ = 0;
			return;
		}
		long n = static_cast<long>(p - this->buf_);
		if( n < 0 )
		{
			//aot_log_error(AOT_LM_ALERT, "buffer_impl::rd_ptr param [p=%p] out of range [buf_=%p]", p, buf_);
			n = 0;
		}
		this->rd_pos_ = (size_t)n;
	}
	virtual char* rd_ptr(){ return this->buf_ + this->rd_pos_; }
	virtual void wr_ptr(size_t n)
	{
		this->wr_pos_ += n;
		if( this->wr_pos_ > this->size_ )
		{
			this->wr_pos_ = this->size_;
			//aot_log_error(AOT_LM_ALERT, "buffer_impl::wr_ptr param [n=%d] out of range [buf_size=%d]", n, size_);
		}
	}
	virtual void wr_ptr(char* p)
	{
		if( NULL == p )
		{
			this->wr_pos_ = 0;
			return;
		}
		long n = static_cast<long>(p - this->buf_);
		if( n < 0 )
		{
			//aot_log_error(AOT_LM_ALERT, "buffer_impl::wr_ptr param [p=%p] out of range [buf_=%p]", p, buf_);
			n = 0;
		}
		this->wr_pos_ = (size_t)n;
	}
	virtual char* wr_ptr(){ return this->buf_ + this->wr_pos_; }
	virtual void recycle(){ recycle_i(); }
	virtual bool copy_from(const char* data, size_t data_len)
	{
		if( data_len > space() )
		{
			if( !enlarge_space(data_len) )
				return false;
		}
		memcpy(wr_ptr(), data, data_len);
		this->wr_pos_ += data_len;

		return true;
	}
	virtual bool clone(aot::tt::ibuffer** out)
	{
		buffer_impl* p = new buffer_impl();

		if( !p->create(this->size_) )
		{
			p->destroy();
			return false;
		}
		p->rd_pos_ = this->rd_pos_;
		p->wr_pos_ = this->wr_pos_;
		if( this->wr_pos_ > 0 )
		{
			memcpy(p->buf_, this->buf_, this->wr_pos_);
		}

		*out = (aot::tt::ibuffer*)p;
		return true;
	}
	virtual void attach(char* buf, size_t buf_len, size_t rd_pos = 0, size_t wr_pos = 0)
	{
		if( this->buf_ )
		{
			delete this->buf_;
			this->buf_ = NULL;
		}
		this->buf_ = buf;
		this->size_ = buf_len;
		this->rd_pos_ = rd_pos;
		this->wr_pos_ = wr_pos;
	}
	virtual char* detach()
	{
		char* r = this->buf_;
		this->buf_ = NULL;
		recycle_i();
		return r;
	}
	virtual void set_str_eof()
	{
		if( this->buf_ && space() > 0 )
			this->buf_[this->wr_pos_] = 0;
	}
public:
	void need_destroy(bool b){ this->need_destroy_ = b; }
private:
	void recycle_i()
	{
		rd_pos_ = 0;
		wr_pos_ = 0;
	}
private:
	char* buf_;
	size_t size_;
	size_t rd_pos_;
	size_t wr_pos_;
	bool need_destroy_;
};

}}
#endif /// __BUFFER_INTERFACE_H__