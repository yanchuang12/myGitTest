/** Copyright (c) 2008-2009
* All rights reserved.
* 
* �ļ�����:		api_tools.h  
* ժ	 Ҫ:	���ߺ���
* 
* ��ǰ�汾��	1.0
* ��	 ��:	������
* ��	 ��:	�½�
* �������:		2009��7��12��
*/

#ifndef __API_TOOLS_H__
#define __API_TOOLS_H__

#include <string>
#include <vector>

namespace aot{ namespace tt{ namespace utility{

bool file_exist(const char *ptr_fn);

/**
 *	ȡ��ģ������·��
 */
std::string	get_module_path(HMODULE h);

/**
 *	ȡexe����������·��, ��β���� \
 */
std::string	get_app_path();

/// ȡģ�������ļ���
std::string get_module_filename(HMODULE h);

std::string 
get_module_name(HMODULE h, bool without_ext_name = false);

/**
 *	��֤buf[buf_size - 1] ����Ϊ'\0'
 */
int safe_snprintf(char* buf, int buf_size, const char* fmt, ...);

/** 
 *	if( strlen(src) >= n) , ������n���ַ��� ����dest[n] = 0, ����Ӧ��: n < dest�Ļ���������
 *	 ����ʵ�ʿ�������Ч�ַ���������'\0')
 */
size_t safe_strncpy(char* dest, const char* src, size_t n);

/// �ж�Ŀ¼�Ƿ����
bool dir_exist(const char *path);

/// ����Ŀ¼, @path ĩβ��'\\'
bool dir_create(const char * path);

/// ɾ��Ŀ¼(������Ŀ¼���ļ�)
bool dir_delete(const char* dir);

bool del_all_files(const char* dir, bool enter_sub_dir = true);

/*	����str�����һ�� ==key ���ַ�, ���ظ�λ�õ�ָ��, ���򷵻�NULL
 *	���ָ����len, ���str[len-1]��ʼ�������, Ч�ʿ��ܻ����
 */
char* str_find_lastof(char* str, char key, int len = -1);
/// ͬ��
const char* str_find_lastof(const char* str, char key, int len = -1);

/**	�������str, ���key�е�����һ���ַ���str�г���, �򷵻ظ�λ�õ�ָ��, ���򷵻�NULL
 *	���ָ����len, ���str[len-1]��ʼ�������, Ч�ʿ��ܻ����
 */
char* str_find_lastof(char* str, const char* key, int len = -1);
/// ͬ��	
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

/// ��ȡϵͳ��������
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