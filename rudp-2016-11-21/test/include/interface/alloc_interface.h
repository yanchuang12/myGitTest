/** Copyright (c) 2008-2009
 * All rights reserved.
 * 
 * �ļ�����:		alloc_interface.h 
 * ժ	 Ҫ:	�ӿ���
 * 
 * ��ǰ�汾:	1.0
 * ��	 ��:	������
 * ��	 ��:	�½�
 * �������:		2009��7��12��
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



