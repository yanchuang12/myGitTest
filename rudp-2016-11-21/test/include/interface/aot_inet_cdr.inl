//#include "stdafx.h"
//#include "aot_inet_cdr.h"

namespace aot{ namespace inet{
inline
cdr_reader::cdr_reader()
{
	recyle_i(NULL, 0);
}

inline
cdr_reader::~cdr_reader()
{
}

inline void 
cdr_reader::recyle_i(char* buf, aot_uint32_t size)
{
	this->buf_ = buf;
	this->buf_size_ = size;
	this->curr_pos_ = 0;
}

inline void 
cdr_reader::recyle()
{
	recyle_i(NULL, 0);
}

inline aot_uint8_t 
cdr_reader::read_1()
{
	bool b = ( this->buf_ && (this->curr_pos_ + 1 <= this->buf_size_) );
	assert( b );
	if( !b )
	{
		/// aot_log_error(AOT_LM_ALERT, "cdr_reader::read_1() out of range");
		return 0;
	}

	aot_uint8_t r = static_cast<aot_uint8_t>(this->buf_[this->curr_pos_]);
	++this->curr_pos_;
	return r;
}

inline aot_uint16_t 
cdr_reader::read_2(bool to_host_order/* = true*/)
{
	bool b = (this->buf_ && (this->curr_pos_ + 2 <= this->buf_size_) );
	assert( b );
	if( !b )
	{
		/// aot_log_error(AOT_LM_ALERT, "cdr_reader::read_2() out of range");
		return 0;
	}

	aot_uint16_t r = *(reinterpret_cast<aot_uint16_t*>(this->buf_ + this->curr_pos_));
	this->curr_pos_ += 2;
	return to_host_order ? ::ntohs(r) : r;
}

inline aot_uint32_t 
cdr_reader::read_4(bool to_host_order/* = true*/)
{
	bool b = ( this->buf_ && (this->curr_pos_ + 4 <= this->buf_size_) );
	assert( b );
	if( !b )
	{
		/// aot_log_error(AOT_LM_ALERT, "cdr_reader::read_4() out of range");
		return 0;
	}

	aot_uint32_t r = *(reinterpret_cast<aot_uint32_t*>(this->buf_ + this->curr_pos_));
	this->curr_pos_ += 4;
	return to_host_order ? ::ntohl(r) : r;
}

inline aot_uint64_t 
cdr_reader::read_8(bool to_host_order/* = true*/)
{
	bool b = ( this->buf_ && (this->curr_pos_ + 8 <= this->buf_size_) );
	assert( b );
	if( !b )
	{
		/// aot_log_error(AOT_LM_ALERT, "cdr_reader::read_8() out of range");
		return 0;
	}

	aot_uint64_t r = *(reinterpret_cast<aot_uint64_t*>(this->buf_ + this->curr_pos_));
	this->curr_pos_ += 8;
	return to_host_order ? __aot_ntohll(r) : r;
}

inline aot_nbrnum_t 
cdr_reader::read_nbrnum()
{
	return read_4();
}

inline aot_uint32_t 
cdr_reader::read_array_elem_num(bool to_host_order/* = true*/)
{
	return read_4(to_host_order);
}

inline bool 
cdr_reader::read_date_time(aot::inet::date_time_t* dt)
{
	dt->file_time = read_8();

	dt->year = read_2();
	dt->month = read_1();
	dt->day = read_1();
	dt->hour = read_1();
	dt->minute = read_1();
	dt->sec = read_1();
	return true;
}

inline bool 
cdr_reader::read_bin_data(aot::inet::aot_bin_data_t* s)
{
	s->length = read_array_elem_num(true);

	if( 0 == s->length )
	{
		s->data = NULL;
	}
	else
	{
		bool b = (this->curr_pos_ + s->length <= this->buf_size_);
		if( !b )
		{
			/// aot_log_error(AOT_LM_ALERT, "cdr_reader::read_str() out of range");
			assert(0);
			s->data = "";
			s->length = 0;
			return false;
		}
		s->data = this->buf_ + this->curr_pos_;

		if( s->data[s->length - 1] != '\0' )
		{
			assert(0);
			s->data = "";
			s->length = 0;
			return false;
		}

		this->curr_pos_ += s->length;
		/// 因为末尾多了个'\0'结束符,所以要将str的程度减一
		--s->length;
	}
	return true;
}

inline bool 
cdr_reader::skip_str()
{
	aot_uint32_t length = read_array_elem_num(true);
	this->curr_pos_ += length;
	if( this->curr_pos_ > this->buf_size_ )
	{
		assert(0);
		this->curr_pos_ -= length;
		return false;
	}
	return true;
}

inline bool 
cdr_reader::read_str(aot::inet::aot_string_t* s)
{
	s->length = read_array_elem_num(true);

	if( 0 == s->length )
	{
		s->data = "";
	}
	else
	{
		bool b = (this->curr_pos_ + s->length <= this->buf_size_);
		if( !b )
		{
			/// aot_log_error(AOT_LM_ALERT, "cdr_reader::read_str() out of range");
			assert(0);
			s->data = "";
			s->length = 0;
			return false;
		}
		s->data = this->buf_ + this->curr_pos_;

		if( s->data[s->length - 1] != '\0' )
		{
			assert(0);
			s->data = "";
			s->length = 0;
			return false;
		}

		this->curr_pos_ += s->length;
		/// 因为末尾多了个'\0'结束符,所以要将str的程度减一
		--s->length;
	}
	return true;
}

inline bool 
cdr_reader::read_str(std::string* s)
{
	*s = "";
	aot::inet::aot_string_t as;
	if( !read_str(&as) )
	{
		return false;
	}
	if( as.data && as.length > 0 )
	{
		*s = as.data;
	}
	return true;
}

inline bool 
cdr_reader::read_encode_data(aot::inet::aot_encode_data_t* p)
{
	p->length = read_array_elem_num(true);

	if( 0 == p->length )
	{
		p->data = NULL;
	}
	else
	{
		/// length(aot_uint32_t) + key(aot_uint8_t) + data(length 字节) + '\0'(aot_uint8_t)

		bool b = (this->curr_pos_ + 1 + p->length + 1 <= this->buf_size_);
		if( !b )
		{
			/// aot_log_error(AOT_LM_ALERT, "cdr_reader::read_str() out of range");
			assert(0);
			p->data = NULL;
			p->length = 0;
			return false;
		}
		p->key = read_1();
		p->data = this->buf_ + this->curr_pos_;

		if( p->data[p->length] != '\0' )
		{
			assert(0);
			p->data = NULL;
			p->length = 0;
			return false;
		}

		this->curr_pos_ += p->length;
		this->curr_pos_++; /// '\0'
	}
	return true;
}

inline bool 
cdr_reader::read_1_array(void* buf, aot_uint32_t elem_num)
{
	return read_array_i(buf, elem_num, 1, false);
}

inline bool 
cdr_reader::read_2_array(void* buf, aot_uint32_t elem_num, bool to_host_order)
{
	return read_array_i(buf, elem_num, 2, to_host_order);
}

inline bool 
cdr_reader::read_4_array(void* buf, aot_uint32_t elem_num, bool to_host_order)
{
	return read_array_i(buf, elem_num, 4, to_host_order);
}

inline bool 
cdr_reader::read_array_i(void* buf, aot_uint32_t elem_num, aot_uint32_t elem_len, bool to_host_order)
{
	if( elem_num == 0 )
		return true;

	aot_uint32_t len = elem_num * elem_len;
	bool b = ( this->curr_pos_ + len <= this->buf_size_ );
	assert( b );
	if( !b )
	{
		/// aot_log_error(AOT_LM_ALERT, "cdr_reader::read_array_i() out of range");
		return false;
	}

	if( !to_host_order )
	{
		memcpy(buf, this->buf_ + this->curr_pos_, len);
		this->curr_pos_ += len;
		return true;
	}
	/// convert to host byte order
	switch( elem_len )
	{
	case 1:
		{
			memcpy(buf, this->buf_ + this->curr_pos_, len);
			this->curr_pos_ += len;
			return true;
		}
		break;
	case 2:
		{
			aot_uint16_t v;
			aot_uint16_t* d1 = reinterpret_cast<aot_uint16_t*>(this->buf_ + this->curr_pos_);
			aot_uint16_t* d2 = reinterpret_cast<aot_uint16_t*>(buf);
			for( aot_uint32_t i = 0; i < elem_num; ++i, ++d1, ++d2 )
			{
				v = *d1;
				v = ::ntohs(v);
				*d2 = v;
			}
			this->curr_pos_ += len;
			return true;
		}
		break;
	case 4:
		{
			aot_uint32_t v;
			aot_uint32_t* d1 = reinterpret_cast<aot_uint32_t*>(this->buf_ + this->curr_pos_);
			aot_uint32_t* d2 = reinterpret_cast<aot_uint32_t*>(buf);
			for( aot_uint32_t i = 0; i < elem_num; ++i, ++d1, ++d2 )
			{
				v = *d1;
				v = ::ntohl(v);
				*d2 = v;
			}
			this->curr_pos_ += len;
			return true;
		}
		break;
	default:
		assert(false);
		/// aot_log_error(AOT_LM_ALERT, "cdr_reader::read_array_i() no surpport: to_host_order = true and elem_len = %d", elem_len);
		return false;
	}

	return false;
}

inline void 
cdr_reader::set_buf(char* buf, aot_uint32_t size)
{
	recyle_i(buf, size);
}

inline char* 
cdr_reader::get_buf()
{
	return this->buf_;
}

inline aot_uint32_t 
cdr_reader::get_curr_pos()
{
	return this->curr_pos_;
}

inline void 
cdr_reader::set_curr_pos(aot_uint32_t v)
{
	this->curr_pos_ = v;
}

inline char* 
cdr_reader::next_rd_ptr()
{
	return this->buf_ + this->curr_pos_;
}

/* - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - */
inline 
cdr_writer::cdr_writer()
{
	recyle_i(NULL, 0);
}

inline 
cdr_writer::~cdr_writer()
{
	;
}

inline bool 
cdr_writer::write_1(aot_uint8_t v)
{
	bool b = ( this->buf_ && (this->curr_pos_ + 1 <= this->buf_size_) );
	assert( b );
	if( !b )
	{
		/// aot_log_error(AOT_LM_ALERT, "cdr_writer::write_1() out of range");
		return false;
	}

	*(reinterpret_cast<aot_uint8_t*>(this->buf_ + this->curr_pos_)) = v;
	++this->curr_pos_;
	return true;
}

inline bool 
cdr_writer::write_2(aot_uint16_t v, bool to_net_order/* = true*/)
{
	bool b = ( this->buf_ && (this->curr_pos_ + 2 <= this->buf_size_) );
	assert( b );
	if( !b )
	{
		/// aot_log_error(AOT_LM_ALERT, "cdr_writer::write_2() out of range");
		return false;
	}

	if( to_net_order )
	{
		v = ::htons(v);
	}

	*(reinterpret_cast<aot_uint16_t*>(this->buf_ + this->curr_pos_)) = v;
	this->curr_pos_ += 2;
	return true;
}

inline bool 
cdr_writer::write_4(aot_uint32_t v, bool to_net_order/* = true*/)
{
	bool b = ( this->buf_ && (this->curr_pos_ + 4 <= this->buf_size_) );
	assert( b );
	if( !b )
	{
		/// aot_log_error(AOT_LM_ALERT, "cdr_writer::write_4() out of range");
		return false;
	}

	if( to_net_order )
	{
		v = ::htonl(v);
	}

	*(reinterpret_cast<aot_uint32_t*>(this->buf_ + this->curr_pos_)) = v;
	this->curr_pos_ += 4;
	return true;
}

inline bool 
cdr_writer::write_8(aot_uint64_t v, bool to_net_order/* = true*/)
{
	bool b = ( this->buf_ && (this->curr_pos_ + 8 <= this->buf_size_) );
	assert( b );
	if( !b )
	{
		/// aot_log_error(AOT_LM_ALERT, "cdr_writer::write_4() out of range");
		return false;
	}

	if( to_net_order )
	{
		v = __aot_htonll(v);
	}

	*(reinterpret_cast<aot_uint64_t*>(this->buf_ + this->curr_pos_)) = v;
	this->curr_pos_ += 8;
	return true;
}

inline bool 
cdr_writer::write_nbrnum(aot_nbrnum_t v)
{
	return write_4(v);
}

inline bool 
cdr_writer::write_str(const char* buf, aot_uint32_t len)
{
	if( 0 == len || NULL == buf )
	{
		return write_array_elem_num(0);
	}

	if( !write_array_elem_num(len + 1) )
		return false;
	
	if( !write_array_i(buf, len, 1, false) )
		return false;
	return write_1(0);
}

inline bool 
cdr_writer::write_str(aot::inet::aot_string_t* s)
{
	return write_str(s->data, s->length);
}
inline bool 
cdr_writer::write_str(std::string* s)
{
	return write_str(s->c_str(), (aot_uint32_t)s->length());
}

inline bool 
cdr_writer::write_bin_data(aot::inet::aot_bin_data_t* s)
{
	if( s->length == 0 || NULL == s->data )
	{
		return write_array_elem_num(0);
	}

	if( !write_array_elem_num(s->length + 1) )
		return false;

	if( !write_array_i(s->data, s->length, 1, false) )
		return false;
	return write_1(0);
}

inline bool 
cdr_writer::write_date_time(aot::inet::date_time_t* dt)
{
	write_8(dt->file_time);

	write_2(dt->year);
	write_1(dt->month);
	write_1(dt->day);
	write_1(dt->hour);
	write_1(dt->minute);
	write_1(dt->sec);
	return true;
}

inline bool 
cdr_writer::write_encode_data(aot::inet::aot_encode_data_t* p)
{
	if( 0 == p->length || NULL == p->data )
	{
		return write_array_elem_num(0);
	}
	/// length(aot_uint32_t) + key(aot_uint8_t) + data(length 字节) + '\0'(aot_uint8_t) 

	/// 先写入数据长度(不含key, 不含末尾'\0')
	if( !write_array_elem_num(p->length) )
		return false;

	/// 写入 key
	if( !write_1(p->key) )
		return false;

	/// 写入数据
	if( !write_array_i(p->data, p->length, 1, false) )
		return false;

	/// 末尾写入'\0', 如果原数据是string的话, 可以高效的decode后,直接操作该string
	return write_1(0);
}

inline bool 
cdr_writer::write_array_elem_num(aot_uint32_t elem_num, bool to_net_order/* = true*/)
{
	return write_4(elem_num, to_net_order);
}

inline bool 
cdr_writer::write_1_array(const void* buf, aot_uint32_t elem_num)
{
	return write_array_i(buf, elem_num, 1, false);
}

inline bool 
cdr_writer::write_2_array(const void* buf, aot_uint32_t elem_num, bool to_net_order/* = true*/)
{
	return write_array_i(buf, elem_num, 2, to_net_order);
}

inline bool 
cdr_writer::write_4_array(const void* buf, aot_uint32_t elem_num, bool to_net_order/* = true*/)
{
	return write_array_i(buf, elem_num, 4, to_net_order);
}

inline void 
cdr_writer::recyle()
{
	recyle_i(NULL, 0);
}

inline void 
cdr_writer::set_buf(char* buf, aot_uint32_t size)
{
	recyle_i(buf, size);
}

inline char* 
cdr_writer::get_buf()
{
	return this->buf_;
}

inline aot_uint32_t 
cdr_writer::get_curr_pos()
{
	return this->curr_pos_;
}

inline void 
cdr_writer::set_curr_pos(aot_uint32_t v)
{
	this->curr_pos_ = v;
}

inline char* 
cdr_writer::next_wr_ptr()
{
	return this->buf_ + this->curr_pos_;
}

inline void 
cdr_writer::recyle_i(char* buf, aot_uint32_t size)
{
	this->buf_ = buf;
	this->buf_size_ = size;
	this->curr_pos_ = 0;
}

inline bool 
cdr_writer::write_array_i(const void* buf, aot_uint32_t elem_num, aot_uint32_t elem_len, bool to_net_order)
{
	if( elem_num == 0 )
	{
		return true;
	}

	aot_uint32_t len = elem_num * elem_len;
	bool b = ( this->curr_pos_ + len <= this->buf_size_ );
	assert( b );
	if( !b )
	{
		/// aot_log_error(AOT_LM_ALERT, "cdr_reader::write_array_i() out of range");
		return false;
	}	

	if( !to_net_order )
	{
		memcpy(this->buf_ + this->curr_pos_, buf, len);
		this->curr_pos_ += len;
		return true;
	}
	/// convert to network byte order
	switch( elem_len )
	{
	case 1:
		{
			memcpy(this->buf_ + this->curr_pos_, buf, len);
			this->curr_pos_ += len;
			return true;
		}
		break;
	case 2:
		{
			aot_uint16_t v;
			aot_uint16_t* d1 = reinterpret_cast<aot_uint16_t*>(this->buf_ + this->curr_pos_);
			const aot_uint16_t* d2 = reinterpret_cast<const aot_uint16_t*>(buf);
			for( aot_uint32_t i = 0; i < elem_num; ++i, ++d1, ++d2 )
			{
				v = *d2;
				v = ::htons(v);
				*d1 = v;
			}
			this->curr_pos_ += len;
			return true;
		}
		break;
	case 4:
		{
			aot_uint32_t v;
			aot_uint32_t* d1 = reinterpret_cast<aot_uint32_t*>(this->buf_ + this->curr_pos_);
			const aot_uint32_t* d2 = reinterpret_cast<const aot_uint32_t*>(buf);
			for( aot_uint32_t i = 0; i < elem_num; ++i, ++d1, ++d2 )
			{
				v = *d2;
				v = ::htonl(v);
				*d1 = v;
			}
			this->curr_pos_ += len;
			return true;
		}
		break;
	default:
		assert(false);
		/// aot_log_error(AOT_LM_ALERT, "cdr_reader::write_array_i() no surpport: to_net_order = true and elem_len = %d", elem_len);
		return false;
	}

	return false;
}

}} /// end namespace aot/inet