/** Copyright (c) 2008-2009
 * All rights reserved.
 * 
 * �ļ�����:	aot_prot.h   
 * ժ	 Ҫ:	imЭ��
 * 
 * ��ǰ�汾��	1.0
 * ��	 ��:	������(FTT)
 * ��	 ��:	�½�
 * �������:	2009��11��10��
 */

#ifndef __AOT_PROT_H__
#define __AOT_PROT_H__

#include <commondef/aot_typedef.h>
#include <interface/aot_inet_define.h>
#include <interface/aot_inet_interface.h>
#include <aot_std/aot_list.h>
#include <aot_std/aot_vector.h>
#include <aot_std/aot_alloc.h>
#include <interface/aot_inet_cdr.h>
#include <vector>
#include <string>
#include <list>

#pragma warning(disable:4996)

namespace aot{ namespace prot{

typedef std::string prot_string;

inline aot_nbrnum_t str2nbrnum(const char* s)
{
	if( s && s[0] ) return (aot_nbrnum_t)atol(s);
	else return 0;
}

inline std::string nbrnum2str(aot_nbrnum_t n)
{
	char buf[64];
	_snprintf(buf, sizeof(buf) - 1, "%u", n);
	return buf;
}

inline char* nbrnum2str(char* buf, int buf_len, aot_nbrnum_t n)
{
	_snprintf(buf, buf_len, "%u", n);
	return buf;
}

inline aot_uint32_t string_cdr_size(prot_string* s)
{
	aot_uint32_t n = (aot_uint32_t)s->length();
	return n == 0 ? 4 : 4 + n + 1;
}

enum
{
	e_std_string_cdr_min_size = 4,
};

inline aot_uint32_t 
hostid_to_clusterid(aot_uint32_t host_id)
{
	return (host_id & 0xFFFFFF00);
}

inline aot_uint32_t 
hostid_to_cluster_proxy_id(aot_uint32_t host_id)
{
	/// ��Ⱥ��������(dbs)��id���Ǽ�Ⱥid
	return (host_id & 0xFFFFFF00);
}

inline aot_uint32_t 
clusterid_to_cluster_proxy_id(aot_uint32_t cid)
{
	/// ��Ⱥid���Ǽ�Ⱥ��������(dbs)��id
	return cid;
}

inline bool is_in_same_cluster(aot_uint32_t host_id1, aot_uint32_t host_id2)
{
	return (host_id1 & 0xFFFFFF00) == (host_id2 & 0xFFFFFF00);
}

inline bool
is_public_cluster(aot_uint32_t cid)
{
	return (cid & 0xFFFF0000) == 0x00000000;
}

inline aot_uint32_t
tataid_to_clusterid(aot_uint32_t tata_id)
{
	return 0x0100;
	/*aot_uint32_t r = tata_id / 1000000;
	if( r < 0xFF )
	{
		return (r+1) << 8;
	}
	aot_uint32_t r2 = r / 255;
	if( r2 < 0xFF )
	{
		return ((r+1) << 8) + ((r2+1)<<16);
	}
	assert(0);
	return 0;*/
}

inline aot_uint32_t
tataid_to_cluster_proxy_id(aot_uint32_t tata_id)
{
	aot_uint32_t n = tataid_to_clusterid(tata_id);
	return clusterid_to_cluster_proxy_id(n);
}


enum eattr
{
	e_attr_unknow = 0,
	e_attr_tata = 1,		/// tata�ͻ���
	e_attr_ims,				/// ims
	e_attr_tdbs,			/// Ⱥ��/��ҵ������
	____e_attr_noused1___,
	e_attr_dbs,				/// ims��������
	e_attr_ss,				/// switch server
	e_attr_sc,				/// switch client
	____e_attr_noused2___,
	____e_attr_noused3___,
	e_attr_mques,			/// mques������
	e_attr_mques_client,	/// mques�ͻ���
	e_attr_nms,				/// ���ܷ�����
	e_attr_nmctrl,			/// ���ܿ��ƿͻ���
	e_attr_nm_client,		/// ���ܿͻ���
	e_attr_hts_forward,
	e_attr_htc_forward,
	e_attr_lgs,
	e_attr_lgc,
	e_attr_sms,				/// save msg server
	e_attr_smc,				/// save msg client
	e_attr_fs,				/// feature server
	e_attr_fc,				/// feature client
	e_attr_mps,				/// mps
	e_attr_mps_client,		/// mps client
};

enum euser_status
{
	e_user_status_online = 0,	/// ����
	e_user_status_busy,			/// æµ
	e_user_status_leave,			/// �뿪  
	e_user_status_hide,			/// ����    
	e_user_status_offline,		/// ����
	e_user_status_end = 20,
};

enum etata_type
{
	e_tata_type_pc_win =				0x00000001,			/// 
	e_tata_type_mobile_ios =			0x00000001 << 1,
	e_tata_type_mobile_android =		0x00000001 << 2,
	e_tata_type_x_3	=					0x00000001 << 3,
	e_tata_type_x_4	=					0x00000001 << 4,
	e_tata_type_x_5	=					0x00000001 << 5,
	e_tata_type_x_6	=					0x00000001 << 6,
	e_tata_type_x_7	=					0x00000001 << 7,

	e_tata_type_all = 0xffffffff,
};

/// ����ʱģʽ. ��Ϊ��������˵���ҵ��֯�ܹ�ʱ,�������ʵʱ֪ͨ����,��ô, һ���û���״̬�ı�, ��˲����������ӵ��,Ӱ����������. 
/// ���ͻ����ںܶ�ʱ��, ��������ĳ��Ա״̬�Ƿ�ı�, ��������˲���, �Ż��������
enum etata_runtime_mode
{
	/// ����ģʽ, ��ʾ��ʱ,״̬��ʵʱ֪ͨ(����Ϣ��Ȼ���ӳ�֪ͨ��ͬ, ��������״̬�ı����Ǻ��ѻ�����ҵ��Ա, �ɿͻ��������ж�)
	/// 1. �����ֻ��ͻ���, simba����ǰ̨ʱ, ��Ӧ������Ϊ e_tata_runtime_mode_active ģʽ
	/// 2. ����PC�ͻ���, �������Ҫͳ��״̬�����UI������ָ��ͻ�ʱ, ��Ӧ������Ϊ e_tata_runtime_mode_active ģʽ
	e_tata_runtime_mode_active = 0, 

	/// ��̨ģʽ, ��ʾ��ʱ,״̬���ɷ���˻���, �������ͻ���, ֱ���ͻ�������Ϊ e_tata_runtime_mode_active ģʽʱ, �ŷ��͸��ͻ���
	/// 1. �����ֻ��ͻ���, simba���ں�̨ʱ, ��Ӧ������Ϊ e_tata_runtime_mode_background ģʽ
	/// 2. ����PC�ͻ���, ���û����Ҫͳ��״̬�����UI������ָ��ͻ�ʱ, ��Ӧ������Ϊ e_tata_runtime_mode_background ģʽ
	e_tata_runtime_mode_background,
};

enum eusl_notify_mode
{
	e_usl_delay_mode_realtime = 0,		/// ʵʱ����ģʽ
	e_usl_delay_mode_delay = 1,			/// �ӳ�����ģʽ
	e_usl_delay_mode_runtime = 2,		/// ���ݿͻ��� etata_runtime_mode ģʽȷ��
};

inline bool tata_type_is_ios(int t)
{
	return (t & e_tata_type_mobile_ios) ? true : false;
}

inline bool tata_type_is_mps_surpport(int t )
{
	return (t & e_tata_type_mobile_ios || t & e_tata_type_mobile_android ) ? true : false;
}

/// �����ݿ����� 0: ��ʾ����
inline int tata_status_to_db_status(int status){return (status - 4);}

struct header_t
{
	aot_uint8_t		version;
	aot_uint16_t	type;
	aot_uint8_t		encrypt_type;
	aot_uint32_t	data_len;
	aot_uint32_t	exheader_len;
};

struct send_to_device_info_t
{
	send_to_device_info_t()
	{
		how = AOT_STDI_SEND_TO_ACTIVE_DEVICE; param = 0;
	}
	aot_int8_t how;			/// �μ� enum AOT_PKT_EXHD_STDI_HOW
	aot_uint16_t param;			/// �� how == AOT_STDI_SEND_TO_WITH_DEVICE_TYPE ʱ, @param��ʾָ�����豸����
};



struct mini_user_info_t;
struct mini_user_status_t
{
	enum {size = 13 + sizeof(aot_nbrnum_t) * 2};

	aot_uint32_t	id;				/// �û��ڲ�id(tata�����Ӧ���ڲ�id��ʶ)
	aot_nbrnum_t	nbr_num;		/// �û�nbr
	aot_uint32_t	ims_id;
	aot_uint8_t		status;			/// ����״̬
	aot_uint16_t	login_type;		/// ��¼����(tata/�ֻ�/����) 1: PC
	aot_uint8_t		login_mode;		/// ����GXB(0: ��ͨ, 1: ukey��½)
	aot_nbrnum_t	feature_nbr_num;		/// ����(���°�汾: ���ֶ����ڱ�ʶָ��ʵ���˺ŵ�inner_id)
	aot_uint8_t		tata_version;		/// ����

	mini_user_status_t() : id(0), nbr_num(0), ims_id(0), status(e_user_status_offline), login_type(e_tata_type_pc_win), feature_nbr_num(0), tata_version(0)
	{
		;
	}
	~mini_user_status_t()
	{
		;
	}

	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
	void assign(mini_user_info_t* u);
	void assign(mini_user_status_t* mus);
};

struct bin_data_list_t
{
	std::list<aot::inet::aot_bin_data_t> lst;
	static aot_uint32_t get_lst_cdr_size(std::list<aot::inet::aot_bin_data_t>* p)
	{
		aot_uint32_t len = 0;
		std::list<aot::inet::aot_bin_data_t>::iterator it = p->begin();
		for( ; it != p->end(); ++it )
		{
			len += (*it).cdr_size();
		}
		return len;
	}
	static aot_uint32_t cdr_min_size(){ return 4; }
	aot_uint32_t cdr_size(aot_uint32_t data_len){ return 4 + data_len; }
	aot_uint32_t cdr_size(){ return 4 + get_lst_cdr_size(&this->lst); }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};


struct exheader_t
{
	enum{ __fixed_size = 54 + 11 + aot::inet::date_time_t::size};
	enum{ flag_end_pos = 4};
	enum{ fix_size = __fixed_size + aot::inet::aot_string_t::min_size * 7 + aot::inet::aot_encode_data_t::min_size + 4};
	exheader_t()
	{
		flag = 0;
		busi_code = 0;
		seq = 0;
		key = 0;
		from_id = 0;
		from_attr = 0;
		to_id = 0;
		to_attr = 0;
		ex_code = 0;
		err_code = 0;
		unused1 = 0;
		custom_busi_code = 0;
		arry_size = 0;
		arry = NULL;
		to_src_stdi.how = AOT_STDI_SEND_TO_UNKNOWN;

		session_unique_seq = 0;
		from_login_type = e_tata_type_pc_win;
		from_status = e_user_status_online;
	}
	aot_uint32_t  flag;			/// �μ� enum AOT_PKT_EXHD_FLAG
								
