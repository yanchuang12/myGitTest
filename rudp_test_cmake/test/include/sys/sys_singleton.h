/** Copyright (c) 2008 
* All rights reserved.
* 
* �ļ�����:		sys_singleton.h
* ժ	 Ҫ:	singletonģʽ
* 
* ��ǰ�汾��	1.0
* ��	 ��:	������
* ��	 ��:	�½�
* �������:		2008��5��6��
*/

#ifndef __SYS_SINGLETON_H__
#define __SYS_SINGLETON_H__

#include "sys_globa_def.h"
namespace xy
{

class copy_disable
{
private:
    copy_disable(const copy_disable&);
    const copy_disable& operator=(const copy_disable&);

protected:
    copy_disable() { ;}
    ~copy_disable() { ;}

 };

template <class T, class MUTEX>
class singleton : private copy_disable 
{
private:
	struct __destroy
	{
		__destroy(T** p) : m_p(p)
		{
			///assert(*p);
		}
		~__destroy()
		{
			try
			{
				if(*m_p)
					delete (*m_p);
			}
			catch (...)
			{
				
			}
		}
		T** m_p;
	};
public:
	static T*& instance()
	{
		static T* p = 0;    
		static mutex_adapter<MUTEX> lock;
		
		if(!p) 
		{
			auto_lock< mutex_adapter<MUTEX> > __mon(lock);
			if(!p)        
			{
				p = new T;
				static __destroy des(&p);
			}
			
		}
		
		return p;
	}

	static void destroy()
	{
		T*& p = singleton<T, MUTEX>::instance();

		if(p)
		{
			delete p;
			p = NULL;
		}
	}

};





};

#endif //__SYS_SINGLETON_H__