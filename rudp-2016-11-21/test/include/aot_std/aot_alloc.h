/** Copyright (c) 2015-2016
 * All rights reserved.
 * 
 * �ļ�����:	aot_alloc.h  
 * ժ	 Ҫ:	�������ݽṹ��װ,�ȱ�׼����.��
 * 
 * ��ǰ�汾��	1.0
 * ��	 ��:	������(FTT)
 * ��	 ��:	�½�
 * �������:	2015��05��11��
*/

#ifndef __AOT_STD_ALLOC_201505114_H__
#define __AOT_STD_ALLOC_201505114_H__

#include "aot_std_typedef.h"
#include <new>

namespace aot_std{

struct default_alloc
{
	static void* alloc(size_t size)
	{
		return (void*)::operator new (size);
	}
	static void* calloc(size_t size)
	{
		void* p = (void*) ::operator new (size);
		if( p )
		{
			memset(p, 0, size);
		}
		return p;
	}
	inline static void dealloc(void* p)
	{
		::operator delete (p);
	}
};

/*
template<class T>
struct object_alloc
{
public:
	union node_t
	{
		node_t* next;
		char data;
	};

	object_alloc()
	{
		this->max_num_ = 0;
		this->block_len_ = 0;
		this->lst_.next = NULL;
	}
public:
	bool init(int max_num)
	{
		this->max_num_ = max_num;
		this->block_len_ = block_len;
		return true;
	}
private:
	int block_len_;
	int max_num_;
	node_t lst_;
};
*/

} /// end namespace aot_std

#endif /// __AOT_STD_ALLOC_201505114_H__