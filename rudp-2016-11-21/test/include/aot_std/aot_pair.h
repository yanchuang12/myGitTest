/** Copyright (c) 2015-2016
 * All rights reserved.
 * 
 * 文件名称:	aot_pair.h  
 * 摘	 要:	常用数据结构封装,比标准库简洁.快
 * 
 * 当前版本：	1.0
 * 作	 者:	范涛涛(FTT)
 * 操	 作:	新建
 * 完成日期:	2015年05月11日
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