	aot_uint32_t busi_code;		/// ҵ�����
	aot_uint32_t seq;			/// ���(δʹ��,Ŀǰ��ԭ������)
	aot_uint32_t key;			/// key(δʹ��,Ŀǰ��ԭ������, ע: ������ҵ��Ϣҵ����ʹ�ø��ֶ�)
	aot_uint32_t from_id;		
	aot_uint32_t  from_attr;
	aot_uint32_t to_id;
	aot_uint32_t  to_attr;
	aot_uint32_t ex_code;		/// ��չ����
	aot_int32_t err_code;		/// ��Ӧ�����Ч
	aot_uint32_t unused1;
	aot_uint32_t custom_busi_code;
	send_to_device_info_t to_src_stdi;		/// ָ����ʱ, ���͸�Դ�����Լ�(tata)��ת������,�� �Ƿ���Ӧ���û�й�ϵ. ����ע��һ��Ҫ�������from_XXX, to_XXX ������
	send_to_device_info_t to_dest_stdi;		/// ָ����ʱ, ���͸�Ŀ�����(tata)��ת������,�� �Ƿ���Ӧ���û�й�ϵ. ����ע��һ��Ҫ�������from_XXX, to_XXX ������
	
	/* ---����size = 54 --- */

	///// �������� 26 byte
	aot_uint64_t session_unique_seq;		/// �ỰΨһ���(�ɷ��������)
	aot_uint16_t from_login_type;
	aot_uint8_t  from_status;
	aot::inet::date_time_t  date_time;

	///* ---����size = __fixed_size --- */

	aot::inet::aot_string_t str_key;	/// Ψһ��ʶ(Ӧ���ԭ������)
	aot::inet::aot_string_t call_id;
	aot::inet::aot_string_t from_guid;	/// ����_NBR_�豸ID
	aot::inet::aot_string_t to_guid;
	aot::inet::aot_string_t	from_nbr;
	aot::inet::aot_string_t	to_nbr;
	aot::inet::aot_string_t str_data;
	aot::inet::aot_encode_data_t encode_data;
	aot_uint32_t arry_size;
	void* arry;
	aot_uint32_t size(){ return __fixed_size + str_key.cdr_write_size() + call_id.cdr_write_size() + from_guid.cdr_write_size() + to_guid.cdr_write_size() + from_nbr.cdr_write_size() + to_nbr.cdr_write_size() + str_data.cdr_write_size() + encode_data.cdr_write_size() + 4 + arry_size;}
	aot_uint32_t real_min_size(){return size() - arry_size;} /// ����������arry,(������arry_size�����4�ֽ�), ע��: ��չͷ��min_size���������ݽṹ�ļ��㷽����һ��, ����static ����
	void copy_to(exheader_t* dest){ /**dest = *this;*/copy(dest, this); }
public:
	static void copy(exheader_t* dest, const exheader_t* src){ memcpy(dest, src, sizeof(exheader_t));}
private:
	exheader_t(const exheader_t&);
	exheader_t& operator = (const exheader_t&);
};

class packet_parser
{
public:
	static inline aot_uint32_t __read_pkt_exhd_flag(aot_buf_t* pkt)
	{
		char* p = aot_buf_rd_ptr(pkt) + PKT_HEADER_LEN;
		return ntohl(*((aot_uint32_t*)p));
	}
	static inline void __write_pkt_exhd_flag(aot_buf_t* pkt, aot_uint32_t add_flag)
	{
		char* p = aot_buf_rd_ptr(pkt) + PKT_HEADER_LEN;
		*((aot_uint32_t*)p) = (aot_uint32_t)htonl(add_flag);
	}
public:
	static void read_header(aot_buf_t* pkt, header_t* out);
	static void write_header(header_t* h, aot_buf_t* out);
	static void write_header(aot_buf_t* pkt, aot_uint32_t data_len, aot_uint32_t extern_header_len, aot_uint32_t encrypt_type = 0, aot_uint32_t pkt_type = e_pkt_type_data)
	{
		ph_build(pkt, pkt_type, encrypt_type, data_len, extern_header_len);
	}
public:
	static bool read_exheader(aot_buf_t* pkt, exheader_t* out);
	static bool write_exheader(aot_buf_t* pkt, exheader_t* exh);
	static bool write_min_exheader(aot_buf_t* pkt, exheader_t* exh);
public:
	static bool is_response(exheader_t* exh){ return 0 != (exh->flag & AOT_PEF_RESPONSE);}
	static bool is_response_ex(aot_buf_t* pkt){ aot_uint32_t v = __read_pkt_exhd_flag(pkt); return 0 != (v & AOT_PEF_RESPONSE);}
	static void set_response_flag(exheader_t* exh){ exh->flag |= AOT_PEF_RESPONSE;}
	static void set_response_flag_ex(aot_buf_t* pkt)
	{
		aot_uint32_t v = __read_pkt_exhd_flag(pkt); 
		v |= AOT_PEF_RESPONSE; 
		__write_pkt_exhd_flag(pkt, v);
	}

	static bool is_save_db_msg(exheader_t* exh){ return 0 != (exh->flag & AOT_PEF_SAVE_TO_DB); }
	static bool is_save_db_msg_ex(aot_buf_t* pkt){ aot_uint32_t v = __read_pkt_exhd_flag(pkt); return 0 != (v & AOT_PEF_SAVE_TO_DB); }
	static void set_save_db_flag(exheader_t* exh){ exh->flag |= AOT_PEF_SAVE_TO_DB;}
	static void set_save_db_flag_ex(aot_buf_t* pkt)
	{
		aot_uint32_t v = __read_pkt_exhd_flag(pkt); 
		v |= AOT_PEF_SAVE_TO_DB; 
		__write_pkt_exhd_flag(pkt, v);
	}
	static void clr_save_db_flag(exheader_t* exh){ exh->flag &= ~AOT_PEF_SAVE_TO_DB;}
	static void clr_save_db_flag_ex(aot_buf_t* pkt)
	{
		aot_uint32_t v = __read_pkt_exhd_flag(pkt); 
		v &= ~AOT_PEF_SAVE_TO_DB; 
		__write_pkt_exhd_flag(pkt, v);
	}

	static bool is_offline_msg(exheader_t* exh){ return 0 != (exh->flag & AOT_PEF_OFFLINE_MSG); }
	static bool is_offline_msg_ex(aot_buf_t* pkt){ aot_uint32_t v = __read_pkt_exhd_flag(pkt); return 0 != (v & AOT_PEF_OFFLINE_MSG);}
	static void set_offline_flag(exheader_t* exh){ exh->flag |= AOT_PEF_OFFLINE_MSG;}
	static void set_offline_flag_ex(aot_buf_t* pkt)
	{
		aot_uint32_t v = __read_pkt_exhd_flag(pkt); 
		v |= AOT_PEF_OFFLINE_MSG; 
		__write_pkt_exhd_flag(pkt, v);
	}
	static void clr_offline_flag(exheader_t* exh){ exh->flag &= ~AOT_PEF_OFFLINE_MSG;}
	static void clr_offline_flag_ex(aot_buf_t* pkt)
	{
		aot_uint32_t v = __read_pkt_exhd_flag(pkt); 
		v &= ~AOT_PEF_OFFLINE_MSG; 
		__write_pkt_exhd_flag(pkt, v);
	}

	static bool is_MQUES_sync_msg(exheader_t* exh){ return 0 != (exh->flag & AOT_PEF_MQUES_SYNC_MSG); }
	static bool is_MQUES_sync_msg_ex(aot_buf_t* pkt){ aot_uint32_t v = __read_pkt_exhd_flag(pkt); return 0 != (v & AOT_PEF_MQUES_SYNC_MSG);}
	static void set_MQUES_sync_flag_ex(aot_buf_t* pkt)
	{
		aot_uint32_t v = __read_pkt_exhd_flag(pkt); 
		v |= AOT_PEF_MQUES_SYNC_MSG; 
		__write_pkt_exhd_flag(pkt, v);
	}

	/// 
	static bool is_need_confirm_receipt_flag(exheader_t* exh){ return 0 != (exh->flag & AOT_PEF_NEED_CONFIRM_RECEIPT); }
	static void set_need_confirm_receipt_flag(exheader_t* exh){ exh->flag |= AOT_PEF_NEED_CONFIRM_RECEIPT;}
	static void clr_need_confirm_receipt_flag(exheader_t* exh){ exh->flag &= ~AOT_PEF_NEED_CONFIRM_RECEIPT;}
	static void clr_need_confirm_receipt_flag_ex(aot_buf_t* pkt)
	{
		aot_uint32_t v = __read_pkt_exhd_flag(pkt); 
		v &= ~AOT_PEF_NEED_CONFIRM_RECEIPT; 
		__write_pkt_exhd_flag(pkt, v);
	}

	static bool is_confirm_receipt_ack_flag(exheader_t* exh){ return 0 != (exh->flag & AOT_PEF_CONFIRM_RECEIPT_ACK); }
	static void set_confirm_receipt_ack_flag(exheader_t* exh){ exh->flag |= AOT_PEF_CONFIRM_RECEIPT_ACK;}
	static void set_confirm_receipt_ack_flag_ex(aot_buf_t* pkt)
	{
		aot_uint32_t v = __read_pkt_exhd_flag(pkt); 
		v |= AOT_PEF_CONFIRM_RECEIPT_ACK; 
		__write_pkt_exhd_flag(pkt, v);
	}
	static void clr_confirm_receipt_ack_flag(exheader_t* exh){ exh->flag &= ~AOT_PEF_CONFIRM_RECEIPT_ACK;}

	/// s2s
	static bool is_S2S_need_confirm_receipt_flag(exheader_t* exh){ return 0 != (exh->flag & AOT_PEF_S2S_NEED_CONFIRM_RECEIPT); }
	static void set_S2S_need_confirm_receipt_flag(exheader_t* exh){ exh->flag |= AOT_PEF_S2S_NEED_CONFIRM_RECEIPT;}
	static void set_S2S_need_confirm_receipt_flag_ex(aot_buf_t* pkt)
	{
		aot_uint32_t v = __read_pkt_exhd_flag(pkt); 
		v |= AOT_PEF_S2S_NEED_CONFIRM_RECEIPT; 
		__write_pkt_exhd_flag(pkt, v);
	}
	static void clr_S2S_need_confirm_receipt_flag(exheader_t* exh){ exh->flag &= ~AOT_PEF_S2S_NEED_CONFIRM_RECEIPT;}
	static void clr_S2S_need_confirm_receipt_flag_ex(aot_buf_t* pkt)
	{
		aot_uint32_t v = __read_pkt_exhd_flag(pkt); 
		v &= ~AOT_PEF_S2S_NEED_CONFIRM_RECEIPT; 
		__write_pkt_exhd_flag(pkt, v);
	}

