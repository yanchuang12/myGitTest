/** Copyright (c) 2015-2016
 * All rights reserved.
 * 
 * 文件名称:	aot_prot_crypt.h   
 * 摘	 要:	封装协议层的加解密操作
 * 
 * 当前版本：	1.0
 * 作	 者:	范涛涛(FTT)
 * 操	 作:	新建
 * 完成日期:	2015年08月06日
 */
#ifndef __AOT_PROT_CRYPT_20150806_H__
#define __AOT_PROT_CRYPT_20150806_H__

#include <minilzo/mini_lzo.h>
#include <time.h>
#include <commondef/aot_typedef.h>
#include <interface/aot_inet_define.h>
#include <interface/aot_inet_interface.h>
#include <assert.h>

#define __AOT_PROT_CRYPT_HEAD_FLAG__		(0xBADC)

namespace aot{ namespace prot{ namespace crypt{

template<class ALLOC>
aot_buf_t* aot_encode(ALLOC* ac, const char* buf, int len)
{
	aot_uint8_t method = 1;

	aot_int32_t pad = sizeof(aot_int32_t) * 2 + 2;
	size_t dest_len = len + len / 16 + 32 + 4;

	aot_buf_t* b = ac->create_buf( (aot_uint32_t)dest_len + pad );

	*((aot_int32_t*)aot_buf_wr_ptr(b)) = ::htonl(__AOT_PROT_CRYPT_HEAD_FLAG__);
	aot_buf_add_wr_pos(b, sizeof(aot_int32_t));

	*((aot_int32_t*)aot_buf_wr_ptr(b)) = ::htonl(len);
	aot_buf_add_wr_pos(b, sizeof(aot_int32_t));

	*((aot_uint8_t*)aot_buf_wr_ptr(b)) = method;
	aot_buf_add_wr_pos(b, sizeof(aot_uint8_t));

	srand((aot_uint_t)time(NULL));
	aot_uint8_t opt = (rand() % 250) + 2;  /// 避免出现 0, 1这样的情况
	*((aot_uint8_t*)aot_buf_wr_ptr(b)) = opt;
	aot_buf_add_wr_pos(b, sizeof(aot_uint8_t));

	aot::mini_lzo lzo;
	if( aot::mini_lzo::LZO_E_OK != lzo.compress((const unsigned char*)buf, len, (unsigned char*)aot_buf_wr_ptr(b), &dest_len) )
	{
		ac->destroy_buf(b);
		return NULL;
	}

	aot_uint8_t* tmp = (aot_uint8_t*)aot_buf_wr_ptr(b);
	for( int i = 0; i < (int)dest_len; ++i )
	{
		tmp[i] ^= opt; 
	}

	aot_buf_add_wr_pos(b, (aot_uint32_t)dest_len);
	return b;
}

template<class ALLOC>
char* aot_decode(ALLOC* ac, const char* buf, int len, int* out_len)
{
	aot_int32_t pad = sizeof(aot_int32_t) * 2 + 2;

	if( len < pad )
	{
		return NULL;
	}

	aot_int32_t flag = ntohl(*((aot_int32_t*)buf));
	if( flag != __AOT_PROT_CRYPT_HEAD_FLAG__ )
	{
		return NULL;
	}

	size_t dest_len = ntohl(*((aot_int32_t*)buf + 1));
	size_t dest_len2 = dest_len;

	aot_uint8_t method = *((aot_uint8_t*)buf + sizeof(aot_int32_t) * 2);
	aot_uint8_t opt = *((aot_uint8_t*)buf + sizeof(aot_int32_t) * 2 + 1);

	if( method == 0 )
	{
		/// 1.直接解压  2.完成
		char* p = (char*)ac->alloc((aot_uint32_t)dest_len + 2);

		aot::mini_lzo lzo;
		if( aot::mini_lzo::LZO_E_OK != lzo.decompress((const unsigned char*)buf + pad, len - pad, (unsigned char*)p, &dest_len)
			|| dest_len != dest_len2 )
		{
			ac->dealloc(p);
			return NULL;
		}

		p[dest_len] = 0;
		*out_len = (int)dest_len2;
		return p;
	}
	else if( method == 1 )
	{
		/// 1. 解密  2.解压  3.完成
		int n = len - pad;
		aot_uint8_t* tmp = (aot_uint8_t*)ac->alloc(n);
		aot_uint8_t* s_p = (aot_uint8_t*)buf + pad;

		for( int i = 0; i < n; ++i )
		{
			tmp[i] = s_p[i] ^ opt;
		}

		char* p = (char*)ac->alloc((aot_uint32_t)dest_len + 2);

		aot::mini_lzo lzo;
		if( aot::mini_lzo::LZO_E_OK != lzo.decompress((const unsigned char*)tmp, len - pad, (unsigned char*)p, &dest_len)
			|| dest_len != dest_len2 )
		{
			ac->dealloc(p);
			return NULL;
		}

		p[dest_len] = 0;
		*out_len = (int)dest_len2;
		return p;
	}
	else if( method == 2 )
	{
		/// 1.解压  2.解密 3.完成
		char* p = (char*)ac->alloc((aot_uint32_t)dest_len + 2);

		aot::mini_lzo lzo;
		if( aot::mini_lzo::LZO_E_OK != lzo.decompress((const unsigned char*)buf + pad, len - pad, (unsigned char*)p, &dest_len)
			|| dest_len != dest_len2 )
		{
			ac->dealloc(p);
			return NULL;
		}

		for( int i = 0; i < (int)dest_len; ++i )
		{
			((aot_uint8_t*)p)[i] ^= opt;
		}

		p[dest_len] = 0;
		*out_len = (int)dest_len2;
		return p;
	}
	else if( method == 3 )
	{
		/// 1.解密   2.完成
		int n = len - pad;
		assert( n == (int)dest_len);

		char* p = (char*)ac->alloc((aot_uint32_t)dest_len + 2);
		aot_uint8_t* s_p = (aot_uint8_t*)buf + pad;

		for( int i = 0; i < n; ++i )
		{
			((aot_uint8_t*)p)[i] = s_p[i] ^ opt;
		}

		p[dest_len] = 0;
		*out_len = (int)dest_len;
		return p;
	}
	return NULL;
}

}}} /// } /// end namespace aot/prot/crypt

#endif /// __AOT_PROT_CRYPT_20150806_H__