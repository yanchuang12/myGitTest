/** Copyright (c) 2008 
* All rights reserved.
* 
* 文件名称:		sys_allocate.h
* 摘	 要:	提供底层内存分配/释放的一致接口. 参照sgi_stl实现
* 
* 当前版本：	1.0
* 作	 者:	范涛涛
* 操	 作:	新建
* 完成日期:		2008年4月24日
*/

#ifndef __SYS_ALLOCATE_H__
#define __SYS_ALLOCATE_H__

#include "sys_globa_def.h"
#include "sys_asy.h"
#include "sys_singleton.h"
#include <new>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace xy
{

template <class T>
inline void __destroy (T* pointer)
{
    pointer->~T();
}

inline void __destroy (void*)             {;}
inline void __destroy (char*)             {;}
inline void __destroy (unsigned char*)    {;}
inline void __destroy (short*)            {;}
inline void __destroy (unsigned short*)   {;}
inline void __destroy (int*)              {;}
inline void __destroy (unsigned int*)     {;}
inline void __destroy (long*)             {;}
inline void __destroy (unsigned long*)    {;}
inline void __destroy (float*)            {;}
inline void __destroy (double*)           {;}
inline void __destroy (void**)            {;}
inline void __destroy (char**)            {;}
inline void __destroy (unsigned char**)   {;}
inline void __destroy (short**)           {;}
inline void __destroy (unsigned short**)  {;}
inline void __destroy (int**)             {;}
inline void __destroy (unsigned int**)    {;}
inline void __destroy (long**)            {;}
inline void __destroy (unsigned long**)   {;}
inline void __destroy (float**)           {;}
inline void __destroy (double**)          {;}
inline void __destroy (void***)           {;}
inline void __destroy (char***)           {;}
inline void __destroy (unsigned char***)  {;}
inline void __destroy (short***)          {;}
inline void __destroy (unsigned short***) {;}
inline void __destroy (int***)            {;}
inline void __destroy (unsigned int***)   {;}
inline void __destroy (long***)           {;}
inline void __destroy (unsigned long***)  {;}
inline void __destroy (float***)          {;}
inline void __destroy (double***)         {;}

inline void __destroy (bool*)             {;}
inline void __destroy (bool**)            {;}
inline void __destroy (bool***)           {;}

inline void __destroy (long double*)      {;}
inline void __destroy (long double**)     {;}
inline void __destroy (long double***)    {;}




template <class T1, class T2>
inline void __construct (T1* p, const T2& value)
{
    new (p) T1(value);
}

//分配器
class default_alloc_1nd
{
public:
	default_alloc_1nd(){}
	~default_alloc_1nd(){}
public:

	inline static  void* allocate(size_t n)
	{
		try
		{
			void* p = (void*) ::operator new (n);
			if(!p)
			{
				__throw("memory is empty");
			}
			return p;
		}
		catch(...)
		{
			__throw("memory is empty");
		}
	}

	inline static void* allocate_no_throw(size_t n)
	{
		try
		{
			
			void* p = (void*) ::operator new (n);
			return p;
		}
		catch(...)
		{
			return (void*)0;
		}
	}

	inline static void deallocate(void* p, size_t noused = 0)
	{
		if(p)
			::operator delete (p);
	}
};


class alloc_base
{	
public:
	inline virtual void* allocate(size_t n)
	{
		return default_alloc_1nd::allocate(n);
	}
	inline virtual void* allocate_no_throw(size_t n)
	{
		return default_alloc_1nd::allocate_no_throw(n);
	}
	inline virtual void deallocate(void* p, size_t n)
	{
		default_alloc_1nd::deallocate(p, n);
	}

	template <class T1, class T2>
	static inline void construct (T1* p, const T2& value)
	{
		__construct(p, value);
	}

	template <class T>
	static inline void destroy (T* pointer)
	{
		__destroy(pointer);
	}
protected:
	alloc_base(){;}
	virtual ~alloc_base(){;}
private:
	alloc_base(const alloc_base&);
    const alloc_base& operator=(const alloc_base&);
};

//适配器
template<class ALLOC>
class alloc_adapter : public alloc_base, public ALLOC
{
public:
	alloc_adapter(){;}
	virtual ~alloc_adapter(){;}
public:
	inline virtual void* allocate(size_t n)
	{
		return ALLOC::allocate(n);
	}
	inline virtual void* allocate_no_throw(size_t n)
	{
		return ALLOC::allocate_no_throw(n);
	}
	inline virtual void deallocate(void* p, size_t n)
	{
		ALLOC::deallocate(p, n);
	}
	
};



//小块内存的内存池, 移植sgi_stl的实现
template <class MUTEX, int inst = 0>
class small_obj_alloc : private copy_disable
{
private:
	#define AUTO_LOCK auto_lock_type tmp_lock(m_lock);
	typedef auto_lock< mutex_adapter<MUTEX> > auto_lock_type;
	typedef default_alloc_1nd alloc_type;

	enum {_ALIGN = 8};//对齐基数
	enum {_MAX_BYTES = 128};//所管理的最大内存块长度
	enum {_NFREELISTS = 16}; //空闲链表

	union MM_LINK_T
	{
		MM_LINK_T* next;
		char data[1];
	};	

public:

	static void* allocate(size_t n)
	{
		void* __ret = 0;
		
		if (n > (size_t) _MAX_BYTES) 
		{
			__ret = alloc_type::allocate_no_throw(n);
		}
		else 
		{
			MM_LINK_T* volatile* __my_free_list = m_free_list + FREELIST_INDEX(n);

			AUTO_LOCK;

			MM_LINK_T*  __result = *__my_free_list;
			if (__result == 0)
			{
				__ret = __refill(ROUND_UP(n));
			}
			else 
			{
				*__my_free_list = __result->next;
				__ret = __result;
			}
		}
		
		return __ret;
	};

	static void* allocate_no_throw(size_t n)
	{
		return allocate(n);
	}

	static void deallocate(void* p, size_t n)
	{
		if(0 == p)
			return;

		if (n > (size_t) _MAX_BYTES)
			alloc_type::deallocate(p, n);
		else 
		{
			MM_LINK_T* volatile*  __my_free_list = m_free_list + FREELIST_INDEX(n);
			MM_LINK_T* __q = (MM_LINK_T*)p;
			
			AUTO_LOCK;

			__q->next = *__my_free_list;
			*__my_free_list = __q;
		}
	}


private:	

	//上调至8倍数
	inline static size_t ROUND_UP(size_t bytes) //
	{ 
		return (((bytes) + (size_t) _ALIGN-1) & ~((size_t) _ALIGN - 1)); 
	}

	//根据bytes确定: m_free_list下标
	inline static  size_t FREELIST_INDEX(size_t bytes) 
	{
        return (((bytes) + (size_t)_ALIGN-1)/(size_t)_ALIGN - 1);
	}

	static char* __chunk_alloc(size_t __size, int& __nobjs)
	{
		char* __result;
		size_t __total_bytes = __size * __nobjs;
		size_t __bytes_left = m_end_free - m_start_free;
		
		if (__bytes_left >= __total_bytes) 
		{
			__result = m_start_free;
			m_start_free += __total_bytes;
			return(__result);
		} 
		else if (__bytes_left >= __size) 
		{
			__nobjs = (int)(__bytes_left/__size);
			__total_bytes = __size * __nobjs;
			__result = m_start_free;
			m_start_free += __total_bytes;
			return(__result);
		} 
		else 
		{
			size_t __bytes_to_get =  2 * __total_bytes + ROUND_UP(m_heap_size >> 4);
			
			if (__bytes_left > 0) 
			{
				MM_LINK_T* volatile* __my_free_list =
					m_free_list + FREELIST_INDEX(__bytes_left);
				
				((MM_LINK_T*)m_start_free)->next = *__my_free_list;
				*__my_free_list = (MM_LINK_T*)m_start_free;
			}
			m_start_free = (char*)alloc_type::allocate_no_throw(__bytes_to_get);
			if (0 == m_start_free) 
			{
				size_t __i;
				MM_LINK_T* volatile* __my_free_list;
				MM_LINK_T* __p;
				//从其它块中索取空余内存
				for (__i = __size; __i <= (size_t) _MAX_BYTES; __i += (size_t) _ALIGN) 
				{
					__my_free_list = m_free_list + FREELIST_INDEX(__i);
					__p = *__my_free_list;
					if (0 != __p) 
					{
						*__my_free_list = __p->next;
						m_start_free = (char*)__p;
						m_end_free = m_start_free + __i;
						return(__chunk_alloc(__size, __nobjs));
					}
				}
				m_end_free = 0;	
				return (char*)0;//确实一点内存都没有了
				
			}
			m_heap_size += __bytes_to_get;
			m_end_free = m_start_free + __bytes_to_get;
			return(__chunk_alloc(__size, __nobjs));
		}
	}

	static void* __refill(size_t __n)
	{
		int __nobjs = 20;
		char* __chunk = __chunk_alloc(__n, __nobjs);
		MM_LINK_T* volatile* __my_free_list;
		MM_LINK_T* __result;
		MM_LINK_T* __current_obj;
		MM_LINK_T* __next_obj;
		int __i;

		if (1 == __nobjs) return(__chunk);
		
		__my_free_list = m_free_list + FREELIST_INDEX(__n);

		  __result = (MM_LINK_T*)__chunk;
		  *__my_free_list = __next_obj = (MM_LINK_T*)(__chunk + __n);
		  for (__i = 1; ; __i++) 
		  {
			__current_obj = __next_obj;
			__next_obj = (MM_LINK_T*)((char*)__next_obj + __n);

			if (__nobjs - 1 == __i) 
			{
				__current_obj->next = 0;
				break;
			} 
			else
			{
				__current_obj->next = __next_obj;
			}
		  }
		return(__result);
	}

private:
	static mutex_adapter<MUTEX> m_lock;
	static char* m_start_free;
	static char* m_end_free;
	static size_t m_heap_size;
	static MM_LINK_T*  volatile m_free_list[_NFREELISTS]; 
};

typedef small_obj_alloc<null_mutex, 0> default_alloc;
typedef small_obj_alloc<thread_mutex, 0> mt_alloc;



template<class T, class MUTEX>
class class_obj_alloc
{

};
};



namespace xy
{

	template <class MUTEX, int inst> 
		typename mutex_adapter< MUTEX >  small_obj_alloc<MUTEX, inst>::m_lock;

	template <class MUTEX, int inst> 
		char* small_obj_alloc<MUTEX, inst>::m_start_free = 0;

	template <class MUTEX, int inst> 
		char* small_obj_alloc<MUTEX, inst>::m_end_free = 0;

	template <class MUTEX, int inst>
		size_t small_obj_alloc<MUTEX, inst>::m_heap_size = 0;

	template <class MUTEX, int inst> 
		typename small_obj_alloc<MUTEX, inst>::MM_LINK_T* volatile
		small_obj_alloc<MUTEX, inst>::m_free_list[] = 
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

};



#endif // ifndef __SYS_ALLOCATE_H__