	static bool is_S2S_confirm_receipt_ack_flag(exheader_t* exh){ return 0 != (exh->flag & AOT_PEF_S2S_CONFIRM_RECEIPT_ACK); }
	static void set_S2S_confirm_receipt_ack_flag(exheader_t* exh){ exh->flag |= AOT_PEF_S2S_CONFIRM_RECEIPT_ACK;}
	static void clr_S2S_confirm_receipt_ack_flag(exheader_t* exh){ exh->flag &= ~AOT_PEF_S2S_CONFIRM_RECEIPT_ACK;}
	static void clr_S2S_confirm_receipt_ack_flag_ex(aot_buf_t* pkt)
	{
		aot_uint32_t v = __read_pkt_exhd_flag(pkt); 
		v &= ~AOT_PEF_S2S_CONFIRM_RECEIPT_ACK; 
		__write_pkt_exhd_flag(pkt, v);
	}

	/// 
	static bool is_active_cnn_flag(exheader_t* exh){ return 0 != (exh->flag & AOT_PEF_NEED_SET_ACTIVE_CNN); }
	static void set_active_cnn_flag(exheader_t* exh){ exh->flag |= AOT_PEF_NEED_SET_ACTIVE_CNN;}
	static void clr_active_cnn_flag(exheader_t* exh){ exh->flag &= ~AOT_PEF_NEED_SET_ACTIVE_CNN;}

	static void set_my_device_cc_msg_flag_ex(aot_buf_t* pkt)
	{
		aot_uint32_t v = __read_pkt_exhd_flag(pkt); 
		v |= AOT_PEF_MY_DEVICE_CC_MSG; 
		__write_pkt_exhd_flag(pkt, v);
	}
public:
	static void change_busicode(aot_buf_t* recv_pkt, aot_uint32_t busi_code)
	{
		char* p = aot_buf_rd_ptr(recv_pkt) + PKT_HEADER_LEN + 4;
		*((aot_uint32_t*)p) = (aot_uint32_t)htonl(busi_code);
	}
	static void change_ex_code(aot_buf_t* recv_pkt, aot_uint32_t ex_code)
	{
		char* p = aot_buf_rd_ptr(recv_pkt) + PKT_HEADER_LEN + 32;
		*((aot_uint32_t*)p) = (aot_uint32_t)htonl(ex_code);
	}
	static void change_from_info(aot_buf_t* recv_pkt, aot_uint32_t from_id, aot_uint32_t from_attr)
	{
		char* p = aot_buf_rd_ptr(recv_pkt) + PKT_HEADER_LEN + 16;
		*((aot_uint32_t*)p) = (aot_uint32_t)htonl(from_id);
		*((aot_uint32_t*)(p+4)) = (aot_uint32_t)htonl(from_attr);
	}
	static void change_to_info(aot_buf_t* recv_pkt, aot_uint32_t to_id, aot_uint32_t to_attr)
	{
		char* p = aot_buf_rd_ptr(recv_pkt) + PKT_HEADER_LEN + 24;
		*((aot_uint32_t*)p) = (aot_uint32_t)htonl(to_id);
		*((aot_uint32_t*)(p+4)) = (aot_uint32_t)htonl(to_attr);
	}

	static aot_uint32_t read_exheader_busi_code(aot_buf_t* pkt)
	{
		if( aot_buf_length(pkt) < PKT_HEADER_LEN + 8 )
		{
			return 0;
		}
		char* p = aot_buf_rd_ptr(pkt) + PKT_HEADER_LEN + 4;
		return ntohl(*((aot_uint32_t*)p));
	}
	static aot_uint32_t read_exheader_ex_code(aot_buf_t* pkt)
	{
		if( aot_buf_length(pkt) < PKT_HEADER_LEN + 36 )
		{
			return 0;
		}
		char* p = aot_buf_rd_ptr(pkt) + PKT_HEADER_LEN + 32;
		return ntohl(*((aot_uint32_t*)p));
	}
	static aot_uint32_t read_exheader_from_id(aot_buf_t* pkt)
	{
		if( aot_buf_length(pkt) < PKT_HEADER_LEN + 20 )
		{
			return 0;
		}
		char* p = aot_buf_rd_ptr(pkt) + PKT_HEADER_LEN + 16;
		return ntohl(*((aot_uint32_t*)p));
	}
	static aot_uint32_t read_exheader_from_attr(aot_buf_t* pkt)
	{
		if( aot_buf_length(pkt) < PKT_HEADER_LEN + 24 )
		{
			return 0;
		}
		char* p = aot_buf_rd_ptr(pkt) + PKT_HEADER_LEN + 20;
		return ntohl(*((aot_uint32_t*)p));
	}

	static aot_uint32_t read_exheader_to_id(aot_buf_t* pkt)
	{
		char* p = aot_buf_rd_ptr(pkt) + PKT_HEADER_LEN + 24;
		return ntohl(*((aot_uint32_t*)p));
	}

	/*static aot_uint32_t read_seq(aot_buf_t* pkt)
	{
		char* p = aot_buf_rd_ptr(pkt) + PKT_HEADER_LEN + 8;
		return ntohl(*((aot_uint32_t*)p));
	}
	static void write_seq(aot_buf_t* pkt, aot_uint32_t seq)
	{
		char* p = aot_buf_rd_ptr(pkt) + PKT_HEADER_LEN + 8;
		*((aot_uint32_t*)p) = (aot_uint32_t)htonl(seq);
	}*/

	static void read_date_time(aot_buf_t* pkt, aot::inet::date_time_t* dt);
	static void write_date_time(aot_buf_t* pkt, aot::inet::date_time_t* dt);

	static aot_uint64_t read_session_unique_seq(aot_buf_t* pkt)
	{
		char* p = aot_buf_rd_ptr(pkt) + PKT_HEADER_LEN + 54;
		return (aot_uint64_t)aot::inet::__aot_ntohll(*((aot_uint64_t*)p));
	}
	static void write_session_unique_seq(aot_buf_t* pkt, aot_uint64_t n)
	{
		char* p = aot_buf_rd_ptr(pkt) + PKT_HEADER_LEN + 54;
		*((aot_uint64_t*)p) = (aot_uint64_t)aot::inet::__aot_htonll(n);
	}

	static bool read_strkey(aot_buf_t* pkt, aot_uint32_t pkt_len, aot::inet::aot_string_t* key);
};

struct serv_node_info_t
{
	enum {size = 16};
	/// �ڵ�ID
	aot_uint32_t	id;
	aot_inet_addr_t addr;
	aot_inet_addr_t listen_addr;
public:
	serv_node_info_t();
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

struct serv_node_list_t
{
	typedef std::list<serv_node_info_t> list_type;
	list_type lst;
	static aot_uint32_t cdr_min_size(){return 4;}
	aot_uint32_t cdr_size(){ return 4 + ((aot_uint32_t)this->lst.size()) * serv_node_info_t::size;}
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

template<class ALLOC>
mini_user_status_t* alloc_mini_user_status(ALLOC* ac)
{
	mini_user_status_t* mus = (mini_user_status_t*)ac->alloc(sizeof(mini_user_status_t));
	if( NULL == mus ) {
		return NULL;
	}
	new (mus) mini_user_status_t();
	return mus;
}

template<class ALLOC>
void dealloc_mini_user_status(ALLOC* ac, mini_user_status_t* mus)
{
	if( mus && ac ) {
		mus->~mini_user_status_t();
		ac->dealloc(mus);
	}
}

template<class ALLOC>
struct mini_user_status_list_t
{
	typedef aot_std::list<mini_user_status_t*, ALLOC> list_type;
	list_type lst;
public:
	ALLOC* alloc_;
	bool need_destroy_; /// �ñ�־����ָ������Ԫ��ʱ, �Ƿ���Ҫ�Ƿ��ͷ�mini_user_info_t
public:
	mini_user_status_list_t(ALLOC* alloc) : lst(alloc)
	{
		this->alloc_ = alloc;
		this->need_destroy_ = true;
	}
	~mini_user_status_list_t() { clear(); }
	void clear();
	static aot_uint32_t cdr_min_size(){ return 4; }
	aot_uint32_t cdr_size(){return 4 + this->lst.size() * mini_user_status_t::size;}
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

struct mini_user_info_t
{
	enum {fixed_size = 27 + aot::inet::date_time_t::size };
	aot_uint32_t	id;				/// �û��ڲ�id(tata�����Ӧ���ڲ�id��ʶ)
	aot_uint8_t		status;			/// ����״̬
	aot_uint16_t	login_type;		/// ��¼����(tata/�ֻ�/����) 1: PC
	aot_uint8_t		login_mode;
	aot_uint32_t	ims_id;			/// �û����ڵ�ims
	aot_uint32_t	info_version;	/// ����״̬��Ϣ�汾��, ��Ϊ�ֲ�ʽ��ϵͳ�������ݰ����Ⱥ�˳����û�б�֤��
									/// �󷢵İ����ܻ��ȱ�����,�����Ҫͨ���汾�Ž��б�ʶ
	aot_uint8_t		tata_version;	/// tata�ڲ��汾��
	aot_uint32_t	unused;			/// Ԥ��
	aot_inet_addr_t	addr;			/// tata��ip
	aot::inet::date_time_t     last_offline_tm;
	
	/// 27 + aot::inet::date_time_t::size
	prot_string		nbr;
	prot_string		guid;
	prot_string		str_data;
	prot_string		xml_data;
	prot_string		feature_nbr;
	/// 
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
public:
	static aot_uint32_t cdr_min_size(){ return (fixed_size + e_std_string_cdr_min_size*5 ); }
	aot_uint32_t cdr_size(){ return ( fixed_size + string_cdr_size(&nbr) + string_cdr_size(&guid) + string_cdr_size(&str_data) + string_cdr_size(&xml_data) + string_cdr_size(&feature_nbr) ); }

