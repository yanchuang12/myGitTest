/** Copyright (c) 2015-2016
 * All rights reserved.
 * 
 * 文件名称:	aot_string.h  
 * 摘	 要:	常用数据结构封装,比标准库简洁.快
 * 
 * 当前版本：	1.0
 * 作	 者:	范涛涛(FTT)
 * 操	 作:	新建
 * 完成日期:	2015年05月11日
*/

#ifndef __AOT_STD_STRING_201505114_H__
#define __AOT_STD_STRING_201505114_H__

#include "aot_std_typedef.h"
#include "aot_alloc.h"
#include <string.h>
#include <stdio.h>

namespace aot_std{

template<class ALLOC>
class string
{
#define AOT_STD_STR_EOF		('\0')
public:
	string() { __init(); }
	string(char c)
	{
		this->size_ = 8;
		this->data_ = alloc(this->size_);
		
		this->data_[0] = c;
		this->data_[1] = AOT_STD_STR_EOF;
		this->len_ = 1;
	}
	string(const char* s)
	{ 
		__init(); 
		assign(s); 
	}
	string(const string& s)
	{
		__init(); 
		assign(s.data_, s.len_);
	}

	~string(){ if( this->data_){ dealloc(this->data_); } }
public:
	const char* c_str() const { return this->size_ == 0 ? "" : this->data_; }
	char* data(){ return this->data_; }
	aot_uint_t length() const { return this->len_; }
	void length(aot_uint_t len){ this->len_ = len; }
	/// buf_size: 返回内存大小(总存储空间)
	aot_uint_t buf_size() const { return this->size_; }
	operator const char*() const { return this->size_ == 0 ? "" : this->data_; }
	char operator[] (aot_uint_t pos) const { return this->data_[pos];}
	char& operator[] (aot_uint_t pos){ return this->data_[pos];}
	string& operator= (char c)
	{
		__resize(8);
		this->data_[0] = c;
		this->data_[1] = AOT_STD_STR_EOF;
		this->len_ = 1;
		return (*this);
	}
	string& operator= (const char* s)
	{
		assign(s);
		return (*this);
	}
	string& operator= (const string& s)
	{
		assign(s->data_, s->len_);
		return (*this);
	}
	string& operator += (char c)
	{
		append(c);
		return (*this);
	}
	string& operator += (const char* s)
	{
		append(s);
		return (*this);
	}
	string& operator += (const string& s)
	{
		append(s);
		return (*this);
	}
/*
	bool operator == (const string& s){ return (0 == compare(this->data_, s.data_));}
	bool operator == (const char* s){ return (0 == compare(this->data_, s));}
	bool operator != (const string& s){ return (0 != compare(this->data_, s.data_));}
	bool operator != (const char* s){ return (0 != compare(this->data_, s));}
	bool operator > (const string& s){ return ( compare(this->data_, s.data_) > 0 );}
	bool operator > (const char* s){ return ( compare(this->data_, s) > 0 );}
	bool operator < (const string& s){ return ( compare(this->data_, s.data_) < 0 );}
	bool operator < (const char* s){ return ( compare(this->data_, s) < 0 );}
*/
	void clear()
	{
		this->len_ = 0;
		if( this->size_ > 0 ) {
			this->data_[0] = AOT_STD_STR_EOF;
		}
	}
	void release_buf()
	{
		if( this->data_) { 
			dealloc(this->data_); 
			this->data_ = NULL;
		}
		this->len_ = 0; this->size_ = 0; 
	}
	/** resize()
	 *  增大缓冲区大小, 
	 *  @new_size: 新的缓冲区大小
	 *  1. 如果当前buf_size() >= @new_size, 则不进行任何操作,直接返回
	 *  2. 否则, 新的缓冲区大小 == @new_size, 并且len及数据均原样保留
	 */
	void resize(aot_uint_t new_size)
	{
		if( this->size_ >= new_size )
		{
			return;
		}
		std::string
		char* p = alloc(new_size);
		if( this->len_ > 0 )
		{
			__fast_strcpy(p, this->data_, this->len_);
		}
		else
		{
			p[0] = AOT_STD_STR_EOF;
		}
		dealloc(this->data_);
		this->data_ = p;
		this->size_ = new_size;
	}

