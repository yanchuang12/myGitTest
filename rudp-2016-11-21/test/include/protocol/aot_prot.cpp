#include "stdafx.h"
#include "aot_prot.h"


namespace aot{ namespace prot{

void 
packet_parser::read_header(aot_buf_t* pkt, header_t* out)
{
	char* p = aot_buf_rd_ptr(pkt);
	out->version = ph_get_ver(p);
	out->type = ph_get_type(p);
	out->encrypt_type = ph_get_encrypt_type(p);
	out->data_len = ph_get_data_len(p);
	out->exheader_len = ph_get_extern_header_len(p);
}

void 
packet_parser::read_date_time(aot_buf_t* pkt, aot::inet::date_time_t* dt)
{
	char* p = aot_buf_rd_ptr(pkt) + PKT_HEADER_LEN + 65;
	aot::inet::cdr_reader cdr;
	cdr.set_buf(p, aot::inet::date_time_t::size);
	cdr.read_date_time(dt);
}
void 
packet_parser::write_date_time(aot_buf_t* pkt, aot::inet::date_time_t* dt)
{
	char* p = aot_buf_rd_ptr(pkt) + PKT_HEADER_LEN + 65;
	aot::inet::cdr_writer cdr;
	cdr.set_buf(p, aot::inet::date_time_t::size);
	cdr.write_date_time(dt);
}

bool 
packet_parser::read_strkey(aot_buf_t* pkt, aot_uint32_t pkt_len, aot::inet::aot_string_t* key)
{
	char* p = aot_buf_rd_ptr(pkt) + PKT_HEADER_LEN + exheader_t::__fixed_size;
	aot::inet::cdr_reader cdr;

	cdr.set_buf(p, pkt_len - PKT_HEADER_LEN - exheader_t::__fixed_size);

	if( !cdr.read_str(key) )
	{
		assert(false);
		return false;
	}
	return true;
}

bool 
packet_parser::read_exheader(aot_buf_t* pkt, exheader_t* out)
{
	aot_uint32_t len = aot_buf_length(pkt);
	if( len < exheader_t::fix_size + PKT_HEADER_LEN )
	{
		assert(0);
		return false;
	}
	aot::inet::cdr_reader cdr;
	char* buf = aot_buf_rd_ptr(pkt) + PKT_HEADER_LEN;

	cdr.set_buf(buf, len);
	/// 必须按照字段顺序读
	out->flag = cdr.read_4();
	out->busi_code = cdr.read_4();
	out->seq = cdr.read_4();
	out->key = cdr.read_4();
	out->from_id = cdr.read_4();
	out->from_attr = cdr.read_4();
	out->to_id = cdr.read_4();
	out->to_attr = cdr.read_4();
	out->ex_code = cdr.read_4();
	out->err_code = cdr.read_4();
	out->unused1 = cdr.read_4();
	out->custom_busi_code = cdr.read_4();

	out->to_src_stdi.how = cdr.read_1();
	out->to_src_stdi.param = cdr.read_2();
	out->to_dest_stdi.how = cdr.read_1();
	out->to_dest_stdi.param = cdr.read_2();

	out->session_unique_seq = cdr.read_8();
	out->from_login_type = cdr.read_2();
	out->from_status = cdr.read_1();

	cdr.read_date_time(&out->date_time);

	if( !cdr.read_str(&out->str_key) )
	{
		assert(false);
		return false;
	}
	if( !cdr.read_str(&out->call_id) )
	{
		assert(false);
		return false;
	}
	if( !cdr.read_str(&out->from_guid) )
	{
		assert(false);
		return false;
	}
	if( !cdr.read_str(&out->to_guid) )
	{
		assert(false);
		return false;
	}
	if( !cdr.read_str(&out->from_nbr) )
	{
		assert(false);
		return false;
	}
	if( !cdr.read_str(&out->to_nbr) )
	{
		assert(false);
		return false;
	}
	
	if( !cdr.read_str(&out->str_data) )
	{
		assert(false);
		return false;
	}
	if( !cdr.read_encode_data(&out->encode_data) )
	{
		assert(false);
		return false;
	}

	out->arry_size = cdr.read_4();

	if( out->arry_size == 0 )
	{
		out->arry = NULL;
	}
	else
	{
		out->arry = reinterpret_cast<void*>(cdr.next_rd_ptr());
	}

	if( cdr.get_curr_pos() + out->arry_size > len )
	{
		/// out of range
		assert(false);
		out->arry = NULL;
		return false;
	}
	return true;
}

void 
packet_parser::write_header(header_t* h, aot_buf_t* out)
{
	char* p = aot_buf_wr_ptr(out);
	memset(p, 0, PKT_HEADER_LEN);

	ph_set_ver(p, h->version);
	ph_set_type(p, h->type);
	ph_set_encrypt_type(p, h->encrypt_type);
	ph_set_data_len(p, h->data_len);
	ph_set_extern_header_len(p, h->exheader_len);

	aot_buf_add_wr_pos(out, PKT_HEADER_LEN);
}

bool 
packet_parser::write_exheader(aot_buf_t* pkt, exheader_t* exh)
{
	aot_uint32_t buf_space = aot_buf_space(pkt);
	if( buf_space < exh->size() )
	{
		/// out of range
		assert(false);
		return false;
	}

	aot::inet::cdr_writer cdr;
	char* buf = aot_buf_wr_ptr(pkt);
	cdr.set_buf(buf, buf_space);
	cdr.write_4(exh->flag);
	cdr.write_4(exh->busi_code);
	cdr.write_4(exh->seq);
	cdr.write_4(exh->key);
	cdr.write_4(exh->from_id);
	cdr.write_4(exh->from_attr);
	cdr.write_4(exh->to_id);
	cdr.write_4(exh->to_attr);
	cdr.write_4(exh->ex_code);
	cdr.write_4(exh->err_code);
	cdr.write_4(exh->unused1);
	cdr.write_4(exh->custom_busi_code);
	cdr.write_1(exh->to_src_stdi.how);
	cdr.write_2(exh->to_src_stdi.param);
	cdr.write_1(exh->to_dest_stdi.how);
	cdr.write_2(exh->to_dest_stdi.param);

	cdr.write_8(exh->session_unique_seq);
	cdr.write_2(exh->from_login_type);
	cdr.write_1(exh->from_status);

	cdr.write_date_time(&exh->date_time);

	if( !cdr.write_str(&exh->str_key) )
	{
		assert(false);
		return false;
	}
	if( !cdr.write_str(&exh->call_id) )
	{
		assert(false);
		return false;
	}
	if( !cdr.write_str(&exh->from_guid) )
	{
		assert(false);
		return false;
	}
	if( !cdr.write_str(&exh->to_guid) )
	{
		assert(false);
		return false;
	}
	if( !cdr.write_str(&exh->from_nbr) )
	{
		assert(false);
		return false;
	}
	if( !cdr.write_str(&exh->to_nbr) )
	{
		assert(false);
		return false;
	}
	
	if( !cdr.write_str(&exh->str_data) )
	{
		assert(false);
		return false;
	}
	if( !cdr.write_encode_data(&exh->encode_data) )
	{
		assert(false);
		return false;
	}
	cdr.write_4(exh->arry_size);
	if( exh->arry_size > 0 )
	{
		assert(exh->arry);
		if( !cdr.write_1_array(exh->arry, exh->arry_size) )
		{
			assert(0);
			return false;
		}
	}
#ifdef _DEBUG
	assert(cdr.get_curr_pos() == exh->size());
#endif
	aot_buf_add_wr_pos(pkt, cdr.get_curr_pos());
	return true;
}

bool 
packet_parser::write_min_exheader(aot_buf_t* pkt, exheader_t* exh)
{
	aot_uint32_t buf_space = aot_buf_space(pkt);
	if( buf_space < exh->real_min_size() )
	{
		/// out of range
		assert(false);
		return false;
	}

	aot::inet::cdr_writer cdr;
	char* buf = aot_buf_wr_ptr(pkt);
	cdr.set_buf(buf, buf_space);

	cdr.write_4(exh->flag);
	cdr.write_4(exh->busi_code);
	cdr.write_4(exh->seq);
	cdr.write_4(exh->key);
	cdr.write_4(exh->from_id);
	cdr.write_4(exh->from_attr);
	cdr.write_4(exh->to_id);
	cdr.write_4(exh->to_attr);
	cdr.write_4(exh->ex_code);
	cdr.write_4(exh->err_code);
	cdr.write_4(exh->unused1);
	cdr.write_4(exh->custom_busi_code);
	cdr.write_1(exh->to_src_stdi.how);
	cdr.write_2(exh->to_src_stdi.param);
	cdr.write_1(exh->to_dest_stdi.how);
	cdr.write_2(exh->to_dest_stdi.param);

	cdr.write_8(exh->session_unique_seq);
	cdr.write_2(exh->from_login_type);
	cdr.write_1(exh->from_status);

	cdr.write_date_time(&exh->date_time);

	if( !cdr.write_str(&exh->str_key) )
	{
		assert(false);
		return false;
	}
	if( !cdr.write_str(&exh->call_id) )
	{
		assert(false);
		return false;
	}
	if( !cdr.write_str(&exh->from_guid) )
	{
		assert(false);
		return false;
	}
	if( !cdr.write_str(&exh->to_guid) )
	{
		assert(false);
		return false;
	}
	if( !cdr.write_str(&exh->from_nbr) )
	{
		assert(false);
		return false;
	}
	if( !cdr.write_str(&exh->to_nbr) )
	{
		assert(false);
		return false;
	}
	
	if( !cdr.write_str(&exh->str_data) )
	{
		assert(false);
		return false;
	}
	if( !cdr.write_encode_data(&exh->encode_data) )
	{
		assert(false);
		return false;
	}

	if( !cdr.write_4(exh->arry_size) )
	{
		assert(false);
		return false;
	}

	aot_buf_add_wr_pos(pkt, cdr.get_curr_pos());
	return true;
}

/* - * - */

/* - * - */

aot_uint32_t 
bin_data_list_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);