	mini_user_info_t()
	{
		init();
	}
	bool update(mini_user_info_t* new_mu);
	void init();
	void copy_to(mini_user_info_t* dest)
	{
		*dest = *this;
	}
};

template<class ALLOC>
mini_user_info_t* alloc_mini_user_info(ALLOC* ac)
{
	mini_user_info_t* mu = (mini_user_info_t*)ac->alloc(sizeof(mini_user_info_t));
	if( NULL == mu ) {
		return NULL;
	}

	new (mu) mini_user_info_t();
	return mu;
}

template<class ALLOC>
void dealloc_mini_user_info(ALLOC* ac, mini_user_info_t* mu)
{
	if( mu && ac ) {
		mu->~mini_user_info_t();
		ac->dealloc(mu);
	}
}

template<class ALLOC>
struct mini_user_info_list_t
{
	enum{ __MU_LIST_S__ = 'S', __MU_LIST_E__ = 'E'};
	/// CDR order: elem_count(aot_uint32_t) + 'S' + mini_user_info_t + 'E' + 'S' + mini_user_info_t + 'E'...
	typedef aot_std::list<mini_user_info_t*, ALLOC> list_type;
	list_type lst;					/// ������tata�����ĺ����б�
public:
	ALLOC* alloc_;
	bool need_destroy_; /// �ñ�־����ָ������Ԫ��ʱ, �Ƿ���Ҫ�Ƿ��ͷ�mini_user_info_t
public:
	mini_user_info_list_t(ALLOC* alloc) : lst(alloc)
	{
		this->alloc_ = alloc;
		this->need_destroy_ = true;
	}
	~mini_user_info_list_t()
	{
		clear();
	}
	void clear();
	static aot_uint32_t cdr_min_size(){ return 4; }
	aot_uint32_t cdr_size();
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);

	aot_uint32_t cdr_write(char* buf, aot_uint32_t len, aot_uint32_t cdr_len = 0 );
};

template<class ALLOC>
struct get_my_device_list_ret_t
{
	typedef mini_user_info_list_t<ALLOC> mini_user_info_list_type;
	mini_user_info_list_type lst;
public:
	get_my_device_list_ret_t(ALLOC* ac): lst(ac)
	{
	}

	static aot_uint32_t cdr_min_size(){ return mini_user_info_list_type::cdr_min_size(); }
	aot_uint32_t cdr_size(){ return lst.cdr_size(); }

	aot_uint32_t cdr_read(char* buf, aot_uint32_t len){return this->lst.cdr_read(buf, len);}
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len){return this->lst.cdr_write(buf, len);}
};

struct tata_forceout_t
{
	aot_uint32_t flag;
	struct mini_user_info_t mini_user_info;
	aot::inet::aot_string_t str_data;
public:
	static aot_uint32_t cdr_min_size(){ return (4 + mini_user_info_t::cdr_min_size() + aot::inet::aot_string_t::min_size); }
	aot_uint32_t cdr_size(){ return (4 + mini_user_info.cdr_size() + this->str_data.cdr_write_size()); }
	tata_forceout_t() { this->flag = 0; }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

struct tata_regist_t
{
	aot_uint32_t flag;
	aot::inet::aot_string_t token;
	aot::inet::aot_string_t str_data;
	/// ƽ̨����. �ͻ����ڵ�½ʱ����֤������. �������²���ʱ, ָ���汾�ͻ���ֻ�����½ָ�������
	aot::inet::aot_string_t str_platform_name;
	struct mini_user_info_t mu;
public:
	static aot_uint32_t cdr_min_size(){ return 4 + aot::inet::aot_string_t::min_size * 3 + mini_user_info_t::cdr_min_size(); }
	aot_uint32_t cdr_size(){ return 4 + this->token.cdr_write_size() +  this->str_data.cdr_write_size() + this->str_platform_name.cdr_write_size() + this->mu.cdr_size(); }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

struct tata_regist_ret_t
{
	aot_uint32_t flag;
	aot_uint32_t dbs_id;
	struct mini_user_info_t mini_user_info;
	aot::inet::aot_string_t token;
	aot::inet::aot_string_t redirect_ims_addr;
	aot::inet::aot_string_t str_data;
public:
	static aot_uint32_t cdr_min_size(){ return (8 + mini_user_info_t::cdr_min_size() + aot::inet::aot_string_t::min_size * 3); }
	aot_uint32_t cdr_size(){ return (8 + mini_user_info.cdr_size() + this->token.cdr_write_size() + this->redirect_ims_addr.cdr_write_size() + this->str_data.cdr_write_size()); }
	tata_regist_ret_t(){init();}
	void init(){this->flag = 0; this->dbs_id = 0; this->mini_user_info.init();}
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

/// ȡָ���û���״̬��Ϣ
struct get_the_user_status_t
{
	enum{size = 4};
	aot_uint32_t from_ims_id;	/// ���������ڵ�ims_id, �缯ȺӦ��ʱʹ��
	aot_uint32_t tata_id;	/// ��ȡ���û���״̬��Ϣ
public:
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

/// ȡ���ߺ����б���Ϣ
struct get_online_buddy_list_t
{
	aot_uint32_t tata_id;
	aot_uint32_t ims_id;
	aot_uint32_t unused;
	aot::inet::aot_string_t str_data;
public:
	get_online_buddy_list_t()
	{ 
		this->tata_id = 0; this->ims_id = 0; this->unused = 0; 
	}
	static aot_uint32_t cdr_min_size(){ return 12 + aot::inet::aot_string_t::min_size; }
	aot_uint32_t cdr_size(){ return 12 + this->str_data.cdr_write_size(); }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

template<class ALLOC>
struct get_online_buddy_list_ret_t
{
	aot_uint32_t tata_id;
	aot_uint32_t ims_id;
	aot_uint32_t unused;
	typedef mini_user_status_list_t<ALLOC> list_type;
	list_type lst;
public:
	get_online_buddy_list_ret_t(ALLOC* alloc) : lst(alloc)
	{
		this->tata_id = 0; this->ims_id = 0; this->unused = 0; 
	}
	static aot_uint32_t cdr_min_size(){ return 12 + list_type::cdr_min_size(); }
	aot_uint32_t cdr_size(){ return 12 + lst.cdr_size(); }
	aot_uint32_t cdr_size(aot_std::vector<mini_user_info_t*, ALLOC, false>* mui_lst){ return 12 + 4 + mui_lst->size() * mini_user_status_t::size; }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len, aot_std::vector<mini_user_info_t*, ALLOC, false>* mui_lst);
};

struct ims2dbs_regist_t
{
	aot_uint16_t listen_port;
	aot::inet::aot_string_t addr;
	aot::inet::aot_string_t str_data;
public:
	static aot_uint32_t cdr_min_size(){ return 2 + aot::inet::aot_string_t::min_size * 2; }
	aot_uint32_t cdr_size(){ return 2 + this->addr.cdr_write_size() + this->str_data.cdr_write_size(); }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

struct ims2dbs_regist_ret_t
{
	aot_uint32_t ims_id;			/// �����ims��id
	aot_uint32_t dbs_id;			/// dbs��id
	aot::inet::aot_string_t str_data;
	serv_node_list_t serv_lst;
public:
	static aot_uint32_t cdr_min_size(){ return 8 + aot::inet::aot_string_t::min_size + serv_node_list_t::cdr_min_size(); }
	aot_uint32_t cdr_size(){ return 8 + this->str_data.cdr_write_size() + this->serv_lst.cdr_size(); }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

struct ims2dbs_report_addrinfo_t
{
	aot::inet::aot_string_t addr;	/// ip : port
public:
	static aot_uint32_t cdr_min_size(){ return 4; }
	aot_uint32_t cdr_size(){ return this->addr.cdr_write_size(); }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

enum eload_balance_opt_type
{
	e_load_balance_opt_update = 0,
	e_load_balance_opt_add,
	e_load_balance_opt_remove,
};

struct dbs2nms_report_load_balance_t
{
	aot_uint32_t opt;
	aot::inet::aot_string_t info;
public:
	static aot_uint32_t cdr_min_size(){ return 4 + 4; }
	aot_uint32_t cdr_size(){ return 4 + this->info.cdr_write_size(); }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

struct nms2lgs_report_load_balance_t
{
	aot_uint32_t opt;
	aot::inet::aot_string_t info;
public:
	static aot_uint32_t cdr_min_size(){ return 4 + aot::inet::aot_string_t::min_size; }
	aot_uint32_t cdr_size(){ return 4 + this->info.cdr_write_size(); }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

struct d2d_direct_transit_t
{
	aot_uint32_t inner_id;
	aot_uint32_t unused0;
	/// ��չ����, Ԥ����������������ҵ��ʹ��
	aot_uint32_t arry_size;
	void* arry;
public:
	static aot_uint32_t cdr_min_size(){ return 8 + 4; }
	aot_uint32_t cdr_size(){ return cdr_min_size() + arry_size; }
	d2d_direct_transit_t()
	{
		inner_id = 0; unused0 = 0; arry_size = 0; arry = NULL;
	}
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

struct tata2tata_direct_transit_t
{
	aot_uint32_t from_ims_id;
	aot_uint32_t to_ims_id;
	aot_uint32_t flag;
	aot_uint32_t unused0;	/// �û��Զ�����Ϣ
	/// ��չ����, Ԥ����������������ҵ��ʹ��
	aot_uint32_t arry_size;
	void* arry;
public:
	static aot_uint32_t cdr_min_size(){ return 16 + 4; }
	aot_uint32_t cdr_size(){ return cdr_min_size() + arry_size; }
	tata2tata_direct_transit_t()
	{
		from_ims_id = 0; to_ims_id = 0; flag = 0;
		unused0 = 0; arry_size = 0; arry = NULL;
	}
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

template<class ALLOC>
struct broadcast_status_t
{
	mini_user_status_t mus;				/// ״̬�ı��˵��Ǹ�tata
	aot::inet::aot_string_t guid;		/// ״̬�ı��˵��Ǹ�tata��guid
	typedef aot_std::list<aot_uint32_t, ALLOC> list_type;
	list_type to_lst;					/// ������tata�����ĺ����б�
public:
	broadcast_status_t(ALLOC* alloc) : to_lst(alloc)
	{}
	static aot_uint32_t cdr_min_size() { return mini_user_status_t::size + aot::inet::aot_string_t::min_size + 4; }
	aot_uint32_t cdr_size() { return mini_user_status_t::size + this->guid.cdr_write_size() + 4 + 4 * to_lst.size(); }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

/// 
struct cluster_info_t
{
	enum {size = 4};
	aot_uint32_t cluster_id;
public:
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

/// ss׼������
struct notify_ss_ready_t
{
	enum {size = 6};
	aot_inet_addr_t ss_addr;
public:
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

/// ������ݽṹ��������,��Ԫ�غ���,ֱ����C++��׼�������
struct ss_info_t
{
public:
	typedef std::vector<cluster_info_t> list_type;
	list_type cluster_info;
public:
	aot_uint32_t cdr_size(){ return 4 + cluster_info_t::size * (aot_uint32_t)cluster_info.size();}
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

struct group_pair_t
{
	/// ע��group_pair_t �ıȽ�ֻ�Ǹ��� �ֶ�group_id������, �����ļ�ĩβ��ȫ�ֱȽϺ���
	enum{size = 8};
	aot_uint32_t group_id;
	aot_uint32_t serv_id;
};

/// �û����μӵ�Ⱥ����Ϣ
template<class ALLOC>
struct user_tribe_info_t
{
	typedef aot_std::vector<group_pair_t, ALLOC, false> group_list_type;
	group_list_type tribe_lst;			/// Ⱥ���б�
public:
	user_tribe_info_t(ALLOC* alloc) : tribe_lst(alloc)
	{ }
public:
	static aot_uint32_t cdr_min_size(){ return 4; }
	aot_uint32_t cdr_size(){ return cdr_min_size() + group_pair_t::size * tribe_lst.size();}
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

/// �û�������ҵ��Ϣ
template<class ALLOC>
struct user_ent_info_t
{
	aot_uint32_t ent_id;				/// ��ҵid
	aot_uint32_t ent_serv_id;			/// ��ҵ������id
	aot_uint32_t ent_total_member_size;	/// ��ҵ������, ������ֽ��������ο�,����׼ȷ������,Ҳ��ά�������ֵ�׼ȷ��
	typedef aot_std::vector<group_pair_t, ALLOC, false> group_list_type;
	group_list_type ent_dep_lst;		/// ��ҵ�����б�(�û������ڶ������)
public:
	user_ent_info_t(ALLOC* alloc) : ent_dep_lst(alloc)
	{
		ent_id = ent_total_member_size = 0;
	}
public:
	static aot_uint32_t cdr_min_size(){ return 12 + 4; }
	aot_uint32_t cdr_size(){ return cdr_min_size() + group_pair_t::size * ent_dep_lst.size();}
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

/// ��ҵ��Ϣ����ģʽ: ��ָ������(����������ҵ)�Ĺ㲥��Ϣ(��ͨ����ҵ��������������ڴ�ģʽ)
struct ent_broadcast_msg_t
{
	aot_uint32_t ent_id;
	aot_uint32_t dep_id;
	aot_uint32_t from_tata_id;
	aot_uint32_t from_ims_id;
	/// �����˵�tata_id, ��Ϊ��ҵ���������������ŵ�����ĳ�����ŷ�����Ϣ,
	/// ����˲��ڸò�����,��ʱ,���˾��ǳ��Ͷ���
	aot_uint32_t cc_tata_id;
	/// ���������ڵ�ims_id
	aot_uint32_t cc_ims_id;
	aot_uint32_t unused0;			/// �û��Զ�����Ϣ
	aot_uint32_t unused;
	/// ��չ����, Ԥ����������������ҵ��ʹ��
	aot_uint32_t arry_size;
	void* arry;
	ent_broadcast_msg_t()
	{
		ent_id = dep_id =0;
		from_tata_id = from_ims_id = cc_tata_id = cc_ims_id = 0;
		unused0 = 0; unused = 0;
		arry_size = 0;
		arry = NULL;
	}
public:
	static aot_uint32_t cdr_min_size(){return 32 + 4;}
	aot_uint32_t cdr_size(){return cdr_min_size() + this->arry_size;}
	void change_cc_ims_id(char* buf, aot_uint32_t len, aot_uint32_t new_cc_ims_id);
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

struct get_ent_online_member_list_t
{
	enum{size = 20};
	aot_uint32_t ent_id;
	aot_uint32_t dep_id;
	aot_uint32_t from_tata_id;
	aot_uint32_t from_ims_id;
	aot_uint32_t flag;
public:
	get_ent_online_member_list_t()
	{
		this->ent_id = 0;this->dep_id = 0;this->from_tata_id = 0;this->from_ims_id = 0;this->flag = 0;
	}
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

template<class ALLOC>
struct get_ent_online_member_list_ret_t
{
	aot_uint32_t ent_id;
	aot_uint32_t dep_id;
	aot_uint32_t from_tata_id;
	aot_uint32_t from_ims_id;
	aot_uint32_t flag;

