#ifndef __AOT_ERR_CODE_20100502_H__
#define __AOT_ERR_CODE_20100502_H__

namespace aot{ namespace err{

enum
{
	ok = 0,
	unknown = -1,
	no_data = -3,
	redirect = -201,
	not_found = -404,
	sent_failed = -405,
	sys_error = -500,
	lisence_error = -501,
	
	db_error = -600,
	auth_err = -601,
	param_err = -602,
	time_out = -603,
	disconnection = -604,
	no_interface = -605,
	only_finger_auth = -606,
	ip_addr_auth_err = -607,
	mac_addr_auth_err = -608,
	no_match_cs = -609,			/// platform_name ≤ª∆•≈‰
	no_match_sid = -610,		/// SID ≤ª∆•≈‰

	__client_err_begin__ = -2000,
	client_err_unknow,
};

}} 
#endif /// __AOT_ERR_CODE_20100502_H__