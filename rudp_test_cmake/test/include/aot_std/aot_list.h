/** Copyright (c) 2015-2016
 * All rights reserved.
 * 
 * 文件名称:	aot_list.h  
 * 摘	 要:	常用数据结构封装,比标准库简洁.快
 * 
 * 当前版本：	1.0
 * 作	 者:	范涛涛(FTT)
 * 操	 作:	新建
 * 完成日期:	2015年05月11日
*/

#ifndef __AOT_STD_LIST_201505114_H__
#define __AOT_STD_LIST_201505114_H__

#include "aot_std_typedef.h"

namespace aot_std{

template<class T, class ALLOC>
class list
{
	struct node_t
	{
		__AOT_QUEUE_DECALRE__(node_t);
		T val;
		node_t(const T& t) : val(t)
		{
			;
		}
		node_t()
		{
			;
		}
	};
public:
	typedef ALLOC alloc_type;
	typedef list<T, ALLOC> this_type;
	typedef node_t* iterator;
	

	iterator find(const T& val)
	{
		iterator r;
		aot_queue_for_each(r, &this->header_)
		{
			if( r->val == val )
			{
				return r;
			}
		}
		return NULL;
	}
	iterator goto_begin()
	{
		if( aot_queue_is_empty(&this->header_) )
			return NULL;
		return aot_queue_head(&this->header_);
	}
	iterator goto_next(iterator it)
	{
		if( NULL == it || it == &this->header_ )
			return NULL;
		return it->next;
	}
	iterator goto_prev(iterator it)
	{
		if( NULL == it || it == &this->header_ )
			return NULL;
		return it->prev;
	}
	bool is_end(iterator it)
	{
		return (NULL == it) || (it == &this->header_);
	}
	iterator remove_iter(iterator it)
	{	
		if( is_end(it) )
		{
			return NULL;
		}
		iterator next = it->next;
		aot_queue_remove(it);
		--this->size_;
		it->~node_t();
		this->alloc_->dealloc(it);
		return next;
	}
	void move_iter_to_end(iterator it)
	{
		if( is_end(it) )
		{
			return;
		}
		aot_queue_remove(it);
		it->next = NULL;
		aot_queue_insert_tail(&this->header_, it);
	}
	void move_iter_to_header(iterator it)
	{
		if( !aot_queue_is_empty(&this->header_) )
		{
			node_t* t = aot_queue_head(&this->header_);
			if( t == it )
			{
				return;
			}
		}
		else
		{
			assert(0);
		}

		aot_queue_remove(it);
		it->next = NULL;
		it->prev = NULL;

		aot_queue_insert_head(&this->header_, it);
	}
	T& get_val(iterator it)
	{
		return it->val;
	}
public:
	list(ALLOC* alloc)
	{
		alloc_ = alloc;
		assert(NULL != alloc_);
		size_ = 0;
		aot_queue_init(&this->header_);
	}
	~list()
	{
		clear();
	}
public: /// 反向操作
	typedef node_t* r_iterator;
	r_iterator r_begin()
	{
		if( aot_queue_is_empty(&this->header_) )
			return NULL;
		return aot_queue_tail(&this->header_);
	}
	r_iterator r_next(r_iterator it)
	{
		if ( NULL == it || it == &this->header_ )
			return NULL;
		return it->prev;
	}
	bool r_is_end(r_iterator it)
	{
		return (NULL == it) || (it == &this->header_);
	}
	T& r_get_val(iterator it)
	{
		return it->val;
	}
public:
	aot_uint32_t size()
	{
		return this->size_;
	}
	bool push_back(const T& val)
	{
		node_t* p = (node_t*)this->alloc_->alloc(sizeof(node_t));
		if( NULL == p )
		{
			return false;
		}
		new (p) node_t(val);
		aot_queue_insert_tail(&this->header_, p);
		++this->size_;
		return true;
	}
	bool push_front(const T& val)
	{
		node_t* p = (node_t*)this->alloc_->alloc(sizeof(node_t));
		if( NULL == p )
		{
			return false;
		}
		new (p) node_t(val);
		aot_queue_insert_head(&this->header_, p);
		++this->size_;
		return true;
	}
	T& front()
	{
		assert( false == aot_queue_is_empty(&this->header_) );
		return (aot_queue_head(&this->header_))->val;
	}
	void pop_front()
	{
		if( aot_queue_is_empty(&this->header_) )
		{
			assert(0);
			return;
		}
		node_t* t = aot_queue_head(&this->header_);
		remove_iter(t);
	}
	T& back()
	{
		assert( false == aot_queue_is_empty(&this->header_) );
		return (aot_queue_tail(&this->header_))->val;
	}
	void pop_back()
	{
		if( aot_queue_is_empty(&this->header_) )
		{
			assert(0);
			return;
		}
		node_t* t = aot_queue_tail(&this->header_);
		remove_iter(t);
	}
	void remove(const T& val)
	{
		iterator next;
		iterator it = goto_begin();
		for( ; !is_end(it); )
		{
			next = goto_next(it);
			if( it->val == val )
			{
				remove_iter(it);
			}
			it = next;
		}
	}
	void remove_tail(T* out)
	{
		if( aot_queue_is_empty(&this->header_) )
		{
			*out = NULL;
			return;
		}
		node_t* t = aot_queue_tail(&this->header_);
		*out = t->val;
		remove_iter(t);
	}
	void remove_head(T* out)
	{
		if( aot_queue_is_empty(&this->header_) )
		{
			*out = NULL;
			return;
		}
		node_t* t = aot_queue_head(&this->header_);
		*out = t->val;
		remove_iter(t);
	}
	void clear()
	{
		node_t* t = NULL;
		while( !aot_queue_is_empty(&this->header_) )
		{
			t = aot_queue_tail(&this->header_);
			aot_queue_remove(t);
			t->~node_t();
			this->alloc_->dealloc(t);
		}
		this->size_ = 0;
	}
private:
	node_t header_;
	ALLOC* alloc_;
	aot_uint32_t size_;
};




} /// end namespace aot_std

#endif /// __AOT_STD_LIST_201505114_H__