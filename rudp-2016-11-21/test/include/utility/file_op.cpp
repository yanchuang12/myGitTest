// file_op.cpp : Implementation of file_op
#include "stdafx.h"
#include "file_op.h"

#pragma warning(disable:4996)

/////////////////////////////////////////////////////////////////////////////
namespace aot{ namespace tt{ namespace utility{

#define _IS_VALID(x) do{if(!hf_) return (x);}while(0)
#define _IS_VALID_NORETURN() (hf_)


file_op::file_op()
{
	fn_ = "";
	hf_ = NULL;
	mode_ = -1;
}

file_op::~file_op()
{
	close();
}

bool 
file_op::open(const char* fn, int mode)
{
    this->fn_ = fn;
	this->mode_ = mode;
	//以附加的方式打开读写文件文件
	if (mode == e_append)
	{
		this->hf_ = fopen(fn, "ab+");
	}
	//新建方式打开读写文件文件
	else if(mode == e_create) 
	{
		this->hf_ = fopen(fn, "wb+");	
	}
    else  //只读方式方式打开文件
	{
		this->hf_ = fopen(fn, "rb");
	}
	
	return this->hf_ != NULL;
}

bool 
file_op::is_valid()
{
	return NULL != this->hf_;
}

size_t 
file_op::read(char* buf, size_t len)
{
	_IS_VALID((size_t)-1);

	const int MAX_READ_LEN = 64*1024;
	size_t rd_len = 0;
	size_t n = 0;
	size_t total_rd_len = 0;
	while(1)
	{
		rd_len = (len - total_rd_len) >= MAX_READ_LEN ? MAX_READ_LEN : (len - total_rd_len);

		if( rd_len == 0 )
		{
			return total_rd_len;
		}

		n = fread(buf + total_rd_len, sizeof(char), rd_len, this->hf_);
		if( size_t(-1) == n )
			return -1;

		total_rd_len += n;

		if(n != rd_len)
		{
			return total_rd_len;
		}
		
	}
	return total_rd_len;

}

size_t 
file_op::write(const char* buf,  size_t len)
{
	_IS_VALID(-1);

	const size_t MAX_WRITE_LEN = 64*1024;
	size_t wt_len = 0;
	size_t n = 0;
	size_t total_wt_len = 0;

	while(1)
	{
		wt_len = (len - total_wt_len) >= MAX_WRITE_LEN ? MAX_WRITE_LEN : (len - total_wt_len);

		if( wt_len == 0 )
		{
			return total_wt_len;
		}

		n = fwrite((buf + total_wt_len), sizeof(char), wt_len, this->hf_);

		if( (size_t)-1 == n )
			return -1;
		
		total_wt_len += n;

		if(n != wt_len)
		{
			return total_wt_len;
		}
		
	}
	return total_wt_len;
}

bool  
file_op::clear()
{
	_IS_VALID(false);

	std::string fn = this->fn_;
	
	if( !close() )
		return false;

	return open(fn.c_str(), e_create);
}

bool  
file_op::seek(int offset, int origin)
{
	_IS_VALID(false);

	if ((origin !=SEEK_SET) && (origin !=SEEK_CUR) && (origin !=SEEK_END))
		origin = SEEK_SET;
	
	return 0 == fseek(this->hf_, offset, origin);
}

bool  
file_op::seek_to_end()
{	
	_IS_VALID(false);
	return 0 == fseek(this->hf_, 0, SEEK_END);
}

bool  
file_op::seek_to_begin()
{	
	_IS_VALID(false);
	return 0 == fseek(this->hf_, 0, SEEK_SET);
}

bool  
file_op::close()
{
	_IS_VALID(false);

	if(0 != fclose(this->hf_))
	{
		return false;
	}
	
	this->hf_ = NULL;
	this->fn_ = "";
	this->mode_ = -1;
	return true;
}

bool  
file_op::save()
{
	_IS_VALID(false);
	return 0 == fflush(this->hf_);
}

bool 
file_op::eof()
{
	_IS_VALID(false);
	return 0 != feof(this->hf_);
}

//-1: 失败
long 
file_op::tell()
{
	_IS_VALID(-1);
	return ftell(this->hf_);
}



}}} /* */
