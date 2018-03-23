/** Copyright (c) 2008-2009
 * All rights reserved.
 * 
 * 文件名称:	out_param_interface.h 
 * 摘	 要:	跨dll接口参数相关类
 * 
 * 当前版本:	1.0
 * 作	 者:	范涛涛
 * 操	 作:	新建
 * 完成日期:	2009年8月12日
 */

/** 
 * 安全起见,我们不允许跨dll传递普通C++对象, 除非该C++对象的相关方法都是virtual虚函数.
 * 因此提供以下这些接口类,用于跨dll传递C++对象.
 */
#ifndef __OUT_PARAM_INTERFACE_H__
#define __OUT_PARAM_INTERFACE_H__

#include "interface_base.h"
#include <string>
#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include "../sys/sys_asy.h"
#include "iid_define.h"

enum eaot_iid_param
{
	e_aot_iid_param_begin = e_aot_iid_out_param_range_begin,
	e_aot_iid_istr,
	e_aot_iid_ivec_str,
	e_aot_iid_ivec_long,
	e_aot_iid_ivec_ulong,
	e_aot_iid_ilist_str,
	e_aot_iid_ilist_long,
	e_aot_iid_ilist_ulong,
	e_aot_iid_istr2int_map,
	e_aot_iid_istr2str_map,
	e_aot_iid_ibuffer,
	e_aot_iid_param_end = e_aot_iid_out_param_range_end,
};

namespace aot{ namespace tt{
class istr : public interface_base
{
public:
	enum{iid = e_aot_iid_istr};
protected:
	virtual ~istr(){;}
public:
	virtual bool query_interface (void** out, const char* key){return false;}
	virtual const char* interface_name (){return "istr";}
public:
	virtual const char* c_str() const = 0;
	virtual size_t length() const = 0;
	virtual void assign(const char* s) = 0;
};

class str_impl : public istr
{
public:
	str_impl(){;}
	str_impl(const str_impl& p)
	{
		this->str_ = p.c_str();
	}
	virtual ~str_impl(){;}
public:
	virtual const char* c_str() const { return this->str_.c_str(); }
	virtual size_t length() const { return this->str_.length();}
	virtual void assign(const char* s){ if( s ) this->str_ = s;}
public:
	str_impl& operator= (const istr* p)
	{
		this->str_ = const_cast<istr*>(p)->c_str();
		return *this;
	}
	str_impl& operator= (const char* p)
	{
		this->str_ = p;
		return *this;
	}
	str_impl& operator= (const str_impl& p)
	{
		this->str_ = p.c_str();
		return *this;
	}
public:
	std::string str_;
};

template<typename T1, typename T2>
struct aot_type_to_type
{
	T2 operator()( T1 s)
	{
		return s;
	}
};

template<>
struct aot_type_to_type<std::string, const char*>
{
	const char* operator()(const std::string& s)
	{
		return s.c_str();
	}
};

#if (0)
template<class T1, class T2>
inline T2 aot_type_to_type( T1 v)
{
	return v;
}

inline
const char* aot_type_to_type(const std::string& v)
{
	return v.c_str();
}
#endif

template <class param_name_t, 
		  class key_type, 
		  class val_type, 
		  class safe_key_type = key_type, 
		  class safe_val_type = val_type,
		  class SYN_TRAITS = xy::MT_SYN_TRAITS>
class istd_map : public interface_base
{
public:
	enum{iid = param_name_t::iid};
public:
	typedef istd_map<param_name_t, key_type, val_type, safe_key_type, safe_val_type, SYN_TRAITS> this_type;
	typedef safe_val_type safe_value_type;
	typedef iobj_auto_lock<this_type> auto_lock_type;