	aot::inet::aot_bin_data_t b;
	aot_uint32_t n = cdr.read_4();
	for( aot_uint32_t i = 0; i < n; ++i )
	{
		if( !cdr.read_bin_data(&b) ) 
		{ 
			assert(0); return 0; 
		}
		this->lst.push_back(b);
	}
	return cdr.get_curr_pos();
}
aot_uint32_t 
bin_data_list_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ){ assert(0); return 0; }

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);

	cdr.write_4((aot_uint32_t)this->lst.size());

	aot::inet::aot_bin_data_t b;

	std::list<aot::inet::aot_bin_data_t>::iterator it = this->lst.begin();
	for( ; it != this->lst.end(); ++it )
	{
		b = (*it);
		if( !cdr.write_bin_data(&b) )
		{
			assert(0); return 0; 
		}
	}

	return cdr.get_curr_pos();
}

/* - * - */


/* - * - */




/* - * - */

aot_uint32_t 
mini_user_status_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < mini_user_status_t::size ) { assert(0); return 0; }
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	this->id = cdr.read_4();
	this->nbr_num = cdr.read_nbrnum();
	this->ims_id = cdr.read_4();
	this->status = cdr.read_1();
	this->login_type = cdr.read_2();
	this->login_mode = cdr.read_1();
	this->feature_nbr_num = cdr.read_nbrnum();
	this->tata_version = cdr.read_1();
	return cdr.get_curr_pos();
}

aot_uint32_t 
mini_user_status_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < mini_user_status_t::size ) { assert(0); return 0; }

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(this->id);
	cdr.write_nbrnum(this->nbr_num);
	cdr.write_4(this->ims_id);
	cdr.write_1(this->status);
	cdr.write_2(this->login_type);
	cdr.write_1(this->login_mode);
	cdr.write_nbrnum(this->feature_nbr_num);
	cdr.write_1(this->tata_version);
	return cdr.get_curr_pos();
}

void 
mini_user_status_t::assign(mini_user_info_t* u)
{
	this->id = u->id;
	this->ims_id = u->ims_id;
	this->login_type = u->login_type;
	this->login_mode = u->login_mode;
	this->status = u->status;
	this->nbr_num = str2nbrnum(u->nbr.c_str());
	this->feature_nbr_num = str2nbrnum(u->feature_nbr.c_str());
	this->tata_version = u->tata_version;
}

void 
mini_user_status_t::assign(mini_user_status_t* mus)
{
	*this = *mus;
}

/* - * - */

serv_node_info_t::serv_node_info_t()
: id(0)
{
	addr.ip = 0; addr.port = 0;
}

aot_uint32_t 
serv_node_info_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < size )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	this->id = cdr.read_4();
	this->addr.port = cdr.read_2();
	this->addr.ip = cdr.read_4();
	this->listen_addr.port = cdr.read_2();
	this->listen_addr.ip = cdr.read_4();
	return cdr.get_curr_pos();
}

aot_uint32_t 
serv_node_info_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < size )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_writer cdr;
	if( len < serv_node_info_t::size )
	{
		return 0;
	}
	cdr.set_buf(buf, len);
	cdr.write_4(this->id);
	cdr.write_2(this->addr.port);
	cdr.write_4(this->addr.ip);
	cdr.write_2(this->listen_addr.port);
	cdr.write_4(this->listen_addr.ip);
	return cdr.get_curr_pos();
}

/* - * - */

aot_uint32_t 
mini_user_info_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	this->id = cdr.read_4();
	this->status = cdr.read_1();
	this->login_type = cdr.read_2();
	this->login_mode = cdr.read_1();
	this->ims_id = cdr.read_4();
	this->info_version = cdr.read_4();
	this->tata_version = cdr.read_1();
	this->unused = cdr.read_4();
	this->addr.port = cdr.read_2();
	this->addr.ip = cdr.read_4();

	cdr.read_date_time(&this->last_offline_tm);

	if( !cdr.read_str(&this->nbr) )
	{
		assert(0);
		return 0;
	}
	if( !cdr.read_str(&this->guid) )
	{
		assert(0);
		return 0;
	}
	if( !cdr.read_str(&this->str_data) )
	{
		assert(0);
		return 0;
	}
	if( !cdr.read_str(&this->xml_data) )
	{
		assert(0);
		return 0;
	}
	if( !cdr.read_str(&this->feature_nbr) )
	{
		assert(0);
		return 0;
	}

	return cdr.get_curr_pos();
}

aot_uint32_t 
mini_user_info_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(this->id);
	cdr.write_1(this->status);
	cdr.write_2(this->login_type);
	cdr.write_1(this->login_mode);
	cdr.write_4(this->ims_id);
	cdr.write_4(this->info_version);
	cdr.write_1(this->tata_version);
	cdr.write_4(this->unused);
	cdr.write_2(this->addr.port);
	cdr.write_4(this->addr.ip);

	cdr.write_date_time(&this->last_offline_tm);

	if( !cdr.write_str(&this->nbr) )
	{
		assert(0);
		return 0;
	}
	if( !cdr.write_str(&this->guid) )
	{
		assert(0);
		return 0;
	}
	if( !cdr.write_str(&this->str_data) )
	{
		assert(0);
		return 0;
	}
	if( !cdr.write_str(&this->xml_data) )
	{
		assert(0);
		return 0;
	}
	if( !cdr.write_str(&this->feature_nbr) )
	{
		assert(0);
		return 0;
	}

	return cdr.get_curr_pos();
}

/* - * - */

aot_uint32_t 
serv_node_list_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	aot_uint32_t cnt = cdr.read_4();
	if( cnt == 0 ){ return cdr.get_curr_pos(); }

	aot_uint32_t n = 0;
	for( aot_uint32_t i = 0; i < cnt; ++i )
	{
		serv_node_info_t si;
		n = si.cdr_read(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
		if( n != serv_node_info_t::size ) { assert(0); return 0; }

		n += cdr.get_curr_pos();
		cdr.set_curr_pos(n);
		this->lst.push_back(si);
	}

	return cdr.get_curr_pos();
}

aot_uint32_t 
serv_node_list_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() ) { assert(0); return 0; }

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4((aot_uint32_t)this->lst.size());

	list_type::iterator it = this->lst.begin();
	for( ; it != this->lst.end(); ++it )
	{
		aot_uint32_t n = (*it).cdr_write(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
		if( n != serv_node_info_t::size ) { assert(0); return 0; }

		n += cdr.get_curr_pos();
		cdr.set_curr_pos(n);
	}
	return cdr.get_curr_pos();
}