	typedef mini_user_status_list_t<ALLOC> list_type;
	list_type member_lst;
public:
	get_ent_online_member_list_ret_t(ALLOC* alloc) : member_lst(alloc)
	{
		this->ent_id = 0;this->dep_id = 0;this->from_tata_id = 0;this->from_ims_id = 0;this->flag = 0;
	}
	static aot_uint32_t cdr_min_size(){ return 20 + list_type::cdr_min_size(); }
	aot_uint32_t cdr_size(){ return 20 + member_lst.cdr_size(); }
	aot_uint32_t cdr_size(aot_std::vector<mini_user_info_t*, ALLOC, false>* mui_lst){ return 20 + 4 + mui_lst->size() * mini_user_status_t::size; }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len, aot_std::vector<mini_user_info_t*, ALLOC, false>* mui_lst);
};

/// Ⱥ����Ϣ����ģʽ: ��ָ��Ⱥ��Ĺ㲥��Ϣ(��ͨ��Ⱥ������������ڴ�ģʽ)
struct tribe_broadcast_msg_t
{
	aot_uint32_t tribe_id;
	aot_uint32_t from_tata_id;
	aot_uint32_t from_ims_id;
	/// ��ȻȺ��û������: ����Ⱥ�����Ҳ�������Ⱥ�鷢����Ϣ, �����ﻹ��ʵ���������
	aot_uint32_t cc_tata_id;
	/// ���������ڵ�ims_id
	aot_uint32_t cc_ims_id;
	aot_uint32_t unused0;			/// �û��Զ�����Ϣ
	aot_uint32_t unused;
	/// ��չ����, Ԥ����������������ҵ��ʹ��
	aot_uint32_t arry_size;
	void* arry;
	tribe_broadcast_msg_t()
	{
		tribe_id =0;
		from_tata_id = from_ims_id = cc_tata_id = cc_ims_id = 0;
		unused0 = 0; unused = 0;
		arry_size = 0; arry = NULL;
	}
public:
	static aot_uint32_t cdr_min_size(){return 28 + 4;}
	aot_uint32_t cdr_size(){return cdr_min_size() + this->arry_size;}
	void change_cc_ims_id(char* buf, aot_uint32_t len, aot_uint32_t new_cc_ims_id);
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

struct get_tribe_online_member_list_t
{
	enum{size = 12};
	aot_uint32_t tribe_id;
	aot_uint32_t from_tata_id;
	aot_uint32_t from_ims_id;
public:
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

template<class ALLOC>
struct get_tribe_online_member_list_ret_t
{
	aot_uint32_t tribe_id;
	aot_uint32_t from_tata_id;
	aot_uint32_t from_ims_id;