	struct aot_func_t
	{
		virtual bool fun(safe_key_type k, safe_value_type v){return false;}
	};
protected:
	virtual ~istd_map(){;}
public:
	virtual bool query_interface (void** out, const char* key){return false;}
	virtual const char* interface_name () {param_name_t t; return t.name();}
public:
	virtual size_t size() const = 0;
	virtual void goto_begin() = 0;
	virtual void goto_next() = 0;
	virtual bool is_end() const = 0;
	virtual safe_value_type get_value()  = 0;
	virtual safe_key_type get_key() = 0;
	virtual void set_value( safe_value_type v) = 0;
	virtual void clear() = 0;
	virtual bool empty() = 0;
	virtual void erase(safe_key_type k) = 0;
	/// erase_curr_elem 后, 当前迭代器处于什么位置, 由实际的stl容器的实现确定
	virtual void erase_curr_elem() = 0;
	virtual void lock() = 0;
	virtual void unlock() = 0;
	virtual bool find(safe_key_type key,  bool set_curr_pos = true) = 0;
	virtual bool insert(safe_key_type key,  safe_value_type v, bool set_curr_pos = true) = 0;
	virtual void remove_all(aot_func_t* f) = 0;
	virtual void for_each(aot_func_t* f) = 0;
};

template <class param_name_t, 
			class key_type, 
			class val_type, 
			class safe_key_type = key_type, 
			class safe_val_type = val_type,
			class SYN_TRAITS = xy::MT_SYN_TRAITS>
class std_map_impl : public istd_map<param_name_t, key_type, val_type, safe_key_type, safe_val_type, SYN_TRAITS>
{
public:
	typedef std::map<key_type, val_type> map_type;
	typedef typename SYN_TRAITS::THREAD_MUTEX mutex_type;
	typedef xy::auto_lock< mutex_type > auto_lock_type;
	typedef istd_map<param_name_t, key_type, val_type, safe_key_type, safe_val_type, SYN_TRAITS> base_type;
	typedef typename map_type::iterator iterator_type;
	typedef typename val_type value_type;
	typedef safe_val_type safe_value_type;
	typedef typename istd_map::aot_func_t aot_func_t;
public:
	std_map_impl() { this->lock_.open(); }
	virtual ~std_map_impl() { this->lock_.close(); }
public:
	virtual size_t size() const { return this->cont_.size(); }
	virtual void goto_begin() { this->iter_ = this->cont_.begin(); }
	virtual void goto_next() { ++this->iter_; }
	virtual bool is_end() const { return this->iter_ == this->cont_.end(); }
	virtual safe_value_type get_value()
	{
		return aot_type_to_type<value_type, safe_value_type>()(this->iter_->second);
	}
	virtual safe_key_type get_key()
	{
		return aot_type_to_type<key_type, safe_key_type>()(this->iter_->first);
	}

	virtual void set_value(safe_value_type v) { this->iter_->second = v; }
	virtual void clear() { this->cont_.clear(); }
	virtual bool empty() { return this->cont_.empty(); }
	virtual void erase(safe_key_type k) { this->cont_.erase(k); }
	virtual void erase_curr_elem() { this->iter_ = this->cont_.erase(this->iter_); }
	virtual void lock() { this->lock_.acquire();}
	virtual void unlock() { this->lock_.release();}
	virtual bool find(safe_key_type key,  bool set_curr_pos = true )
	{
		if( set_curr_pos )
		{
			this->iter_ = this->cont_.find(key);
			return (this->iter_ != this->cont_.end());
		}
		else
		{
			iterator_type it = this->cont_.find(key);
			return (it != this->cont_.end());
		}
		return false;
	}

	virtual bool insert(safe_key_type key,  safe_value_type v, bool set_curr_pos = true)
	{
		std::pair<iterator_type, bool> ret = this->cont_.insert(std::make_pair(key, v));
		if( set_curr_pos )
		{
			this->iter_ = ret.first;		
		}
		return ret.second;
	}

	virtual void remove_all(aot_func_t* f)
	{
		size_t ret = 0;
		iterator_type it = this->cont_.begin();
		iterator_type tmp;
		for( ; it != this->cont_.end(); )
		{
			tmp = it;
			++it;
			f->fun( aot_type_to_type<key_type, safe_key_type>()(tmp->first),
				aot_type_to_type<value_type, safe_value_type>()(tmp->second) );
			this->cont_.erase(tmp);
		}
		this->cont_.clear();
	}

	virtual void for_each(aot_func_t* f)
	{
		iterator_type it = this->cont_.begin();
		for( ; it != this->cont_.end(); ++it )
		{
			f->fun( aot_type_to_type<key_type, safe_key_type>()(it->first),
				aot_type_to_type<value_type, safe_value_type>()(it->second) );
		}
	}
public:
	map_type& impl() { return this->cont_; }
public:
	map_type		cont_;
	iterator_type	iter_;
	mutex_type		lock_;
};

template <class param_name_t, class T, class safe_val_type = T::value_type, class SYN_TRAITS = xy::MT_SYN_TRAITS>
class istd_container : public interface_base
{
public:
	enum{iid = param_name_t::iid};
public:
	typedef istd_container<param_name_t, T, safe_val_type, SYN_TRAITS> this_type;
	typedef safe_val_type safe_value_type;
	typedef iobj_auto_lock<this_type> auto_lock_type;

