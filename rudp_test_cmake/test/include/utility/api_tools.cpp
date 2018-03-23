#include "stdafx.h"  
#include "api_tools.h"

#pragma warning(disable:4996)
#pragma warning(disable:4311)

namespace aot{ namespace tt{namespace utility{

bool 
file_exist(const char *ptr_fn)
{
	bool ret = false;
	WIN32_FIND_DATA fd; 
	HANDLE find = FindFirstFile(ptr_fn, &fd);
	if ( (find !=  INVALID_HANDLE_VALUE) ) 
	{
		if( !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
		{
			ret = true;
		}
	}
	FindClose(find);
	
	return ret;
}

std::string 
get_module_path(HMODULE h)
{
	int i , n;
	char buf[MAX_PATH+1];

	memset(buf, 0, sizeof(buf));
	::GetModuleFileName(h, buf, MAX_PATH);

	n = (int)strlen(buf);

	for( i = n-1; i > 0; --i){

		if( buf[i]=='\\' || buf[i]=='/' ){

			buf[i+1] = 0;
			break;
		}
	}
	return buf;
}

std::string 
get_app_path()
{
	return get_module_path(NULL);
}

std::string 
get_module_filename(HMODULE h)
{
	char buf[MAX_PATH+1];

	memset(buf, 0, sizeof(buf));
	::GetModuleFileName(h, buf, MAX_PATH);
	return buf;
}

std::string 
get_module_name(HMODULE h, bool without_ext_name)
{
	int i , n;
	char buf[MAX_PATH+1];

	memset(buf, 0, sizeof(buf));
	::GetModuleFileName(h, buf, MAX_PATH);

	n = (int)strlen(buf);

	for( i = n-1; i > 0; --i){

		if( buf[i] == '.' && without_ext_name ){

			buf[i] = 0;
			continue;
		}
		if( buf[i] == '\\' || buf[i] == '/' )
		{
			return buf + i + 1;
		}
	}
	return "";
}

int 
safe_snprintf(char* buf, int buf_size, const char* fmt, ...)
{
	int len = 0;

	va_list vl;
	va_start( vl, fmt );
	len = ::_vsnprintf( buf, buf_size - 1, fmt, vl );
	va_end(vl);
	
	buf[buf_size - 1] = 0;

	if( len == -1 )
	{
		return (buf_size - 1);
	}
	
	return len < 1 ? 0 : len;

}

size_t	
safe_strncpy(char* dest, const char* src, size_t n)
{
	size_t i = 0;
	char* ret = dest;

	if(n == 0 || NULL == src){
		dest[0] = 0; return 0;
	}

	for( i = 0; i < n; ++i ){
		if( 0 == (ret[i] = src[i]) ) return (i);
	}

	dest[n] = 0;	
	return n;
}

bool 
dir_exist(const char *path)
{
	bool b = false;
	char tmp[MAX_PATH];

	size_t n = safe_strncpy(tmp, path, sizeof(tmp)-2);

	for( int i=0; i< (int)n; ++i){
		if(tmp[i] == '/') tmp[i] = '\\';
	}

	if( tmp[n-1] == '\\' ) {
		tmp[n-1] = 0;
	}

	WIN32_FIND_DATA fd; 
	HANDLE hFind = FindFirstFile(tmp, &fd);

	if ( hFind != INVALID_HANDLE_VALUE ) {

		if( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			b = true;
	}

	FindClose(hFind);
	return b;
}

bool 
dir_create(const char * path)
{
	if( dir_exist(path) )
		return true;

	int i, n;
	char szPath[MAX_PATH] = {0};
	char szTmp[MAX_PATH] = {0};

	n = (int)safe_strncpy( szTmp, path, sizeof(szTmp)-1 );

	for( i=0; i<n; i++ ) {		

		if( szTmp[i] == '\\' || szTmp[i] == '/' ) {

			safe_strncpy( szPath, szTmp, i );
			::CreateDirectory( szPath, NULL );
		}
	}

	return dir_exist(path);
}

bool
dir_delete(const char* dir)
{
	del_all_files(dir, true);

	return TRUE == ::RemoveDirectory(dir);

}

bool 
del_all_files(const char* dir, bool enter_sub_dir)
{
	bool ret = true;
	WIN32_FIND_DATA wfd; 
	HANDLE h = NULL;
	std::string tmp = "";
	std::string cur_dir = dir;
	std::string fmt = "";
	char c = cur_dir[cur_dir.length()-1];

	if( c != '\\' && c != '/' )
		cur_dir += "\\";

	fmt = cur_dir + "*.*";

	h = ::FindFirstFile(fmt.c_str(), &wfd); 

	while( h != INVALID_HANDLE_VALUE && 0 != ::FindNextFile(h, &wfd))
	{
		tmp = cur_dir + wfd.cFileName;

		if( !(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
		{
			::DeleteFile(tmp.c_str());
		}
		else
		{
			if( enter_sub_dir ) /// 允许删除子目录
			{
				bool b = false;
				if (wfd.cFileName[0] == '.')
				{
					if (wfd.cFileName[1] == '\0' || (wfd.cFileName[1] == '.' && wfd.cFileName[2] == '\0'))
						b = true;
				}

				if( !b )
				{
					del_all_files(tmp.c_str(), enter_sub_dir);
					::RemoveDirectory(tmp.c_str());
				}
			}
		}
	}

	FindClose(h);
	return true;
}


char* 
str_find_lastof(char* str, char key, int len/* = -1*/)
{
	char* p = str;
	char* r = NULL;

	if( -1 == len )
	{
		while( *p ) {

			if( *p == key)	{ r = p; }

			++p;
		}
	}
	else
	{
		while( --len >= 0 ) {

			if( str[len] == key)
				return (str + len);
		}
	}

	return r;
}

const char* 
str_find_lastof(const char* str, char key, int len/* = -1*/)
{
	const char* p = str;
	const char* r = NULL;

	if( -1 == len )
	{
		while( *p ) {

			if( *p == key)	{ r = p; }

			++p;
		}
	}
	else {

		while( --len >= 0 ) {

			if( str[len] == key)
				return (str + len);
		}
	}

	return r;
}


char* 
str_find_lastof(char* str, const char* key, int len/* = -1*/)
{
	char* p = str;
	char* r = NULL;
	const char* k = key;

	if( -1 == len )
	{
		while( *p ) {

			k = key;

			while( *k ) {

				if( *k == *p ) {
					r = p; break;
				}

				++k;
			}

			++p;
		}
	}
	else {

		while( --len >= 0 )
		{
			k = key;

			while( *k ) {

				if( *k == str[len] ) {
					return (str + len);
				}

				++k;
			}

		}
	}

	return r;
}

const char* 
str_find_lastof(const char* str, const char* key, int len/* = -1*/)
{
	const char* p = str;
	const char* r = NULL;
	const char* k = key;

	if( -1 == len )
	{
		while( *p ) {

			k = key;

			while( *k ) {

				if( *k == *p ) {
					r = p; break;
				}

				++k;
			}

			++p;
		}
	}
	else
	{
		while( --len >= 0 )
		{
			k = key;

			while( *k ) {

				if( *k == str[len] ) {
					return (str + len);
				}

				++k;
			}

		}
	}

	return r;
}

char* to_str(char* buf, int v)
{
	sprintf(buf, "%d", v);
	return buf;
}
char* to_str(char* buf, unsigned int v)
{
	sprintf(buf, "%u", v);
	return buf;
}
char* to_str(char* buf, long v)
{
	sprintf(buf, "%ld", v);
	return buf;
}
char* to_str(char* buf, unsigned long v)
{
	sprintf(buf, "%lu", v);
	return buf;
}
char* to_str(char* buf, float v)
{
	sprintf(buf, "%.2f", v);
	return buf;
}
char* to_str(char* buf, double v)
{
	sprintf(buf, "%.2f", v);
	return buf;
}

std::string to_str(int v)
{
	char s[32];
	sprintf(s, "%d", v);
	return s;
}

std::string to_str(unsigned int v)
{
	char s[32];
	sprintf(s, "%u", v);
	return s;
}

std::string to_str(long v)
{
	char s[32];
	sprintf(s, "%ld", v);
	return s;
}

std::string to_str(unsigned long v)
{
	char s[32];
	sprintf(s, "%lu", v);
	return s;
}

std::string to_str(float v)
{
	char s[32];
	sprintf(s, "%.2f", v);
	return s;
}

std::string to_str(double v)
{
	char s[32];
	sprintf(s, "%.2f", v);
	return s;
}

bool
str_format(std::string& s, const char* fmt, ...)
{
	char* buf = NULL;
	int len = 0;
	bool ret = true;

	va_list vl;
	va_start( vl, fmt );

	len = ::_vscprintf(fmt, vl);

	if( len > 0)
	{
		buf = new char[len + 1];
		len = ::_vsnprintf( buf, len, fmt, vl );
		buf[len] = 0;
		s = buf;
		delete []buf;
	}
	else
	{
		s.clear();
		ret = false;
	}

	va_end(vl);
	return ret;
}

void 
str_trim_all(std::string& s, const char* k/* = "\t\n\f\v\b\r "*/)
{
	str_trim_left(s, k);
	str_trim_right(s, k);
}

void 
str_trim_left(std::string& s, const char* k/* = "\t\n\f\v\b\r "*/)
{
	if(!s.empty())
	{
		std::string::size_type pos = s.find_first_not_of(k);
		s.erase(0, pos);
	}
}

void 
str_trim_right(std::string& s, const char* k/* = "\t\n\f\v\b\r "*/)
{
	if(!s.empty())
	{
		std::string::size_type pos = s.find_last_not_of(k);
		if( std::string::npos == pos )
		{
			s = "";
		}
		else
		{
			s.erase(pos + 1);
		}
	}
}

char* 
get_date_time(char* buf)
{
	SYSTEMTIME tm;
	::GetLocalTime(&tm);
	sprintf(buf, "%4d-%02d-%02d %02d:%02d:%02d", 
		tm.wYear, tm.wMonth, tm.wDay,
		tm.wHour, tm.wMinute, tm.wSecond);
	return buf;
}

char* 
str_replace(char* src, char cs, char cd)
{
	int i = 0;
	for( ; src && src[i]; ++i )
	{
		if( src[i] == cs ) src[i] = cd;
	}
	return src;
}

std::string 
get_env_val(const char* name, const char* def)
{
	char buf[1024];
	DWORD n = GetEnvironmentVariable(name, buf, sizeof(buf) - 1);
	if( n == 0 ) return (def ? def : "");

	if( n <= sizeof(buf) - 1 )
	{
		buf[n] = 0;
	}
	else
	{
		buf[sizeof(buf) - 1] = 0;
	}

	return buf;
}

void
base64_encode(char *dst, int* dst_len, char *src, int src_len)
{
	unsigned char         *d, *s;
	size_t          len;
	static unsigned char   basis64[] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	len = src_len;
	s = (unsigned char*)src;
	d = (unsigned char*)dst;

	while (len > 2) {
		*d++ = basis64[(s[0] >> 2) & 0x3f];
		*d++ = basis64[((s[0] & 3) << 4) | (s[1] >> 4)];
		*d++ = basis64[((s[1] & 0x0f) << 2) | (s[2] >> 6)];
		*d++ = basis64[s[2] & 0x3f];

		s += 3;
		len -= 3;
	}

	if (len) {
		*d++ = basis64[(s[0] >> 2) & 0x3f];

		if (len == 1) {
			*d++ = basis64[(s[0] & 3) << 4];
			*d++ = '=';

		} else {
			*d++ = basis64[((s[0] & 3) << 4) | (s[1] >> 4)];
			*d++ = basis64[(s[1] & 0x0f) << 2];
		}

		*d++ = '=';
	}

	*dst_len = (int)(d - (unsigned char*)dst);
}


bool
base64_decode(char *dst, int* dst_len, char *src, int src_len)
{
	size_t          len;
	unsigned char   *d, *s;
	static unsigned char   basis64[] = {
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 62, 77, 77, 77, 63,
		52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 77, 77, 77, 77, 77, 77,
		77,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
		15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 77, 77, 77, 77, 77,
		77, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
		41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 77, 77, 77, 77, 77,

		77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77
	};

	for (len = 0; (int)len < src_len; len++) {
		if (src[len] == '=') {
			break;
		}

		if (basis64[src[len]] == 77) {
			return false;
		}
	}

	if (len % 4 == 1) {
		return false;
	}

	s = (unsigned char*)src;
	d = (unsigned char*)dst;

	while (len > 3) {
		*d++ = (unsigned char) (basis64[s[0]] << 2 | basis64[s[1]] >> 4);
		*d++ = (unsigned char) (basis64[s[1]] << 4 | basis64[s[2]] >> 2);
		*d++ = (unsigned char) (basis64[s[2]] << 6 | basis64[s[3]]);

		s += 4;
		len -= 4;
	}

	if (len > 1) {
		*d++ = (unsigned char) (basis64[s[0]] << 2 | basis64[s[1]] >> 4);
	}

	if (len > 2) {
		*d++ = (unsigned char) (basis64[s[1]] << 4 | basis64[s[2]] >> 2);
	}

	*dst_len = (int)(d - (unsigned char*)dst);

	return true;
}

bool 
base64_decode(std::string* s, char *src, int src_len)
{
	int n = base64_decoded_length(src_len);

	char *buf = new char[n + 2];

	if( !base64_decode(buf, &n, src, src_len) )
	{
		delete buf;
		return false;
	}

	buf[n] = '\0';

	*s = buf;
	delete buf;

	return true;
}

char* make_rand_str(char *out, int len)
{	
	const char base[] = 
	{
		'0','1','2','3','4','5','6','7','8','9',	/*  0-9 */
		'a','b','c','d','e','f','g','h','i','j',	/* 10-19 */
		'k','l','m','n','o','p','q','r','s','t',	/* 20-29 */
		'u','v','w','x','y','z',					/* 30-35 */
		'A','B','C','D','E','F','G','H','I','J',	/* 36-45 */
		'K','L','M','N','O','P','Q','R','S','T',	/* 46-54 */
		'U','V','W','X','Y','Z',					/* 55-60 */
		'~','!','@','#','$','%','^','&','*','<',	/* 61-70 */
		'>','`',';','[',']','{','}'					/* 71-77 */
	};

	::srand(GetTickCount());

	for( int i = 0; i < len; i++ )
	{
		out[i] = base[ ::rand()%60 + 1 ];
	}

	return out;
}

void string_split(const char* str, std::vector<std::string>* v, char chr, bool ignore_empty_val)
{
	/// 适用格式 xx|xx|xx
	int i;
	std::string val = "";

	for( i = 0; str && str[i]; ++i )
	{
		if( str[i] == chr )
		{
			if( !ignore_empty_val 
				|| (ignore_empty_val && val.length() > 0) )
			{
				v->push_back(val);
				val.clear();
			}
		}
		else
		{
			val += str[i];
		}
	}

	if( val.length() > 0 )
	{
		v->push_back(val);
	}
}

}}}
#pragma warning(default:4996)
#pragma warning(default:4311)