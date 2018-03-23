#ifndef __SYS_GLOBA_DEF_H__
#define __SYS_GLOBA_DEF_H__

#include <exception>
#include <assert.h>
namespace xy
{
#define __throw(msg)	throw(std::exception(msg))

//´íÎóÂë
namespace err_code
{
	enum
	{
		e_ok = 0,
		e_time_out = -1,
		e_unknow = -2,
		e_shut_down = -3
	};
};
};
#endif // #ifndef __SYS_GLOBA_DEF_H__