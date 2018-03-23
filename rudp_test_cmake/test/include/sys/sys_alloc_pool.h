#ifndef __AOT_SYS_ALLOC_POOL_H__
#define __AOT_SYS_ALLOC_POOL_H__

#include <commondef/aot_typedef.h>
#include <new>
#include <sys/sys_asy.h>

#ifdef _DEBUG
#define AOT_SYS_POOL_NEED_FREE
#endif

#ifdef AOT_SYS_POOL_NEED_FREE
#include <list>
#include <algorithm>
#endif

namespace xy{

struct default_cpp_alloc_1nd
{
	inline static void* allocate(size_t n)
	{
		try { void* p = (void*) ::operator new (n); return p; }
		catch(...) { return NULL; }
	}
	inline static void deallocate(void* p, size_t inst = 0) { if(p) ::operator delete (p); }
};

struct default_c_alloc_1nd
{
	inline static void* allocate(size_t n)
	{
		try { void* p = (void*) ::operator new (n); return p; }
		catch(...) { return NULL; }
	}
	inline static void deallocate(void* p, size_t inst = 0) { if(p) ::operator delete (p); }
};

template<class DEFAULT_ALLOC, class MUTEX> 
class fix_alloc_pool
{
public:
	union link_t
	{
		union link_t* next;
		char data;
	};
	typedef xy::auto_lock<MUTEX> auto_lock_type;
public:
	fix_alloc_pool(size_t block_size, size_t init_block_num, size_t max_block_num = 0x7fffffff)
		: block_size_(block_size), max_block_num_(max_block_num), current_block_num_(0)
	{
		assert( init_block_num > 0 );
		free_link_.next = NULL;
		enlarge(&init_block_num);
	}

	~fix_alloc_pool() 
	{
#ifdef AOT_SYS_POOL_NEED_FREE
		std::for_each(this->destroy_lst_.begin(), this->destroy_lst_.end(), __release_data());
#endif
	}

	MUTEX& mutex(){ return this->mutex_; }
	void* allocate(size_t inst = 0)
	{
		auto_lock_type __mon(this->mutex_);
		return allocate_i(inst);
	}
	/// �黹һ���ڴ��,�����ڴ��
	void deallocate(void* p, size_t inst = 0)
	{
		auto_lock_type __mon(this->mutex_);

		link_t* free_link_ptr = this->free_link_.next;
		this->free_link_.next = (link_t*)(p); 
		this->free_link_.next->next = free_link_ptr;
		--this->current_block_num_;
	}
protected:
	fix_alloc_pool(const fix_alloc_pool<MUTEX>& other);
	fix_alloc_pool& operator=(const fix_alloc_pool<MUTEX>& other);
private:
	/// ���ڴ�ػ�ȡһ���ڴ��,���޷���ȡ�ڴ�,����null
	void* allocate_i(size_t inst = 0)
	{
		void* ret_ptr = NULL;

		if( this->free_link_.next )
		{	/// �п����ڴ�
			ret_ptr = (void*)this->free_link_.next;
			this->free_link_.next = this->free_link_.next->next;
			++this->current_block_num_;
			return ret_ptr;
		}
		else
		{	/// û�п����ڴ�
			if( this->max_block_num_ <= this->current_block_num_ )
			{
				return NULL;
			}
			size_t num = this->max_block_num_ - this->current_block_num_;
			if( num > 128 )
				num = 128;
			if( !enlarge(&num) )
				return NULL;
			/// �ݹ�ص�
			return allocate_i();
		}
		return NULL;
	}
	/// ����*block_num ���ڴ�, ����޷����䵽�㹻���ڴ�, ���Զ���*block_num���ͺ������³���,
	/// false: �޷���ȡ�ڴ�
	bool enlarge(size_t* block_num)
	{
		link_t* free_link_ptr = NULL;
		char* p = (char*)ALLOC::allocate(this->block_size_ * (*block_num) );
		if( 0 == p )
		{
			/// �����һ���ڴ涼�޷�����
			if( 1 >= *block_num )
				return false;
			/// ʧ��. ���ٳ���ֻ����һ����ڴ�
			(*block_num) /= 10;
			if( *block_num < 1 )
				return false;
			return enlarge(block_num);
		}
#ifdef AOT_SYS_POOL_NEED_FREE
		this->destroy_lst_.push_back((void*)p);
#endif
		/// ����free����
		for(size_t i = 0; i < (*block_num); ++i)
		{
			free_link_ptr = this->free_link_.next;
			this->free_link_.next = (link_t*)(p + i * this->block_size_); 
			this->free_link_.next->next = free_link_ptr;
		}
		return true;
	}
private:
	size_t block_size_;			/// ÿ���ڴ��Ĵ�С
	size_t max_block_num_;		/// ����ڴ������
	size_t current_block_num_;	/// ��ǰ��ʹ�õ��ڴ����
	link_t free_link_;			/// �����ڴ������
	MUTEX mutex_;
#ifdef AOT_SYS_POOL_NEED_FREE
	std::list<void*> destroy_lst_;

