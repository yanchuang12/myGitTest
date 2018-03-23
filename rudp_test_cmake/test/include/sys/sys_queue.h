/** Copyright (c) 2008 
* All rights reserved.
* 
* 文件名称:		sys_queue.h
* 摘	 要:	队列
* 
* 当前版本：	1.0
* 作	 者:	范涛涛
* 操	 作:	新建
* 完成日期:		2008年5月20日
*/

#ifndef __SYS_QUEUE_H__
#define __SYS_QUEUE_H__

#include "sys_globa_def.h"
#include "sys_asy.h"
#include <queue>
#include <functional>
#include <deque>
namespace xy
{
	template<class SYN_TRAITS, class T>
	class queue : private std::queue<T>
	{
	public:
		enum 
		{
			e_low_water = 0,
			e_high_water = 1024*32
		};
		enum
		{
			e_active = 0x01,
			e_shutdown
		};
		typedef typename SYN_TRAITS::THREAD_MUTEX mutex_type;
		typedef typename SYN_TRAITS::THREAD_CONDITION condition_type;
		typedef queue<SYN_TRAITS, T> this_type;
		typedef std::queue<T> base_type;
	public:
		queue(size_t high_water = 0x7fffffff, size_t low_water = e_low_water)
			: m_high_water(high_water), m_low_water(low_water), m_status(e_shutdown)
		{
			if(!__init())
				__throw("msg_queue init error");
		}

		~queue()
		{
			__uninit();
		}

		void clear()
		{
			auto_lock<mutex_type> __mon(m_mutex);
			while(!base_type::empty())
			{
				base_type::pop();
			}
		}
		int status() const {return m_status;}
		bool is_shut_down(){return m_status == e_shutdown;}
		void active(){m_status = e_active;}
		void shut_down(bool bfree = false)
		{
			if(m_status != e_shutdown)
			{
				m_status = e_shutdown;
				m_cond_producer.broadcast();
				m_cond_customer.broadcast();
				if(bfree)
					clear();
			}
		}

		size_t high_water() const {return m_high_water;}
		size_t low_water() const {return m_low_water;}

		void high_water(size_t n)  { m_high_water = n;}
		void low_water(size_t n)  { m_low_water = n;}

		size_t enqueue_waiters(){return m_cond_producer.waiters_num();}
		size_t dequeue_waiters(){return m_cond_customer.waiters_num();}
		//0 : ok
		int enqueue(T& p, long tm = -1)
		{
			if(m_status == e_shutdown)
				return err_code::e_shut_down;
			
			int ret = 0;
			auto_lock<mutex_type> __mon(m_mutex);
			while(base_type::size() >= m_high_water && 0 == ret && m_status == e_active)
			{
				if( tm == 0 )
				{
					ret = err_code::e_time_out;
				}
				else
				{
					ret = m_cond_producer.wait(tm);
				}
			}
			if(m_status == e_shutdown)
				return err_code::e_shut_down;

			if(0 != ret)
				return ret;

			base_type::push(p);

			//if(base_type::size() == m_low_water + 1)
				m_cond_customer.signal();

			return err_code::e_ok;
		}

		//0 : ok
		int dequeue(T& pout, long tm = -1)
		{
			if(m_status == e_shutdown)
				return err_code::e_shut_down;

			int ret = 0;
			auto_lock<mutex_type> __mon(m_mutex);
			while(base_type::size() <= m_low_water && 0 == ret && m_status == e_active)
			{
				if( tm == 0 )
				{
					ret = err_code::e_time_out;
				}
				else
				{
					ret = m_cond_customer.wait(tm);
				}
			}
			if(m_status == e_shutdown)
				return err_code::e_shut_down;

			if(0 != ret)
				return ret;
			
			pout = base_type::front();
			base_type::pop();

			//if(base_type::size() == m_high_water - 1)
				m_cond_producer.signal();

			return err_code::e_ok;
		}


		//Add by fan05
		template<class _Ty> 
		bool _remove(_Ty _Pred)
		{
			bool bRet = false;

			auto_lock<mutex_type> __mon(m_mutex);
			std::deque<T>::iterator pos = remove_if(c.begin(), c.end(), _Pred);
			if (pos != c.end())
			{
				bRet = true; 
			}
			c.erase(pos, c.end());

			return bRet;
		}
		

		int dequeue_non_check_status(T& pout, long tm = -1)
		{
			int ret = 0;
			auto_lock<mutex_type> __mon(m_mutex);
			while(base_type::size() <= m_low_water && 0 == ret)
			{
				ret = m_cond_customer.wait(tm);
			}

			if(err_code::e_ok != ret)
				return ret;
			
			pout = base_type::front();
			base_type::pop();

			//if(base_type::size() == m_high_water - 1)
				m_cond_producer.signal();

			return err_code::e_ok;
		}

		mutex_type* mutex() {return &m_mutex;}
		size_t size() const {return base_type::size();}
		bool is_empty() {return 0 == base_type::size();}
		bool is_full() {return base_type::size() >= m_high_water;}
	protected:
		this_type& operator = (const this_type&);
		queue(const queue&);

