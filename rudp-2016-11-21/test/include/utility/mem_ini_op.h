/** Copyright (c) 2008-2009
 * All rights reserved.
 * 
 * �ļ�����:	mem_ini_op.h   
 * ժ	 Ҫ:	�����ڴ��е�ini��ʽ������
 * 
 * ��ǰ�汾:	1.0
 * ��	 ��:	������
 * ��	 ��:	�½�
 * �������:	2009��12��30��
 */
#ifndef __MEM_INI_OP_H__
#define __MEM_INI_OP_H__

#include <map>
#include <string>

namespace aot{ namespace tt{ namespace utility{

class mem_ini_op
{
public:
	typedef std::map<std::string, std::string> node_type;
	typedef std::map<std::string, node_type> nodes_list_type;
public:
	mem_ini_op();
	~mem_ini_op();
public:
	bool parse(const char* buf, size_t buf_len);
	bool go_to_node(const char* node_name);
	std::string read_str(const char* key, const char* def);
	int read_int(const char* key, int def);
public:
	/// 
	std::string read(const char* node_name, const char* key, const char* def);
	int read(const char* node_name, const char* key, int def);

	void write(const char* node_name, const char* key, const char* val);
	void write(const char* node_name, const char* key, int val);
	void remove(const char* node_name);
	std::string build();
public:
	nodes_list_type* get_nodes_list(){ return &this->nodes_lst_; }
	node_type* get_node(const char* node_name);
private:
	bool is_curr_node_valid_;
	nodes_list_type nodes_lst_;
	nodes_list_type::iterator curr_node_;
};

}}}

#endif /// __MEM_INI_OP_H__