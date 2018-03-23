
/** Copyright (c) 2008 
* All rights reserved.
* 
* 文件名称:		Thread_mutex.h
* 摘	 要:	封装了一些线(进)程同步原语
* 
* 当前版本：	1.0
* 作	 者:	范涛涛
* 操	 作:	新建
* 完成日期:		2008年4月24日
*/
#ifndef __SYS_ASY__
#define __SYS_ASY__

#include "sys_globa_def.h"
#include "sys_singleton.h"

namespace xy
{

namespace mutex_flag
{
	enum
	{
		e_null = 0,
		e_thread ,
		e_process
	};
};

class mutex_base
{
public:
	mutex_base(){;}
	virtual ~mutex_base() {;}
public:
	virtual int type() const = 0;
	virtual bool open() = 0;
	virtual bool close() = 0;
	virtual bool acquire() = 0;
	virtual int acquire(long tm) = 0;
	virtual bool try_acquire() = 0;
	virtual bool release() = 0;
private:
	mutex_base(const mutex_base&);
    const mutex_base& operator=(const mutex_base&);
};

class null_mutex : private copy_disable
{
public:
	inline int type() const { return mutex_flag::e_null; }
	inline bool open() { return true; }
	inline bool close() { return true; }
	inline bool acquire() { return true; }
	inline int acquire(long tm) { return 0; }
	inline bool try_acquire() { return true; }
	inline bool release() { return true; }
};

class thread_mutex : private copy_disable  
{
public:
	typedef CRITICAL_SECTION mutex_type;
	thread_mutex() { ::InitializeCriticalSection(&this->mutex_); }
	~thread_mutex() { ::DeleteCriticalSection(&this->mutex_); }
public:
	inline int type() const { return mutex_flag::e_thread; }
	inline bool open() { return true; }
	inline bool close() { return true; }
	inline bool acquire() { ::EnterCriticalSection(&this->mutex_); return true;}
	inline int acquire(long tm) { return err_code::e_unknow; }
	inline bool try_acquire()
	{
# ifdef _WIN32_WINNT
		return false;//TRUE == ::TryEnterCriticalSection (&m_CS);
# else
		return false;
# endif
	}

	inline bool release()//释放锁
	{
		::LeaveCriticalSection(&this->mutex_);
		return true;
	}

	inline mutex_type* mutex()//取内部锁对象
	{
		return &this->mutex_;
	}
private:
	mutex_type mutex_;
private:
	thread_mutex& operator = (const thread_mutex&);
	thread_mutex (const thread_mutex&);
};

//进程锁
class process_mutex : private copy_disable
{
public:
	typedef HANDLE mutex_type;

	process_mutex() : mutex_(0)
	{;}
	~process_mutex() { }

	inline int type() const { return mutex_flag::e_process; }

	inline bool open(const char* name = NULL)
	{
		mutex_ = ::CreateMutex(NULL, FALSE, name);
		return (NULL != mutex_);
	}

	inline bool close()
	{
		if( mutex_ )
		{
			::CloseHandle(mutex_);
			mutex_ = NULL;
		}
		return true;
	}

	inline bool acquire()/// 捕获锁
	{
		switch (::WaitForSingleObject (mutex_, INFINITE))
		{
		case WAIT_OBJECT_0:
		case WAIT_ABANDONED:
			return true;
		default:
			return false;
		}
	}

	/// 捕获锁, tm: 超时时间单位:ms
	inline int acquire(long tm)
	{
		switch (::WaitForSingleObject (mutex_, tm))
		{
		case WAIT_OBJECT_0:
		case WAIT_ABANDONED:
			return 0;
		case WAIT_TIMEOUT:
			return err_code::e_time_out;
		default:
			return err_code::e_unknow;
		}
	}

	inline bool try_acquire()//非阻塞的捕获锁
	{
		return 0 == acquire(0);
	}

	inline bool release()//释放锁
	{
		if( mutex_ )
		{
			::ReleaseMutex(mutex_);
		}
		return true;
	}

