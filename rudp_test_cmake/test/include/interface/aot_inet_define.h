/** Copyright (c) 2009-2010
 * All rights reserved.
 * 
 * 文件名称:	aot_inet_define.h   
 * 摘	 要:	封装网络通讯的操作接口
 * 
 * 当前版本：	1.0
 * 作	 者:	范涛涛(FTT)
 * 操	 作:	新建
 * 完成日期:	2010年02月02日
 */
#ifndef __AOT_INET_DEFINE_H__
#define __AOT_INET_DEFINE_H__

#include <commondef/aot_typedef.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

#ifdef __cplusplus
extern "C" {
#endif

#define AOT_HTTP_TUNNEL				(1)

#define AIO_DEFAULT_HEARTBEAT_TM	(55000)
#define MAX_HEARTBEAT_TM_OUT		(200000)/* sec */
#define AOT_IOV_MAX					(64) /*WIN没有定义上限值,这里由原来的14扩大为64,以增加IO吞吐量(14)*/
#define AOT_SO_HIGH_SPEED_SEND_BUF_SIZE	(64*1024-1)
#define AOT_SO_HIGH_SPEED_RECV_BUF_SIZE	(64*1024-1)

#define AOT_HIGH_SPEED_SEND_MSS			(32*1024-AOT_MEM_ALIGN)
#define AOT_HIGH_SPEED_RECV_MSS			(32*1024-AOT_MEM_ALIGN)

#define __AOT_SOCK_ERR_BASE__						(100)
#define AOT_SOCK_ERR_UNKNOWN						( __AOT_SOCK_ERR_BASE__ + 1)
#define AOT_SOCK_ERR_RECV_EOF						( __AOT_SOCK_ERR_BASE__ + 2)
#define AOT_SOCK_ERR_HEARTBEAT_TIMEOUT_SEND			( __AOT_SOCK_ERR_BASE__ + 3)
#define AOT_SOCK_ERR_HEARTBEAT_TIMEOUT_RECV			( __AOT_SOCK_ERR_BASE__ + 4)
#define AOT_SOCK_ERR_NOBUF							( __AOT_SOCK_ERR_BASE__ + 5)
#define AOT_SOCK_ERR_PKT_HEADER_INVALID				( __AOT_SOCK_ERR_BASE__ + 6)
#define AOT_SOCK_ERR_SYS_BUG						( __AOT_SOCK_ERR_BASE__ + 7)
#define AOT_SOCK_ERR_NO_OPEN						( __AOT_SOCK_ERR_BASE__ + 8)
#define AOT_SOCK_ERR_SEND_ONE_BYTE_ERR				( __AOT_SOCK_ERR_BASE__ + 9)

typedef enum 
{ 
	PKT_VERSION = 8,
	PKT_FLAG = 0x2233, 
	PKT_HEADER_LEN = sizeof(aot_uint32_t)*7,
	PKT_MAX_EXHEADER_LEN = 1024*1024, /// 扩展头允许的最大长度
	PKT_MAX_BODY_LEN = 1024*1024*10, /// 数据段允许的最大长度
	PKT_MAX_TOTAL_LEN = (PKT_HEADER_LEN + PKT_MAX_EXHEADER_LEN + PKT_MAX_BODY_LEN),

	UDP_PKT_MAX_LEN = 1024*32,	/// 仅仅针对UDP
	PKT_SEND_HIGH_WATER = 1024*128 + 1024,
}PKT_INFO;

typedef enum
{
	AOT_PEF_RESPONSE				= 0x00000001,					/// 0: 请求包, 1: 应答包
	AOT_PEF_SAVE_TO_DB				= 0x00000001 << 1,				/// 0: 不保存数据库; 1: 保存数据库
	AOT_PEF_OFFLINE_MSG				= 0x00000001 << 2,				/// 1: 离线消息

	AOT_PEF_NEED_CONFIRM_RECEIPT	= 0x00000001 << 3,				/// 1: 服务端消息需要确认已经收到
	AOT_PEF_CONFIRM_RECEIPT_ACK		= 0x00000001 << 4,				/// 1: 服务端返回确认已经收到的ACK
	AOT_PEF_NEED_SET_ACTIVE_CNN		= 0x00000001 << 5,				/// 1: 将发送者设置为活动连接, 仅当请求包(即非应答)时有效
	AOT_PEF_MY_DEVICE_CC_MSG		= 0x00000001 << 6,				/// 1: 发送给我的设备的抄送信息

	AOT_PEF_MQUES_SYNC_MSG			= 0x00000001 << 7,				/// 1: 同步消息

	AOT_PEF_S2S_NEED_CONFIRM_RECEIPT		= 0x00000001 << 8,			/// 1: xx服务端-->xx服务端, 需要确认已经收到(仅用于服务端与服务端之间)
	AOT_PEF_S2S_CONFIRM_RECEIPT_ACK			= 0x00000001 << 9,			/// 1: xx服务端-->xx服务端, 返回确认已经收到的ACK(仅用于服务端与服务端之间)

	AOT_PEF_SAVE_TO_MQUES_B0				= 0x00000001 << 10,
	AOT_PEF_MQUES_BO_MSG					= 0x00000001 << 11,
}AOT_PKT_EXHD_FLAG;

typedef enum
{
	AOT_STDI_SEND_TO_UNKNOWN = -1,
	AOT_STDI_SEND_TO_ACTIVE_DEVICE = 0,		/// 数据包只发给当前活动的终端设备(默认)
	AOT_STDI_SEND_TO_ALL_DEVICE,			/// 数据包发给当前所有在线的终端设备
	AOT_STDI_SEND_TO_WITH_GUID,				/// 只发给指定标识为guid的终端设备
	AOT_STDI_SEND_TO_WITH_DEVICE_TYPE,		/// 发送给指定的终端类型

}AOT_PKT_EXHD_STDI_HOW;

#define AOT_BUF_PAGE_SIZE				(4096 - 16)
#define AOT_DEFAULT_RECV_MSS		AOT_BUF_PAGE_SIZE
#define AOT_DEFAULT_SEND_MSS		AOT_BUF_PAGE_SIZE
#define AOT_RECVBUF_MIN_FREE_SIZE		(256 + PKT_HEADER_LEN)

typedef enum
{
	/// 
	e_pkt_type_heartbeat = 0,
	e_pkt_type_echo,
	e_pkt_type_echo_reply,
	e_pkt_type_data,
	__e_pkt_type_user_define_begin__ = 1001,
}epkt_type;

typedef enum
{
	e_inet_prot_private = 0,		/// 私有协议(默认)
	e_inet_prot_unknow,				/// 其它未知
}einet_prot_type;

/* - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - */


typedef struct aot_buf_s				aot_buf_t;

typedef struct
{
	char*			start;
	aot_uint32_t	size;
	aot_thread_volatile aot_long_t		ref_count;
#define aot_data_lock_inc_ref(d)		InterlockedIncrement( &((d)->ref_count) )/// ( ++(d)->ref_count )
#define aot_data_lock_dec_ref(d)		InterlockedDecrement( &((d)->ref_count) )/// ( --(d)->ref_count )
#define aot_data_lock_inc_ref_n(d, n)		InterlockedExchangeAdd(&((d)->ref_count), (n))
}aot_data_t;

struct aot_buf_s
{
	/// 以下三个字段留给外部做标识用
	aot_uint32_t			channel_attr;
	aot_uint32_t			nkey;
	void*					pkey;
	aot_inet_addr_t			addr_key;
	
	aot_uint32_t			rd_pos;		/// 读指针
	aot_uint32_t			wr_pos;		/// 写指针
	aot_uint32_t			mark_size;
	char*					mark_start;
	aot_data_t*		data;
	aot_buf_t*		next;
};

//#define aot_buf_space(b)			(aot_uint32_t)( (b)->mark_size - (b)->wr_pos )
//#define aot_buf_length(b)			(aot_uint32_t)( (b)->wr_pos - (b)->rd_pos )
//#define aot_buf_rd_ptr(b)			( (char*)((b)->mark_start) + (b)->rd_pos )
//#define aot_buf_wr_ptr(b)			( (char*)((b)->mark_start) + (b)->wr_pos )
//#define aot_buf_add_rd_pos(b, add)		( (b)->rd_pos + (add) )
//#define aot_buf_add_wr_pos(b, add)		( (b)->wr_pos + (add) )

__inline aot_uint32_t aot_buf_space(aot_buf_t* b){ return b->mark_size - b->wr_pos;}
__inline aot_uint32_t aot_buf_length(aot_buf_t* b){ return b->wr_pos - b->rd_pos;}
__inline char* aot_buf_rd_ptr(aot_buf_t* b){ return (char*)(b->mark_start) + b->rd_pos;}
__inline char* aot_buf_wr_ptr(aot_buf_t* b){ return (char*)(b->mark_start) + b->wr_pos;}
__inline void aot_buf_add_rd_pos(aot_buf_t*b, aot_uint32_t offset){ b->rd_pos += offset;}
__inline void aot_buf_add_wr_pos(aot_buf_t*b, aot_uint32_t offset){ b->wr_pos += offset;}
__inline void aot_buf_dec_rd_pos(aot_buf_t*b, aot_uint32_t offset){ b->rd_pos -= offset;}
__inline void aot_buf_dec_wr_pos(aot_buf_t*b, aot_uint32_t offset){ b->wr_pos -= offset;}
__inline void aot_buf_recycle(aot_buf_t*b){ b->wr_pos=0; b->rd_pos=0;}
//__inline aot_long_t aot_buf_lock_inc_ref(aot_buf_t*b){ return InterlockedIncrement(&b->ref_count);}
//__inline aot_long_t aot_buf_lock_dec_ref(aot_buf_t*b){ return InterlockedDecrement(&b->ref_count);}
//__inline aot_long_t aot_buf_lock_inc_ref_n(aot_buf_t*b, aot_long_t n){ return InterlockedExchangeAdd(&b->ref_count, n);}

/* - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - */

typedef aot_bool_t		(*aot_hashmap_cmp_fun_ptr)(void* k1, void* k2);
typedef aot_uint32_t	(*aot_hashmap_hash_fun_ptr)(void* k);

typedef struct aot_hashmap_elem_s		aot_hashmap_elem_t;
typedef struct aot_hashmap_bucket_s		aot_hashmap_bucket_t;

struct aot_hashmap_elem_s
{
	__AOT_QUEUE_DECALRE__(aot_hashmap_elem_t);
	void*					key;
	void*					val;
	aot_hashmap_bucket_t*	bucket;
};

struct aot_hashmap_bucket_s
{
	__AOT_QUEUE_DECALRE__(aot_hashmap_bucket_t);
	aot_hashmap_elem_t		elem_queue;
	aot_uint32_t			elem_count;
};

typedef struct
{
	aot_uint32_t			elem_count;
	aot_uint32_t			init_elem_count;
	aot_uint32_t			max_bucket_count;
	aot_hashmap_bucket_t	used_bucket_queue;
	aot_hashmap_bucket_t*	bucket;
	void*					elem_pool;
	aot_hashmap_cmp_fun_ptr		cmp_fun;
	aot_hashmap_hash_fun_ptr	hash_fun;
}aot_hashmap_t;

typedef struct
{
	aot_hashmap_t* (*create)(aot_uint32_t max_bucket_count, aot_uint32_t init_elem_count,
		aot_hashmap_cmp_fun_ptr cmp_fun, aot_hashmap_hash_fun_ptr hash_fun);

	void			(*destroy)(aot_hashmap_t* m);
	void			(*clear)(aot_hashmap_t* m);
	aot_bool_t		(*insert)(aot_hashmap_t* m, void* key, void* val);
	aot_bool_t		(*find)(aot_hashmap_t* m, void* key, void** out_val);
	aot_bool_t		(*find_pos)(aot_hashmap_t* m, void* key, aot_hashmap_elem_t** out_pos);
	aot_bool_t		(*remove)(aot_hashmap_t* m, void* key, void** out_val);
	aot_bool_t		(*remove_with_pos)(aot_hashmap_t* m, aot_hashmap_elem_t* pos);
	aot_uint32_t	(*size)(aot_hashmap_t* m);
}aot_hashmap_op_t;

__inline aot_hashmap_elem_t*
aot_hashmap_goto_begin(aot_hashmap_t* m)
{
	if( m && m->used_bucket_queue.next != &m->used_bucket_queue )
	{
		if( m->used_bucket_queue.next->elem_queue.next != &m->used_bucket_queue.next->elem_queue )
			return m->used_bucket_queue.next->elem_queue.next;
	}
	return NULL;
}

__inline aot_bool_t
aot_hashmap_is_end(aot_hashmap_elem_t* pos)
{
	return NULL == pos;
}

__inline aot_hashmap_elem_t*
aot_hashmap_goto_next(aot_hashmap_t* m, aot_hashmap_elem_t* pos)
{
	if( !pos || !m )
	{
		return NULL;
	}

	if( pos->next != &pos->bucket->elem_queue )
	{
		return pos->next;
	}

	if( pos->bucket->next != &m->used_bucket_queue && 
		pos->bucket->next->elem_queue.next != &pos->bucket->next->elem_queue )
	{
		return pos->bucket->next->elem_queue.next;
	}

	return NULL;
}
/* - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - */

/* - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - */


/**   通讯协议数据包头定义:
*  
*    00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
*   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*   |    version(uint8)     |  T1       |     T2    |           packet type(uint_16)                |
*   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*   |                                   body  length (uint32)                                       |
*   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*   |                          extern header length(uint32)                                         |
*   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*   |                 T3(3字节)保留(置0)                                    |  T4(路由,最大跳数TTL) |
*   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*   |                               src id    路由选项(源端id)                                      |
*   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*   |                               dest id   路由选项(目标端id)                                    |
*   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*   |                                        保留(置0)                                              |
*   +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*   
*
*   字段说明:
*   version(1字节):		版本号(当前版本为1)
*   packet type(1字节):	数据包类型:
*								0: 心跳包
*								1: echo请求包(测试用)
*								2: echo应答包(测试用)
*								3: 业务数据包
*   header length(1字节):	包头长度
*   T1( 4 bit):			加密方式 0: 不加密
*   T2( 4 bit):			保留,必须置为全0
*   body length(4字节):	包体长度
*   extern header length(2字节):	扩展包头长度
*   T3(1字节):			保留,必须置为全0
*   T4(1字节):          路由选项(最大跳数TTL, time to live)
*   src id:             路由选项(源端id)
*   dest id:            路由选项(目标端id)
*/

__inline aot_uint8_t	ph_get_ver(char* h){ return *((aot_uint8_t*)h);}
__inline void			ph_set_ver(char* h, aot_uint8_t v){ *((aot_uint8_t*)h) = v; }

__inline aot_uint16_t	ph_get_type(char* h) { return ntohs( ((aot_uint16_t*)h)[1] );}
__inline void			ph_set_type(char* h, aot_uint16_t v) { ((aot_uint16_t*)h)[1] = htons(v);}

__inline aot_uint8_t	ph_get_encrypt_type(char* h) { return ((aot_uint8_t*)h)[1] & 0x0f;}
__inline void			ph_set_encrypt_type(char* h, aot_uint8_t v) { ((aot_uint8_t*)h)[1] = (v & 0x0f);}

__inline aot_uint32_t	ph_get_data_len(char* h) { return ntohl( ((aot_uint32_t*)h)[1] );}
__inline void			ph_set_data_len(char* h, aot_uint32_t v) { ((aot_uint32_t*)h)[1] = htonl(v);}

__inline aot_uint32_t	ph_get_extern_header_len(char* h) { return ntohl( ((aot_uint32_t*)h)[2]);}
__inline void			ph_set_extern_header_len(char* h, aot_uint32_t v) { ((aot_uint32_t*)h)[2] = htonl(v);}

__inline aot_int8_t		ph_get_ttl(char* h){ return ((aot_int8_t*)h)[15]; }
__inline void			ph_set_ttl(char* h, aot_int8_t v){ ((aot_int8_t*)h)[15] = v; }

__inline aot_uint32_t	ph_get_src_id(char* h) { return ntohl( ((aot_uint32_t*)h)[4] );}
__inline void			ph_set_src_id(char* h, aot_uint32_t v) { ((aot_uint32_t*)h)[4] = htonl(v);}

__inline aot_uint32_t	ph_get_dest_id(char* h) { return ntohl( ((aot_uint32_t*)h)[5] );}
__inline void			ph_set_dest_id(char* h, aot_uint32_t v) { ((aot_uint32_t*)h)[5] = htonl(v);}

__inline aot_bool_t		ph_is_valid(char* h)
{ 
	return (PKT_VERSION == ph_get_ver(h) && ph_get_extern_header_len(h) < PKT_MAX_EXHEADER_LEN && ph_get_data_len(h) < PKT_MAX_BODY_LEN); 
}

__inline void ph_build_heartbeat(aot_buf_t* buf)
{
	char* p;
	p = aot_buf_wr_ptr(buf);
	memset(p, 0, PKT_HEADER_LEN);
	ph_set_ver(p, PKT_VERSION);
	ph_set_type(p, e_pkt_type_heartbeat);
	ph_set_data_len(p, 0);
	ph_set_extern_header_len(p, 0);

	aot_buf_add_wr_pos(buf, PKT_HEADER_LEN);
}

__inline void ph_build(aot_buf_t* buf, aot_uint16_t pkt_type, aot_uint8_t encrypt_type, 
					   aot_uint32_t data_len, aot_uint16_t extern_header_len)
{
	char* p;
	p = aot_buf_wr_ptr(buf);
	memset(p, 0, PKT_HEADER_LEN);
	ph_set_ver(p, PKT_VERSION);
	ph_set_type(p, pkt_type);
	ph_set_encrypt_type(p, encrypt_type);
	ph_set_data_len(p, data_len);
	ph_set_extern_header_len(p, extern_header_len);

	aot_buf_add_wr_pos(buf, PKT_HEADER_LEN);
}

__inline void ph_build2(char* buf, aot_uint16_t pkt_type, aot_uint8_t encrypt_type, 
					   aot_uint32_t data_len, aot_uint16_t extern_header_len)
{
	memset(buf, 0, PKT_HEADER_LEN);
	ph_set_ver(buf, PKT_VERSION);
	ph_set_type(buf, pkt_type);
	ph_set_encrypt_type(buf, encrypt_type);
	ph_set_data_len(buf, data_len);
	ph_set_extern_header_len(buf, extern_header_len);
}

/* - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - */

typedef void*	AIO_EVLD_HANDLE;
typedef void*	AIO_CNN_HANDLE;
typedef void*	AIO_PKT_HANDLE;
typedef void*	AIO_POOL_HANDLE;
typedef void*	AIO_ACCEPTOR_HANDLE;
#define AIO_INVALID_HANDLE		(NULL)


typedef struct aio_evld_op_s			aio_evld_op_t;

typedef struct
{
	aot_buf_t*		(*create)(AIO_POOL_HANDLE pool, aot_uint32_t size);
	void			(*destroy)(AIO_POOL_HANDLE pool, aot_buf_t* b);
	aot_buf_t*		(*duplicate)(AIO_POOL_HANDLE pool, aot_buf_t* b);		/// 仅对b进行浅层复制,不包括b->next
	aot_uint32_t	(*total_length)(aot_buf_t* b);
	aot_bool_t		(*is_empty)(aot_buf_t* b);
	aot_buf_t*		(*duplicate_ex)(AIO_POOL_HANDLE pool, aot_buf_t* b);	/// 对b的整个链表进行浅层复制
	aot_buf_t*		(*merge)(AIO_POOL_HANDLE pool, aot_buf_t* b);	/// 将b的链表合并成一个缓冲区
}aot_buf_op_t;

typedef struct
{
	AIO_POOL_HANDLE	(*create)(aot_uint32_t n, aot_uint32_t* chunk_size, aot_uint32_t* low_water, 
							  aot_uint32_t* high_water, aot_bool_t lock, aot_uint8_t* idx, aot_uint32_t idx_num);
	void			(*destroy)(AIO_POOL_HANDLE pool);
	void*			(*alloc)(AIO_POOL_HANDLE pool, size_t size);
	void*			(*calloc)(AIO_POOL_HANDLE pool, size_t size);
	void			(*free)(AIO_POOL_HANDLE pool, void* p);
}aio_pool_op_t;

typedef enum
{
	e_ht_null = 0,
	e_ht_wait,
	__e_ht_finished__ = 6,
	e_ht_finished_ok,
	e_ht_finished_err_ok,
}eht_status;

typedef struct
{
	unsigned					accept:1;				/// 是否是一个被动socket
	unsigned					sock_type:2;			/// 0: tcp	1:udp
	unsigned					check:3;				/// 给外部使用的字段, 0: 未确认的连接
	unsigned					recv_pkt_fragment:1;	/// 0: 接收的数据包不允许分段(即:只能存放在一个buf内; 1: 允许分段
	unsigned					removed:1;				/// 给外部使用的字段, 0: 该连接未被移除; 1: 已移除
	unsigned					manual_process_heartbeat:1;/// 0: 底层自动处理心跳(默认) 1: 不处理
	unsigned					support_ht:1;			/// 1: 支持http tunnel
	unsigned					reserved8:8;
	aot_int8_t					ht_status;
	aot_uint32_t				attr;
	aot_uint32_t				mark;
	void*						token;
	void*						user_data;
	aot_inet_addr_t				local_addr;
	aot_inet_addr_t				remote_addr;
	aot_milli_sec_t				max_heartbeat_tm_out;
	aot_milli_sec_t				heartbeat_tm;
	aot_uint32_t				send_high_water;
	aot_milli_sec_t				last_send_tm;
	aot_milli_sec_t				last_recv_tm;
	aot_err_t					err_code;
	aot_ulong_t					processed_send_io_count;
	aot_ulong_t					processed_recv_io_count;
	aot_ulong_t					processed_send_bytes;
	aot_ulong_t					processed_recv_bytes;
	aot_uint32_t				udp_max_pkt_size;			/// 仅仅针对UDP
	aot_int32_t					cont_process_pkt_cnt;	/// 如果一次收到的包里面有多个业务包, 指定一次性连续处理多少个(默认为1)
	aot_uint32_t				send_mss;				/// 单次发送的最大长度(最大分节大小, 注意: 该参数不同于协议栈的MSS)
	aot_uint32_t				recv_mss;				/// 单次接收的最大长度(最大分节大小, 注意: 该参数不同于协议栈的MSS)
	einet_prot_type				inet_prot;
}aio_connection_attr_t;

typedef struct
{
	aot_int32_t					use_aio;
	aot_uint32_t				max_connections;
	aot_ulong_t					pending_send_bytes;
	aot_ulong_t					pending_send_io_count;
	aot_ulong_t					max_pending_send_bytes;
	aot_ulong_t					max_pending_send_io_count;
	aot_ulong_t					processed_send_io_count;
	aot_ulong_t					processed_recv_io_count;
	aot_ulong_t					processed_send_bytes;
	aot_ulong_t					processed_recv_bytes;
	void*						token;
}aio_evld_attr_t;

typedef struct
{
	aot_int32_t nval;
	void*		pval;
}aio_timer_queue_user_data_t;

typedef struct
{
	/// 成功接受了一个连接 
	void (*on_accepted_connection)(AIO_CNN_HANDLE c, aio_connection_attr_t* attr);
	void (*on_incoming_pkt)(AIO_CNN_HANDLE c, aio_connection_attr_t* attr, aot_buf_t* recv_buf);
	void (*on_incoming_raw_pkt)(AIO_CNN_HANDLE c, aio_connection_attr_t* attr, aot_buf_t* recv_buf);
	/// 连接被关闭, 此后ctx不能再被使用, 用户不能释放ctx, 
	/// 系统会自动在回调完成后,释放该对象.
	void (*on_close_connection)(AIO_CNN_HANDLE c, aio_connection_attr_t* attr);
	void (*on_post_msg)(AIO_EVLD_HANDLE evld, aot_int32_t msg, void* param);
	void (*on_locked_post_msg)(AIO_EVLD_HANDLE evld, aot_int32_t msg, void* param);
	void (*on_evld_end_event_loop)(AIO_EVLD_HANDLE evld);

	/// 与指定连接@c 绑定的定时器
	void (*on_timer_out)(AIO_CNN_HANDLE c, aio_connection_attr_t* attr, aot_int_t key, aio_timer_queue_user_data_t* data);
	void (*on_timer_cancel)(AIO_CNN_HANDLE c, aio_connection_attr_t* attr, aot_int_t key, aio_timer_queue_user_data_t* data);

	/// 新增定时器功能, 该定时器直接与evld关联, 不与某个连接绑定 by FTT 2015-03-31
	void (*on_evld_raw_timer_out)(AIO_EVLD_HANDLE evld, aot_int_t raw_timer_id, aio_timer_queue_user_data_t* data);
	void (*on_evld_raw_timer_cancel)(AIO_EVLD_HANDLE evld, aot_int_t raw_timer_id, aio_timer_queue_user_data_t* data);

	/// 主动连接时回调的接口
	void (*on_connect_result)(AIO_CNN_HANDLE c, aio_connection_attr_t* attr, aot_err_t err);
	/// for udp
	void (*on_udp_incoming_pkt)(AIO_CNN_HANDLE c, aio_connection_attr_t* attr, aot_buf_t* recv_buf, aot_inet_addr_t* remote_addr);
}aio_user_event_handler_t;

struct aio_evld_op_s
{
	AIO_EVLD_HANDLE	(*create)(AIO_POOL_HANDLE pool, aot_uint32_t th_num, aio_user_event_handler_t* ev, aot_uint32_t max_connections, aot_uint32_t max_timer_queue_size);
	void			(*close_wait)(AIO_EVLD_HANDLE evld);
	void			(*destroy)(AIO_EVLD_HANDLE evld);
	aot_err_t		(*post_msg)(AIO_EVLD_HANDLE evld, aot_ulong_t msg, void* param);
	aot_err_t		(*locked_post_msg)(AIO_EVLD_HANDLE evld, aot_ulong_t msg, void* param);
	aot_err_t		(*run_event_loop)(AIO_EVLD_HANDLE evld, aot_milli_sec_t msec);
	aot_err_t		(*end_event_loop)(AIO_EVLD_HANDLE evld);
	aot_bool_t		(*is_event_loop_end)(AIO_EVLD_HANDLE evld);
	aio_evld_attr_t*		(*get_attr)(AIO_EVLD_HANDLE evld);
	AIO_CNN_HANDLE			(*connect)(AIO_EVLD_HANDLE evld, aot_ulong_t ip, aot_uint16_t port, aot_uint32_t mark, void* token);
	aot_err_t				(*listen)(AIO_EVLD_HANDLE evld, aot_ulong_t ip, aot_uint16_t port, aot_uint32_t backlog);
	AIO_ACCEPTOR_HANDLE		(*add_listen)(AIO_EVLD_HANDLE evld, aot_ulong_t ip, aot_uint16_t port, aot_uint32_t backlog);

	/// 新增定时器功能, 该定时器直接与evld关联, 不与某个连接绑定 by FTT 2015-03-31
	aot_err_t		(*raw_schedule_timer)(AIO_EVLD_HANDLE evld, aot_int_t* out_tm_id, aot_milli_sec_t tm_out, aot_bool_t is_loop, aio_timer_queue_user_data_t* user_data);
	aot_err_t		(*raw_cancel_timer)(AIO_EVLD_HANDLE evld, aot_int_t timer_id, aot_bool_t is_notify);
};

typedef struct
{
	void			(*close)(AIO_CNN_HANDLE c, aot_bool_t is_notify);
	aot_err_t		(*schedule_timer)(AIO_CNN_HANDLE c, aot_milli_sec_t tm_out, aot_int_t key, aio_timer_queue_user_data_t* user_data);
	aot_err_t		(*cancel_timer)(AIO_CNN_HANDLE c, aot_int_t key, aot_bool_t is_notify);
	void			(*cancel_all_timer)(AIO_CNN_HANDLE c, aot_bool_t is_notify);
	/// 发送数据, 如果返回值 != AOT_RET_OK, 由调用者释放@b, 否则,由系统自动释放
	aot_err_t		(*send)(AIO_CNN_HANDLE c, aot_buf_t* b);
	aot_err_t		(*check_heartbeat_status)(AIO_CNN_HANDLE c, aot_milli_sec_t curr_tm);
	void			(*start_recv)(AIO_CNN_HANDLE c);
	void			(*pause_recv)(AIO_CNN_HANDLE c);
	AIO_EVLD_HANDLE	(*get_evld)(AIO_CNN_HANDLE c);
	aio_connection_attr_t*	(*get_attr)(AIO_CNN_HANDLE c);

	/// 提供一个快捷接口直接获取evld的attr
	aio_evld_attr_t*		(*get_evld_attr)(AIO_CNN_HANDLE c);

	/// for udp
	AIO_CNN_HANDLE	(*create_udp_handle)(AIO_EVLD_HANDLE evld);
	/// @local_addr: udp本地要绑定的地址
	aot_bool_t		(*udp_open)(AIO_CNN_HANDLE c, aot_inet_addr_t* local_addr);	
	/// 非阻塞模式的发送, 无论返回成功还是失败,@pkt都应该由调用者释放
	aot_err_t		(*udp_nonblock_send_to)(AIO_CNN_HANDLE c, aot_inet_addr_t* remote_addr, aot_buf_t* pkt);
	/// 关闭udp通道. 与TCP不同, udp连接只能由调用者调用该接口来进行关闭,并且不会发出通知(关闭事件)
	void			(*udp_close)(AIO_CNN_HANDLE c);
	/// 设置socket选项
	aot_int_t		(*set_sock_opt)(AIO_CNN_HANDLE c, aot_int_t level, aot_int_t optname, const char* optval, aot_int_t optlen);
}aio_channel_op_t;

typedef struct
{
	AIO_ACCEPTOR_HANDLE	(*create)(AIO_EVLD_HANDLE evld);
	void				(*destroy)(AIO_ACCEPTOR_HANDLE a);
	void				(*close)(AIO_ACCEPTOR_HANDLE a);
	aot_inet_addr_t*	(*get_listen_addr)(AIO_ACCEPTOR_HANDLE a);
	aot_inet_addr_t*	(*get_evld_default_acceptor_listen_addr)(AIO_EVLD_HANDLE evld);
}aio_acceptor_op_t;

/// 返回毫秒数
__inline aot_uint64_t aot_gettimeofday()
{
	FILETIME filetime;
	aot_uint64_t time = 0;
	GetSystemTimeAsFileTime(&filetime);

	time |= filetime.dwHighDateTime;
	time <<= 32;
	time |= filetime.dwLowDateTime;

	time /= 10;
	time -= 11644473600000000LL;
	time /= 1000;

	return time;
}

#ifdef __cplusplus
}
#endif

#endif /// __AOT_INET_DEFINE_H__