	typedef mini_user_status_list_t<ALLOC> list_type;
	list_type member_lst;
public:
	get_tribe_online_member_list_ret_t(ALLOC* alloc) : member_lst(alloc)
	{ }
	static aot_uint32_t cdr_min_size(){ return 12 + list_type::cdr_min_size(); }
	aot_uint32_t cdr_size(){ return 12 + member_lst.cdr_size(); }
	aot_uint32_t cdr_size(aot_std::vector<mini_user_info_t*, ALLOC, false>* mui_lst){ return 12 + 4 + mui_lst->size() * mini_user_status_t::size; }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len, aot_std::vector<mini_user_info_t*, ALLOC, false>* mui_lst);
};

/// ������ҵ,Ⱥ����Ϣ
template<class ALLOC>
struct subscribe_msg_t
{
	aot_uint32_t ims_id;
	typedef aot_std::vector<aot_uint32_t, ALLOC, false> list_type;
	list_type lst;
	subscribe_msg_t(ALLOC* alloc): lst(alloc)
	{ }
	static aot_uint32_t cdr_min_size(){ return 8; }
	aot_uint32_t cdr_size(){ return cdr_min_size() + 4 * this->lst.size(); }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

/// �û��ĺ����б�
template<class ALLOC>
struct buddy_list_t
{
	typedef aot_std::vector<aot_uint32_t, ALLOC, false> list_type;
	list_type lst;
	buddy_list_t(ALLOC* alloc): lst(alloc)
	{ }
	static aot_uint32_t cdr_min_size(){ return 4; }
	aot_uint32_t cdr_size(){ return 4 + 4 * this->lst.size(); }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

struct exchange_buddy_info_t
{
	exchange_buddy_info_t(){is_reply = 0;}
	aot_uint8_t is_reply;
	mini_user_info_t mu;
	static aot_uint32_t cdr_min_size(){ return 1 + mini_user_info_t::cdr_min_size(); }
	aot_uint32_t cdr_size(){ return 1 + mu.cdr_size(); }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

struct t2t_user_define_data_t
{
	t2t_user_define_data_t() { data_type = 0; unused = 0; }
	aot_uint32_t data_type;
	aot_uint32_t unused;
	static aot_uint32_t cdr_min_size(){ return 8; }
	aot_uint32_t cdr_size(){ return 8; }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

struct tbm_user_define_data_t
{
	aot_uint32_t data_type;
	aot_uint32_t unused;
	tbm_user_define_data_t() { data_type = 0; unused = 0; }
	static aot_uint32_t cdr_min_size(){ return 8; }
	aot_uint32_t cdr_size(){ return 8; }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

struct sys_tbm_msg_t
{
	aot_uint32_t serv_id;
	static aot_uint32_t cdr_min_size(){ return 4; }
	aot_uint32_t cdr_size(){ return 4; }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

struct sys_ebm_msg_t
{
	aot_uint32_t serv_id;
	static aot_uint32_t cdr_min_size(){ return 4; }
	aot_uint32_t cdr_size(){ return 4; }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

template<class ALLOC>
struct multicast_msg_t
{
	aot_uint32_t from_tata_id;
	aot_uint32_t from_tata_ims;
	typedef aot_std::vector<aot_uint32_t, ALLOC, false> list_type;
	list_type to_lst;					/// ������tata�����ĺ����б�
public:
	multicast_msg_t(ALLOC* alloc) : to_lst(alloc)
	{ }
	static aot_uint32_t cdr_min_size(){ return 8 + 4; }
	aot_uint32_t cdr_size() { return cdr_min_size() + 4*to_lst.size(); }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

struct broadcast_msg_t
{
	aot_uint32_t from_tata_id;
	aot_uint32_t from_ims_id;
	/// ��չ����, Ԥ����������������ҵ��ʹ��
	aot_uint32_t arry_size;
	void* arry;
public:
	static aot_uint32_t cdr_min_size(){ return 12; }
	aot_uint32_t cdr_size(){ return cdr_min_size() + arry_size; }
	broadcast_msg_t()
	{
		from_ims_id = 0; from_tata_id = 0; arry_size = 0; arry = NULL;
	}
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

struct nmc_regist_t
{
	aot_uint32_t id;
	aot_uint32_t svc_type;
	aot::inet::aot_string_t user;
	aot::inet::aot_string_t pwd;
	static aot_uint32_t cdr_min_size(){ return 16; }
	aot_uint32_t cdr_size(){ return 8 + this->user.cdr_write_size() + this->pwd.cdr_write_size(); }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

struct lgc_regist_t
{
	aot_uint32_t id;
	aot_uint32_t svc_type;
	aot::inet::aot_string_t user;
	aot::inet::aot_string_t pwd;
	static aot_uint32_t cdr_min_size(){ return 16; }
	aot_uint32_t cdr_size(){ return 8 + this->user.cdr_write_size() + this->pwd.cdr_write_size(); }

	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

/* - * - * - * - * - * - * - * - * - * - * - begin logins * - * - * - * - * - * - * - * - * - * - * - * - * - */

struct lgs_get_login_config_info_t
{
	aot::inet::aot_string_t call_id;
	aot::inet::aot_string_t sid;
	aot::inet::aot_string_t version;
	static aot_uint32_t cdr_min_size(){ return 12; }
	aot_uint32_t cdr_size(){ return this->call_id.cdr_write_size() + this->sid.cdr_write_size() + this->version.cdr_write_size(); }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

/* - * - * - * - * - * - * - * - * - * - * - end   logins * - * - * - * - * - * - * - * - * - * - * - * - * - */

struct feature_tata_login_regist_t
{
	aot_uint32_t tata_id;
	aot::inet::aot_string_t tata_nbr;
	aot::inet::aot_string_t token;
	static aot_uint32_t cdr_min_size(){ return 12; }
	aot_uint32_t cdr_size(){ return 4 + this->tata_nbr.cdr_write_size() + this->token.cdr_write_size(); }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};


/// tata ��ims����ʱ, ims ͨ�� feature server �����ݿⷴע��
struct feature_tata_regist_out_t
{
	aot_uint32_t tata_id;
	aot_uint32_t ims_id;
	aot::inet::aot_string_t tata_nbr;
	aot::inet::aot_string_t token;
	static aot_uint32_t cdr_min_size(){ return 16; }
	aot_uint32_t cdr_size(){ return 8 + this->tata_nbr.cdr_write_size() + this->token.cdr_write_size(); }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

/// ����˶Կͻ���"ǩ����Ϣ"��ȷ��(ע��: �����ǩ����Ϣ ��ָ: �ͻ��˷���������Ϣ, �������Ҫȷ���Ѿ��յ�)
struct confirm_receipt_ack_t
{
	aot_uint32_t old_ex_code;
	aot_uint32_t old_busi_code;
	aot_uint32_t ack_ret;
	aot_uint32_t arry_size;
	void* arry;
public:
	static aot_uint32_t cdr_min_size(){ return 16; }
	confirm_receipt_ack_t()
	{
		old_ex_code = 0; old_busi_code = 0;
		ack_ret = 0; arry_size = 0; arry = NULL;
	}
	aot_uint32_t cdr_size(){ return cdr_min_size() + this->arry_size; }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

/// �û��ĺ����б�
template<class ALLOC>
struct get_userlist_status_t
{
	aot_uint32_t tata_id;
	aot_uint32_t ims_id;
	aot_uint8_t	 is_buddy;	/// 
	aot_uint32_t unused;
	typedef aot_std::vector<aot_uint32_t, ALLOC, false> list_type;
	list_type lst;
	get_userlist_status_t(ALLOC* alloc): lst(alloc)
	{
		tata_id = 0; ims_id = 0; is_buddy = 0; unused = 0;
	}
	static aot_uint32_t cdr_min_size(){ return 13 + 4; }
	aot_uint32_t cdr_size(){ return 13 + 4 + 4 * this->lst.size(); }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

template<class ALLOC>
struct get_userlist_status_ret_t
{
	aot_uint32_t tata_id;
	aot_uint32_t ims_id;
	aot_uint8_t	 is_buddy;	/// 
	aot_uint32_t unused;
	typedef mini_user_status_list_t<ALLOC> list_type;
	list_type lst;
public:
	get_userlist_status_ret_t(ALLOC* alloc) : lst(alloc)
	{
		this->tata_id = 0; this->ims_id = 0; this->unused = 0; 
	}
	static aot_uint32_t cdr_min_size(){ return 13 + list_type::cdr_min_size(); }
	aot_uint32_t cdr_size(){ return 13 + lst.cdr_size(); }
	aot_uint32_t cdr_size(aot_std::vector<mini_user_info_t*, ALLOC, false>* mui_lst){ return 13 + 4 + mui_lst->size() * mini_user_status_t::size; }
	static aot_uint8_t cdr_read_is_buddy(char* buf) { return *(buf + 8); }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len, aot_std::vector<mini_user_info_t*, ALLOC, false>* mui_lst);
};

/// msg push service

/// ims ---> mps ������Ϣ����
template<class ALLOC>
struct ims2mps_subscribe_t
{
	mini_user_info_t mui;
	aot_uint32_t	ent_id;				/// ��ҵid
	aot_uint32_t	ent_serv_id;		/// ��ҵ������id
	aot_uint32_t    flag;				/// noused
	std::string		mps_device_guid;	/// ������Ϣ���͵�guid(����IOS, ��IOSϵͳ�����ṩ)
	std::string		mps_channel_group_key;
	std::string		str_reserved;

	typedef aot_std::vector<group_pair_t, ALLOC, false> group_list_type;
	group_list_type tribe_lst;			/// Ⱥ���б�
	group_list_type ent_dep_lst;
public:
	ims2mps_subscribe_t(ALLOC* alloc) : ent_dep_lst(alloc), tribe_lst(alloc)
	{
		ent_id = 0; ent_serv_id = 0; flag = 0;
	}

	static aot_uint32_t cdr_min_size(){ return mini_user_info_t::cdr_min_size() + 12 + e_std_string_cdr_min_size *3 + 4 + 4; }
	aot_uint32_t cdr_size(){ return mui.cdr_size() + 12 + string_cdr_size(&mps_device_guid) +  
		string_cdr_size(&mps_channel_group_key) + string_cdr_size(&str_reserved) + 4 + group_pair_t::size * tribe_lst.size() + 4 + group_pair_t::size * ent_dep_lst.size();}
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

/// ims ---> mps ȡ��������Ϣ����
struct ims2mps_undo_subscribe_t
{
	mini_user_info_t mui;
	std::string		mps_device_guid;	/// ������Ϣ���͵�guid(����IOS, ��IOSϵͳ�����ṩ)
	std::string		mps_channel_group_key;
	std::string		str_reserved;
public:
	ims2mps_undo_subscribe_t()
	{ }
	static aot_uint32_t cdr_min_size(){ return mini_user_info_t::cdr_min_size() + e_std_string_cdr_min_size *3; }
	aot_uint32_t cdr_size(){ return mui.cdr_size() + string_cdr_size(&mps_device_guid) + string_cdr_size(&mps_channel_group_key) + string_cdr_size(&str_reserved);}
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};


/// ims ---> mps tata״̬�ı�
struct ims2mps_notify_tata_status_change_t
{
	mini_user_info_t mui;
	std::string		mps_device_guid;	/// ������Ϣ���͵�guid(����IOS, ��IOSϵͳ�����ṩ)
	std::string		mps_channel_group_key;
	std::string		str_reserved;
public:
	ims2mps_notify_tata_status_change_t() { }
	static aot_uint32_t cdr_min_size(){ return mini_user_info_t::cdr_min_size() + e_std_string_cdr_min_size *3; }
	aot_uint32_t cdr_size(){ return mui.cdr_size() + string_cdr_size(&mps_device_guid) + string_cdr_size(&mps_channel_group_key) + string_cdr_size(&str_reserved);}
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

/// tata ---> ims ������Ϣ����
struct tata2ims_subscribe_mps_t
{
	aot_uint32_t	id;					/// �û��ڲ�id(tata�����Ӧ���ڲ�id��ʶ)
	aot_uint16_t	login_type;			/// ��¼����(tata/�ֻ�/����) 1: PC
	std::string		nbr;
	std::string		mps_device_guid;
	std::string		mps_channel_group_key;
	std::string		str_reserved;
public:
	static aot_uint32_t cdr_min_size(){ return 4 + 2 + e_std_string_cdr_min_size * 4; }
	aot_uint32_t cdr_size(){ return 4 + 2 +  string_cdr_size(&nbr) + string_cdr_size(&mps_device_guid) + string_cdr_size(&mps_channel_group_key) + string_cdr_size(&str_reserved);}
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

/// tata ---> ims ȡ��������Ϣ����
struct tata2ims_undo_subscribe_mps_t
{
	aot_uint32_t	id;					/// �û��ڲ�id(tata�����Ӧ���ڲ�id��ʶ)
	aot_uint16_t	login_type;			/// ��¼����(tata/�ֻ�/����) 1: PC
	std::string		nbr;
	std::string		mps_device_guid;
	std::string		mps_channel_group_key;
	std::string		str_reserved;
public:
	static aot_uint32_t cdr_min_size(){ return 4 + 2 + e_std_string_cdr_min_size * 4; }
	aot_uint32_t cdr_size(){ return 4 + 2 +  string_cdr_size(&nbr) + string_cdr_size(&mps_device_guid) + string_cdr_size(&mps_channel_group_key) + string_cdr_size(&str_reserved);}
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};


///////////////////////////////////////////////////////////////////////////////////////////////////////

enum MQUES_how_read_msg
{
	e_how_MQUES_read_msg_from_begin = 0x0001,			/// ��seq_begin ���� seq_end
	e_how_MQUES_read_msg_from_end  = 0x0001 << 1,		/// ��seq_end ���� seq_begin
	e_how_MQUES_read_msg_include_begin  = 0x0001 << 2,
	e_how_MQUES_read_msg_include_end  = 0x0001 << 3,
};

enum MQUES_how_write_msg
{
	e_MQUES_how_write_msg_normal_order = 0x0001,		/// ��ʱ��˳������(���Զ ---> ���)
	e_MQUES_how_write_msg_reverse_order = 0x0001 << 1,	/// ��ʱ�䵹������(��� ---> ���Զ)
};

struct MQUES_msg_key_info_t
{
	MQUES_msg_key_info_t()
	{
		id = 0; tm = 0; seq = 0;
	}
	int compare(MQUES_msg_key_info_t* p )
	{
		if( this->tm < p->tm ){ return -1; }
		if( this->tm > p->tm ){ return 1; }
		if( this->tm == p->tm )
		{ 
			/*if( this->seq < p->seq ) return -1;
			if( this->seq > p->seq ) return 1;*/
		}
		return 0;
	}
	aot_uint32_t id; /// ����id ���� ����id ����Ⱥ��id
	aot_uint64_t tm;
	aot_uint64_t seq;
	aot::inet::aot_string_t msg_key;
	static aot_uint32_t cdr_min_size(){ return 20 + aot::inet::aot_string_t::min_size; }
	aot_uint32_t cdr_size(){ return 20 + this->msg_key.cdr_write_size(); }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

struct MQUES_chat_record_info_t
{
	MQUES_chat_record_info_t()
	{
		id = 0; sync_msg_count = 0; offline_msg_count = 0;
	}
	aot_uint32_t id; /// ����id ���� ����id ����Ⱥ��id
	aot_uint16_t sync_msg_count;
	aot_uint16_t offline_msg_count;
	aot_uint32_t cdr_size(){ return 8;}
};

/// �û���ͬ����Ϣ�����б�
struct MQUES_get_recent_contacts_buddylist_t
{
	enum{size = 4};
	MQUES_get_recent_contacts_buddylist_t()
	{
		tata_id = 0;
	}
	aot_uint32_t tata_id;
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

template<class ALLOC>
struct MQUES_get_recent_contacts_buddylist_ret_t
{
	aot_uint32_t tata_id;
	typedef aot_std::vector<MQUES_msg_key_info_t, ALLOC, true> list_type;
	list_type lst;