	struct aot_func_t
	{
		virtual bool fun(safe_value_type v){return false;}
		virtual bool sort(safe_value_type v1, safe_value_type v2){return false;}
	};
protected:
	virtual ~istd_container(){;}
public:
	virtual bool query_interface (void** out, const char* key){return false;}
	virtual const char* interface_name () {param_name_t t; return t.name();}
public:
	virtual size_t size() const = 0;
	virtual void goto_begin() = 0;
	virtual void goto_next() = 0;
	virtual bool is_end() const = 0;
	virtual safe_value_type get_value()  = 0;
	virtual void set_value( safe_value_type v) = 0;
	virtual void push_back( safe_value_type v) = 0;
	virtual void clear() = 0;
	virtual bool empty() = 0;
	virtual void remove(safe_value_type v) = 0;
	virtual void erase(safe_value_type v) = 0;
	/// erase_curr_elem 后, 当前迭代器处于什么位置, 由实际的stl容器的实现确定
	virtual void erase_curr_elem() = 0;
	virtual bool find_if(aot_func_t* f, bool set_curr_pos = true) = 0;
	virtual size_t count_if(aot_func_t* f) = 0;
	virtual void remove_if(aot_func_t* f) = 0;
	virtual void remove_all(aot_func_t* f) = 0;
	virtual void for_each(aot_func_t* f) = 0;
	virtual void lock() = 0;
	virtual void unlock() = 0;
};

template <class param_name_t, class T, class safe_val_type = T::value_type, class SYN_TRAITS = xy::MT_SYN_TRAITS >
class std_container_impl : public istd_container<param_name_t, T, safe_val_type, SYN_TRAITS>
{
public:
	typedef typename SYN_TRAITS::THREAD_MUTEX mutex_type;
	typedef xy::auto_lock< mutex_type > auto_lock_type;
	typedef istd_container<param_name_t, T, safe_val_type, SYN_TRAITS> base_type;
	typedef typename T::iterator iterator_type;
	typedef typename T::value_type value_type;
	typedef safe_val_type safe_value_type;
	typedef typename istd_container::aot_func_t aot_func_t;
public:
	std_container_impl() { this->lock_.open(); }
	virtual ~std_container_impl() { this->lock_.close(); }
public:
	virtual size_t size() const { return this->cont_.size(); }
	virtual void goto_begin() { this->iter_ = this->cont_.begin(); }
	virtual void goto_next() { ++this->iter_; }
	virtual bool is_end() const { return this->iter_ == this->cont_.end(); }
	virtual safe_value_type get_value() { return aot_type_to_type<value_type, safe_value_type>()(*this->iter_); }
	virtual void set_value(safe_value_type v) { *this->iter_ = v; }
	virtual void push_back(safe_value_type v) { this->cont_.push_back(v); }
	virtual void clear() { this->cont_.clear(); }
	virtual bool empty() { return this->cont_.empty(); }
	virtual void remove(safe_value_type v) { std::remove(this->cont_.begin(), this->cont_.end(), v); }
	virtual void erase(safe_value_type v) { this->cont_.erase(std::remove(this->cont_.begin(), this->cont_.end(), v), this->cont_.end()); }
	virtual void erase_curr_elem() { this->iter_ = this->cont_.erase(this->iter_); }

	virtual bool find_if(aot_func_t* f, bool set_curr_pos = true)
	{
		iterator_type it = this->cont_.begin();
		for( ; it != this->cont_.end(); ++it )
		{
			if( f->fun( aot_type_to_type<value_type, safe_value_type>()(*it) ) )
			{
				if( set_curr_pos )
				{
					this->iter_ = it;
				}
				return true;
			}
		}
		return false;
	}

	virtual size_t count_if(aot_func_t* f)
	{
		size_t ret = 0;
		iterator_type it = this->cont_.begin();
		for( ; it != this->cont_.end(); ++it )
		{
			if( f->fun( aot_type_to_type<value_type, safe_value_type>()(*it) ) )
			{
				++ret;
			}
		}
		return ret;
	}

	virtual void remove_if(aot_func_t* f)
	{
		remove_func_t t;
		t.f = f;
		std::remove_if(this->cont_.begin(), this->cont_.end(), t);
	}

	virtual void remove_all(aot_func_t* f)
	{
		iterator_type it = this->cont_.begin();
		for( ; it != this->cont_.end(); ++it )
		{
			f->fun( aot_type_to_type<value_type, safe_value_type>()(*it) );
		}
		this->cont_.clear();
	}

	virtual void for_each(aot_func_t* f)
	{
		iterator_type it = this->cont_.begin();
		for( ; it != this->cont_.end(); ++it )
		{
			f->fun( aot_type_to_type<value_type, safe_value_type>()(*it) );
		}
	}