/* - * - */
bool 
mini_user_info_t::update(mini_user_info_t* new_mu)
{
	if( NULL == new_mu ) return false;
	/// 0: 表示用户重置了版本号,必须进行更新. 否则只有版本号大于当前时,才需要更新
	if( 1 /*|| this->guid != new_mu->guid || (new_mu->info_version == 0 || new_mu->info_version >= this->info_version)*/ ) 
	{
		*this = *new_mu;
		if( this->status == e_user_status_offline )
		{
			this->last_offline_tm.get_curr_time();
			this->ims_id = 0;
		}
		return true;
	}
	return false;
}
void 
mini_user_info_t::init()
{
	this->status = e_user_status_offline;
	this->login_type = e_tata_type_pc_win; this->login_mode = 0;
	this->id = 0; this->ims_id = 0; this->info_version = 0; this->unused = 0;
	this->nbr = ""; this->guid = ""; this->str_data = ""; this->xml_data = ""; this->feature_nbr = "";
}
/* - * - */

aot_uint32_t 
tata_forceout_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	flag = cdr.read_4();

	aot_uint32_t n = this->mini_user_info.cdr_read(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	n += cdr.get_curr_pos();
	cdr.set_curr_pos(n);

	if( !cdr.read_str(&this->str_data) )
	{
		assert(0);
		return 0;
	}

	return cdr.get_curr_pos();
}
aot_uint32_t 
tata_forceout_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(flag);

	aot_uint32_t n = this->mini_user_info.cdr_write(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	n += cdr.get_curr_pos();
	cdr.set_curr_pos(n);

	if( !cdr.write_str(this->str_data.data, this->str_data.length) )
	{
		assert(0);
		return 0;
	}

	return cdr.get_curr_pos();
}
/* - * - */
aot_uint32_t 
tata_regist_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() )
	{
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	this->flag = cdr.read_4();

	if( !cdr.read_str(&this->token) ) { assert(0); return 0; }
	if( !cdr.read_str(&this->str_data) ) { assert(0); return 0; }
	if( !cdr.read_str(&this->str_platform_name) ) { assert(0); return 0; }

	aot_uint32_t n = this->mu.cdr_read(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	if( n == 0 )  { assert(0); return 0; }

	n += cdr.get_curr_pos();
	cdr.set_curr_pos(n);

	return cdr.get_curr_pos();
}
aot_uint32_t 
tata_regist_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() )
	{
		return 0;
	}
	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(this->flag);

	if( !cdr.write_str(&this->token) ) { assert(0); return 0; }
	if( !cdr.write_str(&this->str_data) ) { assert(0); return 0; }
	if( !cdr.write_str(&this->str_platform_name) ) { assert(0); return 0; }

	aot_uint32_t n = this->mu.cdr_write(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	n += cdr.get_curr_pos();
	cdr.set_curr_pos(n);

	return cdr.get_curr_pos();
}
/* - * - */

aot_uint32_t 
tata_regist_ret_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	flag = cdr.read_4();
	dbs_id = cdr.read_4();

	aot_uint32_t n = this->mini_user_info.cdr_read(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	n += cdr.get_curr_pos();
	cdr.set_curr_pos(n);

	if( !cdr.read_str(&this->token) ) { assert(0); return 0; }
	if( !cdr.read_str(&this->redirect_ims_addr) ){ assert(0); return 0; }
	if( !cdr.read_str(&this->str_data) ){ assert(0); return 0; }

	return cdr.get_curr_pos();
}
aot_uint32_t 
tata_regist_ret_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() ) { assert(0); return 0; }

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(flag);
	cdr.write_4(dbs_id);

	aot_uint32_t n = this->mini_user_info.cdr_write(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	n += cdr.get_curr_pos();
	cdr.set_curr_pos(n);

	if( !cdr.write_str(&this->token) ) { assert(0); return 0; }
	if( !cdr.write_str(&this->redirect_ims_addr) ) { assert(0); return 0; }
	if( !cdr.write_str(&this->str_data) ) { assert(0); return 0; }

	return cdr.get_curr_pos();
}

/* - * - */

aot_uint32_t 
get_the_user_status_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < size )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	tata_id = cdr.read_4();
	return cdr.get_curr_pos();
}
aot_uint32_t 
get_the_user_status_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < size )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(tata_id);
	return cdr.get_curr_pos();
}

/* - * - */
aot_uint32_t 
get_online_buddy_list_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	this->tata_id = cdr.read_4();
	this->ims_id = cdr.read_4();
	this->unused = cdr.read_4();
	if( !cdr.read_str(&this->str_data) ) { assert(0); return 0; }

	return cdr.get_curr_pos();
}
aot_uint32_t 
get_online_buddy_list_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() ) { assert(0); return 0; }

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(this->tata_id);
	cdr.write_4(this->ims_id);
	cdr.write_4(this->unused);
	if( !cdr.write_str(&this->str_data) ) { assert(0); return 0; }

	return cdr.get_curr_pos();
}
/* - * - */
aot_uint32_t 
ims2dbs_regist_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	listen_port = cdr.read_2();
	if( !cdr.read_str(&addr) )
	{
		assert(0);
		return 0;
	}
	if( !cdr.read_str(&str_data) )
	{
		assert(0);
		return 0;
	}
	return cdr.get_curr_pos();
}
aot_uint32_t 
ims2dbs_regist_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_2(listen_port);
	if( !cdr.write_str(&addr) )
	{
		assert(0);
		return 0;
	}
	if( !cdr.write_str(&str_data) )
	{
		assert(0);
		return 0;
	}
	return cdr.get_curr_pos();
}
/* - * - */

aot_uint32_t 
ims2dbs_regist_ret_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	ims_id = cdr.read_4();
	dbs_id = cdr.read_4();
	if( !cdr.read_str(&str_data) )
	{
		assert(0);
		return 0;
	}

	aot_uint32_t n = this->serv_lst.cdr_read(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	if( n == 0 ) { assert(0); return 0; }

	n += cdr.get_curr_pos();
	cdr.set_curr_pos(n);

	return cdr.get_curr_pos();
}
aot_uint32_t 
ims2dbs_regist_ret_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(ims_id);
	cdr.write_4(dbs_id);
	if( !cdr.write_str(&str_data) )
	{
		assert(0);
		return 0;
	}

	aot_uint32_t n = this->serv_lst.cdr_write(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	if( n == 0 ) { assert(0); return 0; }

	n += cdr.get_curr_pos();
	cdr.set_curr_pos(n);

	return cdr.get_curr_pos();
}

/* - * - */

aot_uint32_t 
ims2dbs_report_addrinfo_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	if( !cdr.read_str(&addr) )
	{
		assert(0);
		return 0;
	}
	return cdr.get_curr_pos();
}
aot_uint32_t 
ims2dbs_report_addrinfo_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	if( !cdr.write_str(addr.data, addr.length) )
	{
		assert(0);
		return 0;
	}
	return cdr.get_curr_pos();
}

/* - * - */

aot_uint32_t 
dbs2nms_report_load_balance_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	opt = cdr.read_4();
	if( !cdr.read_str(&info) )
	{
		assert(0);
		return 0;
	}
	return cdr.get_curr_pos();
}
aot_uint32_t 
dbs2nms_report_load_balance_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(opt);
	if( !cdr.write_str(info.data, info.length) )
	{
		assert(0);
		return 0;
	}
	return cdr.get_curr_pos();
}

/* - * - */

aot_uint32_t 
nms2lgs_report_load_balance_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	opt = cdr.read_4();
	if( !cdr.read_str(&info) ) { assert(0); return 0; }

	return cdr.get_curr_pos();
}
aot_uint32_t 
nms2lgs_report_load_balance_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() ) { assert(0); return 0; }
	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(opt);
	if( !cdr.write_str(info.data, info.length) ) { assert(0); return 0; }
	return cdr.get_curr_pos();
}

/* - * - */