	MQUES_get_recent_contacts_buddylist_ret_t(ALLOC* alloc): lst(alloc)
	{ tata_id = 0; }

	static aot_uint32_t list_cdr_size(list_type* s)
	{
		aot_uint32_t r= 0;
		aot_uint32_t n =  s->size();
		for( aot_uint32_t i = 0; i < n; ++i )
		{
			r += (*s)[i].cdr_size();
		}
		return r;
	}
	static aot_uint32_t cdr_min_size(){ return 4 + 4; }
	aot_uint32_t cdr_size(){ return 4 + 4 + list_cdr_size(&this->lst); }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};



struct MQUES_read_buddy_sync_msg_t
{
	MQUES_read_buddy_sync_msg_t()
	{
		tata_id = 0; buddy_id = 0; max_count = 20; max_len = 4*1024; 
		how_read = e_how_MQUES_read_msg_from_end | e_how_MQUES_read_msg_include_end;
		how_write = e_MQUES_how_write_msg_normal_order;
		unused = 0;
	}
	aot_uint32_t tata_id;
	aot_uint32_t buddy_id;
	aot_uint32_t max_count;
	aot_uint32_t max_len;
	aot_uint32_t unused;
	aot_uint8_t how_read;		/// enum MQUES_how_read_msg
	aot_uint8_t how_write;		/// enum MQUES_how_read_msg
	aot::inet::date_time_t tm;
	MQUES_msg_key_info_t msg_key_begin;
	MQUES_msg_key_info_t msg_key_end;

	static aot_uint32_t cdr_min_size(){ return 22 + aot::inet::date_time_t::size + MQUES_msg_key_info_t::cdr_min_size() * 2; }
	aot_uint32_t cdr_size(){ return 22 + aot::inet::date_time_t::size + this->msg_key_begin.cdr_size() + this->msg_key_end.cdr_size(); }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

struct MQUES_send_buddy_sync_msg_t
{
	MQUES_send_buddy_sync_msg_t()
	{
		tata_id = 0; buddy_id = 0; msg_count = 0;
	}
	aot_uint32_t tata_id;
	aot_uint32_t buddy_id;
	aot_uint32_t msg_count;
	MQUES_msg_key_info_t next_msg_key; /// ����@how ָ���Ķ�ȡ����, ����һ��δ����Ϣ��key
	/// ע��: ���صĽ�������� һ��pkt��list�б�, ��û�з���exheader����, ���Ƿŵ�body(��data)��

	static aot_uint32_t cdr_min_size(){ return 12 + MQUES_msg_key_info_t::cdr_min_size(); }
	aot_uint32_t cdr_size(){ return 12 + this->next_msg_key.cdr_size(); }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};


template<class ALLOC>
struct MQUES_read_buddylist_last_sync_msg_t
{
	aot_uint32_t tata_id;
	aot::inet::date_time_t tm;
	typedef aot_std::vector<MQUES_msg_key_info_t, ALLOC, true> list_type;
	list_type lst;
	list_type end_lst;
	
	MQUES_read_buddylist_last_sync_msg_t(ALLOC* alloc): lst(alloc), end_lst(alloc)
	{ tata_id = 0; }

	static aot_uint32_t cdr_min_size(){ return 4 + aot::inet::date_time_t::size + 4 + 4; }
	aot_uint32_t cdr_size();
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

template<class ALLOC>
struct MQUES_send_buddylist_last_sync_msg_t
{
	aot_uint32_t tata_id;
	aot_uint32_t msg_count;
	typedef aot_std::vector<MQUES_chat_record_info_t, ALLOC, true> list_type;
	list_type lst;
	/// ע��: ���صĽ�������� һ��pkt��list�б�, ��û�з���exheader����, ���Ƿŵ�body(��data)��
	MQUES_send_buddylist_last_sync_msg_t(ALLOC* alloc) : lst(alloc)
	{
		tata_id = 0; msg_count = 0;
	}
	static aot_uint32_t cdr_min_size(){ return 8 + 4; }
	aot_uint32_t cdr_size();
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

struct MQUES_read_buddy_offline_msg_t
{
	MQUES_read_buddy_offline_msg_t()
	{
		tata_id = 0; max_count = 20; max_len = 4*1024; 
		how_read = e_how_MQUES_read_msg_from_end | e_how_MQUES_read_msg_include_end;
		how_write = e_MQUES_how_write_msg_normal_order;
		unused = 0;
	}
	aot_uint32_t tata_id;
	aot_uint32_t max_count;
	aot_uint32_t max_len;
	aot_uint32_t unused;
	aot_uint8_t how_read;		/// enum MQUES_how_read_msg
	aot_uint8_t how_write;		/// enum MQUES_how_read_msg
	aot::inet::date_time_t tm;
	MQUES_msg_key_info_t msg_key_begin;
	MQUES_msg_key_info_t msg_key_end;

	static aot_uint32_t cdr_min_size(){ return 18 + aot::inet::date_time_t::size + MQUES_msg_key_info_t::cdr_min_size() * 2; }
	aot_uint32_t cdr_size(){ return 18 + aot::inet::date_time_t::size + this->msg_key_begin.cdr_size() + this->msg_key_end.cdr_size(); }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

struct MQUES_send_buddy_offline_msg_t
{
	MQUES_send_buddy_offline_msg_t()
	{
		tata_id = 0; msg_count = 0; msg_seq_begin = 0; msg_seq_end = 0;
	}
	aot_uint32_t tata_id;
	aot_uint32_t msg_count;
	aot_uint32_t msg_seq_begin;
	aot_uint32_t msg_seq_end;
	MQUES_msg_key_info_t next_msg_key; /// ����@how ָ���Ķ�ȡ����, ����һ��δ����Ϣ��key
	/// ע��: ���صĽ�������� һ��pkt��list�б�, ��û�з���exheader����, ���Ƿŵ�body(��data)��

	static aot_uint32_t cdr_min_size(){ return 16 + MQUES_msg_key_info_t::cdr_min_size(); }
	aot_uint32_t cdr_size(){ return 16 + this->next_msg_key.cdr_size(); }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

struct MQUES_delete_buddy_offline_msg_t
{
	enum{size = 20};
	aot_uint32_t tata_id;
	aot_uint64_t seq_begin;
	aot_uint64_t seq_end;
public:
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

template<class ALLOC>
struct MQUES_read_ent_dep_list_last_sync_msg_t
{
	aot_uint32_t tata_id;
	aot_uint32_t ent_id;
	aot_uint32_t serv_id;
	aot_uint32_t ims_id;
	aot::inet::date_time_t tm;
	typedef aot_std::vector<MQUES_msg_key_info_t, ALLOC, true> list_type;
	list_type lst;
	list_type end_lst;
	

	MQUES_read_ent_dep_list_last_sync_msg_t(ALLOC* alloc): lst(alloc), end_lst(alloc)
	{ tata_id = 0; ent_id = 0; serv_id = 0; ims_id = 0; }

	static aot_uint32_t cdr_min_size(){ return 16 + aot::inet::date_time_t::size + 4 + 4; }
	aot_uint32_t cdr_size();
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	static aot_uint32_t cdr_read_serv_id(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};


template<class ALLOC>
struct MQUES_send_ent_dep_list_last_sync_msg_t
{
	aot_uint32_t tata_id;
	aot_uint32_t ent_id;
	aot_uint32_t ims_id;
	aot_uint32_t msg_count;
	typedef aot_std::vector<MQUES_chat_record_info_t, ALLOC, true> list_type;
	list_type lst;

	/// ע��: ���صĽ�������� һ��pkt��list�б�, ��û�з���exheader����, ���Ƿŵ�body(��data)��

