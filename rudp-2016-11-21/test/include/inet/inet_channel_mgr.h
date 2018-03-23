/** Copyright (c) 2008-2009
 * All rights reserved.
 * 
 * �ļ�����:	inet_channel_mgr.h   
 * ժ	 Ҫ:	hash_map��װ�Լ������ӵĹ���ĸ�����ʩ
 * 
 * ��ǰ�汾��	1.0
 * ��	 ��:	������(FTT)
 * ��	 ��:	�½�
 * �������:	2010��03��10��
 */
#ifndef __AOT_INET_CHANNEL_MGR_H_20100311__
#define __AOT_INET_CHANNEL_MGR_H_20100311__

#include <interface/aot_inet_interface.h>
#include <sys/sys_asy.h>

#pragma warning(disable:4312)
#pragma warning(disable:4311)

extern "C"{

aot_bool_t
__addr_cmp_fun(void* k1, void* k2);

aot_uint32_t
__addr_hash_fun(void* k);

aot_bool_t
__uint_cmp_fun(void* k1, void* k2);

aot_uint32_t
__uint_hash_fun(void* k);
};

namespace aot { namespace inet{ 

inline void set_channel_opt(aio_channel_op_t* cp,
							AIO_CNN_HANDLE c, 
							aio_connection_attr_t* attr, 
							aot_uint32_t send_high_water, 
							aot_int32_t cont_process_pkt_cnt, 
							aot_uint32_t send_mss, 
							aot_uint32_t recv_mss,
							int so_send_buf, 
							int so_recv_buf)
{
	attr->send_high_water = send_high_water;
	attr->cont_process_pkt_cnt = cont_process_pkt_cnt;
	attr->send_mss = send_mss;
	attr->recv_mss = recv_mss;
	int opt = so_send_buf;
	cp->set_sock_opt(c, SOL_SOCKET, SO_SNDBUF, (char*)&opt, sizeof (opt));
	opt = so_recv_buf;
	cp->set_sock_opt(c, SOL_SOCKET, SO_RCVBUF, (char*)&opt, sizeof (opt));
}

/// ���ø�������(һ���� ������ �� ������ ֮�������)����
inline void set_high_speed_channel_opt(aio_channel_op_t* cp, AIO_CNN_HANDLE c, aio_connection_attr_t* attr)
{
	set_channel_opt(cp, 
					c, 
					attr, 
					1024*1024*50,				/// send_high_water
					200,						/// cont_process_pkt_cnt
					AOT_HIGH_SPEED_SEND_MSS,			/// send_mss
					AOT_HIGH_SPEED_RECV_MSS,			/// recv_mss
					AOT_SO_HIGH_SPEED_SEND_BUF_SIZE,	/// so_send_buf
					AOT_SO_HIGH_SPEED_RECV_BUF_SIZE	/// so_recv_buf
					);
}

inline void set_normal_channel_opt(aio_channel_op_t* cp, AIO_CNN_HANDLE c, aio_connection_attr_t* attr)
{
	/// do nothing Ĭ�����ò������������ͨ���ӵ�
}

struct __hashmap_default_destroy_strategy
{
	template<class T>
	void destroy(T val){}
};

template<class INET_OP, class SYN_TRAITS, class DESTROY_STRATEGY = __hashmap_default_destroy_strategy>
class inet_channel_mgr_with_addrkey
{
public:
	typedef typename SYN_TRAITS::THREAD_MUTEX mutex_type;
	typedef xy::auto_lock<mutex_type> auto_lock_type;
public:
	typedef aot_hashmap_elem_t* iterator;
	iterator goto_begin()
	{
		return aot_hashmap_goto_begin(this->channels_);
	}
	iterator goto_next(iterator it)
	{
		return aot_hashmap_goto_next(this->channels_, it);
	}
	BOOL is_end(iterator it){ return aot_hashmap_is_end(it); }
	AIO_CNN_HANDLE get_val(iterator it){ return (AIO_CNN_HANDLE)it->val; }
	aot_inet_addr_t* get_key(iterator it){ return (aot_inet_addr_t*)it.key;}
	iterator remove_iter(iterator it)
	{ 
		AIO_CNN_HANDLE c = (AIO_CNN_HANDLE)it->val;
		iterator next = aot_hashmap_goto_next(this->channels_, it);
		this->inet_op_->aot_hashmap_op_->remove_with_pos(this->channels_, it);
		close_channel(c);
		return next;
	}
public:
	inet_channel_mgr_with_addrkey()	: inet_op_(NULL), channels_(NULL)
	{
		key_is_remote_addr_ = true;
	}
	~inet_channel_mgr_with_addrkey()
	{
		if( this->channels_ && this->inet_op_ )
		{
			clear();
			this->inet_op_->aot_hashmap_op_->destroy(this->channels_);
		}
	}
public:
	bool init(INET_OP* inet_op, aot_uint32_t max_bucket_count, aot_uint32_t init_elem_count)
	{
		this->inet_op_ = inet_op;

		this->channels_ = this->inet_op_->aot_hashmap_op_->create(max_bucket_count, init_elem_count, 
			&__addr_cmp_fun, &__addr_hash_fun);

		if( NULL == this->channels_ )
		{
			return false;
		}

		return true;
	}
	