aot_uint32_t 
d2d_direct_transit_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	inner_id = cdr.read_4();
	unused0 = cdr.read_4();
	arry_size = cdr.read_4();
	if( arry_size == 0 )
	{
		arry = NULL;
	}
	else
	{
		this->arry = reinterpret_cast<void*>(cdr.next_rd_ptr());
		if( len < cdr_size() )
		{
			assert(0);
			this->arry = NULL;
			this->arry_size = 0;
		}
	}
	return cdr.get_curr_pos() + this->arry_size;
}
aot_uint32_t 
d2d_direct_transit_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(this->inner_id);
	cdr.write_4(this->unused0);
	cdr.write_4(this->arry_size);
	if( this->arry_size > 0 )
	{
		if( this->arry == NULL )
		{
			assert(0);
		}
		else
		{
			cdr.write_1_array(this->arry, this->arry_size);
		}
	}
	return cdr.get_curr_pos();
}

/* - * - */

aot_uint32_t 
tata2tata_direct_transit_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	from_ims_id = cdr.read_4();
	to_ims_id = cdr.read_4();
	flag = cdr.read_4();
	unused0 = cdr.read_4();
	arry_size = cdr.read_4();
	if( arry_size == 0 )
	{
		arry = NULL;
	}
	else
	{
		this->arry = reinterpret_cast<void*>(cdr.next_rd_ptr());
		if( len < cdr_size() )
		{
			assert(0);
			this->arry = NULL;
			this->arry_size = 0;
		}
	}
	return cdr.get_curr_pos() + this->arry_size;
}
aot_uint32_t 
tata2tata_direct_transit_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(from_ims_id);
	cdr.write_4(to_ims_id);
	cdr.write_4(this->flag);
	cdr.write_4(this->unused0);
	cdr.write_4(this->arry_size);
	if( this->arry_size > 0 )
	{
		if( this->arry == NULL )
		{
			assert(0);
		}
		else
		{
			cdr.write_1_array(this->arry, this->arry_size);
		}
	}
	return cdr.get_curr_pos();
}

/* - * - */

aot_uint32_t 
cluster_info_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < size )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	cluster_id = cdr.read_4();
	return cdr.get_curr_pos();
}
aot_uint32_t 
cluster_info_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < size )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(cluster_id);
	return cdr.get_curr_pos();
}

/* - * - */

aot_uint32_t 
notify_ss_ready_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < size )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	ss_addr.port = cdr.read_2();
	ss_addr.ip = cdr.read_4();
	return cdr.get_curr_pos();
}
aot_uint32_t 
notify_ss_ready_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < size )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_2(ss_addr.port);
	cdr.write_4(ss_addr.ip);
	return cdr.get_curr_pos();
}

/* - * - */
aot_uint32_t 
ss_info_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < 4 )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	aot_uint32_t n = cdr.read_4();

	for( aot_uint32_t i = 0; i < n; ++i )
	{
		cluster_info_t c;
		c.cdr_read(buf + cdr.get_curr_pos() + i * cluster_info_t::size, cluster_info_t::size);
		cluster_info.push_back(c);
	}
	return cdr.get_curr_pos() + n * cluster_info_t::size;
}
aot_uint32_t 
ss_info_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() )
	{
		assert(0);
		return 0;
	}

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4((aot_uint32_t)cluster_info.size());
	for( aot_uint32_t i = 0; i < cluster_info.size(); ++i )
	{
		cluster_info[i].cdr_write(buf + cdr.get_curr_pos() + i * cluster_info_t::size, cluster_info_t::size);
	}
	return cdr.get_curr_pos() + cluster_info.size() * cluster_info_t::size;
}
/* - * - */

void 
ent_broadcast_msg_t::change_cc_ims_id(char* buf, aot_uint32_t len, aot_uint32_t new_cc_ims_id)
{
	if( len < cdr_min_size() )
	{
		assert(0);
		return;
	}
	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf + 20, len - 20);
	cdr.write_4(new_cc_ims_id);
}
aot_uint32_t 
ent_broadcast_msg_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	this->ent_id = cdr.read_4();
	this->dep_id = cdr.read_4();
	this->from_tata_id = cdr.read_4();
	this->from_ims_id = cdr.read_4();		
	this->cc_tata_id = cdr.read_4();
	this->cc_ims_id = cdr.read_4();
	this->unused0 = cdr.read_4();
	this->unused = cdr.read_4();
	this->arry_size = cdr.read_4();
	if( this->arry_size == 0 )
	{
		this->arry = NULL;
	}
	else
	{
		this->arry = reinterpret_cast<void*>(cdr.next_rd_ptr());
		if( len < cdr_size() )
		{
			assert(0);
			this->arry = NULL;
			this->arry_size = 0;
		}
	}
	return cdr.get_curr_pos() + this->arry_size;
}
aot_uint32_t 
ent_broadcast_msg_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() )
	{
		assert(0);
		return 0;
	}

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(this->ent_id);
	cdr.write_4(this->dep_id);
	cdr.write_4(this->from_tata_id);
	cdr.write_4(this->from_ims_id);
	cdr.write_4(this->cc_tata_id);
	cdr.write_4(this->cc_ims_id);
	cdr.write_4(this->unused0);
	cdr.write_4(this->unused);
	cdr.write_4(this->arry_size);
	if( this->arry_size > 0 )
	{
		if( this->arry == NULL )
		{
			assert(0);
		}
		else
		{
			cdr.write_1_array(this->arry, this->arry_size);
		}
	}
	return cdr.get_curr_pos();
}
/* - * - */

aot_uint32_t 
get_ent_online_member_list_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < size )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	this->ent_id = cdr.read_4();
	this->dep_id = cdr.read_4();
	this->from_tata_id = cdr.read_4();
	this->from_ims_id = cdr.read_4();
	this->flag = cdr.read_4();
	return cdr.get_curr_pos();
}
aot_uint32_t 
get_ent_online_member_list_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < size )
	{
		assert(0);
		return 0;
	}

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(this->ent_id);
	cdr.write_4(this->dep_id);
	cdr.write_4(this->from_tata_id);
	cdr.write_4(this->from_ims_id);
	cdr.write_4(this->flag);
	return cdr.get_curr_pos();
}

/* - * - */

void 
tribe_broadcast_msg_t::change_cc_ims_id(char* buf, aot_uint32_t len, aot_uint32_t new_cc_ims_id)
{
	if( len < cdr_min_size() )
	{
		assert(0);
		return;
	}
	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf + 16, len - 16);
	cdr.write_4(new_cc_ims_id);
}
aot_uint32_t 
tribe_broadcast_msg_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	this->tribe_id = cdr.read_4();
	this->from_tata_id = cdr.read_4();
	this->from_ims_id = cdr.read_4();		
	this->cc_tata_id = cdr.read_4();
	this->cc_ims_id = cdr.read_4();
	this->unused0 = cdr.read_4();
	this->unused = cdr.read_4();
	this->arry_size = cdr.read_4();
	if( this->arry_size == 0 )
	{
		this->arry = NULL;
	}
	else
	{
		this->arry = reinterpret_cast<void*>(cdr.next_rd_ptr());
		if( len < cdr_size() )
		{
			assert(0);
			this->arry = NULL;
			this->arry_size = 0;
		}
	}
	return cdr.get_curr_pos() + this->arry_size;
}
aot_uint32_t 
tribe_broadcast_msg_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() )
	{
		assert(0);
		return 0;
	}

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(this->tribe_id);
	cdr.write_4(this->from_tata_id);
	cdr.write_4(this->from_ims_id);
	cdr.write_4(this->cc_tata_id);
	cdr.write_4(this->cc_ims_id);
	cdr.write_4(this->unused0);
	cdr.write_4(this->unused);
	cdr.write_4(this->arry_size);
	if( this->arry_size > 0 )
	{
		if( this->arry == NULL )
		{
			assert(0);
		}
		else
		{
			cdr.write_1_array(this->arry, this->arry_size);
		}
	}
	return cdr.get_curr_pos();
}

/* - * - */

aot_uint32_t 
get_tribe_online_member_list_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < size )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	this->tribe_id = cdr.read_4();
	this->from_tata_id = cdr.read_4();
	this->from_ims_id = cdr.read_4();
	return cdr.get_curr_pos();
}
aot_uint32_t 
get_tribe_online_member_list_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < size )
	{
		assert(0);
		return 0;
	}

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(this->tribe_id);
	cdr.write_4(this->from_tata_id);
	cdr.write_4(this->from_ims_id);
	return cdr.get_curr_pos();
}
/* - * - */

