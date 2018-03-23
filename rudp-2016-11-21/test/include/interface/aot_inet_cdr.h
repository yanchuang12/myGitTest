/** Copyright (c) 2008-2009
 * All rights reserved.
 * 
 * 文件名称:	aot_inet_cdr.h  
 * 摘	 要:	网络数据整编/解编
 * 
 * 当前版本：	1.0
 * 作	 者:	范涛涛(FTT)
 * 操	 作:	新建
 * 完成日期:	2010年03月12日
 */

#ifndef __AOT_INET_CDR_H__
#define __AOT_INET_CDR_H__

#include <interface/aot_inet_interface.h>
#include <assert.h>
#include <string>

namespace aot{ namespace inet{

inline bool __aot_is_big_endian()
{
	/// 如果低地址存储高位数据, 则 是big_endian;
	unsigned short i = 0x1122;
	return (*((unsigned char*)(&i)) == 0x11) ? true : false;
}

inline
aot_uint64_t __aot_ntohll(aot_uint64_t val)
{
	if( !__aot_is_big_endian() )
	{
		return (((aot_uint64_t)htonl((int)((val << 32) >> 32))) << 32) | (unsigned int)htonl((int)(val >> 32));
	}
	else
	{
		return val;
	}
}

inline
aot_uint64_t __aot_htonll(aot_uint64_t val)
{
	if( !__aot_is_big_endian() )
	{
		return (((aot_uint64_t )htonl((int)((val << 32) >> 32))) << 32) | (unsigned int)htonl((int)(val >> 32));
	}
	else
	{
		return val;
	}
}

class cdr_reader
{
public:
	cdr_reader();
	~cdr_reader();
public:
	aot_uint8_t read_1();
	aot_uint16_t read_2(bool to_host_order = true);
	aot_uint32_t read_4(bool to_host_order = true);
	aot_uint64_t read_8(bool to_host_order = true);
	aot_nbrnum_t read_nbrnum();
	bool skip_str();
	bool read_str(/* in/out */aot::inet::aot_string_t* s);
	bool read_str(/* in/out */std::string* s);
	bool read_encode_data(aot::inet::aot_encode_data_t* p);
	bool read_bin_data(aot::inet::aot_bin_data_t* s);
	bool read_date_time(aot::inet::date_time_t* dt);
	///  获取数组元素个数
	aot_uint32_t read_array_elem_num(bool to_host_order = true);
	/// 调用read_*_array之前, 要先调用read_array_elem_num 获取数组元素个数
	bool read_1_array(void* buf, aot_uint32_t elem_num);
	bool read_2_array(void* buf, aot_uint32_t elem_num, bool to_host_order = true);
	bool read_4_array(void* buf, aot_uint32_t elem_num, bool to_host_order = true);
public:
	void recyle();
	void set_buf(char* buf, aot_uint32_t size);
	char* get_buf();
	aot_uint32_t get_curr_pos();
	void set_curr_pos(aot_uint32_t v);
	char* next_rd_ptr();
private:
	void recyle_i(char* buf, aot_uint32_t size);
	bool read_array_i(void* buf, aot_uint32_t elem_num, aot_uint32_t char_len, bool to_host_order);
private:
	char* buf_;
	aot_uint32_t buf_size_;
	aot_uint32_t curr_pos_;
};

class cdr_writer
{
public:
	cdr_writer();
	~cdr_writer();
public:
	bool write_1(aot_uint8_t v);
	bool write_2(aot_uint16_t v, bool to_net_order = true);
	bool write_4(aot_uint32_t v, bool to_net_order = true);
	bool write_8(aot_uint64_t v, bool to_net_order = true);
	bool write_nbrnum(aot_nbrnum_t v);
	bool write_str(aot::inet::aot_string_t* s);
	bool write_str(std::string* s);
	bool write_str(const char* buf, aot_uint32_t len);
	bool write_encode_data(aot::inet::aot_encode_data_t* p);
	bool write_bin_data(aot::inet::aot_bin_data_t* s);
	bool write_date_time(aot::inet::date_time_t* dt);
	///  写入数组元素个数
	bool write_array_elem_num(aot_uint32_t elem_num, bool to_net_order = true);
	/// 调用write_*_array之前, 要先调用write_array_elem_num 写入数组元素个数
	bool write_1_array(const void* buf, aot_uint32_t elem_num);
	bool write_2_array(const void* buf, aot_uint32_t elem_num, bool to_net_order = true);
	bool write_4_array(const void* buf, aot_uint32_t elem_num, bool to_net_order = true);
public:
	void recyle();
	void set_buf(char* buf, aot_uint32_t len);
	char* get_buf();
	aot_uint32_t get_curr_pos();
	void set_curr_pos(aot_uint32_t v);
	char* next_wr_ptr();
private:
	void recyle_i(char* buf, aot_uint32_t size);
	bool write_array_i(const void* buf, aot_uint32_t elem_num, aot_uint32_t elem_len, bool to_net_order);
private:
	char* buf_;
	aot_uint32_t buf_size_;
	aot_uint32_t curr_pos_;
};

}} /// end namespace aot/inet

#include "aot_inet_cdr.inl"

#endif /// __AOT_INET_CDR_H__