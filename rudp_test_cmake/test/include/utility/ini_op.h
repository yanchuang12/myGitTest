/** Copyright (c) 2008-2009
* All rights reserved.
* 
* 文件名称:		dll_manager.h   
* 摘	 要:	dll包装类
* 
* 当前版本：	1.0
* 作	 者:	范涛涛
* 操	 作:	新建
* 完成日期:		2009年7月12日
*/

#ifndef __INI_OP_H__
#define __INI_OP_H__

#include <string>

#pragma warning(disable:4996)

namespace aot{ namespace tt{ namespace utility{


class ini_op  
{
public:
	bool remove(const char* pszNode)
	{
		return TRUE == ::WritePrivateProfileString( pszNode, NULL, NULL, m_fn.c_str() );
	}

	bool write(const char *pszNode, const char *pszKey, const char* pszVal)
	{
		return TRUE == WritePrivateProfileString(pszNode, pszKey, pszVal, m_fn.c_str());
	}

	bool write(const char* pszNode, const char* pszKey, int nVal)
	{
		char szVal[65];
		_snprintf(szVal, sizeof(szVal), "%d", nVal);
		return TRUE == WritePrivateProfileString(pszNode, pszKey, szVal, m_fn.c_str());
	}

	std::string read(const char*pszNode, const char* pszKey, const char* pszDefault)
	{
		std::string strRet = "";
		char szTmp[256] = {0};

		GetPrivateProfileString(pszNode, pszKey, pszDefault, szTmp,
			sizeof(szTmp)-1, m_fn.c_str());

		szTmp[ sizeof(szTmp)-1 ] = 0;
		strRet = szTmp;

		return strRet;
	}

	int read(const char* pszNode, const char* pszKey, int nDefault)
	{
		return GetPrivateProfileInt(pszNode, pszKey, nDefault, m_fn.c_str());
	}

	void file_name(const char* pszFn)
	{
		m_fn = pszFn;
	}

	const char* file_name() const
	{
		return m_fn.c_str();
	}

private:
	std::string m_fn;

};



}}}

#pragma warning(default:4996)

#endif /* __INI_OP_H__ */