aot_uint32_t 
exchange_buddy_info_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	this->is_reply = cdr.read_1();

	aot_uint32_t n = this->mu.cdr_read(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	n += cdr.get_curr_pos();
	cdr.set_curr_pos(n);

	return cdr.get_curr_pos();
}
aot_uint32_t 
exchange_buddy_info_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() ) { assert(0); return 0; }

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_1(this->is_reply);

	aot_uint32_t n = this->mu.cdr_write(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	n += cdr.get_curr_pos();
	cdr.set_curr_pos(n);

	return cdr.get_curr_pos();
}

/* - * - */

aot_uint32_t 
t2t_user_define_data_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	this->data_type = cdr.read_4();
	this->unused = cdr.read_4();
	return cdr.get_curr_pos();
}
aot_uint32_t 
t2t_user_define_data_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() )
	{
		assert(0);
		return 0;
	}

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(this->data_type);
	cdr.write_4(this->unused);
	return cdr.get_curr_pos();
}

/* - * - */

aot_uint32_t 
tbm_user_define_data_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	this->data_type = cdr.read_4();
	this->unused = cdr.read_4();
	return cdr.get_curr_pos();
}
aot_uint32_t 
tbm_user_define_data_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() )
	{
		assert(0);
		return 0;
	}

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(this->data_type);
	cdr.write_4(this->unused);
	return cdr.get_curr_pos();
}

/* - * - */
/* - * - */
/* - * - */
aot_uint32_t 
sys_tbm_msg_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	this->serv_id = cdr.read_4();
	return cdr.get_curr_pos();
}
aot_uint32_t 
sys_tbm_msg_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() )
	{
		assert(0);
		return 0;
	}

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(this->serv_id);
	return cdr.get_curr_pos();
}

/* - * - */

aot_uint32_t 
sys_ebm_msg_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	this->serv_id = cdr.read_4();
	return cdr.get_curr_pos();
}
aot_uint32_t 
sys_ebm_msg_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() )
	{
		assert(0);
		return 0;
	}

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(this->serv_id);
	return cdr.get_curr_pos();
}

/* - * - */

aot_uint32_t 
broadcast_msg_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	from_tata_id = cdr.read_4();
	from_ims_id = cdr.read_4();
	arry_size = cdr.read_4();
	if( arry_size == 0 )
	{
		arry = NULL;
	}
	else
	{
		this->arry = reinterpret_cast<void*>(cdr.next_rd_ptr());
		if( len < cdr_size() )
		{
			assert(0);
			this->arry = NULL;
			this->arry_size = 0;
		}
	}
	return cdr.get_curr_pos() + this->arry_size;
}
aot_uint32_t 
broadcast_msg_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(from_tata_id);
	cdr.write_4(from_ims_id);
	cdr.write_4(this->arry_size);
	if( this->arry_size > 0 )
	{
		if( this->arry == NULL )
		{
			assert(0);
		}
		else
		{
			cdr.write_1_array(this->arry, this->arry_size);
		}
	}
	return cdr.get_curr_pos();
}

/* - * - */

aot_uint32_t 
nmc_regist_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	this->id = cdr.read_4();
	this->svc_type = cdr.read_4();
	if( !cdr.read_str(&this->user) )
	{
		assert(0);
		return 0;
	}
	if( !cdr.read_str(&this->pwd) )
	{
		assert(0);
		return 0;
	}
	return cdr.get_curr_pos();
}
aot_uint32_t 
nmc_regist_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() )
	{
		assert(0);
		return 0;
	}

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(this->id);
	cdr.write_4(this->svc_type);
	if( !cdr.write_str(this->user.data, this->user.length) )
	{
		assert(0);
		return 0;
	}
	if( !cdr.write_str(this->pwd.data, this->pwd.length) )
	{
		assert(0);
		return 0;
	}
	return cdr.get_curr_pos();
}

/* - * - */

aot_uint32_t 
lgc_regist_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	this->id = cdr.read_4();
	this->svc_type = cdr.read_4();
	if( !cdr.read_str(&this->user) )
	{
		assert(0);
		return 0;
	}
	if( !cdr.read_str(&this->pwd) )
	{
		assert(0);
		return 0;
	}
	return cdr.get_curr_pos();
}
aot_uint32_t 
lgc_regist_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() )
	{
		assert(0);
		return 0;
	}

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(this->id);
	cdr.write_4(this->svc_type);
	if( !cdr.write_str(this->user.data, this->user.length) )
	{
		assert(0);
		return 0;
	}
	if( !cdr.write_str(this->pwd.data, this->pwd.length) )
	{
		assert(0);
		return 0;
	}
	return cdr.get_curr_pos();
}

/* - * - */

aot_uint32_t 
lgs_get_login_config_info_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	if( !cdr.read_str(&this->call_id) )
	{
		assert(0);
		return 0;
	}
	if( !cdr.read_str(&this->sid) )
	{
		assert(0);
		return 0;
	}
	if( !cdr.read_str(&this->version) )
	{
		assert(0);
		return 0;
	}
	return cdr.get_curr_pos();
}
aot_uint32_t 
lgs_get_login_config_info_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() )
	{
		assert(0);
		return 0;
	}

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	if( !cdr.write_str(this->call_id.data, this->call_id.length) )
	{
		assert(0);
		return 0;
	}
	if( !cdr.write_str(this->sid.data, this->sid.length) )
	{
		assert(0);
		return 0;
	}
	if( !cdr.write_str(this->version.data, this->version.length) )
	{
		assert(0);
		return 0;
	}
	return cdr.get_curr_pos();
}

/* - * - */

aot_uint32_t 
feature_tata_login_regist_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);

	this->tata_id = cdr.read_4();
	if( !cdr.read_str(&this->tata_nbr) )
	{
		assert(0);
		return 0;
	}

	if( !cdr.read_str(&this->token) )
	{
		assert(0);
		return 0;
	}
	return cdr.get_curr_pos();
}
aot_uint32_t 
feature_tata_login_regist_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() )
	{
		assert(0);
		return 0;
	}

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(this->tata_id);
	if( !cdr.write_str(this->tata_nbr.data, this->tata_nbr.length) )
	{
		assert(0);
		return 0;
	}
	if( !cdr.write_str(this->token.data, this->token.length) )
	{
		assert(0);
		return 0;
	}
	return cdr.get_curr_pos();
}

/* - * - */

aot_uint32_t 
feature_tata_regist_out_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);

	this->tata_id = cdr.read_4();
	this->ims_id = cdr.read_4();
	if( !cdr.read_str(&this->tata_nbr) )
	{
		assert(0);
		return 0;
	}

	if( !cdr.read_str(&this->token) )
	{
		assert(0);
		return 0;
	}
	return cdr.get_curr_pos();
}
aot_uint32_t 
feature_tata_regist_out_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() )
	{
		assert(0);
		return 0;
	}

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(this->tata_id);
	cdr.write_4(this->ims_id);
	if( !cdr.write_str(this->tata_nbr.data, this->tata_nbr.length) )
	{
		assert(0);
		return 0;
	}
	if( !cdr.write_str(this->token.data, this->token.length) )
	{
		assert(0);
		return 0;
	}
	return cdr.get_curr_pos();
}

/* - * - */

aot_uint32_t 
confirm_receipt_ack_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	this->old_ex_code = cdr.read_4();
	this->old_busi_code = cdr.read_4();
	this->ack_ret = cdr.read_4();

	this->arry_size = cdr.read_4();
	if( this->arry_size == 0 )
	{
		this->arry = NULL;
	}
	else
	{
		this->arry = reinterpret_cast<void*>(cdr.next_rd_ptr());
		if( len < cdr_size() )
		{
			assert(0);
			this->arry = NULL;
			this->arry_size = 0;
		}
	}
	return cdr.get_curr_pos() + this->arry_size;
}
aot_uint32_t 
confirm_receipt_ack_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(this->old_ex_code);
	cdr.write_4(this->old_busi_code);
	cdr.write_4(this->ack_ret);

	cdr.write_4(this->arry_size);
	if( this->arry_size > 0 )
	{
		if( this->arry == NULL )
		{
			assert(0);
		}
		else
		{
			cdr.write_1_array(this->arry, this->arry_size);
		}
	}
	return cdr.get_curr_pos();
}

