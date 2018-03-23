#include "StdAfx.h"
#include "inet_channel_mgr.h"

#pragma warning(disable:4311)
#pragma warning(disable:4312)

extern "C"
{
aot_bool_t
__addr_cmp_fun(void* k1, void* k2)
{
	if( ((aot_inet_addr_t*)k1)->ip == ((aot_inet_addr_t*)k2)->ip &&
		((aot_inet_addr_t*)k1)->port == ((aot_inet_addr_t*)k2)->port )
	{
		return 1;
	}
	return 0;
}

aot_uint32_t
__addr_hash_fun(void* k)
{
	return ((aot_inet_addr_t*)k)->ip + ((aot_inet_addr_t*)k)->port;
}

aot_bool_t
__uint_cmp_fun(void* k1, void* k2)
{
	return (aot_uint32_t)k1 == (aot_uint32_t)k2;
}

aot_uint32_t
__uint_hash_fun(void* k)
{
	return (aot_uint32_t)k;
}

} /// end extern "C"

namespace aot { namespace inet{ 

}}

#pragma warning(default:4311)
#pragma warning(default:4312)