	struct __release_data
	{
		void operator()(void* p)
		{
			ALLOC::deallocate(p);
		}
	};
#endif
};

struct alloc_param_t
{
	/// �������
	enum{ e_size = 11 };

	alloc_param_t()
	{
		table_[0] = 0;
		for( size_t i = 1; i <= 32768; ++i )
		{
			if( i <= 32 ) table_[i] = 0;
			else if( i > 32 && i <= 64 ) table_[i] = 1;
			else if( i > 64 && i <= 128 ) table_[i] = 2;
			else if( i > 128 && i <= 256 ) table_[i] = 3;
			else if( i > 256 && i <= 512 ) table_[i] = 4;
			else if( i > 512 && i <= 1024 ) table_[i] = 5;
			else if( i > 1024 && i <= 2048 ) table_[i] = 6;
			else if( i > 2048 && i <= 4096 ) table_[i] = 7;
			else if( i > 4096 && i <= 8192 ) table_[i] = 8;
			else if( i > 8192 && i <= 16384 ) table_[i] = 9;
			else if( i > 16384 && i <= 32768 ) table_[i] = 10;
		}
	}
	/// ���������block��С
	inline static const size_t* block_size_v()
	{
		///                              1   2    4    8    16   32    64    128   256   512    1024
		static const size_t v[e_size] = {32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768};
		return v;
	}
	/// ���ݳ���ȷ�������±�
	inline size_t index(size_t len)
	{ 
		return len > 32768 ? (0x7fffffff) : this->table_[len];
	}
	inline const size_t* low_water_v()
	{
		static const size_t v[e_size] = {1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 512, 256};
		return v;
	}
	inline const size_t* high_water_v()
	{
		///�ܼ�: �شﵽ��ˮλʱ�󿬻� �õ�: 480M�ڴ�
		///                               320K     640K      1M       2M       5M       10M     20M       40M     80M       160M    160M
		static const size_t v[e_size] = {1024*10, 1024*10, 1024*10, 1024*10, 1024*10, 1024*10, 1024*10, 1024*10, 1024*10, 1024*10, 1024*5};
		return v;
	}
private:
	uint8_t table_[32768 + 1];
};


template<class PARAM_T, class DEFAULT_ALLOC, class MUTEX>
class sys_alloc_pool
{
	typedef fix_alloc_pool<DEFAULT_ALLOC, MUTEX> pool_type;
	enum {e_size = PARAM_T::e_size};
public:
	sys_alloc_pool()
	{
		init();
	}
	~sys_alloc_pool()
	{
		for( size_t i = 0; i < e_size; ++i )
		{
			delete this->pool_[i];
		}
	}
public:
	void init()
	{
		for( size_t i = 0; i < e_size; ++i )
		{
			this->pool_[i] = new pool_type( this->param_.block_size_v()[i], this->param_.low_water_v()[i], this->param_.high_water_v()[i]);
		}
	}
	void* allocate(size_t len)
	{
		size_t i = this->param_.index(len);
		if( i < e_size )
			return this->pool_[i]->allocate(len);

		return default_alloc_1nd::allocate(len);
	}
	void deallocate(void* p, size_t len)
	{
		size_t i = this->param_.index(len);
		if( i < e_size )
		{
			this->pool_[i]->deallocate(p, len);
		}
		else
		{
			default_alloc_1nd::deallocate(p, len);
		}
	}
private:
	PARAM_T param_;
	pool_type* pool_[e_size];
};

typedef sys_alloc_pool<alloc_param_t, xy::thread_mutex> mt_sys_cpp_alloc_pool;

} /// end namespace xy

#endif /// __SYS_ALLOC_POOL_H__