	bool insert(AIO_CNN_HANDLE c, aio_connection_attr_t* attr, bool close_if_failed = true)
	{
		void* k = this->key_is_remote_addr_ ? (void*)&attr->remote_addr : (void*)&attr->local_addr;
		aot_hashmap_elem_t* pos = NULL;

		auto_lock_type __mon(this->mutex_);

		if( this->inet_op_->aot_hashmap_op_->find_pos(this->channels_, k, &pos) && pos )
		{
			/// Ҳ��ĳ��������ͬkey������û�м�ʱɾ��(����: �Զ˶Ͽ���,�������û��������⵽)
			close_channel(pos->val);
			/// close_channel���ʹposʧЧ,������ﲻ�ܼ���ʹ��pos,�������²���
		}

		if( !this->inet_op_->aot_hashmap_op_->insert(this->channels_, k, c) )
		{
			if( close_if_failed )
			{
				close_channel(c);
			}
			return false;
		}
		return true;
	}
	void remove(AIO_CNN_HANDLE c, aio_connection_attr_t* attr, bool close = false)
	{
		void* k = this->key_is_remote_addr_ ? (void*)&attr->remote_addr : (void*)&attr->local_addr;

		{
			auto_lock_type __mon(this->mutex_);
			this->inet_op_->aot_hashmap_op_->remove(this->channels_, k, NULL);
		}
		if( close )
		{
			close_channel(c);
		}
	}
	void remove(aot_inet_addr_t* addr, bool close = false)
	{
		void* k = (void*)addr;
		AIO_CNN_HANDLE c = NULL;

		{
			auto_lock_type __mon(this->mutex_);
			this->inet_op_->aot_hashmap_op_->remove(this->channels_, k, &c);
		}
		if( close && c )
		{
			close_channel(c);
		}
	}
	void clear()
	{
		auto_lock_type __mon(this->mutex_);

		if( this->channels_ )
		{
			AIO_CNN_HANDLE c;
			aot_hashmap_elem_t* nx;

			auto_lock_type __mon(this->mutex_);
			aot_hashmap_elem_t* pos = aot_hashmap_goto_begin(this->channels_);

			for( ; !aot_hashmap_is_end(pos);  )
			{
				nx = aot_hashmap_goto_next(this->channels_, pos);
				c = pos->val;
				this->inet_op_->aot_hashmap_op_->remove_with_pos(this->channels_, pos);
				close_channel(c);
				pos = nx;
			}
		}
	}
	void check_status()
	{
		auto_lock_type __mon(this->mutex_);
		if( this->channels_ )
		{			
			AIO_CNN_HANDLE c;
			aot_hashmap_elem_t* nx;
			DWORD t = GetTickCount();

			aot_hashmap_elem_t* pos = aot_hashmap_goto_begin(this->channels_);

			for( ; !aot_hashmap_is_end(pos);  )
			{
				nx = aot_hashmap_goto_next(this->channels_, pos);
				if( AOT_RET_ERROR == this->inet_op_->aio_channel_op_->check_heartbeat_status(pos->val, t) )
				{
					c = pos->val;
					this->inet_op_->aot_hashmap_op_->remove_with_pos(this->channels_, pos);
					close_channel(c);
				}
				pos = nx;
			}
		}
	}
	/// ����ʹ��pkt�Ŀ���,�������۽�����, @pkt�ɵ������ͷ�
	/// @skip_addr: ����ָ������(���㲥����)
	void broadcast(AIO_POOL_HANDLE pool, aot_buf_t* pkt, aot_inet_addr_t* skip_addr = NULL, bool close_if_failed = false)
	{
		auto_lock_type __mon(this->mutex_);
		if( this->channels_ )
		{			
			AIO_CNN_HANDLE c;
			aot_hashmap_elem_t* nx;
			aot_hashmap_elem_t* pos = aot_hashmap_goto_begin(this->channels_);

			for( ; !aot_hashmap_is_end(pos);  )
			{
				nx = aot_hashmap_goto_next(this->channels_, pos);
				if( skip_addr && __addr_cmp_fun(skip_addr, pos->key) )
				{
					pos = nx;
					continue;
				}

				aot_buf_t* b = this->inet_op_->aot_buf_op_->duplicate_ex(pool, pkt);
				if( b )
				{
					c = pos->val;
					if( AOT_RET_OK != this->inet_op_->aio_channel_op_->send(c, b) )
					{
						this->inet_op_->aot_buf_op_->destroy(b);
						if( close_if_failed )
						{
							this->inet_op_->aot_hashmap_op_->remove_with_pos(this->channels_, pos);
							close_channel(c);
						}
					}
				}

				pos = nx;
			}
		}
		
	}
	bool send_to(aot_inet_addr_t* addr, aot_buf_t* pkt, bool close_if_failed = false)
	{
		aot_hashmap_elem_t* pos = NULL;
		AIO_CNN_HANDLE c;
		auto_lock_type __mon(this->mutex_);
		if( this->inet_op_->aot_hashmap_op_->find_pos(this->channels_, (void*)addr, &pos) && pos )
		{
			if( AOT_RET_OK != this->inet_op_->aio_channel_op_->send(pos->val, pkt) )
			{
				if( close_if_failed )
				{
					c = pos->val;
					this->inet_op_->aot_hashmap_op_->remove_with_pos(this->channels_, pos);
					close_channel(c);
				}
				return false;
			}
			return true;
		}
		return false;
	}
	AIO_CNN_HANDLE find(aot_inet_addr_t* addr)
	{
		aot_hashmap_elem_t* pos = NULL;
		auto_lock_type __mon(this->mutex_);
		if( this->inet_op_->aot_hashmap_op_->find_pos(this->channels_, (void*)addr, &pos) && pos )
		{
			return pos->val;
		}
		return NULL;
	}
public:
	size_t connection_size()
	{ 
		if(this->channels_) 
			return this->channels_->elem_count;
		return 0;
	}
private:
	void close_channel(AIO_CNN_HANDLE c)
	{
		/// Ϊ��ͳһ����socket�Ĺر����,(ͳһ��inet_event_handler::on_close_connection������)
		/// close_channel�������Զ������ص�inet_event_handler::on_close_connection,
		/// ������Ȼ�ر�����ʱ,�������һ�β���(��ʵ�Ѿ���hashmap���Ƴ���),��: 
		/// 1. ���ӵĹر����ͳһ,���벻�»���; 
		/// 2. �ر����ӱ���������ƿ��, ����hashmap�������Ҳ�ǳ���

		this->destroy_strategy_.destroy(c);
		this->inet_op_->aio_channel_op_->close(c, 1);
	}
public:
	INET_OP* inet_op_;
public:
	aot_hashmap_t* channels_;
	mutex_type mutex_;
	DESTROY_STRATEGY destroy_strategy_;
	bool key_is_remote_addr_;
};

template<class INET_OP, class SYN_TRAITS, class DESTROY_STRATEGY = __hashmap_default_destroy_strategy>
class inet_channel_mgr_with_uintkey
{
public:
	typedef typename SYN_TRAITS::THREAD_MUTEX mutex_type;
	typedef xy::auto_lock<mutex_type> auto_lock_type;
public:
	typedef aot_hashmap_elem_t* iterator;
	iterator goto_begin()
	{
		return aot_hashmap_goto_begin(this->channels_);
	}
	iterator goto_next(iterator it)
	{
		return aot_hashmap_goto_next(this->channels_, it);
	}
	BOOL is_end(iterator it){ return aot_hashmap_is_end(it); }
	AIO_CNN_HANDLE get_val(iterator it){ return (AIO_CNN_HANDLE)it->val; }
	aot_uint32_t get_key(iterator it){ return reinterpret_cast<aot_uint32_t>(it->key);}
	iterator remove_iter(iterator it)
	{ 
		AIO_CNN_HANDLE c = (AIO_CNN_HANDLE)it->val;
		iterator next = aot_hashmap_goto_next(this->channels_, it);
		this->inet_op_->aot_hashmap_op_->remove_with_pos(this->channels_, it);
		close_channel(c);
		return next;
	}
public:
	inet_channel_mgr_with_uintkey(): inet_op_(NULL), channels_(NULL)
	{
	}
	~inet_channel_mgr_with_uintkey()
	{
		if( this->channels_ && this->inet_op_ )
		{
			clear();
			this->inet_op_->aot_hashmap_op_->destroy(this->channels_);
		}
	}
public:
	bool init(INET_OP* inet_op, aot_uint32_t max_bucket_count, aot_uint32_t init_elem_count)
	{
		this->inet_op_ = inet_op;

		this->channels_ = this->inet_op_->aot_hashmap_op_->create(max_bucket_count, init_elem_count, 
			&__uint_cmp_fun, &__uint_hash_fun);

		if( NULL == this->channels_ )
		{
			return false;
		}

		return true;
	}
	bool insert(AIO_CNN_HANDLE c, aio_connection_attr_t* attr, bool close_if_failed = true)
	{
		void* k = reinterpret_cast<void*>(attr->mark);
		aot_hashmap_elem_t* pos = NULL;

		auto_lock_type __mon(this->mutex_);

		if( this->inet_op_->aot_hashmap_op_->find_pos(this->channels_, k, &pos) && pos )
		{
			/// Ҳ��ĳ��������ͬkey������û�м�ʱɾ��(����: �Զ˶Ͽ���,�������û��������⵽)
			close_channel(pos->val);
			/// close_channel���ʹposʧЧ,������ﲻ�ܼ���ʹ��pos,�������²���
		}

		if( !this->inet_op_->aot_hashmap_op_->insert(this->channels_, k, c) )
		{
			if( close_if_failed )
			{
				close_channel(c);
			}
			return false;
		}
		return true;
	}
	void remove(AIO_CNN_HANDLE c, aio_connection_attr_t* attr, bool close = false)
	{
		void* k = (void*)attr->mark;

		{
			auto_lock_type __mon(this->mutex_);
			this->inet_op_->aot_hashmap_op_->remove(this->channels_, k, NULL);
		}
		if( close && c )
		{
			close_channel(c);
		}
	}
	void remove(aot_uint32_t key, bool close = false)
	{
		void* k = (void*)key;
		AIO_CNN_HANDLE c = NULL;

		{
			auto_lock_type __mon(this->mutex_);
			this->inet_op_->aot_hashmap_op_->remove(this->channels_, k, &c);
		}
		if( close && c )
		{
			close_channel(c);
		}
	}
	void clear()
	{
		if( this->channels_ )
		{
			AIO_CNN_HANDLE c;
			aot_hashmap_elem_t* nx;

			auto_lock_type __mon(this->mutex_);			
			aot_hashmap_elem_t* pos = aot_hashmap_goto_begin(this->channels_);

			for( ; !aot_hashmap_is_end(pos);  )
			{
				nx = aot_hashmap_goto_next(this->channels_, pos);
				c = pos->val;
				this->inet_op_->aot_hashmap_op_->remove_with_pos(this->channels_, pos);
				close_channel(c);
				pos = nx;
			}
		}
	}
	void check_status()
	{
		auto_lock_type __mon(this->mutex_);

		if( this->channels_ )
		{
			AIO_CNN_HANDLE c;
			aot_hashmap_elem_t* nx;
			DWORD t = GetTickCount();
			
			aot_hashmap_elem_t* pos = aot_hashmap_goto_begin(this->channels_);

			for( ; !aot_hashmap_is_end(pos);  )
			{
				nx = aot_hashmap_goto_next(this->channels_, pos);
				if( AOT_RET_ERROR == this->inet_op_->aio_channel_op_->check_heartbeat_status(pos->val, t) )
				{
					c = pos->val;
					this->inet_op_->aot_hashmap_op_->remove_with_pos(this->channels_, pos);
					close_channel(c);
				}
				pos = nx;
			}
		}
	}
	/// ����ʹ��pkt�Ŀ���,�������۽�����, @pkt�ɵ������ͷ�
	void broadcast(AIO_POOL_HANDLE pool, aot_buf_t* pkt, aot_uint32_t skip_id = 0, bool close_if_failed = false)
	{	
		auto_lock_type __mon(this->mutex_);
		if( this->channels_ )
		{			
			AIO_CNN_HANDLE c;
			aot_hashmap_elem_t* nx;
			aot_hashmap_elem_t* pos = aot_hashmap_goto_begin(this->channels_);

			for( ; !aot_hashmap_is_end(pos);  )
			{
				nx = aot_hashmap_goto_next(this->channels_, pos);
				if( 0 != skip_id && skip_id == (aot_uint32_t)pos->key )
				{
					pos = nx;
					continue;
				}

				aot_buf_t* b = this->inet_op_->aot_buf_op_->duplicate_ex(pool, pkt);
				if( b )
				{
					c = pos->val;
					if( AOT_RET_OK != this->inet_op_->aio_channel_op_->send(c, b) )
					{
						this->inet_op_->aot_buf_op_->destroy(pool, b);
						if( close_if_failed )
						{
							this->inet_op_->aot_hashmap_op_->remove_with_pos(this->channels_, pos);
							close_channel(c);
						}
					}
				}
				pos = nx;
			}
		}
	}
	bool send_to(aot_uint32_t mark, aot_buf_t* pkt, bool close_if_failed = false)
	{
		aot_hashmap_elem_t* pos = NULL;
		AIO_CNN_HANDLE c;
		auto_lock_type __mon(this->mutex_);

		if( this->inet_op_->aot_hashmap_op_->find_pos(this->channels_, (void*)mark, &pos) && pos )
		{
			if( AOT_RET_OK != this->inet_op_->aio_channel_op_->send(pos->val, pkt) )
			{
				if( close_if_failed )
				{
					c = pos->val;
					this->inet_op_->aot_hashmap_op_->remove_with_pos(this->channels_, pos);
					close_channel(c);
				}
				return false;
			}
			return true;
		}
		return false;
	}
	AIO_CNN_HANDLE find(aot_uint32_t key)
	{
		aot_hashmap_elem_t* pos = NULL;

		auto_lock_type __mon(this->mutex_);

		if( this->inet_op_->aot_hashmap_op_->find_pos(this->channels_, (void*)key, &pos) && pos )
		{
			return pos->val;
		}
		return NULL;
	}
public:
	size_t connection_size()
	{ 
		if(this->channels_) 
			return this->channels_->elem_count;
		return 0;
	}
private:
	void close_channel(AIO_CNN_HANDLE c)
	{
		/// Ϊ��ͳһ����socket�Ĺر����,(ͳһ��inet_event_handler::on_close_connection������)
		/// close_channel�������Զ������ص�inet_event_handler::on_close_connection,
		/// ������Ȼ�ر�����ʱ,�������һ�β���(��ʵ�Ѿ���hashmap���Ƴ���),��: 
		/// 1. ���ӵĹر����ͳһ,���벻�»���; 
		/// 2. �ر����ӱ���������ƿ��, ����hashmap�������Ҳ�ǳ���
		this->destroy_strategy_.destroy(c);
		this->inet_op_->aio_channel_op_->close(c, 1);
	}
public:
	INET_OP* inet_op_;
public:
	aot_hashmap_t* channels_;
	mutex_type mutex_;
	DESTROY_STRATEGY destroy_strategy_;
};

/// Ԫ��ֻ����ָ��
template<class VAL_PTR_TYPE, class SYN_TRAITS, class DESTROY_STRATEGY = __hashmap_default_destroy_strategy >
class uintkey_hashmap
{
public:
	typedef aot_hashmap_elem_t* iterator;
	typedef VAL_PTR_TYPE val_ptr_type;
	typedef typename SYN_TRAITS::THREAD_MUTEX mutex_type;
	typedef xy::auto_lock<mutex_type> auto_lock_type;
public:
	uintkey_hashmap() : map_(NULL), aot_hashmap_op_(NULL)
	{;}
	~uintkey_hashmap()
	{
		if( this->map_ && this->aot_hashmap_op_ )
		{
			clear();
			this->aot_hashmap_op_->destroy(this->map_);
		}
	}
	iterator goto_begin()
	{
		return aot_hashmap_goto_begin(this->map_);
	}
	iterator goto_next(iterator it)
	{
		return aot_hashmap_goto_next(this->map_, it);
	}
	BOOL is_end(iterator it){ return aot_hashmap_is_end(it); }
	val_ptr_type get_val(iterator it){ return (val_ptr_type)it->val; }
	aot_uint32_t get_key(iterator it){ return reinterpret_cast<aot_uint32_t>(it->key);}
	iterator remove_iter(iterator it)
	{ 
		iterator next = aot_hashmap_goto_next(this->map_, it);
		this->aot_hashmap_op_->remove_with_pos(this->map_, it);
		return next;
	}
public:
	void destroy(void* p)
	{
		this->destroy_strategy_.destroy((val_ptr_type)p);
	}

