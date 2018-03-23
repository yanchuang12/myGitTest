/** Copyright (c) 2008-2009
 * All rights reserved.
 * 
 * �ļ�����:	aot_inet_interface.h   
 * ժ	 Ҫ:	��װ����ͨѶ�Ĳ����ӿ�
 * 
 * ��ǰ�汾��	1.0
 * ��	 ��:	������(FTT)
 * ��	 ��:	�½�
 * �������:	20010��11��10��
 */
#ifndef __AOT_INET_INTERFACE_H__
#define __AOT_INET_INTERFACE_H__

#include <interface/aot_inet_define.h>
#include <string>

#define AOT_INET_IID_VAL(type)	aot_iid_aot_inet_##type

enum
{
	__aot_iid_aot_inet_begin_range__ = 0x10000,

	AOT_INET_IID_VAL(aot_buf_op_t),
	AOT_INET_IID_VAL(aot_hashmap_op_t),
	AOT_INET_IID_VAL(aio_pool_op_t),
	AOT_INET_IID_VAL(aio_evld_op_t),
	AOT_INET_IID_VAL(aio_channel_op_t),
	AOT_INET_IID_VAL(aio_acceptor_op_t),

	__aot_iid_aot_inet_end_range__ = 0x11000,
};

inline bool operator < (const aot_inet_addr_t& a1, const aot_inet_addr_t& a2)
{
	if( a1.ip < a2.ip ) return true;
	if( a1.ip == a2.ip ) return a1.port < a2.port;
	return false;
}

inline bool operator == (const aot_inet_addr_t& a1, const aot_inet_addr_t& a2)
{
	return a1.ip == a2.ip && a1.port == a2.port;
}

inline bool operator != (const aot_inet_addr_t& a1, const aot_inet_addr_t& a2)
{
	return !(a1 == a2);
}


namespace aot{ namespace inet{

#pragma warning(disable:4996)

struct aot_string_t
{
	enum{ min_size = 4};
	aot_string_t() : length(0)
	{
		data = "";
	}
	void clear(){ length = 0; data = "";}
	aot_uint32_t length;
	char* data;
	aot_uint32_t cdr_write_size(){ return length == 0 ? 4 : 4 + length + 1;}
};

struct aot_encode_data_t
{
	enum{ min_size = 4};
	aot_encode_data_t() : length(0)
	{
		data = NULL; key = 0;
	}
	void clear(){ length = 0; data = NULL;}
	aot_uint32_t length;
	char* data;
	aot_uint8_t key;
	aot_uint32_t cdr_write_size()
	{
		/// length(aot_uint32_t) + key(aot_uint8_t) + data(length �ֽ�) + '\0'(aot_uint8_t)
		return length == 0 ? 4 : 4 + 1 + length + 1;
	}
};


struct aot_bin_data_t
{
	aot_bin_data_t() : length(0), data(NULL)
	{
		;
	}
	static aot_uint32_t cdr_min_size(){return 4;}
	/// Ϊ�˷����stringת��, ĩβ��Ȼ���'\0'
	aot_uint32_t cdr_size(){ return length == 0 ? 4 : 4 + length + 1;}
	char* data;
	aot_uint32_t length;
};

struct date_time_t
{
	enum{size = 15};
	aot_uint64_t file_time; /// ����
	aot_uint16_t year;
	aot_uint8_t month;
	aot_uint8_t day;
	aot_uint8_t hour;
	aot_uint8_t minute;
	aot_uint8_t sec;

	date_time_t()
	{
		year = 0;
		month = day = hour = minute = sec = 0;
		file_time = 0;
	}
	void get_curr_time()
	{
		this->file_time = aot_gettimeofday();
		SYSTEMTIME tm;
		::GetLocalTime(&tm);
		this->year = tm.wYear;
		this->month = (aot_uint8_t)tm.wMonth;
		this->day = (aot_uint8_t)tm.wDay;
		this->hour = (aot_uint8_t)tm.wHour;
		this->minute = (aot_uint8_t)tm.wMinute;
		this->sec = (aot_uint8_t)tm.wSecond;
	}
	aot_int_t compare(date_time_t* p)
	{
		if( this->year > p->year ) return 1;
		if( this->year < p->year ) return -1;
		if( this->month > p->month ) return 1;
		if( this->month < p->month ) return -1;
		if( this->day > p->day ) return 1;
		if( this->day < p->day ) return -1;
		if( this->hour > p->hour ) return 1;
		if( this->hour < p->hour ) return -1;
		if( this->minute > p->minute ) return 1;
		if( this->minute < p->minute ) return -1;
		if( this->sec > p->sec ) return 1;
		if( this->sec < p->sec ) return -1;
		return 0;
	}
	aot_int_t compare_with_file_time(date_time_t* p)
	{
		if( this->file_time > p->file_time ) return 1;
		if( this->file_time < p->file_time ) return -1;
		return 0;
	}
};

/// ip, port : �����ֽ���
inline 
char* inet_addr_to_str(aot_uint32_t ip, aot_uint16_t port, char* buf)
{
	ip = ::htonl(ip);
	aot_uint8_t* p = (aot_uint8_t*)(&ip);
	sprintf(buf, "%d.%d.%d.%d:%d", 
		p[0], p[1], p[2], p[3],
		port);
	return buf;
}

inline 
std::string inet_addr_to_str(aot_uint32_t ip, aot_uint16_t port)
{
	ip = ::htonl(ip);
	aot_uint8_t* p = (aot_uint8_t*)(&ip);
	char buf[64];
	sprintf(buf, "%d.%d.%d.%d:%d", 
		p[0], p[1], p[2], p[3],
		port);
	return buf;
}

inline 
std::string inet_addr_to_str(aot_uint32_t ip)
{
	ip = ::htonl(ip);
	aot_uint8_t* p = (aot_uint8_t*)(&ip);
	char buf[64];
	sprintf(buf, "%d.%d.%d.%d", 
		p[0], p[1], p[2], p[3]);
	return buf;
}

inline 
std::string inet_addr_to_str(const aot_inet_addr_t* addr)
{
	return inet_addr_to_str(addr->ip, addr->port);
}

inline 
char* inet_addr_to_str(aot_uint32_t ip, char* buf)
{
	ip = ::htonl(ip);
	aot_uint8_t* p = (aot_uint8_t*)(&ip);
	sprintf(buf, "%d.%d.%d.%d", 
		p[0], p[1], p[2], p[3]);
	return buf;
}

#pragma warning(default:4996)

}} /// end namespace aot/inet




#endif /// __AOT_INET_INTERFACE_H__