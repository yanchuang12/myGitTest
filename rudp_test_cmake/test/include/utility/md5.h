
#ifndef __MD5_20130408_H__
#define __MD5_20130408_H__

#include <commondef/aot_typedef.h>
typedef aot_uint32_t		md_dword_t;

class CMD5  
{
public:

	bool MD5_Buf(unsigned char* pBuf, unsigned int nLength, unsigned char* pOutMD5);
	bool MD5_File(const char* pszFn, unsigned char* pOutMD5);

	CMD5();
	virtual ~CMD5() {};

public:
	void update(unsigned char* Input, md_dword_t nInputLen);
	bool Final(unsigned char* pOutMD5);
protected:
	void Transform(unsigned char Block[64]);
	
	inline md_dword_t RotateLeft(md_dword_t x, int n);
	inline void FF( md_dword_t& A, md_dword_t B, md_dword_t C, md_dword_t D, md_dword_t X, md_dword_t S, md_dword_t T);
	inline void GG( md_dword_t& A, md_dword_t B, md_dword_t C, md_dword_t D, md_dword_t X, md_dword_t S, md_dword_t T);
	inline void HH( md_dword_t& A, md_dword_t B, md_dword_t C, md_dword_t D, md_dword_t X, md_dword_t S, md_dword_t T);
	inline void II( md_dword_t& A, md_dword_t B, md_dword_t C, md_dword_t D, md_dword_t X, md_dword_t S, md_dword_t T);
	
	//utility functions
	void DWordToByte(unsigned char* Output, md_dword_t* Input, unsigned int nLength);
	void ByteToDWord(md_dword_t* Output, unsigned char* Input, unsigned int nLength);
	
private:
	unsigned char  m_lpszBuffer[64];  //input buffer
	md_dword_t m_nCount[2];   //number of bits, modulo 2^64 (lsb first)
	md_dword_t m_lMD5[4];   //MD5 checksum
};

#endif // __MD5_20130408_H__
