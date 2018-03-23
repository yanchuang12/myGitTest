/** Copyright (c) 2016-2017
 * All rights reserved.
 * 
 * 文件名称:	nd_typedef.h   
 * 摘	 要:	基础数据类型定义
 * 
 * 当前版本：	1.0
 * 作	 者:	范涛涛(FTT)
 * 操	 作:	新建
 * 完成日期:	2016年08月02日
 */

#ifndef __ND_TYPEDEF_H_20160802__
#define __ND_TYPEDEF_H_20160802__

#ifdef WIN32
#define __ND_WIN32__
#endif

#if defined(__GNUC__) && ( defined(__APPLE_CPP__) || defined(__APPLE_CC__) || defined(__MACOS_CLASSIC__) ) 
#define __ND_IOS
#endif

#define ND_MEM_ALIGN					(sizeof(void*))

#include "nd_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __ND_WIN32__
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
	#include <WinSock2.h>
	#include <ws2tcpip.h>
	#include <errno.h>
	#include <assert.h>
	#include <stdlib.h>
	#include <stdio.h>
	#include <time.h>
	#pragma comment(lib, "ws2_32.lib")

	/// for timeBeginPeriod(), timeEndPeriod()
	#include <MMSystem.h>
	#pragma comment(lib,"winmm.lib")

	typedef UINT8			nd_uint8_t;
	typedef UINT16			nd_uint16_t;
	typedef UINT32			nd_uint32_t;
	typedef UINT64			nd_uint64_t;

	typedef INT8			nd_int8_t;
	typedef INT16			nd_int16_t;
	typedef INT32			nd_int32_t;
	typedef INT64			nd_int64_t;

	typedef WPARAM			nd_wparam_t;
	typedef LPARAM			nd_lparam_t;
	typedef INT_PTR			nd_int_ptr;
	typedef UINT_PTR		nd_uint_ptr;
	typedef LONG_PTR		nd_long_ptr;

	typedef int				nd_socklen_t;
	typedef SOCKET			ND_SOCKET;

	#define ND_INVALID_SOCKET	INVALID_SOCKET
	#define ND_CALL_PRO			WINAPI
	#define ND_INLINE			__inline

	#define aot_set_last_error(x)	do{errno = x;}while(0)
	#define aot_get_last_error()	errno
	#define aot_get_socket_err_no()		WSAGetLastError()

	//#define EWOULDBLOCK         WSAEWOULDBLOCK
	//#define ETIMEDOUT           WSAETIMEDOUT
	//#define EINPROGRESS         WSAEINPROGRESS

#else /// __ND_WIN32__
	#define AOT_CALL_PRO

	#include <stdlib.h>
	#include <stdio.h>
	#include <sys/time.h>
	#include <sys/types.h>
	#include <pthread.h>
	#include <semaphore.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <string.h>
	#include <netdb.h>
	#include <ctype.h>
	#include <fcntl.h>
	#include <errno.h>
	#include <assert.h>
	#include <signal.h>

	typedef uint8_t			nd_uint8_t;
	typedef uint16_t		nd_uint16_t;
	typedef uint32_t		nd_uint32_t;
	typedef uint64_t		nd_uint64_t;

	typedef int8_t			nd_int8_t;
	typedef int16_t			nd_int16_t;
	typedef int32_t			nd_int32_t;
	typedef int64_t			nd_int64_t;

	typedef intptr_t		nd_long_ptr;
	typedef nd_long_ptr		nd_wparam_t;
	typedef nd_long_ptr		nd_lparam_t;
	typedef intptr_t		nd_int_ptr;
	typedef uintptr_t		nd_uint_ptr;
	

	typedef socklen_t		nd_socklen_t;
	typedef int				ND_SOCKET;

	#define ND_INVALID_SOCKET (-1)
	#define ND_CALL_PRO
	#define ND_INLINE		inline

	#define aot_set_last_error(x)	do{errno = x;}while(0)
	#define aot_get_last_error()	errno
	#define aot_get_socket_err_no() errno

#endif /// __ND_WIN32__

/// 平台无关

#ifndef NULL
	#ifdef __cplusplus
		#define NULL    0
	#else
		#define NULL    ((void *)0)
	#endif
#endif

