/** Copyright (c) 2009-2010
 * All rights reserved.
 * 
 * 文件名称:	aot_typedef.h   
 * 摘	 要:	基础数据类型定义
 * 
 * 当前版本：	1.0
 * 作	 者:	范涛涛(FTT)
 * 操	 作:	新建
 * 完成日期:	2010年02月02日
 */

#ifndef __AOT_TYPEDEF_H_20100202__
#define __AOT_TYPEDEF_H_20100202__

#define __AOT_PM_LINE__(x)				#x
#define __AOT_PM_LINE_STRING__(x)		__AOT_PM_LINE__(x)
#define AOT_PM_MESS(msg)				__FILE__"("__AOT_PM_LINE_STRING__(__LINE__)"): "msg

#define AOT_MEM_ALIGN					(8)


#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

typedef UINT8			aot_uint8_t;
typedef UINT16			aot_uint16_t;
typedef UINT32			aot_uint32_t;
typedef UINT64			aot_uint64_t;
typedef unsigned long   aot_ulong_t;
typedef unsigned int	aot_uint_t;

typedef INT8			aot_int8_t;
typedef INT16			aot_int16_t;
typedef INT32			aot_int32_t;
typedef INT64			aot_int64_t;
typedef long			aot_long_t;
typedef int				aot_int_t;
typedef int				aot_err_t;
typedef int				aot_bool_t;
typedef aot_uint32_t	aot_nbrnum_t;

#define aot_thread_volatile	volatile
#define AOT_RET_OK		(0)
#define AOT_RET_ERROR	(-1)
#define AOT_RET_AGAIN	(-2)

#define AOT_RET_IS_FAILED(x)		( (x) != AOT_RET_OK)
#define AOT_RET_IS_SUCESSED(x)		( (x) == AOT_RET_OK)

#define __AOT_INT32_B0(n)			((aot_uint8_t)((aot_uint32_t)(n) & 0xff))
#define __AOT_INT32_B1(n)			((aot_uint8_t) (((aot_uint32_t)(n) & 0xff00) >> 8))
#define __AOT_INT32_B2(n)			((aot_uint8_t) (((aot_uint32_t)(n) & 0xff0000) >> 16))
#define __AOT_INT32_B3(n)			((aot_uint8_t) (((aot_uint32_t)(n) & 0xff000000) >> 24))

#define	AOT_PTR_TO_ULONG(p)			PtrToUlong( p )
#define AOT_PTR_TO_LONG(p)			PtrToLong( p )
#define AOT_PTR_TO_UINT(p)			PtrToUint( p )
#define AOT_PTR_TO_INT(p)			PtrToInt( p )
#define AOT_PTR_TO_USHORT(p)		PtrToUshort( p )
#define AOT_PTR_TO_SHORT(p)			PtrToShort( p )
#define AOT_INT_TO_PTR(i)			IntToPtr( i )
#define AOT_UINT_TO_PTR(ui)			UIntToPtr( ui )
#define AOT_LONG_TO_PTR(ui)			LongToPtr( l )
#define AOT_ULONG_TO_PTR(ui)		ULongToPtr( ul )

#ifdef __cplusplus
extern "C" {
#endif

	/// 必须在队列结构的声明内加入该宏
#define __AOT_QUEUE_DECALRE__(queue_type)	\
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

#define aot_queue_init(q)	do{(q)->prev = (q); (q)->next = (q);}while(0)
#define aot_queue_is_empty(h)	( (h) == (h)->prev )
#define aot_queue_head(h)	((h)->next)
#define aot_queue_tail(h)	((h)->prev)
#define aot_queue_insert_head(h, x) do{(x)->next = (h)->next; (x)->next->prev = (x); (h)->next = (x); (x)->prev = (h); }while(0)
#define aot_queue_insert_tail(h, x)	do{ (x)->prev = (h)->prev; (x)->prev->next = (x); (h)->prev = (x); (x)->next = (h);}while(0)
	/// 注意: 移除元素前,必须判断队列是否为空
#define aot_queue_remove(x)	do{ (x)->prev->next = (x)->next; (x)->next->prev = (x)->prev;}while(0)

#define aot_queue_for_each(tmp, h)	for( (tmp) = (h)->next; (tmp) != (h); (tmp) = (tmp)->next )



#if defined (_MSC_VER) && (_MSC_VER < 1300)
	typedef ULONG aot_act_t;
#else
	typedef ULONG_PTR aot_act_t;
#endif

typedef DWORD	aot_milli_sec_t;

typedef struct
{
	aot_long_t sec;
	aot_long_t usec;
}aot_time_t;


#define aot_inet_addr_cdr_size	(6)

typedef struct
{
	/// host byte order
	aot_uint16_t port;
	/// host byte order
	aot_uint32_t ip;
}aot_inet_addr_t;

#ifdef __cplusplus
}
#endif
#endif /// __AOT_TYPEDEF_H_20100202__