	bool init(aot_hashmap_op_t* hash_op, aot_uint32_t max_bucket_count, aot_uint32_t init_elem_count)
	{
		this->aot_hashmap_op_ = hash_op;

		this->map_ = this->aot_hashmap_op_->create(max_bucket_count, init_elem_count, 
			&__uint_cmp_fun, &__uint_hash_fun);

		if( NULL == this->map_ )
		{
			return false;
		}

		return true;
	}

	bool insert(aot_uint32_t key, val_ptr_type val, bool chk_unique = true)
	{
		void* k = reinterpret_cast<void*>(key);
		aot_hashmap_elem_t* pos = NULL;

		auto_lock_type __mon(this->mutex_);

		if( chk_unique )
		{
			if( this->aot_hashmap_op_->find_pos(this->map_, k, &pos) && pos )
			{
				destroy(pos->val);
				pos->key = k;
				pos->val = (void*)val;
				return true;
			}
		}

		return 0 != this->aot_hashmap_op_->insert(this->map_, k, val);
	}

	val_ptr_type find(aot_uint32_t key)
	{
		aot_hashmap_elem_t* pos = NULL;
		auto_lock_type __mon(this->mutex_);

		if( this->aot_hashmap_op_->find_pos(this->map_, (void*)key, &pos) && pos )
		{
			return (val_ptr_type)pos->val;
		}
		return (val_ptr_type)0;
	}