/* - * - */
aot_uint32_t 
ims2mps_undo_subscribe_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);

	aot_uint32_t n = this->mui.cdr_read(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	n += cdr.get_curr_pos();
	cdr.set_curr_pos(n);

	if( !cdr.read_str(&this->mps_device_guid) ){ assert(0); return 0; }
	if( !cdr.read_str(&this->mps_channel_group_key) ) { assert(0); return 0; }
	if( !cdr.read_str(&this->str_reserved) ) { assert(0); return 0; }

	return cdr.get_curr_pos();
}
aot_uint32_t 
ims2mps_undo_subscribe_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() ){ assert(0); return 0; }

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);

	aot_uint32_t n = this->mui.cdr_write(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	n += cdr.get_curr_pos();
	cdr.set_curr_pos(n);

	if( !cdr.write_str(&this->mps_device_guid) ){ assert(0); return 0; }
	if( !cdr.write_str(&this->mps_channel_group_key) ){ assert(0); return 0; }
	if( !cdr.write_str(&this->str_reserved) ){ assert(0); return 0; }

	return cdr.get_curr_pos();
}
/* - * - */
aot_uint32_t 
ims2mps_notify_tata_status_change_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);

	aot_uint32_t n = this->mui.cdr_read(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	n += cdr.get_curr_pos();
	cdr.set_curr_pos(n);

	if( !cdr.read_str(&this->mps_device_guid) ){ assert(0); return 0; }
	if( !cdr.read_str(&this->mps_channel_group_key) ) { assert(0); return 0; }
	if( !cdr.read_str(&this->str_reserved) ) { assert(0); return 0; }

	return cdr.get_curr_pos();
}
aot_uint32_t 
ims2mps_notify_tata_status_change_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() ){ assert(0); return 0; }

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);

	aot_uint32_t n = this->mui.cdr_write(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	n += cdr.get_curr_pos();
	cdr.set_curr_pos(n);

	if( !cdr.write_str(&this->mps_device_guid) ){ assert(0); return 0; }
	if( !cdr.write_str(&this->mps_channel_group_key) ){ assert(0); return 0; }
	if( !cdr.write_str(&this->str_reserved) ){ assert(0); return 0; }

	return cdr.get_curr_pos();
}
/* - * - */

aot_uint32_t 
tata2ims_subscribe_mps_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	this->id = cdr.read_4();
	this->login_type = cdr.read_2();

	if( !cdr.read_str(&this->nbr) ){ assert(0); return 0; }
	if( !cdr.read_str(&this->mps_device_guid) ) { assert(0); return 0; }
	if( !cdr.read_str(&this->mps_channel_group_key) ) { assert(0); return 0; }
	if( !cdr.read_str(&this->str_reserved) ) { assert(0); return 0; }

	return cdr.get_curr_pos();
}
aot_uint32_t 
tata2ims_subscribe_mps_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() ){ assert(0); return 0; }

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);

	cdr.write_4(this->id);
	cdr.write_2(this->login_type);

	if( !cdr.write_str(&this->nbr) ){ assert(0); return 0; }
	if( !cdr.write_str(&this->mps_device_guid) ){ assert(0); return 0; }
	if( !cdr.write_str(&this->mps_channel_group_key) ){ assert(0); return 0; }
	if( !cdr.write_str(&this->str_reserved) ){ assert(0); return 0; }

	return cdr.get_curr_pos();
}

/* - * - */

aot_uint32_t 
tata2ims_undo_subscribe_mps_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	this->id = cdr.read_4();
	this->login_type = cdr.read_2();

	if( !cdr.read_str(&this->nbr) ){ assert(0); return 0; }
	if( !cdr.read_str(&this->mps_device_guid) ) { assert(0); return 0; }
	if( !cdr.read_str(&this->mps_channel_group_key) ) { assert(0); return 0; }
	if( !cdr.read_str(&this->str_reserved) ) { assert(0); return 0; }

	return cdr.get_curr_pos();
}
aot_uint32_t 
tata2ims_undo_subscribe_mps_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() ){ assert(0); return 0; }

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);

	cdr.write_4(this->id);
	cdr.write_2(this->login_type);

	if( !cdr.write_str(&this->nbr) ){ assert(0); return 0; }
	if( !cdr.write_str(&this->mps_device_guid) ){ assert(0); return 0; }
	if( !cdr.write_str(&this->mps_channel_group_key) ){ assert(0); return 0; }
	if( !cdr.write_str(&this->str_reserved) ){ assert(0); return 0; }

	return cdr.get_curr_pos();
}
/* - * - */

aot_uint32_t 
MQUES_msg_key_info_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);

	this->id = cdr.read_4();
	this->tm = cdr.read_8();
	this->seq = cdr.read_8();

	if( !cdr.read_str(&this->msg_key) ){ assert(0); return 0; }

	return cdr.get_curr_pos();
}
aot_uint32_t 
MQUES_msg_key_info_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() ){ assert(0); return 0; }

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);

	cdr.write_4(this->id);
	cdr.write_8(this->tm);
	cdr.write_8(this->seq);

	if( !cdr.write_str(&this->msg_key) ){ assert(0); return 0; }

	return cdr.get_curr_pos();
}

/* - * - */
/* - * - */
/* - * - */
/* - * - */
aot_uint32_t 
MQUES_read_buddy_offline_msg_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);

	this->tata_id = cdr.read_4();
	this->max_count = cdr.read_4();
	this->max_len = cdr.read_4();
	this->unused = cdr.read_4();
	this->how_read = cdr.read_1();
	this->how_write = cdr.read_1();

	cdr.read_date_time(&this->tm);

	aot_uint32_t mki_cdr_len = this->msg_key_begin.cdr_read(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	if( mki_cdr_len == 0 ) { assert(0); return 0; }

	mki_cdr_len += cdr.get_curr_pos();
	cdr.set_curr_pos(mki_cdr_len);

	mki_cdr_len = this->msg_key_end.cdr_read(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	if( mki_cdr_len == 0 ) { assert(0); return 0; }

	mki_cdr_len += cdr.get_curr_pos();
	cdr.set_curr_pos(mki_cdr_len);

	return cdr.get_curr_pos();
}
aot_uint32_t 
MQUES_read_buddy_offline_msg_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() ){ assert(0); return 0; }

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);

	cdr.write_4(this->tata_id);
	cdr.write_4(this->max_count);
	cdr.write_4(this->max_len);
	cdr.write_4(this->unused);
	cdr.write_1(this->how_read);
	cdr.write_1(this->how_write);

	cdr.write_date_time(&this->tm);

	aot_uint32_t mki_cdr_len = this->msg_key_begin.cdr_write(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	if( mki_cdr_len == 0 ) { assert(0); return 0; }

	mki_cdr_len += cdr.get_curr_pos();
	cdr.set_curr_pos(mki_cdr_len);

	mki_cdr_len = this->msg_key_end.cdr_write(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	if( mki_cdr_len == 0 ) { assert(0); return 0; }

	mki_cdr_len += cdr.get_curr_pos();
	cdr.set_curr_pos(mki_cdr_len);

	return cdr.get_curr_pos();
}

/* - * - */
aot_uint32_t 
MQUES_send_buddy_offline_msg_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);

	this->tata_id = cdr.read_4();
	this->msg_count = cdr.read_4();
	this->msg_seq_begin = cdr.read_4();
	this->msg_seq_end = cdr.read_4();

	aot_uint32_t mki_cdr_len = this->next_msg_key.cdr_read(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	if( mki_cdr_len == 0 ) { assert(0); return 0; }

	mki_cdr_len += cdr.get_curr_pos();
	cdr.set_curr_pos(mki_cdr_len);

	return cdr.get_curr_pos();
}
aot_uint32_t 
MQUES_send_buddy_offline_msg_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() ){ assert(0); return 0; }

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);

	cdr.write_4(this->tata_id);
	cdr.write_4(this->msg_count);
	cdr.write_4(this->msg_seq_begin);
	cdr.write_4(this->msg_seq_end);

	aot_uint32_t mki_cdr_len = this->next_msg_key.cdr_write(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	if( mki_cdr_len == 0 ) { assert(0); return 0; }

	mki_cdr_len += cdr.get_curr_pos();
	cdr.set_curr_pos(mki_cdr_len);

	return cdr.get_curr_pos();
}
/* - * - */