	/// 提供attch() 和 deattch() 接口, 是为了方便外部更高效的构建string.这里应该特别注意: 在attch后 内存的分配释放等操作
	void attch(char* s, aot_uint_t len)
	{
		if( this->data_ == s )
		{
			assert(0);
			return;
		}
		if( this->data_ ){ dealloc(this->data_);}
		this->data_ = s; this->len_ = len; this->size_ = len;
	}
	char* deattch()
	{
		char* p = this->data_;
		this->len_ = 0; this->size_ = 0; this->data_ = NULL;
		return p;
	}
	void assign(const char* s)
	{
		aot_uint_t len;
		if( s == NULL ) { len = 0; }
		else{ len = (aot_uint_t)strlen(s); }

		assign(s, len);
	}
	void assign(const char* s, aot_uint_t len)
	{
		if( s == NULL || len == 0 ) {
			if( this->size_ > 0 ) {
				this->data_[0] = AOT_STD_STR_EOF;
			}
			this->len_ = 0;
			return;
		}

		if( this->data_ == s )
		{
			assert(0);
			return;
		}

		__resize(len + 1);
		__fast_strcpy(this->data_, s, len);
		this->len_ = len;
	}
	void format(const char* fmt, ...)
	{
		int len = 0;

		va_list vl;
		va_start( vl, fmt );

		len = ::_vscprintf(fmt, vl);

		if( len > 0)
		{
			__resize(len + 1);
			this->len_ = ::_vsnprintf(this->data_, len, fmt, vl );
			this->data_[this->len_] = AOT_STD_STR_EOF;
		}
		else
		{
			clear();
		}

		va_end(vl);
		return;
	}
	void append(char c)
	{
		resize(this->len_ + 1 + 1);
		this->data_[this->len_++] = c;
		this->data_[this->len_] = AOT_STD_STR_EOF;
	}
	void append(const char* s)
	{
		aot_uint_t len;
		if( s == NULL ) { return; }

		aot_uint_t len = (aot_uint_t)strlen(s);
		append(s, len);
	}
	void append(const char* s, aot_uint_t len)
	{
		if( NULL == s || len == 0 ){return;}
		resize(this->len_ + len + 1);
		__fast_strcpy(this->data_ + this->len_, s, len);
		this->len_ += len;
	}
	static int compare(const char* s, const char* d)
	{
		if( !s && !d ){ return 0;}
		if( !s ) {  return *d ? -1 : 0; }
		else if( !d ) { return *s ? 1 : 0; }

		int r = 0;
		for( ; *s && *d ; s++, d++ )
		{
			r = *(unsigned char*)s - *(unsigned char*)d;
			if( r > 0) { return 1;}
			else if( r < 0) { return -1;}
		}
		if( !(*s) && !(*d) ){ return 0;}
		else if( (*s) && !(*d) ){ return 1; }
		else{ return -1;}
	}
private:
	/// 因为string可能是一个大量创建的对象, 所以为了减少内存占用, 这里的ALLOC要求提供静态成员函数
	inline char* alloc(aot_uint_t size) { return (char*)ALLOC::alloc(size); }
	inline void dealloc(void* p) { if( p ) {ALLOC::dealloc(p);} }
	inline void __init(){ this->data_ = NULL; this->len_ = 0; this->size_ = 0; }
	void __resize(aot_uint_t size)
	{
		if( this->size_ < size ) {
			this->size_ = size;
			dealloc(this->data_);
			this->data_ = alloc(this->size_);
		}
	}

	void __fast_strcpy(char *d, const char *s, aot_uint_t s_len)
	{
		memcpy(d, s, s_len);
		d[s_len] = AOT_STD_STR_EOF;
	}
	static aot_uint_t __strlen(const char *s)
	{
		if( NULL == s ){ return 0;}
		aot_uint_t len = 0;
		const char* p = s;
		while( *p++ != AOT_STD_STR_EOF )
		{
			len++;
		}
		return len;
	}
private:
	char* data_;
	aot_uint_t len_;
	aot_uint_t size_;
};


} /// end namespace aot_std

template<class ALLOC>
bool operator == (const aot_std::string<ALLOC>& s1, const aot_std::string<ALLOC>& s2)
{ 
	if( s1.length() != s2.length() )
	{
		return false;
	}
	return (0 == aot_std::string<ALLOC>::compare(s1.c_str(), s2.c_str()));
}

template<class ALLOC>
bool operator == (const aot_std::string<ALLOC>& s1, const char* s2)
{ 
	return (0 == aot_std::string<ALLOC>::compare(s1.c_str(), s2));
}

template<class ALLOC>
bool operator != (const aot_std::string<ALLOC>& s1, const aot_std::string<ALLOC>& s2)
{ 
	if( s1.length() != s2.length() )
	{
		return true;
	}
	return (0 != aot_std::string<ALLOC>::compare(s1.c_str(), s2.c_str()));
}

template<class ALLOC>
bool operator != (const aot_std::string<ALLOC>& s1, const char* s2)
{ 
	return (0 != aot_std::string<ALLOC>::compare(s1.c_str(), s2));
}

template<class ALLOC>
bool operator > (const aot_std::string<ALLOC>& s1, const aot_std::string<ALLOC>& s2)
{ 
	return ( aot_std::string<ALLOC>::compare(s1.c_str(), s2.c_str()) > 0 );
}

template<class ALLOC>
bool operator > (const aot_std::string<ALLOC>& s1, const char* s2)
{
	return ( aot_std::string<ALLOC>::compare(s1.c_str(), s2) > 0 );
}

template<class ALLOC>
bool operator < (const aot_std::string<ALLOC>& s1, const aot_std::string<ALLOC>& s2)
{ 
	return ( aot_std::string<ALLOC>::compare(s1.c_str(), s2.c_str()) < 0 );
}

template<class ALLOC>
bool operator < (const aot_std::string<ALLOC>& s1, const char* s2)
{ 
	return ( aot_std::string<ALLOC>::compare(s1.c_str(), s2) < 0 );
}

#endif /// __AOT_STD_STRING_201505114_H__
