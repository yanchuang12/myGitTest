/** Copyright (c) 2008 
* All rights reserved.
* 
* 文件名称:		sys_mm_pool.h
* 摘	 要:	内存管理类(定长内存块的内存池)
* 
* 当前版本：	1.0
* 作	 者:	范涛涛
* 操	 作:	新建
* 完成日期:		2008年4月24日
*/

#ifndef __SYS_MM_POOL_H__
#define __SYS_MM_POOL_H__

#include "sys_globa_def.h"
#include "sys_allocate.h"
#include "sys_asy.h"
#include<algorithm>
#include <vector>
namespace xy
{
//MUTEX: thread_mutex, null_mutex
template<class MUTEX> 
class fix_mm_pool
{
	typedef union tag_MM_LINK_T
	{
		union tag_MM_LINK_T* next;
		char data;
	}MM_LINK_T;

	#define AUTO_LOCK auto_lock_type tmp_lock(m_lock);

public:
	typedef default_alloc_1nd alloc_type;
	typedef MUTEX	lock_type;
	typedef auto_lock<MUTEX>	auto_lock_type;


	
public:
	fix_mm_pool(size_t block_size, 
		size_t init_block_num = 8,
		size_t max_block_num = 0x7fffffff)
		: m_block_size(block_size), 
		  m_max_block_num(max_block_num),
		  m_current_block_num(0)
		  
	{
		m_free_link.next = 0;
		m_lock.open();
		enlarge(&init_block_num);

	}

	virtual ~fix_mm_pool()
	{
		__free_all();
		m_lock.close();
	}

	//从内存池获取一块内存块,如无法获取内存,返回null
	void* allocate(size_t inst=0)
	{
		AUTO_LOCK;
		void* ret_ptr = 0;
		if(m_free_link.next)
		{	//有空闲内存
			ret_ptr = (void*)m_free_link.next;
			m_free_link.next = m_free_link.next->next;
			m_current_block_num++;
			return ret_ptr;
		}
		else
		{	//没有空闲内存
			size_t num = (m_current_block_num < 1 ? 1: m_current_block_num) * 2;
			if(!enlarge(&num))
				return (void*)0;
			//递归回调
			return allocate();
		}
		return (void*)0;
	}

	void* allocate_no_throw(size_t inst=0)
	{
		return allocate(0);
	}

	//归还一块内存块,放入内存池
	void deallocate(void* p, size_t inst=0)
	{
		AUTO_LOCK;
		MM_LINK_T* free_link_ptr = 0;
		free_link_ptr = m_free_link.next;
		m_free_link.next = (MM_LINK_T*)(p); 
		m_free_link.next->next = free_link_ptr;

		if(--m_current_block_num < 0)
		{
			m_current_block_num = 0;
		}
		p = 0;
	}
protected:


	fix_mm_pool(const fix_mm_pool<MUTEX>& other)
	{
		
	}

	fix_mm_pool& operator=(const fix_mm_pool<MUTEX>& other)
	{
		return (*this);
	}
private:
	struct __release_data
	{
		void operator()(void* p)
		{
			alloc_type::deallocate(p, 0);
		}
	};

	//释放内存池
	void __free_all()
	{
		std::for_each(m_v_data_ptr.begin(), m_v_data_ptr.end(), __release_data());
		m_v_data_ptr.clear();
		
		m_free_link.next = 0;
		m_current_block_num = 0;
	}
	
	//增加*block_num 块内存, 如果无法分配到足够的内存, 则自动将*block_num降低后再重新尝试,
	//flase: 无法获取内存,
	bool enlarge(size_t* block_num)
	{
		MM_LINK_T* free_link_ptr = 0;
		char* p = 0;

		assert(*block_num > 0);
		if((*block_num) + m_current_block_num > m_max_block_num)
		{
			(*block_num) = m_max_block_num - m_current_block_num;
		}

		//先尝试分配尽量满足要求的内存块, 采用不抛出异常的方式分配
		p = (char*)alloc_type::allocate_no_throw(m_block_size * (*block_num));
		if(!p)
		{
			//如果连一块内存都无法分配
			if(1 >= *block_num)
				return false;
			//失败. 则再尝试只分配一半的内存
			(*block_num) = (*block_num / 2);
			return enlarge(block_num);
		}
		//加入free链表
		for(int i = 0; i < (*block_num); ++i)
		{
			free_link_ptr = m_free_link.next;
			m_free_link.next = (MM_LINK_T*)(p + i * m_block_size); 
			m_free_link.next->next = free_link_ptr;
		}

		m_v_data_ptr.push_back((void*)p);
		return true;
	}

	

private:
	size_t m_block_size;		//每个内存块的大小
	size_t m_max_block_num;		//最大内存块数量
	size_t m_current_block_num;	//当前已使用的内存块数
	MM_LINK_T m_free_link;		//空闲内存块链表
	std::vector<void*> m_v_data_ptr;		//分配的内存指针链表, 释放内存时,必须通过这个结构来释放
	lock_type m_lock;		//线程锁

};

};

#endif