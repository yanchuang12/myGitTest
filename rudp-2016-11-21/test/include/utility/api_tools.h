/** Copyright (c) 2008-2009
* All rights reserved.
* 
* 文件名称:		api_tools.h  
* 摘	 要:	工具函数
* 
* 当前版本：	1.0
* 作	 者:	范涛涛
* 操	 作:	新建
* 完成日期:		2009年7月12日
*/

#ifndef __API_TOOLS_H__
#define __API_TOOLS_H__

#include <string>
#include <vector>

namespace aot{ namespace tt{ namespace utility{

bool file_exist(const char *ptr_fn);

/**
 *	取该模块所在路径
 */
std::string	get_module_path(HMODULE h);

/**
 *	取exe主程序所在路径, 结尾包含 \
 */
std::string	get_app_path();

/// 取模块完整文件名
std::string get_module_filename(HMODULE h);

std::string 
get_module_name(HMODULE h, bool without_ext_name = false);

/**
 *	保证buf[buf_size - 1] 总是为'\0'
 */
int safe_snprintf(char* buf, int buf_size, const char* fmt, ...);

/** 
 *	if( strlen(src) >= n) , 将拷贝n个字符， 并且dest[n] = 0, 所以应该: n < dest的缓冲区长度
 *	 返回实际拷贝的有效字符数（不含'\0')
 */
size_t safe_strncpy(char* dest, const char* src, size_t n);

/// 判断目录是否存在
bool dir_exist(const char *path);

/// 创建目录, @path 末尾加'\\'
bool dir_create(const char * path);

/// 删除目录(包括子目录及文件)
bool dir_delete(const char* dir);

bool del_all_files(const char* dir, bool enter_sub_dir = true);

/*	查找str中最后一个 ==key 的字符, 返回该位置的指针, 否则返回NULL
 *	如果指定了len, 则从str[len-1]开始反向查找, 效率可能会更高
 */
char* str_find_lastof(char* str, char key, int len = -1);
/// 同上
const char* str_find_lastof(const char* str, char key, int len = -1);

/**	反向查找str, 如果key中的任意一个字符在str中出现, 则返回该位置的指针, 否则返回NULL
 *	如果指定了len, 则从str[len-1]开始反向查找, 效率可能会更高
 */
char* str_find_lastof(char* str, const char* key, int len = -1);
/// 同上	
const char* str_find_lastof(const char* str, const char* key, int len = -1);

std::string to_str(int v);
std::string to_str(unsigned int v);
std::string to_str(long v);
std::string to_str(unsigned long v);
std::string to_str(float v);
std::string to_str(double v);

char* to_str(char* buf, int v);
char* to_str(char* buf, unsigned int v);
char* to_str(char* buf, long v);
char* to_str(char* buf, unsigned long v);
char* to_str(char* buf, float v);
char* to_str(char* buf, double v);

bool str_format(std::string& s, const char* fmt, ...);

void str_trim_all(std::string& s, const char* k = "\t\n\f\v\b\r ");
void str_trim_left(std::string& s, const char* k = "\t\n\f\v\b\r ");
void str_trim_right(std::string& s, const char* k = "\t\n\f\v\b\r ");

char* get_date_time(char* buf);

char* str_replace(char* src, char cs, char cd);

/// 读取系统环境变量
std::string get_env_val(const char* name, const char* def);

inline int base64_encoded_length(int len)  { return (((len + 2) / 3) * 4);}
inline int base64_decoded_length(int len)  { return (((len + 3) / 4) * 3);}

void base64_encode(char *dst, int* dst_len, char *src, int src_len);

bool base64_decode(char *dst, int* dst_len, char *src, int src_len);

bool base64_decode(std::string* s, char *src, int src_len);

char* make_rand_str(char *out, int len);

void string_split(const char* str, std::vector<std::string>* v, char chr, bool ignore_empty_val);

}}}


#endif //