	inline mutex_type* mutex()
	{
		return &mutex_;
	}
private:
	mutex_type mutex_;
private:
	process_mutex& operator = (const process_mutex&);
	process_mutex (const process_mutex&);
};

//自动锁
//MUTEX 为: null_mutex, thread_mutex, process_mutex
template<class MUTEX>
class auto_lock
{
public:
	auto_lock(MUTEX& t) : lock_(t)
	{
		lock_.acquire();
	}
	~auto_lock()
	{
		lock_.release();
	}
private:
	MUTEX& lock_;
};

//适配器, 将模板族mutex适配成mutex_base派生类
//MUTEX 为: null_mutex, thread_mutex, process_mutex
template<class MUTEX>
class mutex_adapter : public mutex_base, private MUTEX
{
public:
	mutex_adapter()
	{
		if(!MUTEX::open())
		{
			__throw("mutex open error");
		}
	}

	virtual ~mutex_adapter()
	{
		MUTEX::close();
	}

	virtual int type() const { return MUTEX::type(); }
	virtual bool open() { return true; }
	virtual bool close() { return true; }
	virtual bool acquire() { return MUTEX::acquire(); }
	virtual int acquire(long tm) { return MUTEX::acquire(tm); }
	virtual bool try_acquire() { return MUTEX::try_acquire(); }
	virtual bool release() { return MUTEX::release(); }
};


//信号灯
class semaphore : private copy_disable
{
public:
	semaphore(long nInitCnt = 0, long nMaxWaiters = 0x7fffffff) 
	{
		m_hSema = NULL;
		m_nMaxWaiters = 0;
		open();
	};

	~semaphore() { close();}

	//初始化内核对象
	//nInitCnt: 初始化可用资源数
	//nMaxWaiters: 最大可用资源数
	bool open(long nInitCnt = 0, long nMaxWaiters = 0x7fffffff)
	{
		if( m_hSema )
		{
			return true;
		}

		m_hSema = ::CreateSemaphore(NULL, nInitCnt, nMaxWaiters, NULL);

		if (m_hSema == NULL) 
		{
			return false;
		}
		m_nMaxWaiters = nMaxWaiters;
		return true;
	}

	bool close()//释放内核对象
	{
		if(m_hSema)
		{
			::CloseHandle(m_hSema); 
			m_hSema = NULL;
		}
		return true;
	}

	//阻塞等待.线程将被挂起,直到可用资源数(即信号量)>0, 
	//注意: 如果有多个线程同时挂起(即同时等待在一个信号灯上),
	//当一个post到达时, 同一时刻,只有一个线程被唤醒
	bool wait() 
	{ 
		switch (::WaitForSingleObject (m_hSema, INFINITE))
		{
		case WAIT_OBJECT_0:
		case WAIT_ABANDONED:
			return true;
		default:
			return false;
		}
	}

	//等待, tm:超时值
	int wait(long tm)
	{
		if(tm < 0)
		{
			tm = INFINITE;
		}
		switch (::WaitForSingleObject (m_hSema, tm))
		{
		case WAIT_OBJECT_0:
		case WAIT_ABANDONED:
			return 0;
		case WAIT_TIMEOUT:
			return err_code::e_time_out;
		default:
			return err_code::e_unknow;
		}
	}

	//增加一个信号量
	bool post()
	{
		if(m_hSema)
		{
			return TRUE == ::ReleaseSemaphore(m_hSema, 1, NULL);
		}
		return false;
	}

	//增加n个信号量
	bool post(long n)
	{
		if(m_hSema)
		{
			return TRUE == ::ReleaseSemaphore(m_hSema, n, NULL);
		}
		return false;
	}
	
private:
	HANDLE	m_hSema;
	long  m_nMaxWaiters;
private:
	semaphore& operator = (const semaphore&);
	semaphore (const semaphore&);
};

class null_semaphore : private copy_disable
{
public:
	null_semaphore() { ;}
	~null_semaphore() { ;}

