
#ifndef __AOT_IID_DEFINE_H__
#define __AOT_IID_DEFINE_H__

enum eaot_iid
{
	e_aot_iid_out_param_range_begin = 1000,
	e_aot_iid_out_param_range_end = 2000,

	e_aot_iid_global_data_range_begin = 2001,
	e_aot_iid_global_data_range_end = 3000,

	e_aot_iid_im_range_begin = 3001,
	e_aot_iid_im_range_end = 4000,

	e_aot_iid_sip_parser_range_begin = 4001,
	e_aot_iid_sip_parser_range_end = 4100,

	e_aot_iid_iprotocol_range_begin = 4101,
	e_aot_iid_iprotocol_range_end = 4150,
};

#endif /// __AOT_IID_DEFINE_H__