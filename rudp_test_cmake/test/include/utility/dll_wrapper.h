
/** Copyright (c) 2008-2009
* All rights reserved.
* 
* 文件名称:		dll_manager.h   
* 摘	 要:	dll包装类
* 
* 当前版本：	1.0
* 作	 者:	范涛涛
* 操	 作:	新建
* 完成日期:		2009年7月12日
*/

#ifndef __DLL_WRAPPER_H__
#define __DLL_WRAPPER_H__

#include <string>

namespace aot{ namespace tt{ namespace utility{

/** sys_dll_module
 *	dll wrapper class
 *	@T:	dll main interface
 */
template<class T>
class sys_dll_module
{
	typedef void* (__stdcall *F_create_obj)(int key);
public:
	sys_dll_module()
	{
		__clear();
	}

	virtual ~sys_dll_module()
	{
		if( this->hinst_ ) {	

			if( this->obj_ ) {
				this->obj_->destroy();
			}

			::FreeLibrary(this->hinst_);
		}

		__clear();
	}

public:

	std::string name()
	{
		return this->dll_name_;
	}

	bool init(const char* dll_name, bool create = true, int key = 0)
	{
		this->dll_name_ = dll_name;

		if( !__loading_dll() )
			return false;

		if( create ) {

			this->obj_ = create_obj(key);
			return ( NULL != this->obj_ );
		}

		return true;
	}

	T* create_obj(int key = 0)
	{
		T* obj = NULL;

		if( this->create_obj_ ) {
			obj = (T*)this->create_obj_(key);
		}

		return obj;
	}

	T* obj()
	{
		return obj_;
	}

	void obj(T* p)
	{
		obj_ = p;
	}


private:
	void __clear()
	{
		this->dll_name_		= "";
		this->hinst_		= NULL;
		this->obj_			= NULL;
		this->create_obj_	= NULL;
	}

	bool __loading_dll()
	{
		if ( this->hinst_ )
			return true;

		this->hinst_ = ::LoadLibrary(this->dll_name_.c_str());

		if ( NULL == this->hinst_ )
			return false;

		this->create_obj_ = 
			(F_create_obj)GetProcAddress(this->hinst_, "f_create_obj");

		return true;
	}
protected:
	/// factory
	F_create_obj	create_obj_;
	/// dll handle
	HINSTANCE		hinst_;
	/// dll name (must full path)
	std::string		dll_name_;
	/// default factory object
	T*		obj_;	

};

}}}


#endif /* __DLL_WRAPPER_H__ */