		bool __init()
		{
			m_total_length = 0;

			if(!m_mutex.open())
				return false;
			if(!m_cond_producer.open(&m_mutex))
				return false;
			if(!m_cond_customer.open(&m_mutex))
				return false;

			return true;
		}

		void __uninit()
		{
			m_cond_customer.close();
			m_cond_producer.close();
			m_mutex.close();
		}
	private:
		size_t m_high_water;
		size_t m_low_water;
		size_t m_total_length;
		mutex_type m_mutex;
		int m_status;
		condition_type m_cond_producer;
		condition_type m_cond_customer;
	private:
		
	};

	

	template<class SYN_TRAITS,
			 class T, 
			 class C = std::vector<T>,
			 class P = std::less<C::value_type> >
	class priority_queue
	{
	public:
		enum 
		{
			e_low_water = 0,
			e_high_water = 1024*32
		};
		enum
		{
			e_active = 0x01,
			e_shutdown
		};
		enum{e_default_priority = 10}; //优先级

		typedef typename SYN_TRAITS::THREAD_MUTEX mutex_type;
		typedef typename SYN_TRAITS::THREAD_CONDITION condition_type;
		
		typedef priority_queue<SYN_TRAITS, T, C, P> this_type;
		typedef std::priority_queue< T, C, P > queue_type;
	public:
		priority_queue(size_t high_water = e_high_water, size_t low_water = e_low_water)
			: m_high_water(high_water), m_low_water(low_water), m_status(e_shutdown)
		{
			if(!__init())
				__throw("priority_queue init error");
		}

		~priority_queue()
		{
			clear();
			__uninit();
		}

		//获取当前队列状态
		int status() const {return m_status;}
		//是否已经关闭
		bool is_shut_down(){return m_status == e_shutdown;}
		//激活队列
		void active(){m_status = e_active;}
		//关闭队列
		void shut_down(bool bfree = false)
		{
			if(m_status != e_shutdown)
			{
				m_status = e_shutdown;
				m_cond_producer.broadcast();
				m_cond_customer.broadcast();
				if(bfree)
					clear();
			}
		}

		//获取/设置高低水位
		size_t high_water() const {return m_high_water;}
		size_t low_water() const {return m_low_water;}

		void high_water(size_t n)  { m_high_water = n;}
		void low_water(size_t n)  { m_low_water = n;}

		//获取挂起的入队/出队线程数
		size_t enqueue_waiters(){return m_cond_producer.waiters_num();}
		size_t dequeue_waiters(){return m_cond_customer.waiters_num();}
		//0 : ok, 入队
		int enqueue(T& val, long tm = -1)
		{
			int ret = 0;
			auto_lock<mutex_type> __mon(m_mutex);
			while(m_queue.size() >= m_high_water && 0 == ret && m_status == e_active)
			{
				ret = m_cond_producer.wait(tm);
			}
			if(m_status == e_shutdown)
				return err_code::e_shut_down;

			if(0 != ret)
				return ret;

			m_queue.push(val);

			m_cond_customer.signal();

			return err_code::e_ok;
		}

		//0 : ok 出队
		int dequeue(T& out_val, long tm = -1)
		{
			int ret = 0;
			auto_lock<mutex_type> __mon(m_mutex);
			while(m_queue.size() <= m_low_water && 0 == ret && m_status == e_active)
			{
				ret = m_cond_customer.wait(tm);
			}
			if(m_status == e_shutdown)
				return err_code::e_shut_down;

			if(0 != ret)
				return ret;
			
			out_val = m_queue.top();
			m_queue.pop();

			m_cond_producer.signal();

			return err_code::e_ok;
		}
		int dequeue_non_check_status(T& out_val, long tm = -1)
		{
			int ret = 0;
			auto_lock<mutex_type> __mon(m_mutex);
			while(m_queue.size() <= m_low_water && 0 == ret)
			{
				ret = m_cond_customer.wait(tm);
			}
			if(err_code::e_ok != ret)
				return ret;
			
			out_val = m_queue.top();
			m_queue.pop();

			m_cond_producer.signal();

			return err_code::e_ok;
		}

		

		void clear()
		{
			auto_lock<mutex_type> __mon(m_mutex);
			while(m_queue.size() > 0)
			{
				m_queue.pop();
			}
			
		}
		//获取锁
		mutex_type* mutex() {return &m_mutex;}
		//队列大小
		size_t size() const {return m_queue.size();}
		//队列是否为空
		bool is_empty() {return 0 == m_queue.size();}
		//是否已满
		bool is_full() {return m_queue.size() >= m_high_water;}
	protected:
		this_type& operator = (const this_type&);
		priority_queue(const priority_queue&);

		bool __init()
		{
			m_total_length = 0;

			if(!m_mutex.open())
				return false;
			if(!m_cond_producer.open(&m_mutex))
				return false;
			if(!m_cond_customer.open(&m_mutex))
				return false;

			return true;
		}

		void __uninit()
		{
			m_cond_customer.close();
			m_cond_producer.close();
			m_mutex.close();
		}
	private:
		size_t m_high_water;
		size_t m_low_water;
		size_t m_total_length;
		mutex_type m_mutex;
		int m_status;
		condition_type m_cond_producer;
		condition_type m_cond_customer;
		queue_type m_queue;
	
		
	};

};
#endif //__SYS_QUEUE_H__