	MQUES_send_ent_dep_list_last_sync_msg_t(ALLOC* alloc): lst(alloc)
	{
		tata_id = 0; ent_id = 0; msg_count = 0; ims_id = 0;
	}
	static aot_uint32_t cdr_min_size(){ return 16 + 4; }
	static aot_uint32_t cdr_read_ims_id(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_size();
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

template<class ALLOC>
struct MQUES_read_ent_dep_list_offline_msg_t
{
	aot_uint32_t tata_id;
	aot_uint32_t ent_id;
	aot_uint32_t serv_id;
	aot_uint32_t ims_id;
	aot_uint8_t how_read;		/// enum MQUES_how_read_msg
	aot_uint8_t how_write;		/// enum MQUES_how_read_msg
	aot::inet::date_time_t tm;
	typedef aot_std::vector<aot_uint32_t, ALLOC, false> list_type;
	list_type lst;
	/// ע��: ���صĽ�������� һ��pkt��list�б�, ��û�з���exheader����, ���Ƿŵ�body(��data)��

	MQUES_read_ent_dep_list_offline_msg_t(ALLOC* alloc): lst(alloc)
	{ 
		tata_id = 0; ent_id = 0; serv_id = 0; ims_id = 0; 
		how_read = e_how_MQUES_read_msg_from_end | e_how_MQUES_read_msg_include_end | e_how_MQUES_read_msg_include_begin;
		how_write = e_MQUES_how_write_msg_normal_order;
	}

	static aot_uint32_t cdr_min_size(){ return 18 + aot::inet::date_time_t::size + 4; }
	aot_uint32_t cdr_size(){ return 18 + aot::inet::date_time_t::size + 4 + this->lst.size() * 4; }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	static aot_uint32_t cdr_read_serv_id(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

struct MQUES_send_ent_dep_list_offline_msg_piece_t
{
	aot_uint32_t tata_id;
	aot_uint32_t ent_id;
	aot_uint32_t dep_id;
	aot_uint32_t ims_id;
	aot_uint32_t msg_count;

	/// ע��: 
	/// 1. ������һ�������б�, ���Ƿ��صĽ���ǵ������ŷ��ص�.
	/// 2. ���صĽ�������� һ��pkt��list�б�, ��û�з���exheader����, ���Ƿŵ�body(��data)��

	MQUES_send_ent_dep_list_offline_msg_piece_t()
	{
		tata_id = 0; ent_id = 0; dep_id = 0; msg_count = 0; ims_id = 0;
	}
	static aot_uint32_t cdr_min_size(){ return 20; }
	static aot_uint32_t cdr_read_ims_id(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_size(){ return 20; }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

struct MQUES_send_ent_dep_list_offline_msg_t
{
	aot_uint32_t tata_id;
	aot_uint32_t ent_id;
	aot_uint32_t ims_id;
	aot_uint32_t msg_count;

	MQUES_send_ent_dep_list_offline_msg_t()
	{
		tata_id = 0; ent_id = 0; msg_count = 0; ims_id = 0;
	}
	static aot_uint32_t cdr_min_size(){ return 16; }
	static aot_uint32_t cdr_read_ims_id(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_size(){ return 16; }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

struct MQUES_read_ent_dep_sync_msg_t
{
	MQUES_read_ent_dep_sync_msg_t()
	{
		tata_id = 0; ent_id = 0; dep_id = 0; serv_id = 0; ims_id = 0; 
		max_count = 20; max_len = 4*1024; 
		how_read = e_how_MQUES_read_msg_from_end | e_how_MQUES_read_msg_include_end;
		how_write = e_MQUES_how_write_msg_normal_order;
		unused = 0;
	}
	aot_uint32_t tata_id;
	aot_uint32_t ent_id;
	aot_uint32_t dep_id;
	aot_uint32_t serv_id;
	aot_uint32_t ims_id;
	aot_uint32_t max_count;
	aot_uint32_t max_len;
	aot_uint32_t unused;
	aot_uint8_t how_read;		/// enum MQUES_how_read_msg
	aot_uint8_t how_write;		/// enum MQUES_how_read_msg
	aot::inet::date_time_t tm;
	MQUES_msg_key_info_t msg_key_begin;
	MQUES_msg_key_info_t msg_key_end;

	static aot_uint32_t cdr_min_size(){ return 34 + aot::inet::date_time_t::size + MQUES_msg_key_info_t::cdr_min_size() * 2; }
	static aot_uint32_t cdr_read_serv_id(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_size(){ return 34 + aot::inet::date_time_t::size + this->msg_key_begin.cdr_size() + this->msg_key_end.cdr_size(); }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

struct MQUES_send_ent_dep_sync_msg_t
{
	MQUES_send_ent_dep_sync_msg_t()
	{
		tata_id = 0; ent_id = 0; dep_id = 0; ims_id = 0; msg_count = 0;
	}
	aot_uint32_t tata_id;
	aot_uint32_t ent_id;
	aot_uint32_t dep_id;
	aot_uint32_t ims_id;
	aot_uint32_t msg_count;
	MQUES_msg_key_info_t next_msg_key; /// ����@how ָ���Ķ�ȡ����, ����һ��δ����Ϣ��key
	/// ע��: ���صĽ�������� һ��pkt��list�б�, ��û�з���exheader����, ���Ƿŵ�body(��data)��

	static aot_uint32_t cdr_min_size(){ return 20 + MQUES_msg_key_info_t::cdr_min_size(); }
	static aot_uint32_t cdr_read_ims_id(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_size(){ return 20 + this->next_msg_key.cdr_size(); }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};


template<class ALLOC>
struct MQUES_read_tribe_list_last_sync_msg_t
{
	aot_uint32_t tata_id;
	aot_uint32_t serv_id;
	aot_uint32_t ims_id;
	aot::inet::date_time_t tm;
	typedef aot_std::vector<MQUES_msg_key_info_t, ALLOC, true> list_type;
	list_type lst;
	list_type end_lst;

	MQUES_read_tribe_list_last_sync_msg_t(ALLOC* alloc): lst(alloc), end_lst(alloc)
	{ tata_id = 0; serv_id = 0; ims_id = 0; }

	static aot_uint32_t cdr_min_size(){ return 12 + aot::inet::date_time_t::size + 4 + 4; }
	aot_uint32_t cdr_size();
	static aot_uint32_t cdr_read_serv_id(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};


template<class ALLOC>
struct MQUES_send_tribe_list_last_sync_msg_t
{
	aot_uint32_t tata_id;
	aot_uint32_t ims_id;
	aot_uint32_t msg_count;
	aot_uint32_t serv_id;	/// ��ΪȺ�ķ����������ж��, ����ֶ����ڿͻ������ַ��صĶ�����������
	
	typedef aot_std::vector<MQUES_chat_record_info_t, ALLOC, true> list_type;
	list_type lst;
	MQUES_send_tribe_list_last_sync_msg_t(ALLOC* alloc): lst(alloc)
	{
		tata_id = 0; ims_id = 0; msg_count = 0; serv_id = 0;
	}
	static aot_uint32_t cdr_min_size(){ return 16 + 4; }
	static aot_uint32_t cdr_read_ims_id(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_size();
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

struct MQUES_read_tribe_sync_msg_t
{
	MQUES_read_tribe_sync_msg_t()
	{
		tata_id = 0; tribe_id = 0; serv_id = 0; ims_id = 0; max_count = 20; max_len = 4*1024; 
		how_read = e_how_MQUES_read_msg_from_end | e_how_MQUES_read_msg_include_end;
		how_write = e_MQUES_how_write_msg_normal_order;
		unused = 0;
	}
	aot_uint32_t tata_id;
	aot_uint32_t tribe_id;
	aot_uint32_t serv_id;
	aot_uint32_t ims_id;
	aot_uint32_t max_count;
	aot_uint32_t max_len;
	aot_uint32_t unused;
	aot_uint8_t how_read;		/// enum MQUES_how_read_msg
	aot_uint8_t how_write;		/// enum MQUES_how_read_msg
	aot::inet::date_time_t tm;
	MQUES_msg_key_info_t msg_key_begin;
	MQUES_msg_key_info_t msg_key_end;

	static aot_uint32_t cdr_min_size(){ return 30 + aot::inet::date_time_t::size + MQUES_msg_key_info_t::cdr_min_size() * 2; }
	aot_uint32_t cdr_size(){ return 30 + aot::inet::date_time_t::size + this->msg_key_begin.cdr_size() + this->msg_key_end.cdr_size(); }
	static aot_uint32_t cdr_read_serv_id(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

struct MQUES_send_tribe_sync_msg_t
{
	MQUES_send_tribe_sync_msg_t()
	{
		tata_id = 0; ims_id = 0; tribe_id = 0;  msg_count = 0;
	}
	aot_uint32_t tata_id;
	aot_uint32_t tribe_id;
	aot_uint32_t ims_id;
	aot_uint32_t msg_count;
	MQUES_msg_key_info_t next_msg_key; /// ����@how ָ���Ķ�ȡ����, ����һ��δ����Ϣ��key
	/// ע��: ���صĽ�������� һ��pkt��list�б�, ��û�з���exheader����, ���Ƿŵ�body(��data)��

	static aot_uint32_t cdr_min_size(){ return 16 + MQUES_msg_key_info_t::cdr_min_size(); }
	static aot_uint32_t cdr_read_ims_id(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_size(){ return 16 + this->next_msg_key.cdr_size(); }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

template<class ALLOC>
struct MQUES_read_tribe_list_offline_msg_t
{
	aot_uint32_t tata_id;
	aot_uint32_t serv_id;
	aot_uint32_t ims_id;
	aot_uint8_t how_read;		/// enum MQUES_how_read_msg
	aot_uint8_t how_write;		/// enum MQUES_how_read_msg
	aot::inet::date_time_t tm;
	typedef aot_std::vector<aot_uint32_t, ALLOC, false> list_type;
	list_type lst;

	MQUES_read_tribe_list_offline_msg_t(ALLOC* alloc): lst(alloc)
	{ 
		tata_id = 0; serv_id = 0; ims_id = 0; 
		how_read = e_how_MQUES_read_msg_from_end | e_how_MQUES_read_msg_include_end | e_how_MQUES_read_msg_include_begin;
		how_write = e_MQUES_how_write_msg_normal_order;
	}

	static aot_uint32_t cdr_min_size(){ return 14 + aot::inet::date_time_t::size + 4; }
	aot_uint32_t cdr_size(){ return 14 + aot::inet::date_time_t::size + 4 + this->lst.size() * 4; }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	static aot_uint32_t cdr_read_serv_id(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

struct MQUES_send_tribe_list_offline_msg_piece_t
{
	aot_uint32_t tata_id;
	aot_uint32_t tribe_id;
	aot_uint32_t ims_id;
	aot_uint32_t msg_count;

	/// ע��: 
	/// 1. ������һ��Ⱥ���б�, ���Ƿ��صĽ���ǵ���Ⱥ�鷵�ص�.
	/// 2. ���صĽ�������� һ��pkt��list�б�, ��û�з���exheader����, ���Ƿŵ�body(��data)��

	MQUES_send_tribe_list_offline_msg_piece_t()
	{
		tata_id = 0; tribe_id = 0; msg_count = 0; ims_id = 0;
	}
	static aot_uint32_t cdr_min_size(){ return 16; }
	static aot_uint32_t cdr_read_ims_id(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_size(){ return 16; }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

struct MQUES_send_tribe_list_offline_msg_t
{
	aot_uint32_t tata_id;
	aot_uint32_t ims_id;
	aot_uint32_t msg_count;
	aot_uint32_t serv_id;	/// ��ΪȺ�ķ����������ж��, ����ֶ����ڿͻ������ַ��صĶ�����������

	/// ע��: 
	/// 1. ������һ��Ⱥ���б�, ���Ƿ��صĽ���ǵ���Ⱥ�鷵�ص�.
	/// 2. ���صĽ�������� һ��pkt��list�б�, ��û�з���exheader����, ���Ƿŵ�body(��data)��

	MQUES_send_tribe_list_offline_msg_t()
	{
		tata_id = 0; msg_count = 0; ims_id = 0; serv_id = 0;
	}
	static aot_uint32_t cdr_min_size(){ return 16; }
	static aot_uint32_t cdr_read_ims_id(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_size(){ return 16; }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

struct set_tata_runtime_mode_t
{
	set_tata_runtime_mode_t()
	{
		tata_id = 0; mode = e_tata_runtime_mode_active;
	}
	aot_uint32_t tata_id;
	aot_uint32_t mode;

	static aot_uint32_t cdr_min_size(){ return 8; }
	aot_uint32_t cdr_size(){ return 8; }
	aot_uint32_t cdr_read(char* buf, aot_uint32_t len);
	aot_uint32_t cdr_write(char* buf, aot_uint32_t len);
};

}} /// end namespace aot/prot


inline bool operator == (const aot::prot::group_pair_t& g1, const aot::prot::group_pair_t& g2)
{
	return g1.group_id == g2.group_id;
}
inline bool operator != (const aot::prot::group_pair_t& g1, const aot::prot::group_pair_t& g2)
{
	return !(g1 == g2);
}


#include "aot_prot.inl"
#pragma warning(default:4996)

#endif /// __AOT_PROT_H__