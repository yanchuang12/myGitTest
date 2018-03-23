/** Copyright (c) 2008 
* All rights reserved.
* 
* �ļ�����:		sys_mm_pool.h
* ժ	 Ҫ:	�ڴ������(�����ڴ����ڴ��)
* 
* ��ǰ�汾��	1.0
* ��	 ��:	������
* ��	 ��:	�½�
* �������:		2008��4��24��
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

	//���ڴ�ػ�ȡһ���ڴ��,���޷���ȡ�ڴ�,����null
	void* allocate(size_t inst=0)
	{
		AUTO_LOCK;
		void* ret_ptr = 0;
		if(m_free_link.next)
		{	//�п����ڴ�
			ret_ptr = (void*)m_free_link.next;
			m_free_link.next = m_free_link.next->next;
			m_current_block_num++;
			return ret_ptr;
		}
		else
		{	//û�п����ڴ�
			size_t num = (m_current_block_num < 1 ? 1: m_current_block_num) * 2;
			if(!enlarge(&num))
				return (void*)0;
			//�ݹ�ص�
			return allocate();
		}
		return (void*)0;
	}

	void* allocate_no_throw(size_t inst=0)
	{
		return allocate(0);
	}

	//�黹һ���ڴ��,�����ڴ��
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

	//�ͷ��ڴ��
	void __free_all()
	{
		std::for_each(m_v_data_ptr.begin(), m_v_data_ptr.end(), __release_data());
		m_v_data_ptr.clear();
		
		m_free_link.next = 0;
		m_current_block_num = 0;
	}
	
	//����*block_num ���ڴ�, ����޷����䵽�㹻���ڴ�, ���Զ���*block_num���ͺ������³���,
	//flase: �޷���ȡ�ڴ�,
	bool enlarge(size_t* block_num)
	{
		MM_LINK_T* free_link_ptr = 0;
		char* p = 0;

		assert(*block_num > 0);
		if((*block_num) + m_current_block_num > m_max_block_num)
		{
			(*block_num) = m_max_block_num - m_current_block_num;
		}

		//�ȳ��Է��価������Ҫ����ڴ��, ���ò��׳��쳣�ķ�ʽ����
		p = (char*)alloc_type::allocate_no_throw(m_block_size * (*block_num));
		if(!p)
		{
			//�����һ���ڴ涼�޷�����
			if(1 >= *block_num)
				return false;
			//ʧ��. ���ٳ���ֻ����һ����ڴ�
			(*block_num) = (*block_num / 2);
			return enlarge(block_num);
		}
		//����free����
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
	size_t m_block_size;		//ÿ���ڴ��Ĵ�С
	size_t m_max_block_num;		//����ڴ������
	size_t m_current_block_num;	//��ǰ��ʹ�õ��ڴ����
	MM_LINK_T m_free_link;		//�����ڴ������
	std::vector<void*> m_v_data_ptr;		//������ڴ�ָ������, �ͷ��ڴ�ʱ,����ͨ������ṹ���ͷ�
	lock_type m_lock;		//�߳���

};

};

#endif