aot_uint32_t 
MQUES_delete_buddy_offline_msg_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < size )
	{
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	this->tata_id = cdr.read_4();
	this->seq_begin = cdr.read_8();
	this->seq_end = cdr.read_8();

	return cdr.get_curr_pos();
}
aot_uint32_t 
MQUES_delete_buddy_offline_msg_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < size )
	{
		return 0;
	}
	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(this->tata_id);
	cdr.write_8(this->seq_begin);
	cdr.write_8(this->seq_end);

	return cdr.get_curr_pos();
}

/* - * - */

/* - * - */

aot_uint32_t 
MQUES_read_buddy_sync_msg_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);

	this->tata_id = cdr.read_4();
	this->buddy_id = cdr.read_4();
	this->max_count = cdr.read_4();
	this->max_len = cdr.read_4();
	this->unused = cdr.read_4();
	this->how_read = cdr.read_1();
	this->how_write = cdr.read_1();
	cdr.read_date_time(&this->tm);

	aot_uint32_t mki_cdr_len = this->msg_key_begin.cdr_read(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	if( mki_cdr_len == 0 ) { assert(0); return 0; }

	mki_cdr_len += cdr.get_curr_pos();
	cdr.set_curr_pos(mki_cdr_len);

	mki_cdr_len = this->msg_key_end.cdr_read(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	if( mki_cdr_len == 0 ) { assert(0); return 0; }

	mki_cdr_len += cdr.get_curr_pos();
	cdr.set_curr_pos(mki_cdr_len);

	return cdr.get_curr_pos();
}

aot_uint32_t 
MQUES_read_buddy_sync_msg_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() ){ assert(0); return 0; }

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);

	cdr.write_4(this->tata_id);
	cdr.write_4(this->buddy_id);
	cdr.write_4(this->max_count);
	cdr.write_4(this->max_len);
	cdr.write_4(this->unused);
	cdr.write_1(this->how_read);
	cdr.write_1(this->how_write);

	cdr.write_date_time(&this->tm);

	aot_uint32_t mki_cdr_len = this->msg_key_begin.cdr_write(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	if( mki_cdr_len == 0 ) { assert(0); return 0; }

	mki_cdr_len += cdr.get_curr_pos();
	cdr.set_curr_pos(mki_cdr_len);

	mki_cdr_len = this->msg_key_end.cdr_write(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	if( mki_cdr_len == 0 ) { assert(0); return 0; }

	mki_cdr_len += cdr.get_curr_pos();
	cdr.set_curr_pos(mki_cdr_len);

	return cdr.get_curr_pos();
}

/* - * - */

aot_uint32_t 
MQUES_send_buddy_sync_msg_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);

	this->tata_id = cdr.read_4();
	this->buddy_id = cdr.read_4();
	this->msg_count = cdr.read_4();

	aot_uint32_t mki_cdr_len = this->next_msg_key.cdr_read(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	if( mki_cdr_len == 0 ) { assert(0); return 0; }

	mki_cdr_len += cdr.get_curr_pos();
	cdr.set_curr_pos(mki_cdr_len);

	return cdr.get_curr_pos();
}
aot_uint32_t 
MQUES_send_buddy_sync_msg_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() ){ assert(0); return 0; }

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);

	cdr.write_4(this->tata_id);
	cdr.write_4(this->buddy_id);
	cdr.write_4(this->msg_count);

	aot_uint32_t mki_cdr_len = this->next_msg_key.cdr_write(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	if( mki_cdr_len == 0 ) { assert(0); return 0; }

	mki_cdr_len += cdr.get_curr_pos();
	cdr.set_curr_pos(mki_cdr_len);

	return cdr.get_curr_pos();
}


/* - * - */

aot_uint32_t MQUES_get_recent_contacts_buddylist_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < size ){
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	this->tata_id = cdr.read_4();
	return cdr.get_curr_pos();
}

aot_uint32_t MQUES_get_recent_contacts_buddylist_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < size )
	{
		assert(0);
		return 0;
	}

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(this->tata_id);
	return cdr.get_curr_pos();
}

/* - * - */

/* - * - */
aot_uint32_t 
MQUES_send_ent_dep_list_offline_msg_piece_t::cdr_read_ims_id(char* buf, aot_uint32_t len)
{ 
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf + 12, len);

	return cdr.read_4();
}

aot_uint32_t 
MQUES_send_ent_dep_list_offline_msg_piece_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);

	this->tata_id = cdr.read_4();
	this->ent_id = cdr.read_4();
	this->dep_id = cdr.read_4();
	this->ims_id = cdr.read_4();
	this->msg_count = cdr.read_4();

	return cdr.get_curr_pos();
}

aot_uint32_t 
MQUES_send_ent_dep_list_offline_msg_piece_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() ){ assert(0); return 0; }

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);

	cdr.write_4(this->tata_id);
	cdr.write_4(this->ent_id);
	cdr.write_4(this->dep_id);
	cdr.write_4(this->ims_id);
	cdr.write_4(this->msg_count);

	return cdr.get_curr_pos();
}
/* - * - */

/* - * - */
aot_uint32_t 
MQUES_send_ent_dep_list_offline_msg_t::cdr_read_ims_id(char* buf, aot_uint32_t len)
{ 
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf + 8, len);

	return cdr.read_4();
}

aot_uint32_t 
MQUES_send_ent_dep_list_offline_msg_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);

	this->tata_id = cdr.read_4();
	this->ent_id = cdr.read_4();
	this->ims_id = cdr.read_4();
	this->msg_count = cdr.read_4();

	return cdr.get_curr_pos();
}

aot_uint32_t 
MQUES_send_ent_dep_list_offline_msg_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() ){ assert(0); return 0; }

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);

	cdr.write_4(this->tata_id);
	cdr.write_4(this->ent_id);
	cdr.write_4(this->ims_id);
	cdr.write_4(this->msg_count);

	return cdr.get_curr_pos();
}
/* - * - */

aot_uint32_t 
MQUES_read_ent_dep_sync_msg_t::cdr_read_serv_id(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf + 12, len);

	return cdr.read_4();
}

aot_uint32_t 
MQUES_read_ent_dep_sync_msg_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);

	this->tata_id = cdr.read_4();
	this->ent_id = cdr.read_4();
	this->dep_id = cdr.read_4();
	this->serv_id = cdr.read_4();
	this->ims_id = cdr.read_4();
	this->max_count = cdr.read_4();
	this->max_len = cdr.read_4();
	this->unused = cdr.read_4();
	this->how_read = cdr.read_1();
	this->how_write = cdr.read_1();

	cdr.read_date_time(&this->tm);

	aot_uint32_t mki_cdr_len = this->msg_key_begin.cdr_read(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	if( mki_cdr_len == 0 ) { assert(0); return 0; }

	mki_cdr_len += cdr.get_curr_pos();
	cdr.set_curr_pos(mki_cdr_len);

	mki_cdr_len = this->msg_key_end.cdr_read(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	if( mki_cdr_len == 0 ) { assert(0); return 0; }

	mki_cdr_len += cdr.get_curr_pos();
	cdr.set_curr_pos(mki_cdr_len);

	return cdr.get_curr_pos();
}

aot_uint32_t 
MQUES_read_ent_dep_sync_msg_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() ){ assert(0); return 0; }

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);

	cdr.write_4(this->tata_id);
	cdr.write_4(this->ent_id);
	cdr.write_4(this->dep_id);
	cdr.write_4(this->serv_id);
	cdr.write_4(this->ims_id);
	cdr.write_4(this->max_count);
	cdr.write_4(this->max_len);
	cdr.write_4(this->unused);
	cdr.write_1(this->how_read);
	cdr.write_1(this->how_write);

	cdr.write_date_time(&this->tm);

	aot_uint32_t mki_cdr_len = this->msg_key_begin.cdr_write(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	if( mki_cdr_len == 0 ) { assert(0); return 0; }

	mki_cdr_len += cdr.get_curr_pos();
	cdr.set_curr_pos(mki_cdr_len);

	mki_cdr_len = this->msg_key_end.cdr_write(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	if( mki_cdr_len == 0 ) { assert(0); return 0; }

	mki_cdr_len += cdr.get_curr_pos();
	cdr.set_curr_pos(mki_cdr_len);

	return cdr.get_curr_pos();
}

/* - * - */
aot_uint32_t 
MQUES_send_ent_dep_sync_msg_t::cdr_read_ims_id(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf + 12, len);

	return cdr.read_4();
}

