/** Copyright (c) 2008-2009
 * All rights reserved.
 * 
 * 文件名称:		alloc_interface.h 
 * 摘	 要:	接口类
 * 
 * 当前版本:	1.0
 * 作	 者:	范涛涛
 * 操	 作:	新建
 * 完成日期:		2009年7月12日
 */
#ifndef __ALLOC_INTERFACE_H__
#define __ALLOC_INTERFACE_H__

#include "interface_base.h"
#include "../sys/sys_singleton.h"
#include "../sys/sys_asy.h"

#pragma warning(disable:4996)

namespace aot{ namespace tt{

class ialloc : public interface_base
{
protected:
	virtual ~ialloc (){;}
public:
	virtual bool query_interface (void** out, const char* key) {return false;}
	virtual const char* interface_name () {return "ialloc";}
public:
	virtual void* alloc (size_t size) = 0;
	virtual void dealloc (void* p) = 0;
};

}}

#pragma warning(default:4996)

#endif /// __ALLOC_INTERFACE_H__