	bool open(long nInitCnt = 0, long nMaxWaiters = 0x7fffffff) { return true; }
	bool close() { return true; }
	bool wait() { return true; }
	int wait(long tm) { return 0; }
	bool post() { return true; }
	bool post(long n) { return true; }
	
private:
	null_semaphore& operator = (const null_semaphore&);
	null_semaphore (const null_semaphore&);
};

//条件变量, 必须和某种类型的锁一起使用, 以等待某个条件表达式
//MUTEX 为: null_mutex, thread_mutex, process_mutex
template<class MUTEX>
class condition : private copy_disable
{
public:
	class  cond_t
	{
	public:
	  bool open()
	  {
		  m_waiters = 0;
		  was_broadcast = 0;
		  if(!m_sema.open())
			  return false;
		  if(!m_waiters_lock.open())
			  return false;

		  m_waiters_done = ::CreateEvent(NULL,
							   FALSE,
							   FALSE,
							   NULL);

		  if(NULL == m_waiters_done)
			  return false;
		  
		  return true;
	  }

	  bool close()
	  {
		  if(m_waiters_done)
		  {
			::CloseHandle(m_waiters_done);
		  }

		  m_waiters = 0;
		  was_broadcast = 0;

		  m_waiters_done = NULL;
		  
		  m_waiters_lock.close();
		  return m_sema.close();
	  }

	public:
	  long m_waiters;
	  MUTEX m_waiters_lock;
	  semaphore m_sema;
	  HANDLE m_waiters_done;
	  size_t was_broadcast;
	};
public:
	condition() { }
	~condition() { }
public:
	bool open(MUTEX* mt)
	{
		m_mutex = mt;
		return m_cond.open();
	}

	bool close()
	{
		bool b = true;
		//释放内核对象时, 要唤醒所有挂起的线程
		while(!m_cond.close() && b)
		{
			b = broadcast();
		}
		return b;
	}

	MUTEX* mutex()
	{
		return m_mutex;
	}

	size_t waiters_num(){ return m_cond.m_waiters;}

	//阻塞等待.线程将被挂起,直到可用资源数(即信号量)>0, 
	//注意: 如果有多个线程同时挂起(即同时等待在一个内核对象上),
	//当一个signal到达时, 同一时刻,只有一个线程被唤醒
	//另: 外部调用wait前,必须先加锁, wait返回时,无论结果如何, 仍然持有
	//外部锁
	bool wait()
	{
		m_cond.m_waiters_lock.acquire();
		m_cond.m_waiters++;
		m_cond.m_waiters_lock.release();
		//先释放锁, 因此外部调用wait前,必须先加锁
		if(!m_mutex->release())
			return false;
		//挂起,等待
		bool b = m_cond.m_sema.wait();

		m_cond.m_waiters_lock.acquire();
		m_cond.m_waiters--;
		bool last_waiter = m_cond.was_broadcast && m_cond.m_waiters == 0;
		m_cond.m_waiters_lock.release();

		if(/*b && */last_waiter) /// by FTT 2011-11-15这里不判断b
		{	
			::SetEvent(m_cond.m_waiters_done);	
		}
		//重新获取外部锁
		m_mutex->acquire();

		return b;
	}

