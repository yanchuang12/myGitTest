/** Copyright (c) 2008-2009
 * All rights reserved.
 * 
 * 文件名称:	interface_base.h 
 * 摘	 要:	接口基类
 * 
 * 当前版本:	1.0
 * 作	 者:	范涛涛
 * 操	 作:	新建
 * 完成日期:	2009年7月12日
 */
#ifndef __INTERFACE_BASE_H__
#define __INTERFACE_BASE_H__

namespace aot{ namespace tt{

struct result_t
{
	result_t()
	{
		ret = 0;
		data = NULL;
		msg	= NULL;
	}
	int ret;
	void* data;
	char* msg;
};

template<class interface_type>
class iobj_auto_ptr
{
public:
	typedef iobj_auto_ptr< interface_type > this_type;
	typedef interface_type param_type;
public:
	iobj_auto_ptr() : obj_(NULL) {}
	iobj_auto_ptr(interface_type* p) : obj_(p) {}
	~iobj_auto_ptr() { if( this->obj_ ) this->obj_->destroy();}
	/// 
	operator void**() { return (void**)&this->obj_; }
	operator interface_type**() { return (interface_type**)&this->obj_; }
	interface_type* operator->() { return this->obj_; }
	interface_type* operator*() { return this->obj_; }

	void reset(interface_type* p)
	{
		if( this->obj_ )
			this->obj_->destroy();

		this->obj_ = p;
	}

	interface_type* detach()
	{
		interface_type* p = this->obj_;
		this->obj_ = NULL;
		return p;
	}

	interface_type* get(){ return this->obj_; }
private:
	/// 为使用简单起见， 禁止智能指针之间赋值以及调用拷贝构造函数
	iobj_auto_ptr(const this_type&);
	iobj_auto_ptr& operator = (const this_type&);
private:
	interface_type*	obj_;
};

template<class iobj_type>
class iobj_auto_lock
{
public:
	iobj_auto_lock(iobj_type* t) : obj_(t)
	{
		obj_->lock();
	}
	~iobj_auto_lock()
	{
		obj_->unlock();
	}
private:
	iobj_type* obj_;
};

template<class iobj_type>
class iobj_null_auto_lock
{
public:
	iobj_null_auto_lock(iobj_type* t)
	{
	}
	~iobj_null_auto_lock()
	{
	}
};

class interface_base
{
public:
	interface_base() { this->ref_cnt_ = 1; }
protected:
	virtual ~interface_base(){;}
public:
	virtual void destroy() { if( 0 == dec_ref() ) delete this; }
	virtual bool query_interface(void** out, const char* key) = 0;
	virtual const char* interface_name() = 0;
	virtual bool query_interface_by_iid(void** out, int key){ return false; }
	virtual int interface_iid(){ return 0; }
	virtual long inc_ref(){ return InterlockedIncrement(&this->ref_cnt_); }
	virtual long dec_ref(){ return InterlockedDecrement(&this->ref_cnt_); }
protected:
	long ref_cnt_;
};

}}

#define aot_iobj_safe_destroy(p)	do{ if(p) p->destroy(); }while(0)

#define __AOT_IID_INTERFACE_DECLARE( prefix, class_name )	\
	protected:	\
		virtual ~class_name(){;}\
	public:	\
		enum{ iid = e_aot_iid_##prefix##_##class_name};	\
		virtual  const char* interface_name (){return #class_name;}	\
		virtual  bool query_interface (void** out, const char* key){return false;}\
		virtual  int  interface_iid (){return iid;}\
		virtual  bool clone(class_name** out) {return false;}\
		virtual  bool assign(const class_name* in) {  return false;}\

#define __AOT_IID_DEFINE( prefix, class_name )  e_aot_iid_##prefix##_##class_name

#endif /// __INTERFACE_BASE_H__