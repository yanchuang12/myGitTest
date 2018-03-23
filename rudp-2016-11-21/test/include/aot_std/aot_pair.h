/** Copyright (c) 2015-2016
 * All rights reserved.
 * 
 * �ļ�����:	aot_pair.h  
 * ժ	 Ҫ:	�������ݽṹ��װ,�ȱ�׼����.��
 * 
 * ��ǰ�汾��	1.0
 * ��	 ��:	������(FTT)
 * ��	 ��:	�½�
 * �������:	2015��05��11��
*/

#ifndef __AOT_STD_PAIR_201505114_H__
#define __AOT_STD_PAIR_201505114_H__

#include "aot_std_typedef.h"
#include <new>

namespace aot_std{

template<class T1, class T2>
struct pair_t
{
	pair_t(){}
	pair_t(const T1& t1, const T2& t2) : first(t1), second(t2)
	{
		;
	}
	T1 first;
	T2 second;
};

} /// end namespace aot_std

#endif /// __AOT_STD_PAIR_201505114_H__