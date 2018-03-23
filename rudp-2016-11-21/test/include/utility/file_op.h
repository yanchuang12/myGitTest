/** Copyright (c) 2008-2009
* All rights reserved.
* 
* 文件名称:		file_op.h  
* 摘	 要:	C文件操作函数包装类
* 
* 当前版本：	1.0
* 作	 者:	范涛涛
* 操	 作:	新建
* 完成日期:		2009年7月14日
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
	 *	注意: write可能不会立即写到磁盘文件(有可能被缓存),
	 *		  调用save()可将缓存的数据强制写入磁盘文件
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
	/// 清空文件
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