	bool find2(aot_uint32_t key, val_ptr_type* out_val = NULL)
	{
		aot_hashmap_elem_t* pos = NULL;
		auto_lock_type __mon(this->mutex_);

		if( this->aot_hashmap_op_->find_pos(this->map_, (void*)key, &pos) && pos )
		{
			if( out_val )
			{
				*out_val = (val_ptr_type)pos->val;
			}
			return true;
		}
		return false;
	}

	/*void remove(aot_uint32_t key, val_ptr_type* out_val)
	{
		void* k = reinterpret_cast<void*>(key);
		auto_lock_type __mon(this->mutex_);
		this->aot_hashmap_op_->remove(this->map_, k, (void**)&out_val);
	}*/

	void remove(aot_uint32_t key)
	{
		val_ptr_type val = 0;
		void* k = reinterpret_cast<void*>(key);
		{
			auto_lock_type __mon(this->mutex_);
			this->aot_hashmap_op_->remove(this->map_, k, (void**)&val);
		}
		destroy(val);
	}

	void clear()
	{
		if( this->map_ )
		{
			auto_lock_type __mon(this->mutex_);

			aot_hashmap_elem_t* nx;
			aot_hashmap_elem_t* pos = aot_hashmap_goto_begin(this->map_);

			for( ; !aot_hashmap_is_end(pos);  )
			{
				nx = aot_hashmap_goto_next(this->map_, pos);
				destroy(pos->val);
				this->aot_hashmap_op_->remove_with_pos(this->map_, pos);
				pos = nx;
			}
		}
	}

	aot_uint32_t size()
	{
		return this->map_->elem_count;
	}
public:
	aot_hashmap_op_t* aot_hashmap_op_;
	aot_hashmap_t* map_;
	mutex_type mutex_;
	DESTROY_STRATEGY destroy_strategy_;
};

}} /// end namespace aot/inet

#pragma warning(default:4311)
#pragma warning(default:4312)

#endif /// __AOT_INET_CHANNEL_MGR_H_20100311__