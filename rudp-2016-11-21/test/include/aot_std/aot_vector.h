/** Copyright (c) 2015-2016
 * All rights reserved.
 * 
 * 文件名称:	aot_vector.h  
 * 摘	 要:	常用数据结构封装,比标准库简洁.快
 * 
 * 当前版本：	1.0
 * 作	 者:	范涛涛(FTT)
 * 操	 作:	新建
 * 完成日期:	2015年05月11日
*/

#ifndef __AOT_STD_VECTOR_201505114_H__
#define __AOT_STD_VECTOR_201505114_H__

#include "aot_std_typedef.h"

namespace aot_std{

/// @t_has_construct: true: T需要调用构造和析构函数;   false: 不需要(这可以提升性能)
template<class T, class ALLOC, bool t_has_construct>
class vector
{
public:
	typedef ALLOC alloc_type;
	typedef vector<T, ALLOC, t_has_construct> this_type;
public:
	struct iterator
	{
		iterator(T* e, this_type* v) : val(e), vec(v)
		{;}
		T*			val;
		this_type*	vec;
	};

	iterator find(const T& val)
	{
		for( aot_uint32_t i = 0; i < this->elem_size_; ++i )
		{
			if( this->data_[i] == val )
			{
				return iterator(&this->data_[i], this);
			}
		}
		return iterator(NULL, this);
	}
	bool find2(const T& val)
	{
		for( aot_uint32_t i = 0; i < this->elem_size_; ++i )
		{
			if( this->data_[i] == val )
			{
				return true;
			}
		}
		return false;
	}
	aot_int32_t get_index(const T& val)
	{
		for( aot_uint32_t i = 0; i < this->elem_size_; ++i )
		{
			if( this->data_[i] == val )
			{
				return i;
			}
		}
		return -1;
	}
	iterator goto_begin()
	{
		if( this->elem_size_ > 0 )
		{
			return iterator(&this->data_[0], this);
		}
		return iterator(NULL, this);
	}
	iterator goto_next(iterator& it)
	{
		if( is_end(it) )
		{
			return iterator(NULL, this);
		}
		return iterator(it.val + 1, this);
	}
	bool is_end(iterator& it)
	{
		return ( NULL == this->data_ || it.val < this->data_ || this->data_ + this->elem_size_ <= it.val );
	}
	/// 返回下一位置的迭代器
	iterator remove_iter(iterator it)
	{
		if( is_end(it) || this->elem_size_ == 0 )
		{
			assert(0);
			return iterator(NULL, this);
		}

		T* last = this->data_ + this->elem_size_;

		if( t_has_construct )
		{
			T* n = it.val + 1;
			T* p = it.val;

			for( ; n < last; )
			{
				*(p++) = *(n++);
			}

			--this->elem_size_;
			(this->data_ + this->elem_size_)->~T();
		}
		else
		{
			size_t n = (size_t)((char*)last - (char*)(it.val + 1));
			if( (int)n > 0 )
			{
				memmove(it.val, it.val + 1, n);
			}
			--this->elem_size_;	
		}
		/// 删除后,下一个it仍然是自己
		return it;
	}

	T& get_val(iterator& it)
	{
		return *(it.val);
	}
public:
	vector(ALLOC* alloc)
	{
		this->alloc_ = alloc;
		assert(NULL != this->alloc_);

		this->data_ = NULL;
		this->elem_size_ = 0;
		this->total_size_ = 0;
	}
	vector()
	{
		this->alloc_ = NULL;
		this->data_ = NULL;
		this->elem_size_ = 0;
		this->total_size_ = 0;
	}
	~vector()
	{
		if( this->data_ )
		{
			clear();
			this->alloc_->dealloc(this->data_);
		}
	}
	T& operator[](aot_uint32_t idx)
	{
		assert(idx < this->elem_size_);
		return this->data_[idx];
	}
	T* get_at(aot_uint32_t idx)
	{
		assert(idx < this->elem_size_);
		return &(this->data_[idx]);
	}
public:
	void set_alloc(ALLOC* alloc)
	{
		this->alloc_ = alloc;
		assert(NULL != this->alloc_);
	}
	bool init(aot_uint32_t init_size)
	{
		assert(NULL != this->alloc_);
		/// 允许使用过程中重复调用该函数
		if( this->elem_size_ != 0 )
		{
			assert(0);
			this->elem_size_ = 0;
		}
		if( this->data_ )
		{
			this->alloc_->dealloc(this->data_);
			this->data_ = NULL;
		}
		if( init_size > 0 )
		{
			this->data_ = (T*)this->alloc_->alloc(init_size * sizeof(T));
		}
		this->total_size_ = init_size;
		/// 这里不需要进行构造
		return true;
	}
	aot_uint32_t size()
	{
		return this->elem_size_;
	}
	bool push_back(const T& val)
	{
		assert(NULL != this->alloc_);

		if( this->elem_size_ == this->total_size_ )
		{
			aot_uint32_t new_total_size = this->total_size_ + this->total_size_ ;
			if( new_total_size == 0 )
			{
				/// 首次插入时,如果没有预先指定初始元素个数,默认预分配64个元素的内存空间
				new_total_size = 64;
			}

			T* new_data = (T*)this->alloc_->alloc(new_total_size * sizeof(T));

			if( this->data_ )
			{
				aot_uint32_t old_elem_size = this->elem_size_;
				if( t_has_construct )
				{
					for( aot_uint32_t i = 0; i < this->elem_size_; ++i )
					{
						new (&new_data[i]) T(this->data_[i]);
					}
				}
				else
				{
					memcpy(new_data, this->data_, old_elem_size * sizeof(T));
				}
				clear();
				this->alloc_->dealloc(this->data_);
				this->elem_size_ = old_elem_size;
			}
			this->data_ = new_data;
			this->total_size_ = new_total_size;
		}

		if( t_has_construct )
		{
			new (&this->data_[this->elem_size_++]) T(val);
		}
		else
		{
			this->data_[this->elem_size_++] = val;
		}

		return true;
	}
	T& back()
	{
		assert(this->elem_size_ > 0);
		return this->data_[this->elem_size_ - 1];
	}
	void pop_back()
	{
		assert(this->elem_size_ > 0);
		if( this->elem_size_ == 0 )
		{
			return;
		}
		if( t_has_construct )
		{
			this->data_[this->elem_size_ -1].~T();
		}
		--this->elem_size_;
	}
	void remove(const T& val)
	{
		iterator it = goto_begin();
		for( ; !is_end(it); )
		{
			if( *(it.val) == val )
			{
				it = remove_iter(it);
			}
			else
			{
				it = goto_next(it);
			}
		}
	}
	void clear()
	{
		if( t_has_construct )
		{
			for( aot_uint32_t i = 0; i < this->elem_size_; ++i )
			{
				this->data_[i].~T();
			}
		}
		this->elem_size_ = 0;
	}
private:
	ALLOC* alloc_;
	aot_uint32_t elem_size_;	/// 实际元素个数
	aot_uint32_t total_size_;	/// 总容量
	T* data_;
};



} /// end namespace aot_std

#endif /// __AOT_STD_VECTOR_201505114_H__