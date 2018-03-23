/** Copyright (c) 2008-2009
* All rights reserved.
* 
* �ļ�����:		file_op.h  
* ժ	 Ҫ:	C�ļ�����������װ��
* 
* ��ǰ�汾��	1.0
* ��	 ��:	������
* ��	 ��:	�½�
* �������:		2009��7��14��
*/

#ifndef __NEW_FILE_OP_H__
#define __NEW_FILE_OP_H__



#include <string>
#include <stdio.h>

namespace aot{ namespace tt{ namespace utility{

class file_op
{
public:	
	enum open_mode { e_append = 1, e_create, e_read };

	file_op ();
	~file_op ();

public:
	/**
	 *	e_append: create if not existed, 
	 *	e_create: create_always
	 *	e_read:	  return false if file not existed
	 */
	bool open(const char* fn, int mode);
	///
	bool is_valid();

	/** 
	 *	return:  -1: error;		otherwise: write count
	 *	ע��: write���ܲ�������д�������ļ�(�п��ܱ�����),
	 *		  ����save()�ɽ����������ǿ��д������ļ�
	 */
	size_t write(const char* buf,  size_t len);
	/** 
	 *	return:  -1: error;		otherwise: read count
	 */
	size_t read(char* buf, size_t len);
	
	/** 
	 *	@origin: SEEK_SET, SEEK_CUR,  SEEK_END
	 */
	bool seek( int offset, int origin);
	///
	bool  seek_to_end();
	///
	bool  seek_to_begin();
	/** 
	 *	return:  -1: error;		otherwise: current position
	 */
	long tell();
	/// 
	bool eof();
	/// ����ļ�
	bool  clear();
	///
	bool  save();
	///
	bool close();

	FILE*	file_handle(){ return hf_; }

private:
	FILE*	hf_;
	int		mode_;
	std::string		fn_;		
};


}}}

#endif //__FILE_OP_H__
