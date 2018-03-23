/** Copyright (c) 2008-2009
 * All rights reserved.
 * 
 * 文件名称:	xml_interface.h 
 * 摘	 要:	xml解析器接口基类
 * 
 * 当前版本:	1.0
 * 作	 者:	范涛涛
 * 操	 作:	新建
 * 完成日期:	2009年7月12日
 */
#ifndef __XML_PARSER_INTERFACE_H__
#define __XML_PARSER_INTERFACE_H__

#include "interface_base.h"
#include <ObjBase.h>
namespace aot{ namespace tt{

class cocom_guard
{
public:
	cocom_guard(){ ::CoInitialize(NULL);}
	~cocom_guard() { ::CoUninitialize();}
};

struct xml_string_t
{
	xml_string_t ()
	{
		str_ = NULL;
		len_ = 0;
		size_ = 0;
	}
	char*	str_;
	size_t	len_;
	size_t	size_;
};

template< int buf_size>
class __xml_string_impl : public xml_string_t
{
public:
	__xml_string_impl ()
	{
		size_ = buf_size;
		str_ = buf_;
		len_ = 0;
		str_[0] = 0;
	}
	void clear()
	{
		str_[0] = 0;
		len_ = 0;
	}

	operator const xml_string_t* () { return this; }
	operator xml_string_t* () { return this; }
	size_t length () { return len_; }
	char* c_str () { return str_; }
private:
	char buf_[buf_size];
};

typedef __xml_string_impl<512>	xml_string_impl;
typedef __xml_string_impl<4096>	xml_string_large_impl;

class ires_parser : public virtual interface_base
{
protected:
	virtual ~ires_parser (){}
public:
	virtual bool query_interface (void** out, const char* key){return false;}
	virtual const char* interface_name () {return "ires_parser";}
public:
	virtual bool open(const char* fn) = 0;
	/// @path: e.g: "//root/string_res/"
	virtual bool set_domain(const char* path, const char* label) = 0;
	virtual void get_value_by_key(xml_string_t* str, const char* key_name, const char* key_val, const char* def = "") = 0;
	virtual void get_value_by_id(xml_string_t* str, const char* id, const char* def = "") = 0;
	virtual void get_value_by_id(xml_string_t* str, int id, const char* def = "") = 0;
};

class res_parser_string_helper
{
public:
	res_parser_string_helper()
	{
		this->impl_ = NULL;
	}

	virtual ~res_parser_string_helper()
	{
		if( this->impl_ )
			this->impl_->destroy();
	}

	operator void** () {return (void**)&this->impl_;}
	operator ires_parser** () {return (ires_parser**)&this->impl_;}

	bool open(const char* res_fn)
	{
		if( this->impl_ )
		{
			if( this->impl_->open(res_fn) )
				return this->impl_->set_domain("//res/string/", "str");
		}
		return false;
	}

	const char* get_string(const char* id, const char* def = "")
	{
		if( !this->impl_ )
			return def;
		this->impl_->get_value_by_id(this->str_, id, def);
		return this->str_.c_str();
	}

	ires_parser* obj()
	{
		return this->impl_;
	}
protected:
	ires_parser* impl_;
	xml_string_impl str_;
};


/// 以下所谓根路径并不是是指 '//', 而是 '//' + 根节点名
class ixml_element : public virtual interface_base
{
protected:
	virtual ~ixml_element (){}
public:
	virtual bool query_interface(void** out, const char* key){return false;}
	virtual const char* interface_name() {return "ixml_element";}
public:
	virtual bool is_valid () = 0;
	virtual void get_curr_node_name (xml_string_t* str) = 0;

	/**
	 *	删除当前路径的节点及所有子节点
	 *	bRemoveSelf : 
	 *				true 删除当前路径的节点及所有子节点, 注意,因为当前路径已经被删除,
	 *					 所以必须重新设定当前工作路径
	 *				false 仅删除子节点
	 *	返回: true 成功 ; false 失败
	 */
	virtual bool remove (bool remove_self = false) = 0;

	virtual bool create_child (const char* name, const char* val) = 0;
	virtual bool create_child (ixml_element** out, const char* name) = 0;
	virtual void get_value (xml_string_t* str, const char* def = "") = 0;

	virtual void get_child_attr (xml_string_t* str, const char* path, 
		const char* attr_name, const char* def = "") = 0;

	virtual void get_child_value (xml_string_t* str, const char* path, const char* def = "") = 0;
	virtual bool set_value (const char* val) = 0;
	virtual bool set_child_value (const char* child_name, const char* val, bool create_if_not_found = true) = 0;
	virtual void get_attr (xml_string_t* str, const char* attr_name, const char* def = "") = 0;
	virtual bool set_attr (const char* attr_name, const char* val) = 0;
	/// child_name 当前节点的相对路径
	virtual bool set_child_attr (const char* child_name, const char* attr_name, 
						const char* val, bool create_if_not_found = true) = 0;
	/// -1 : error
	virtual long get_child_count () = 0;
	/// n >= 0
	virtual bool get_child (ixml_element** out, int n /* = 0 */) = 0;
	/// child_name :  eg: sub/sub1/sub2
	virtual bool get_child (ixml_element** out, const char* child_name, bool create_if_not_found) = 0;
	virtual void get_current_pos_xml (xml_string_t* str) = 0;
};

class ixml_parser : public  virtual interface_base
{
protected:
	virtual ~ixml_parser (){}
public:
	virtual bool query_interface (void** out, const char* key){return false;}
	virtual const char* interface_name (){return "ixml_parser";}
public:
	virtual bool is_valid () = 0;
	virtual ixml_element* get_cur_element () = 0;
	virtual bool create (const char* root_name, const char* encoding = "UTF-8") = 0;
	virtual bool open (const char* fn) = 0;
	virtual void close () = 0;
	virtual bool load_from_string (const char* data) = 0;
	virtual void get_whole_xml (xml_string_t* str) = 0;
	/// 将数据保存到(xml)文件
	/// pszFn: 保存文件名, ==NULL或"" 表示沿用上次open时的文件名
	virtual bool save (const char* fn = NULL) = 0;

	/// should changed current pos; 跳转到根路径
	virtual bool goto_root () = 0;

	/// should changed current pos; 
	virtual long nodes_select (const char* xpath) = 0;

	/// should changed current pos; 
	virtual bool nodes_goto_next () = 0;

	/// should changed current pos; 
	virtual bool nodes_goto_i (int n = 0) = 0;//from 0

	/**
	 *	设置当前工作路径
	 *	pszPath: 绝对路径表示 eg: //shine/row
	 *			 xpath表示: eg: //xmlRoot/b/a[@id=1] 注意：属性名必须是id
	 *	bCreateIfNotFound: true: 如果该路径不存在,则创建(采用xpath表示时，属性将忽略）
	 *						 false: 不创建
	 *	返回: true 成功;	false 失败
	 */
	virtual bool set_cur_pos (const char* pszxPath, bool bCreateIfNotFound = false, int n = 0) = 0;

	/// 获取当前工作路径
	virtual const char* get_cur_pos () = 0;
	virtual bool query_single_node (ixml_element** out, const char* xpath) = 0;
	/// 常用
	virtual void query_value (xml_string_t* str, const char* xpath, const char* def = "") = 0;
};

}}

#endif /// __XML_PARSER_INTERFACE_H__