aot_uint32_t 
MQUES_send_ent_dep_sync_msg_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);

	this->tata_id = cdr.read_4();
	this->ent_id = cdr.read_4();
	this->dep_id = cdr.read_4();
	this->ims_id = cdr.read_4();
	this->msg_count = cdr.read_4();

	aot_uint32_t mki_cdr_len = this->next_msg_key.cdr_read(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	if( mki_cdr_len == 0 ) { assert(0); return 0; }

	mki_cdr_len += cdr.get_curr_pos();
	cdr.set_curr_pos(mki_cdr_len);

	return cdr.get_curr_pos();
}

aot_uint32_t 
MQUES_send_ent_dep_sync_msg_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() ){ assert(0); return 0; }

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);

	cdr.write_4(this->tata_id);
	cdr.write_4(this->ent_id);
	cdr.write_4(this->dep_id);
	cdr.write_4(this->ims_id);
	cdr.write_4(this->msg_count);

	aot_uint32_t mki_cdr_len = this->next_msg_key.cdr_write(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	if( mki_cdr_len == 0 ) { assert(0); return 0; }

	mki_cdr_len += cdr.get_curr_pos();
	cdr.set_curr_pos(mki_cdr_len);

	return cdr.get_curr_pos();
}

/* - * - ---------------*/
aot_uint32_t 
MQUES_read_tribe_sync_msg_t::cdr_read_serv_id(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf + 8, len);

	return cdr.read_4();
}

aot_uint32_t 
MQUES_read_tribe_sync_msg_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);

	this->tata_id = cdr.read_4();
	this->tribe_id = cdr.read_4();
	this->serv_id = cdr.read_4();
	this->ims_id = cdr.read_4();
	this->max_count = cdr.read_4();
	this->max_len = cdr.read_4();
	this->unused = cdr.read_4();
	this->how_read = cdr.read_1();
	this->how_write = cdr.read_1();

	cdr.read_date_time(&this->tm);

	aot_uint32_t mki_cdr_len = this->msg_key_begin.cdr_read(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	if( mki_cdr_len == 0 ) { assert(0); return 0; }

	mki_cdr_len += cdr.get_curr_pos();
	cdr.set_curr_pos(mki_cdr_len);

	mki_cdr_len = this->msg_key_end.cdr_read(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	if( mki_cdr_len == 0 ) { assert(0); return 0; }

	mki_cdr_len += cdr.get_curr_pos();
	cdr.set_curr_pos(mki_cdr_len);

	return cdr.get_curr_pos();
}

aot_uint32_t 
MQUES_read_tribe_sync_msg_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() ){ assert(0); return 0; }

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);

	cdr.write_4(this->tata_id);
	cdr.write_4(this->tribe_id);
	cdr.write_4(this->serv_id);
	cdr.write_4(this->ims_id);
	cdr.write_4(this->max_count);
	cdr.write_4(this->max_len);
	cdr.write_4(this->unused);
	cdr.write_1(this->how_read);
	cdr.write_1(this->how_write);

	cdr.write_date_time(&this->tm);

	aot_uint32_t mki_cdr_len = this->msg_key_begin.cdr_write(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	if( mki_cdr_len == 0 ) { assert(0); return 0; }

	mki_cdr_len += cdr.get_curr_pos();
	cdr.set_curr_pos(mki_cdr_len);

	mki_cdr_len = this->msg_key_end.cdr_write(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	if( mki_cdr_len == 0 ) { assert(0); return 0; }

	mki_cdr_len += cdr.get_curr_pos();
	cdr.set_curr_pos(mki_cdr_len);

	return cdr.get_curr_pos();
}

/* - * - */
aot_uint32_t 
MQUES_send_tribe_sync_msg_t::cdr_read_ims_id(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf + 8, len);

	return cdr.read_4();
}

aot_uint32_t 
MQUES_send_tribe_sync_msg_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);

	this->tata_id = cdr.read_4();
	this->tribe_id = cdr.read_4();
	this->ims_id = cdr.read_4();
	this->msg_count = cdr.read_4();

	aot_uint32_t mki_cdr_len = this->next_msg_key.cdr_read(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	if( mki_cdr_len == 0 ) { assert(0); return 0; }

	mki_cdr_len += cdr.get_curr_pos();
	cdr.set_curr_pos(mki_cdr_len);

	return cdr.get_curr_pos();
}

aot_uint32_t 
MQUES_send_tribe_sync_msg_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() ){ assert(0); return 0; }

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);

	cdr.write_4(this->tata_id);
	cdr.write_4(this->tribe_id);
	cdr.write_4(this->ims_id);
	cdr.write_4(this->msg_count);

	aot_uint32_t mki_cdr_len = this->next_msg_key.cdr_write(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	if( mki_cdr_len == 0 ) { assert(0); return 0; }

	mki_cdr_len += cdr.get_curr_pos();
	cdr.set_curr_pos(mki_cdr_len);

	return cdr.get_curr_pos();
}
/* - * - */
/* - * - */
aot_uint32_t 
MQUES_send_tribe_list_offline_msg_piece_t::cdr_read_ims_id(char* buf, aot_uint32_t len)
{ 
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf + 8, len);

	return cdr.read_4();
}

aot_uint32_t 
MQUES_send_tribe_list_offline_msg_piece_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);

	this->tata_id = cdr.read_4();
	this->tribe_id = cdr.read_4();
	this->ims_id = cdr.read_4();
	this->msg_count = cdr.read_4();

	return cdr.get_curr_pos();
}

aot_uint32_t 
MQUES_send_tribe_list_offline_msg_piece_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() ){ assert(0); return 0; }

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);

	cdr.write_4(this->tata_id);
	cdr.write_4(this->tribe_id);
	cdr.write_4(this->ims_id);
	cdr.write_4(this->msg_count);

	return cdr.get_curr_pos();
}
/* - * - */
/* - * - */
aot_uint32_t 
MQUES_send_tribe_list_offline_msg_t::cdr_read_ims_id(char* buf, aot_uint32_t len)
{ 
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf + 4, len);

	return cdr.read_4();
}

aot_uint32_t 
MQUES_send_tribe_list_offline_msg_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);

	this->tata_id = cdr.read_4();
	this->ims_id = cdr.read_4();
	this->msg_count = cdr.read_4();
	this->serv_id = cdr.read_4();

	return cdr.get_curr_pos();
}

aot_uint32_t 
MQUES_send_tribe_list_offline_msg_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() ){ assert(0); return 0; }

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);

	cdr.write_4(this->tata_id);
	cdr.write_4(this->ims_id);
	cdr.write_4(this->msg_count);
	cdr.write_4(this->serv_id);

	return cdr.get_curr_pos();
}
/* - * - */

/* - * - */
aot_uint32_t 
set_tata_runtime_mode_t::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);

	this->tata_id = cdr.read_4();
	this->mode = cdr.read_4();
	return cdr.get_curr_pos();
}

aot_uint32_t 
set_tata_runtime_mode_t::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() ){ assert(0); return 0; }

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);

	cdr.write_4(this->tata_id);
	cdr.write_4(this->mode);

	return cdr.get_curr_pos();
}
/* - * - */
/* - * - */
/* - * - */
/* - * - */
/* - * - */
/* - * - */
/* - * - */
/* - * - */
/* - * - */
/* - * - */
/* - * - */
/* - * - */
/* - * - */
/* - * - */
/* - * - */
/* - * - */
/* - * - */
/* - * - */
/* - * - */
/* - * - */
/* - * - */
/* - * - */
/* - * - */
/* - * - */
/* - * - */
/* - * - */
/* - * - */
/* - * - */
/* - * - */
/* - * - */
/* - * - */
/* - * - */
/* - * - */
/* - * - */
/* - * - */
/* - * - */
/* - * - */
/* - * - */
/* - * - */
/* - * - */

}} /// end namespace aot/prot