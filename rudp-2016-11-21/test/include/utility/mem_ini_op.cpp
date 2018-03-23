#include "stdafx.h"
#include "mem_ini_op.h"

namespace aot{ namespace tt{ namespace utility{

mem_ini_op::mem_ini_op()
{
	is_curr_node_valid_ = false;
}

mem_ini_op::~mem_ini_op()
{
	;
}

bool 
mem_ini_op::parse(const char* buf, size_t buf_len)
{
	enum efind_status{ e_find_null = 0, e_find_node = 1, e_find_key = 2, e_find_val = 3};

	std::string node_name, key, val;
	efind_status find_status = e_find_null;

	if( NULL == buf || buf_len < 1 )
	{
		return false;
	}

	this->nodes_lst_.clear();
	/// we allow the node name is null, but not allow the key is null
	node_type* node = &this->nodes_lst_[""];

	for( size_t i = 0; i < buf_len && buf[i]; ++i )
	{
		switch( buf[i] )
		{
		case '[':
			{
				/// the first character of <key> can not be '[', otherwise, should parse error!
				if( find_status == e_find_val )
				{
					val += buf[i];
				}
				else if( find_status == e_find_key )
				{
					key += buf[i];
				}
				else if( find_status == e_find_node )
				{
					node_name += buf[i];
				}
				else
				{
					/// by default, we begin to find node name 
					find_status = e_find_node;
				}
				continue;
			}
			break;
		case ']':
			{
				if( find_status == e_find_val )
				{
					val += buf[i];
				}
				else if( find_status == e_find_key )
				{
					key += buf[i];
				}
				else if( find_status == e_find_null )
				{
					key += buf[i];
					find_status = e_find_key;
				}
				else
				{
					node = &this->nodes_lst_[node_name];
					node_name.clear();
					find_status = e_find_null;
				}
				continue;
			}
			break;
		case '=':
			{
				if( find_status == e_find_node )
				{
					node_name += buf[i];
				}
				else if( find_status == e_find_val )
				{
					val += buf[i];
				}
				else if( find_status == e_find_key )
				{
					find_status = e_find_val;
				}
				continue;
			}
			break;
		case '\r':
		case '\n':
			{
				if( find_status == e_find_val )
				{			
					if( key.length() > 0 )
					{
						node->insert(std::make_pair(key, val));
						key.clear();
					}
					val.clear();
				}
				find_status = e_find_null;
				continue;
			}
			break;
		default:
			{
				if( find_status == e_find_node )
				{
					node_name += buf[i];
				}
				else if( find_status == e_find_val )
				{
					val += buf[i];
				}
				else if( find_status == e_find_key )
				{
					key += buf[i];
				}
				else if( find_status == e_find_null )
				{
					key += buf[i];
					find_status = e_find_key;
				}
				continue;
			}
			break;
		}
	}

	if( find_status == e_find_val )
	{
		if( key.length() > 0 )
		{
			node->insert(std::make_pair(key, val));
		}
	}
	return true;
}

bool 
mem_ini_op::go_to_node(const char* node_name)
{
	this->curr_node_ = this->nodes_lst_.find(node_name);
	this->is_curr_node_valid_ = ( this->curr_node_ != this->nodes_lst_.end() );
	return this->is_curr_node_valid_;
}

std::string 
mem_ini_op::read_str(const char* key, const char* def)
{
	if( !this->is_curr_node_valid_ )
		return def;

	node_type::iterator it2 = this->curr_node_->second.find(key);
	if( it2 == this->curr_node_->second.end() )
	{
		return def;
	}
	return it2->second;
}

int 
mem_ini_op::read_int(const char* key, int def)
{
	if( !this->is_curr_node_valid_ )
		return def;

	node_type::iterator it2 = this->curr_node_->second.find(key);
	if( it2 == this->curr_node_->second.end() )
	{
		return def;
	}
	return ::atoi(it2->second.c_str());
}

std::string 
mem_ini_op::read(const char* node_name, const char* key, const char* def)
{
	nodes_list_type::iterator it = this->nodes_lst_.find(node_name);
	if( it == this->nodes_lst_.end() )
	{
		return def;
	}

	node_type::iterator it2 = it->second.find(key);
	if( it2 == it->second.end() )
	{
		return def;
	}
	return it2->second;
}

int 
mem_ini_op::read(const char* node_name, const char* key, int def)
{
	nodes_list_type::iterator it = this->nodes_lst_.find(node_name);
	if( it == this->nodes_lst_.end() )
	{
		return def;
	}

	node_type::iterator it2 = it->second.find(key);
	if( it2 == it->second.end() )
	{
		return def;
	}
	return ::atoi(it2->second.c_str());
}

mem_ini_op::node_type* 
mem_ini_op::get_node(const char* node_name)
{
	nodes_list_type::iterator it = this->nodes_lst_.find(node_name);
	if( it == this->nodes_lst_.end() )
	{
		return NULL;
	}

	return &it->second;
}

void 
mem_ini_op::write(const char* node_name, const char* key, const char* val)
{
	this->nodes_lst_[node_name].insert(std::make_pair(key, val));
}

void 
mem_ini_op::write(const char* node_name, const char* key, int val)
{
	char buf[64];
	sprintf(buf, "%d", val);
	buf[63] = 0;
	this->nodes_lst_[node_name].insert(std::make_pair(key, buf));
}

void 
mem_ini_op::remove(const char* node_name)
{
	this->nodes_lst_.erase(node_name);
}

std::string 
mem_ini_op::build()
{
	std::string s = "";
	nodes_list_type::iterator lit = this->nodes_lst_.begin();
	for( ; lit != this->nodes_lst_.end(); ++lit )
	{
		node_type* node = &lit->second;
		if( node->size() > 0 )
		{
			s += "[" + lit->first + "]\n";

			node_type::iterator it = node->begin();
			for( ; it != node->end(); ++it )
			{
				 s += it->first + "=" + it->second + "\n";
			}
		}
	}

	return s;
}

}}}