typedef long			nd_long_t;
typedef int				nd_int_t;
typedef int				nd_err_t;
typedef int				nd_bool_t;
typedef unsigned long   nd_ulong_t;
typedef unsigned int	nd_uint_t;
typedef size_t			nd_size_t;

typedef void (ND_CALL_PRO *ND_THREAD_FUN)(void*);


#define nd_thread_volatile				volatile
#define nd_assert						assert
#define __nd_sys_malloc(size)			malloc(size)//_aligned_malloc( (size), ND_MEM_ALIGN )
#define __nd_sys_free(p)				free(p)//_aligned_free((p))
#define nd_mem_align_ptr(ptr)			(char *) (((uintptr_t)(ptr) + ((uintptr_t)ND_MEM_ALIGN - 1)) & ~((uintptr_t)ND_MEM_ALIGN - 1))

#define nd_min(a,b)						(((a) < (b)) ? (a) : (b))
#define nd_max(a,b)						(((a) > (b)) ? (a) : (b))

	/// 必须在队列结构的声明内加入该宏
#define __ND_QUEUE_DECALRE__(queue_type)	\
	queue_type*	prev;		\
	queue_type* next

	/** 队列操作
	* 1. 初始队列为空时,仅有一个头尾指针分别指向自己的哨兵节点(也可以认为是一对)
	* 2. 数据的插入操作,总是在这个哨兵节点之间进行
	* 3. 哨兵节点的prev指针总是指向队列的尾节点
	* 4. 哨兵节点的next指针总是指向队列的头节点
	* 注意:
	* a. 读取/移除 队列的有效元素前,必须判断队列是否为空
	* b. 插入节点无需判断
	*/

#define nd_queue_init(q)				do{(q)->prev = (q); (q)->next = (q);}while(0)
#define nd_queue_is_empty(h)			((h) == (h)->prev)
#define nd_queue_head(h)				((h)->next)
#define nd_queue_tail(h)				((h)->prev)
#define nd_queue_insert_head(h, x)		do{(x)->next = (h)->next; (x)->next->prev = (x); (h)->next = (x); (x)->prev = (h);}while(0)
#define nd_queue_insert_tail(h, x)		do{(x)->prev = (h)->prev; (x)->prev->next = (x); (h)->prev = (x); (x)->next = (h);}while(0)
	/// 注意: 移除元素前,必须判断队列是否为空
#define nd_queue_remove(x)				do{(x)->prev->next = (x)->next; (x)->next->prev = (x)->prev;}while(0)
#define nd_queue_for_each(tmp, h)		for ((tmp) = (h)->next; (tmp) != (h); (tmp) = (tmp)->next)

typedef nd_ulong_t		nd_milli_sec_t;

typedef struct
{
	nd_long_t sec;
	nd_long_t usec;
}nd_time_t;

typedef struct
{
	/// host byte order
	nd_uint32_t ip;
	nd_uint16_t port;
}nd_inet_addr_t;

typedef struct nd_link_s		nd_link_t;
struct nd_link_s
{
	struct nd_link_s *next;
};

static ND_INLINE int __nd_is_big_endian()
{
	/// 如果低地址存储高位数据, 则 是big_endian;
	unsigned short i = 0x1122;
	return (*((unsigned char*)(&i)) == 0x11) ? 1 : 0;
}

static ND_INLINE
nd_uint64_t nd_ntohll(nd_uint64_t val)
{
	if (!__nd_is_big_endian())
		return (((nd_uint64_t)htonl((int)((val << 32) >> 32))) << 32) | (unsigned int)htonl((int)(val >> 32));
	else
		return val;
}

static ND_INLINE
nd_uint64_t nd_htonll(nd_uint64_t val)
{
	if (!__nd_is_big_endian())
		return (((nd_uint64_t )htonl((int)((val << 32) >> 32))) << 32) | (unsigned int)htonl((int)(val >> 32));
	else
		return val;
}

/* - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - */

typedef struct
{
	nd_uint32_t		chunk_size_lst_num;
	nd_uint32_t*	chunk_size_lst;
	nd_uint32_t		large_page_size;
	nd_uint64_t		low_water;
	nd_uint64_t		high_water;
	nd_bool_t		lock;
	nd_uint8_t*		idx;
	nd_uint32_t		idx_num;
	nd_uint64_t		up_limit;
}nd_pool_init_param_t;

#ifdef __cplusplus
}
#endif
#endif /// __ND_TYPEDEF_H_20160802__