	virtual void lock() { this->lock_.acquire();}
	virtual void unlock() { this->lock_.release();}
public:
	T& impl() { return this->cont_; }
public:
	T cont_;
	iterator_type iter_;
	mutex_type lock_;
private:
	struct remove_func_t
	{
		bool operator ()(typename T::value_type& v)
		{
			return f->fun(aot_type_to_type<value_type, safe_value_type>()(v));
		}
		aot_func_t* f;
	};
	struct sort_func_t
	{
		bool operator ()(typename T::value_type& v1, typename T::value_type& v2)
		{
			return  f->sort(aot_type_to_type<value_type, safe_value_type>()(v1),
				aot_type_to_type<value_type, safe_value_type>()(v2));
		}
		aot_func_t* f;
	};
};

struct aot_param_name_ivec_str_t
{
	enum{iid = e_aot_iid_ivec_str};
	const char* name(){return "ivec_str";}
};
struct aot_param_name_ivec_long_t
{
	enum{iid = e_aot_iid_ivec_long};
	const char* name(){return "ivec_long";}
};
struct aot_param_name_ivec_ulong_t
{
	enum{iid = e_aot_iid_ivec_ulong};
	const char* name(){return "ivec_ulong";}
};
struct aot_param_name_ilist_str_t
{
	enum{iid = e_aot_iid_ilist_str};
	const char* name(){return "ilist_str";}
};
struct aot_param_name_ilist_long_t
{
	enum{iid = e_aot_iid_ilist_long};
	const char* name(){return "ilist_long";}
};
struct aot_param_name_ilist_ulong_t
{
	enum{iid = e_aot_iid_ilist_ulong};
	const char* name(){return "ilist_ulong";}
};

/// map
struct aot_param_name_istr2int_map_t
{
	enum{iid = e_aot_iid_istr2int_map};
	const char* name(){return "istr2int_map";}
};
struct aot_param_name_istr2str_map_t
{
	enum{iid = e_aot_iid_istr2str_map};
	const char* name(){return "istr2str_map";}
};

/// vector
typedef istd_container< aot_param_name_ivec_str_t, std::vector<std::string>, const char*, xy::MT_SYN_TRAITS > ivec_str;
typedef std_container_impl< aot_param_name_ivec_str_t, std::vector<std::string>, const char*, xy::MT_SYN_TRAITS > vec_str_impl;

typedef istd_container< aot_param_name_ivec_long_t, std::vector<long>, long , xy::MT_SYN_TRAITS> ivec_long;
typedef std_container_impl< aot_param_name_ivec_long_t, std::vector<long>, long, xy::MT_SYN_TRAITS > vec_long_impl;

typedef istd_container< aot_param_name_ivec_ulong_t, std::vector<unsigned long>, unsigned long, xy::MT_SYN_TRAITS > ivec_ulong;
typedef std_container_impl< aot_param_name_ivec_ulong_t, std::vector<unsigned long>, unsigned long, xy::MT_SYN_TRAITS > vec_ulong_impl;

/// list
typedef istd_container< aot_param_name_ilist_str_t, std::list<std::string>, const char*, xy::MT_SYN_TRAITS > ilist_str;
typedef std_container_impl< aot_param_name_ilist_str_t, std::list<std::string>, const char*, xy::MT_SYN_TRAITS > list_str_impl;

typedef istd_container< aot_param_name_ilist_long_t, std::list<long>, long, xy::MT_SYN_TRAITS > ilist_long;
typedef std_container_impl< aot_param_name_ilist_long_t, std::list<long>, long, xy::MT_SYN_TRAITS > list_long_impl;

typedef istd_container< aot_param_name_ilist_ulong_t, std::list<unsigned long>, unsigned long, xy::MT_SYN_TRAITS > ilist_ulong;
typedef std_container_impl< aot_param_name_ilist_ulong_t, std::list<unsigned long>, unsigned long, xy::MT_SYN_TRAITS > list_ulong_impl;

/// map
typedef aot::tt::istd_map< aot_param_name_istr2int_map_t, std::string, int, const char*, int > istr2int_map;
typedef aot::tt::std_map_impl< aot_param_name_istr2int_map_t, std::string, int, const char*, int > str2int_map_impl;

typedef aot::tt::istd_map< aot_param_name_istr2str_map_t, std::string, std::string, const char*, const char* > istr2str_map;
typedef aot::tt::std_map_impl< aot_param_name_istr2str_map_t, std::string, std::string, const char*, const char* > str2str_map_impl;

}} /// end namespace aot/tt

#endif  /// __OUT_PARAM_INTERFACE_H__