	//阻塞等待.线程将被挂起,直到可用资源数(即信号量)>0, 
	//注意: 如果有多个线程同时挂起(即同时等待在一个内核对象上),
	//当一个signal到达时, 同一时刻,只有一个线程被唤醒
	//另: 外部调用wait前,必须先加锁, wait返回时,无论结果如何, 仍然持有
	//外部锁
	int wait(long tm)
	{
		m_cond.m_waiters_lock.acquire();
		m_cond.m_waiters++;
		m_cond.m_waiters_lock.release();

		if(!m_mutex->release())
			return err_code::e_unknow;

		int ret = m_cond.m_sema.wait(tm);

		m_cond.m_waiters_lock.acquire();
		m_cond.m_waiters--;
		bool last_waiter = m_cond.was_broadcast && m_cond.m_waiters == 0;
		m_cond.m_waiters_lock.release();

		if(/*ret == err_code::e_ok && */last_waiter) /// by FTT 2011-11-15这里不判断ret
		{	
			//判断是否是最后一个等待者, 如果是,则设置事件,
			//这样调用broadcast的线程将被唤醒
			::SetEvent(m_cond.m_waiters_done);	
		}

		m_mutex->acquire();

		return ret;
	}
	//激发一个信号,唤醒一个被wait挂起的线程
	bool signal ()
	{	
		m_cond.m_waiters_lock.acquire();
		int waiters = m_cond.m_waiters;
		m_cond.m_waiters_lock.release();
		
		if (waiters > 0)
			return m_cond.m_sema.post();
		else
			return true; 
	}
	//广播,唤醒所有被wait挂起的线程
	bool broadcast ()
	{
		m_cond.m_waiters_lock.acquire();
		
		int have_waiters = 0;
		
		if (m_cond.m_waiters > 0)
		{
			m_cond.was_broadcast = 1;
			have_waiters = 1;
		}
		m_cond.m_waiters_lock.release();
		
		bool bRet = true;
		if (have_waiters)
		{
			if (!m_cond.m_sema.post(m_cond.m_waiters))
			{
				return false;
			}

			switch (::WaitForSingleObject (m_cond.m_waiters_done, INFINITE))
			{
			case WAIT_OBJECT_0:
				bRet = true;
				break;

			default:
				bRet = false;
				break;
			}

			m_cond.was_broadcast = 0;
		}
		return bRet;
		
	}
	
private:
	MUTEX* m_mutex;
	cond_t m_cond;
private:
	condition& operator = (const condition&);
	condition (const condition&);
};

typedef condition<thread_mutex> thread_condition;//线程条件变量

template<class MUTEX>
class null_condition
{
public:
	size_t waiters_num(){ return 0;}
	bool broadcast(){return true;}
	bool open(MUTEX* mt = 0) { return true; }
	bool close() { return true; }
	bool wait(){return true;}
	bool wait(long tm){return 0;}
	bool signal () {return true;}
	MUTEX* mutex()
	{
		static null_mutex s;
		return (MUTEX*)&s;
	}
};

///栅栏
///MUTEX 为: null_mutex, thread_mutex, process_mutex
template<class MUTEX>
class barrier : private copy_disable
{
	typedef auto_lock<MUTEX> auto_lock_type;
public:
	//nWaitersNum: 需要同步的线程数
	barrier(int nWaitersNum)
	{
		m_init_num = nWaitersNum;
		m_run_num = nWaitersNum;
		m_mutex.open();
		m_cond.open(&m_mutex);
	}

	~barrier()
	{
		m_cond.close();
		m_mutex.close();
	}

	bool wait()
	{
		auto_lock_type __tmp_lock(m_mutex);

		if(m_run_num == 1)
		{//最后一个到达的线程: 广播,唤醒所有挂起在栅栏上的线程
			m_run_num = m_init_num;
			return m_cond.broadcast();
		}
		else
		{
			--m_run_num;
			while(m_run_num != m_init_num)
			{
				if(!m_cond.wait())
					return false;
			}

			return true;
		}

		return true;
	}
private:
	condition<MUTEX> m_cond;
	MUTEX m_mutex;
	int m_init_num;
	int m_run_num;
	
};

//线程栅栏
typedef barrier<thread_mutex> thread_barrier;

//多线程 traits类, 
struct MT_SYN_TRAITS
{
	typedef thread_mutex		THREAD_MUTEX;
	typedef process_mutex		PROCESS_MUTEX;
	typedef null_mutex			NULL_MUTEX;
	typedef thread_condition	THREAD_CONDITION;
	typedef semaphore			SEMAPHORE;
	typedef thread_barrier		THREAD_BARRIER;
};

//单线程 traits类
struct NULL_SYN_TRAITS
{
	typedef null_mutex			THREAD_MUTEX;
	typedef null_mutex			PROCESS_MUTEX;
	typedef null_mutex			NULL_MUTEX;
	typedef condition<null_mutex>	THREAD_CONDITION;
	typedef semaphore			SEMAPHORE;
	typedef barrier<null_mutex>		THREAD_BARRIER;
};

};
#endif // !defined(__SYS_ASY__)
