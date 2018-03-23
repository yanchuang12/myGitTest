
namespace aot{ namespace prot{
/* - * - */

template<class ALLOC>
void
mini_user_status_list_t<ALLOC>::clear()
{
	if( this->need_destroy_ )
	{
		list_type::iterator it = this->lst.goto_begin();
		for( ; !this->lst.is_end(it); it = this->lst.goto_next(it) )
		{
			mini_user_status_t* mu = this->lst.get_val(it);
			if( mu )
			{
				dealloc_mini_user_status(this->alloc_, mu);
			}
		}
	}
	this->lst.clear();
}

template<class ALLOC>
aot_uint32_t
mini_user_status_list_t<ALLOC>::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	aot_uint32_t cnt = cdr.read_4();
	if( cnt == 0 ){ return cdr.get_curr_pos(); }

	for( aot_uint32_t i = 0; i < cnt; ++i )
	{
		mini_user_status_t* mu = alloc_mini_user_status(this->alloc_);
		if( NULL == mu ) { assert(0); return 0; }

		aot_uint32_t n = mu->cdr_read(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
		if( n == 0 ) { assert(0); dealloc_mini_user_status(this->alloc_, mu); return 0; }

		n += cdr.get_curr_pos();
		cdr.set_curr_pos(n);

		this->lst.push_back(mu);
	}

	return cdr.get_curr_pos();
}

template<class ALLOC>
aot_uint32_t
mini_user_status_list_t<ALLOC>::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() ) { assert(0); return 0; }

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(this->lst.size());

	list_type::iterator it = this->lst.goto_begin();
	for( ; !this->lst.is_end(it); it = this->lst.goto_next(it) )
	{
		mini_user_status_t* mu = this->lst.get_val(it);
		if( NULL == mu ) { assert(0); return 0; }

		aot_uint32_t n = mu->cdr_write(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
		if( n == 0 ) { assert(0); return 0; }

		n += cdr.get_curr_pos();
		cdr.set_curr_pos(n);
	}
	return cdr.get_curr_pos();
}
/* - * - */

template<class ALLOC>
void mini_user_info_list_t<ALLOC>::clear()
{
	if( this->need_destroy_ )
	{
		list_type::iterator it = this->lst.goto_begin();
		for( ; !this->lst.is_end(it); it = this->lst.goto_next(it) )
		{
			mini_user_info_t* mu = this->lst.get_val(it);
			if( mu )
			{
				dealloc_mini_user_info(this->alloc_, mu);
			}
		}
	}
	this->lst.clear();
}

template<class ALLOC>
aot_uint32_t mini_user_info_list_t<ALLOC>::cdr_size()
{
	aot_uint32_t n = 4;
	list_type::iterator it = this->lst.goto_begin();
	for( ; !this->lst.is_end(it); it = this->lst.goto_next(it) )
	{
		mini_user_info_t* mu = this->lst.get_val(it);
		if( NULL == mu ) { assert(0); continue;}
		n += 1 + mu->cdr_size() + 1;
	}
	return n;
}

template<class ALLOC>
aot_uint32_t mini_user_info_list_t<ALLOC>::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	aot_uint32_t cnt = cdr.read_4();
	if( cnt == 0 ){ return cdr.get_curr_pos(); }

	aot_uint8_t c;
	for( aot_uint32_t i = 0; i < cnt; ++i )
	{
		c = cdr.read_1();
		if( c != __MU_LIST_S__ ){ assert(0); return 0; }

		mini_user_info_t* mu = alloc_mini_user_info(this->alloc_);
		if( NULL == mu ) { assert(0); return 0; }

		aot_uint32_t n = mu->cdr_read(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
		if( n == 0 ) { assert(0); dealloc_mini_user_info(this->alloc_, mu); return 0; }

		n += cdr.get_curr_pos();
		cdr.set_curr_pos(n);

		c = cdr.read_1();
		if( c != __MU_LIST_E__ ) { assert(0); dealloc_mini_user_info(this->alloc_, mu); return 0; }

		this->lst.push_back(mu);
	}

	return cdr.get_curr_pos();
}

template<class ALLOC>
aot_uint32_t mini_user_info_list_t<ALLOC>::cdr_write(char* buf, aot_uint32_t len, aot_uint32_t cdr_len = 0 ) /// @cdr_len: 传这个参数的目的是避免重复计算, lst数量多时,重复计算可能会影响性能
{
#if (0)
	if( cdr_len == 0 ){ cdr_len = cdr_size();}
	if( len < cdr_len ) { assert(0); return 0; }
#else
	if( len < cdr_min_size() ) { assert(0); return 0; } /// 这里暂不计算cdr 的准确长度
#endif

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(this->lst.size());

	list_type::iterator it = this->lst.goto_begin();
	for( ; !this->lst.is_end(it); it = this->lst.goto_next(it) )
	{
		mini_user_info_t* mu = this->lst.get_val(it);
		if( NULL == mu ) { assert(0); return 0; }

		cdr.write_1((aot_uint8_t)__MU_LIST_S__);
		aot_uint32_t n = mu->cdr_write(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
		if( n == 0 ) { assert(0); return 0; }

		n += cdr.get_curr_pos();
		cdr.set_curr_pos(n);

		if( !cdr.write_1((aot_uint8_t)__MU_LIST_E__) ) { assert(0); return 0; }
	}
	return cdr.get_curr_pos();
}
/* - * - */
template<class ALLOC>
aot_uint32_t get_online_buddy_list_ret_t<ALLOC>::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	this->tata_id = cdr.read_4();
	this->ims_id = cdr.read_4();
	this->unused = cdr.read_4();

	aot_uint32_t n = this->lst.cdr_read(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	if( n == 0 ) { assert(0); return 0; }

	n += cdr.get_curr_pos();
	cdr.set_curr_pos(n);

	return cdr.get_curr_pos();
}
template<class ALLOC>
aot_uint32_t get_online_buddy_list_ret_t<ALLOC>::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() ) { assert(0); return 0; }
	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(this->tata_id);
	cdr.write_4(this->ims_id);
	cdr.write_4(this->unused);

	aot_uint32_t n = this->lst.cdr_write(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	if( n == 0 ) { assert(0); return 0; }

	n += cdr.get_curr_pos();
	cdr.set_curr_pos(n);

	return cdr.get_curr_pos();
}
template<class ALLOC>
aot_uint32_t get_online_buddy_list_ret_t<ALLOC>::cdr_write(char* buf, aot_uint32_t len, aot_std::vector<mini_user_info_t*, ALLOC, false>* mui_lst)
{
	aot_uint32_t cnt = mui_lst->size();

	if( len < cdr_size(mui_lst) ) { assert(0); return 0; }
	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(this->tata_id);
	cdr.write_4(this->ims_id);
	cdr.write_4(this->unused);

	/// write list ...
	cdr.write_4(cnt);

	mini_user_info_t* mui = NULL;
	mini_user_status_t us;
	aot_uint32_t n;
	for( aot_uint32_t i = 0; i < cnt; ++i )
	{
		mui = (*mui_lst)[i];
		us.assign(mui);
		n = us.cdr_write(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
		if( n == 0 ) { assert(0); return 0; }

		n += cdr.get_curr_pos();
		cdr.set_curr_pos(n);
	}
	return cdr.get_curr_pos();
}
/* - * - */

template<class ALLOC>
aot_uint32_t broadcast_status_t<ALLOC>::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot_uint32_t n = mus.cdr_read(buf, len);
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf + n, len - n);

	if( !cdr.read_str(&guid) ) { assert(0); return 0; }

	aot_uint32_t i = cdr.read_4();
	for( aot_uint32_t k = 0; k < i; ++k )
	{
		to_lst.push_back(cdr.read_4());
	}
	return n + cdr.get_curr_pos();
}
template<class ALLOC>
aot_uint32_t broadcast_status_t<ALLOC>::cdr_write(char* buf, aot_uint32_t len)
{	
	if( len < cdr_size() ) { assert(0); return 0; }

	aot_uint32_t n = mus.cdr_write(buf, len);

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf + n, len - n);

	if( !cdr.write_str(&guid) ) { assert(0); return 0; }

	cdr.write_4(to_lst.size());

	aot_std::list<aot_uint32_t, ALLOC>::iterator it = to_lst.goto_begin();
	for( ; !to_lst.is_end(it); it = to_lst.goto_next(it) )
	{
		cdr.write_4(to_lst.get_val(it));
	}
	return n + cdr.get_curr_pos();
}

/* - * - */

template<class ALLOC>
aot_uint32_t user_tribe_info_t<ALLOC>::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() )
	{
		assert(0);
		return 0;
	}
	aot_uint32_t i;
	aot::inet::cdr_reader cdr;
	group_pair_t gp;

	cdr.set_buf(buf, len);

	aot_uint32_t n = cdr.read_4();
	this->tribe_lst.init( n + 2 );			/// 预留几个元素的位置,因为用户可能会参加群组
	for( i = 0; i < n; ++i )
	{
		gp.group_id = cdr.read_4();
		gp.serv_id = cdr.read_4();
		this->tribe_lst.push_back(gp);
	}
	return cdr.get_curr_pos();
}
template<class ALLOC>
aot_uint32_t user_tribe_info_t<ALLOC>::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() )
	{
		assert(0);
		return 0;
	}

	aot_uint32_t i;
	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);

	aot_uint32_t n = this->tribe_lst.size();
	cdr.write_4(n);
	for( i = 0; i < n; ++i )
	{
		cdr.write_4(this->tribe_lst[i].group_id);
		cdr.write_4(this->tribe_lst[i].serv_id);
	}
	return cdr.get_curr_pos();
}

/* - * - */

template<class ALLOC>
aot_uint32_t user_ent_info_t<ALLOC>::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() )
	{
		assert(0);
		return 0;
	}
	aot_uint32_t i;
	aot::inet::cdr_reader cdr;
	group_pair_t gp;

	cdr.set_buf(buf, len);
	this->ent_id = cdr.read_4();
	this->ent_serv_id = cdr.read_4();
	this->ent_total_member_size = cdr.read_4();

	aot_uint32_t n = cdr.read_4();
	this->ent_dep_lst.init( n + 2 );		/// 预留几个元素的位置,因为用户可能会参加群组
	for( i = 0; i < n; ++i )
	{
		gp.group_id = cdr.read_4();
		gp.serv_id = cdr.read_4();
		this->ent_dep_lst.push_back(gp);
	}

	return cdr.get_curr_pos();
}
template<class ALLOC>
aot_uint32_t user_ent_info_t<ALLOC>::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() )
	{
		assert(0);
		return 0;
	}

	aot_uint32_t i;
	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);

	cdr.write_4(this->ent_id);
	cdr.write_4(this->ent_serv_id);
	cdr.write_4(this->ent_total_member_size);

	aot_uint32_t n = this->ent_dep_lst.size();
	cdr.write_4(n);
	for( i = 0; i < n; ++i )
	{
		cdr.write_4(this->ent_dep_lst[i].group_id);
		cdr.write_4(this->ent_dep_lst[i].serv_id);
	}
	return cdr.get_curr_pos();
}

/* - * - */

template<class ALLOC>
aot_uint32_t get_ent_online_member_list_ret_t<ALLOC>::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	this->ent_id = cdr.read_4();
	this->dep_id = cdr.read_4();
	this->from_tata_id = cdr.read_4();
	this->from_ims_id = cdr.read_4();
	this->flag = cdr.read_4();

	aot_uint32_t n = this->member_lst.cdr_read(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	if( n == 0 ) { assert(0); return 0; }

	n += cdr.get_curr_pos();
	cdr.set_curr_pos(n);

	return cdr.get_curr_pos();
}
template<class ALLOC>
aot_uint32_t get_ent_online_member_list_ret_t<ALLOC>::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() ) { assert(0); return 0; }

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(this->ent_id);
	cdr.write_4(this->dep_id);
	cdr.write_4(this->from_tata_id);
	cdr.write_4(this->from_ims_id);
	cdr.write_4(this->flag);

	aot_uint32_t n = this->member_lst.cdr_write(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	if( n == 0 ) { assert(0); return 0; }

	n += cdr.get_curr_pos();
	cdr.set_curr_pos(n);

	return cdr.get_curr_pos();
}
template<class ALLOC>
aot_uint32_t get_ent_online_member_list_ret_t<ALLOC>::cdr_write(char* buf, aot_uint32_t len, aot_std::vector<mini_user_info_t*, ALLOC, false>* mui_lst)
{
	if( len < cdr_size(mui_lst) ) { assert(0); return 0; }

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(this->ent_id);
	cdr.write_4(this->dep_id);
	cdr.write_4(this->from_tata_id);
	cdr.write_4(this->from_ims_id);
	cdr.write_4(this->flag);

	/// write list ...
	aot_uint32_t cnt = mui_lst->size();
	cdr.write_4(cnt);

	mini_user_info_t* mui = NULL;
	mini_user_status_t us;
	aot_uint32_t n;
	for( aot_uint32_t i = 0; i < cnt; ++i )
	{
		mui = (*mui_lst)[i];
		us.assign(mui);
		n = us.cdr_write(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
		if( n == 0 ) { assert(0); return 0; }

		n += cdr.get_curr_pos();
		cdr.set_curr_pos(n);
	}
	return cdr.get_curr_pos();
}

/* - * - */

template<class ALLOC>
aot_uint32_t get_tribe_online_member_list_ret_t<ALLOC>::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	this->tribe_id = cdr.read_4();
	this->from_tata_id = cdr.read_4();
	this->from_ims_id = cdr.read_4();

	aot_uint32_t n = this->member_lst.cdr_read(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	if( n == 0 ) { assert(0); return 0; }

	n += cdr.get_curr_pos();
	cdr.set_curr_pos(n);

	return cdr.get_curr_pos();
}
template<class ALLOC>
aot_uint32_t get_tribe_online_member_list_ret_t<ALLOC>::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(this->tribe_id);
	cdr.write_4(this->from_tata_id);
	cdr.write_4(this->from_ims_id);

	aot_uint32_t n = this->member_lst.cdr_write(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	if( n == 0 ) { assert(0); return 0; }

	n += cdr.get_curr_pos();
	cdr.set_curr_pos(n);

	return cdr.get_curr_pos();
}
template<class ALLOC>
aot_uint32_t get_tribe_online_member_list_ret_t<ALLOC>::cdr_write(char* buf, aot_uint32_t len, aot_std::vector<mini_user_info_t*, ALLOC, false>* mui_lst)
{
	if( len < cdr_size(mui_lst) ) { assert(0); return 0; }

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(this->tribe_id);
	cdr.write_4(this->from_tata_id);
	cdr.write_4(this->from_ims_id);

	/// write list ...
	aot_uint32_t cnt = mui_lst->size();
	cdr.write_4(cnt);

	mini_user_info_t* mui = NULL;
	mini_user_status_t us;
	aot_uint32_t n;
	for( aot_uint32_t i = 0; i < cnt; ++i )
	{
		mui = (*mui_lst)[i];
		us.assign(mui);
		n = us.cdr_write(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
		if( n == 0 ) { assert(0); return 0; }

		n += cdr.get_curr_pos();
		cdr.set_curr_pos(n);
	}
	return cdr.get_curr_pos();
}

/* - * - */

template<class ALLOC>
aot_uint32_t subscribe_msg_t<ALLOC>::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	this->ims_id = cdr.read_4();
	aot_uint32_t n = cdr.read_4();
	for( aot_uint32_t i = 0; i < n; ++i )
	{
		this->lst.push_back(cdr.read_4());
	}
	return cdr.get_curr_pos();
}
template<class ALLOC>
aot_uint32_t subscribe_msg_t<ALLOC>::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() )
	{
		assert(0);
		return 0;
	}

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(this->ims_id);
	aot_uint32_t n = this->lst.size();
	cdr.write_4(n);
	for( aot_uint32_t i = 0; i < n; ++i )
	{
		cdr.write_4(this->lst[i]);
	}
	return cdr.get_curr_pos();
}

/* - * - */

template<class ALLOC>
aot_uint32_t buddy_list_t<ALLOC>::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	aot_uint32_t n = cdr.read_4();
	lst.init(n + 3); /// 预留几个
	for( aot_uint32_t i = 0; i < n; ++i )
	{
		this->lst.push_back(cdr.read_4());
	}
	return cdr.get_curr_pos();
}
template<class ALLOC>
aot_uint32_t buddy_list_t<ALLOC>::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() )
	{
		assert(0);
		return 0;
	}

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	aot_uint32_t n = this->lst.size();
	cdr.write_4(n);
	for( aot_uint32_t i = 0; i < n; ++i )
	{
		cdr.write_4(this->lst[i]);
	}
	return cdr.get_curr_pos();
}

/* - * - */
/* - * - */
/* - * - */

template<class ALLOC>
aot_uint32_t multicast_msg_t<ALLOC>::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() )
	{
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);

	this->from_tata_id = cdr.read_4();
	this->from_tata_ims = cdr.read_4();

	aot_uint32_t i = cdr.read_4();
	this->to_lst.init(i+2);
	for( aot_uint32_t k = 0; k < i; ++k )
	{
		this->to_lst.push_back(cdr.read_4());
	}
	return cdr.get_curr_pos();
}
template<class ALLOC>
aot_uint32_t multicast_msg_t<ALLOC>::cdr_write(char* buf, aot_uint32_t len)
{	
	if( len < cdr_size() )
	{
		assert(0);
		return 0;
	}

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);

	cdr.write_4(this->from_tata_id);
	cdr.write_4(this->from_tata_ims);

	aot_uint32_t n = this->to_lst.size();
	cdr.write_4(n);

	for( aot_uint32_t i = 0; i < n; ++i )
	{
		cdr.write_4(this->to_lst[i]);
	}
	return cdr.get_curr_pos();
}

/* - * - */

template<class ALLOC>
aot_uint32_t get_userlist_status_t<ALLOC>::cdr_read(char* buf, aot_uint32_t len)
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
	this->is_buddy = cdr.read_1();
	this->unused = cdr.read_4();

	aot_uint32_t n = cdr.read_4();
	lst.init(n + 3); /// 预留几个
	for( aot_uint32_t i = 0; i < n; ++i )
	{
		this->lst.push_back(cdr.read_4());
	}
	return cdr.get_curr_pos();
}
template<class ALLOC>
aot_uint32_t get_userlist_status_t<ALLOC>::cdr_write(char* buf, aot_uint32_t len)
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
	cdr.write_1(this->is_buddy);
	cdr.write_4(this->unused);

	aot_uint32_t n = this->lst.size();
	cdr.write_4(n);
	for( aot_uint32_t i = 0; i < n; ++i )
	{
		cdr.write_4(this->lst[i]);
	}
	return cdr.get_curr_pos();
}

/* - * - */

template<class ALLOC>
aot_uint32_t get_userlist_status_ret_t<ALLOC>::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	this->tata_id = cdr.read_4();
	this->ims_id = cdr.read_4();
	this->is_buddy = cdr.read_1();
	this->unused = cdr.read_4();

	aot_uint32_t n = this->lst.cdr_read(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	if( n == 0 ) { assert(0); return 0; }

	n += cdr.get_curr_pos();
	cdr.set_curr_pos(n);

	return cdr.get_curr_pos();
}
template<class ALLOC>
aot_uint32_t get_userlist_status_ret_t<ALLOC>::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() ) { assert(0); return 0; }
	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(this->tata_id);
	cdr.write_4(this->ims_id);
	cdr.write_1(this->is_buddy);
	cdr.write_4(this->unused);

	aot_uint32_t n = this->lst.cdr_write(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	if( n == 0 ) { assert(0); return 0; }

	n += cdr.get_curr_pos();
	cdr.set_curr_pos(n);

	return cdr.get_curr_pos();
}
template<class ALLOC>
aot_uint32_t get_userlist_status_ret_t<ALLOC>::cdr_write(char* buf, aot_uint32_t len, aot_std::vector<mini_user_info_t*, ALLOC, false>* mui_lst)
{
	aot_uint32_t cnt = mui_lst->size();

	if( len < cdr_size(mui_lst) ) { assert(0); return 0; }
	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(this->tata_id);
	cdr.write_4(this->ims_id);
	cdr.write_1(this->is_buddy);
	cdr.write_4(this->unused);

	/// write list ...
	cdr.write_4(cnt);

	mini_user_info_t* mui = NULL;
	mini_user_status_t us;
	aot_uint32_t n;
	for( aot_uint32_t i = 0; i < cnt; ++i )
	{
		mui = (*mui_lst)[i];
		us.assign(mui);
		n = us.cdr_write(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
		if( n == 0 ) { assert(0); return 0; }

		n += cdr.get_curr_pos();
		cdr.set_curr_pos(n);
	}
	return cdr.get_curr_pos();
}

/* - * - */

template<class ALLOC>
aot_uint32_t ims2mps_subscribe_t<ALLOC>::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	group_pair_t gp;
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);

	aot_uint32_t n = this->mui.cdr_read(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	n += cdr.get_curr_pos();
	cdr.set_curr_pos(n);

	this->ent_id = cdr.read_4();
	this->ent_serv_id = cdr.read_4();
	this->flag = cdr.read_4();

	if( !cdr.read_str(&this->mps_device_guid) ){ assert(0); return 0; }
	if( !cdr.read_str(&this->mps_channel_group_key) ) { assert(0); return 0; }
	if( !cdr.read_str(&this->str_reserved) ) { assert(0); return 0; }

	aot_uint32_t i = 0;

	n = (aot_uint32_t)cdr.read_4();
	this->tribe_lst.init( n + 2 );

	for( i = 0; i < n; ++i )
	{
		gp.group_id = cdr.read_4();
		gp.serv_id = cdr.read_4();
		this->tribe_lst.push_back(gp);
	}

	n = (aot_uint32_t)cdr.read_4();
	this->ent_dep_lst.init( n + 2 );
	for( i = 0; i < n; ++i )
	{
		gp.group_id = cdr.read_4();
		gp.serv_id = cdr.read_4();
		this->ent_dep_lst.push_back(gp);
	}

	return cdr.get_curr_pos();
}
template<class ALLOC>
aot_uint32_t ims2mps_subscribe_t<ALLOC>::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() ){ assert(0); return 0; }

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);

	aot_uint32_t n = this->mui.cdr_write(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
	n += cdr.get_curr_pos();
	cdr.set_curr_pos(n);

	cdr.write_4(this->ent_id);
	cdr.write_4(this->ent_serv_id);
	cdr.write_4(this->flag);

	if( !cdr.write_str(&this->mps_device_guid) ){ assert(0); return 0; }
	if( !cdr.write_str(&this->mps_channel_group_key) ){ assert(0); return 0; }
	if( !cdr.write_str(&this->str_reserved) ){ assert(0); return 0; }

	aot_uint32_t i = 0;

	n = (aot_uint32_t)this->tribe_lst.size();
	cdr.write_4(n);

	for( i = 0; i < n; ++i )
	{
		cdr.write_4(this->tribe_lst[i].group_id);
		cdr.write_4(this->tribe_lst[i].serv_id);
	}

	n = (aot_uint32_t)this->ent_dep_lst.size();
	cdr.write_4(n);
	for( i = 0; i < n; ++i )
	{
		cdr.write_4(this->ent_dep_lst[i].group_id);
		cdr.write_4(this->ent_dep_lst[i].serv_id);
	}

	return cdr.get_curr_pos();
}

/* - * - */


/* - * - */



/* - * - */

template<class ALLOC>
aot_uint32_t MQUES_get_recent_contacts_buddylist_ret_t<ALLOC>::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ){
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);

	this->tata_id = cdr.read_4();

	aot_uint32_t n = cdr.read_4();
	lst.init(n + 3); /// 预留几个
	
	for( aot_uint32_t i = 0; i < n; ++i )
	{
		MQUES_msg_key_info_t info;

		aot_uint32_t mki_cdr_len = info.cdr_read(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
		if( mki_cdr_len == 0 ) { assert(0); return 0; }

		mki_cdr_len += cdr.get_curr_pos();
		cdr.set_curr_pos(mki_cdr_len);
		
		this->lst.push_back(info);
	}
	return cdr.get_curr_pos();
}

template<class ALLOC>
aot_uint32_t MQUES_get_recent_contacts_buddylist_ret_t<ALLOC>::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() )
	{
		assert(0);
		return 0;
	}

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);

	cdr.write_4(this->tata_id);

	aot_uint32_t n = this->lst.size();
	cdr.write_4(n);
	for( aot_uint32_t i = 0; i < n; ++i )
	{
		MQUES_msg_key_info_t& info = this->lst[i];
		aot_uint32_t mki_cdr_len = info.cdr_write(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
		if( mki_cdr_len == 0 ) { assert(0); return 0; }

		mki_cdr_len += cdr.get_curr_pos();
		cdr.set_curr_pos(mki_cdr_len);
	}
	return cdr.get_curr_pos();
}


/* - * - */

template<class ALLOC>
aot_uint32_t MQUES_read_buddylist_last_sync_msg_t<ALLOC>::cdr_size()
{ 
	aot_uint32_t r = 4 + aot::inet::date_time_t::size + 4 + 4;
	aot_uint32_t n = this->lst.size();
	aot_uint32_t i = 0;
	for( i = 0; i < n; ++i )
	{
		r += this->lst[i].cdr_size();
	}

	n = this->end_lst.size();
	for( i = 0; i < n; ++i )
	{
		r += this->end_lst[i].cdr_size();
	}

	return r;
}

template<class ALLOC>
aot_uint32_t MQUES_read_buddylist_last_sync_msg_t<ALLOC>::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ){
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);

	this->tata_id = cdr.read_4();

	cdr.read_date_time(&this->tm);

	aot_uint32_t i = 0;
	aot_uint32_t n = cdr.read_4();
	this->lst.init(n + 2);
	for( i = 0; i < n; ++i )
	{
		MQUES_msg_key_info_t info;
		aot_uint32_t li_cdr_len = info.cdr_read(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
		if( li_cdr_len == 0 ) { assert(0); return 0; }

		li_cdr_len += cdr.get_curr_pos();
		cdr.set_curr_pos(li_cdr_len);
		this->lst.push_back(info);
	}

	n = cdr.read_4();
	this->end_lst.init(n + 2);
	for( i = 0; i < n; ++i )
	{
		MQUES_msg_key_info_t info;
		aot_uint32_t li_cdr_len = info.cdr_read(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
		if( li_cdr_len == 0 ) { assert(0); return 0; }

		li_cdr_len += cdr.get_curr_pos();
		cdr.set_curr_pos(li_cdr_len);
		this->end_lst.push_back(info);
	}

	return cdr.get_curr_pos();
}

template<class ALLOC>
aot_uint32_t MQUES_read_buddylist_last_sync_msg_t<ALLOC>::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() )
	{
		assert(0);
		return 0;
	}

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);

	cdr.write_4(this->tata_id);

	cdr.write_date_time(&this->tm);

	aot_uint32_t i = 0;
	aot_uint32_t n = this->lst.size();
	cdr.write_4(n);
	for( i = 0; i < n; ++i )
	{
		MQUES_msg_key_info_t& info = this->lst[i];
		aot_uint32_t li_cdr_len = info.cdr_write(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
		if( li_cdr_len == 0 ) { assert(0); return 0; }

		li_cdr_len += cdr.get_curr_pos();
		cdr.set_curr_pos(li_cdr_len);
	}

	n = this->end_lst.size();
	cdr.write_4(n);
	for( i = 0; i < n; ++i )
	{
		MQUES_msg_key_info_t& info = this->end_lst[i];
		aot_uint32_t li_cdr_len = info.cdr_write(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
		if( li_cdr_len == 0 ) { assert(0); return 0; }

		li_cdr_len += cdr.get_curr_pos();
		cdr.set_curr_pos(li_cdr_len);
	}

	return cdr.get_curr_pos();
}

/* - * - */

template<class ALLOC>
aot_uint32_t MQUES_send_buddylist_last_sync_msg_t<ALLOC>::cdr_size()
{ 
	aot_uint32_t r = 8 + 4;
	aot_uint32_t n = this->lst.size();
	for( aot_uint32_t i = 0; i < n; ++i )
	{
		r += this->lst[i].cdr_size();
	}
	return r;
}

template<class ALLOC>
aot_uint32_t MQUES_send_buddylist_last_sync_msg_t<ALLOC>::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() )
	{
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);
	this->tata_id = cdr.read_4();
	this->msg_count = cdr.read_4();

	aot_uint32_t n = cdr.read_4();
	lst.init(n + 2); /// 预留几个
	for( aot_uint32_t i = 0; i < n; ++i )
	{
		MQUES_chat_record_info_t info;
		info.id = cdr.read_4();
		info.sync_msg_count = cdr.read_2();
		info.offline_msg_count = cdr.read_2();
		this->lst.push_back(info);
	}

	return cdr.get_curr_pos();
}

template<class ALLOC>
aot_uint32_t MQUES_send_buddylist_last_sync_msg_t<ALLOC>::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() )
	{
		return 0;
	}
	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);
	cdr.write_4(this->tata_id);
	cdr.write_4(this->msg_count);

	aot_uint32_t n = this->lst.size();
	cdr.write_4(n);
	for( aot_uint32_t i = 0; i < n; ++i )
	{
		MQUES_chat_record_info_t& info = this->lst[i];
		cdr.write_4(info.id);
		cdr.write_2(info.sync_msg_count);
		cdr.write_2(info.offline_msg_count);
	}

	return cdr.get_curr_pos();
}

/* - * - */

template<class ALLOC>
aot_uint32_t MQUES_read_ent_dep_list_last_sync_msg_t<ALLOC>::cdr_size()
{ 
	aot_uint32_t r = 16 + aot::inet::date_time_t::size + 4 + 4;
	aot_uint32_t i = 0;
	aot_uint32_t n = this->lst.size();
	for( i = 0; i < n; ++i )
	{
		r += this->lst[i].cdr_size();
	}

	n = this->end_lst.size();
	for( i = 0; i < n; ++i )
	{
		r += this->end_lst[i].cdr_size();
	}
	return r;
}

template<class ALLOC>
aot_uint32_t MQUES_read_ent_dep_list_last_sync_msg_t<ALLOC>::cdr_read_serv_id(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ){
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf + 8, len);

	return cdr.read_4();
}

template<class ALLOC>
aot_uint32_t MQUES_read_ent_dep_list_last_sync_msg_t<ALLOC>::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ){
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);

	this->tata_id = cdr.read_4();
	this->ent_id = cdr.read_4();
	this->serv_id = cdr.read_4();
	this->ims_id = cdr.read_4();

	cdr.read_date_time(&this->tm);

	aot_uint32_t i = 0;
	aot_uint32_t n = cdr.read_4();
	this->lst.init(n + 2); /// 预留几个
	for( i = 0; i < n; ++i )
	{
		MQUES_msg_key_info_t info;
		aot_uint32_t li_cdr_len = info.cdr_read(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
		if( li_cdr_len == 0 ) { assert(0); return 0; }

		li_cdr_len += cdr.get_curr_pos();
		cdr.set_curr_pos(li_cdr_len);
		this->lst.push_back(info);
	}

	n = cdr.read_4();
	this->end_lst.init(n + 2); /// 预留几个
	for( i = 0; i < n; ++i )
	{
		MQUES_msg_key_info_t info;
		aot_uint32_t li_cdr_len = info.cdr_read(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
		if( li_cdr_len == 0 ) { assert(0); return 0; }

		li_cdr_len += cdr.get_curr_pos();
		cdr.set_curr_pos(li_cdr_len);
		this->end_lst.push_back(info);
	}

	return cdr.get_curr_pos();
}

template<class ALLOC>
aot_uint32_t MQUES_read_ent_dep_list_last_sync_msg_t<ALLOC>::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() )
	{
		assert(0);
		return 0;
	}

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);

	cdr.write_4(this->tata_id);
	cdr.write_4(this->ent_id);
	cdr.write_4(this->serv_id);
	cdr.write_4(this->ims_id);

	cdr.write_date_time(&this->tm);

	aot_uint32_t i = 0;
	aot_uint32_t n = this->lst.size();
	cdr.write_4(n);
	for( i = 0; i < n; ++i )
	{
		MQUES_msg_key_info_t& info = this->lst[i];
		aot_uint32_t li_cdr_len = info.cdr_write(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
		if( li_cdr_len == 0 ) { assert(0); return 0; }

		li_cdr_len += cdr.get_curr_pos();
		cdr.set_curr_pos(li_cdr_len);
	}

	n = this->end_lst.size();
	cdr.write_4(n);
	for( i = 0; i < n; ++i )
	{
		MQUES_msg_key_info_t& info = this->end_lst[i];
		aot_uint32_t li_cdr_len = info.cdr_write(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
		if( li_cdr_len == 0 ) { assert(0); return 0; }

		li_cdr_len += cdr.get_curr_pos();
		cdr.set_curr_pos(li_cdr_len);
	}

	return cdr.get_curr_pos();
}

/* - * - */

template<class ALLOC>
aot_uint32_t MQUES_send_ent_dep_list_last_sync_msg_t<ALLOC>::cdr_size()
{ 
	aot_uint32_t r = 16 + 4;
	aot_uint32_t n = this->lst.size();
	for( aot_uint32_t i = 0; i < n; ++i )
	{
		r += this->lst[i].cdr_size();
	}
	return r;
}

template<class ALLOC>
aot_uint32_t MQUES_send_ent_dep_list_last_sync_msg_t<ALLOC>::cdr_read_ims_id(char* buf, aot_uint32_t len)
{ 
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf + 8, len);

	return cdr.read_4();
}

template<class ALLOC>
aot_uint32_t MQUES_send_ent_dep_list_last_sync_msg_t<ALLOC>::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);

	this->tata_id = cdr.read_4();
	this->ent_id = cdr.read_4();
	this->ims_id = cdr.read_4();
	this->msg_count = cdr.read_4();

	aot_uint32_t n = cdr.read_4();
	lst.init(n + 2); /// 预留几个
	for( aot_uint32_t i = 0; i < n; ++i )
	{
		MQUES_chat_record_info_t info;
		info.id = cdr.read_4();
		info.sync_msg_count = cdr.read_2();
		info.offline_msg_count = cdr.read_2();
		this->lst.push_back(info);
	}

	return cdr.get_curr_pos();
}
template<class ALLOC>
aot_uint32_t MQUES_send_ent_dep_list_last_sync_msg_t<ALLOC>::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ){ assert(0); return 0; }

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);

	cdr.write_4(this->tata_id);
	cdr.write_4(this->ent_id);
	cdr.write_4(this->ims_id);
	cdr.write_4(this->msg_count);

	aot_uint32_t n = this->lst.size();
	cdr.write_4(n);
	for( aot_uint32_t i = 0; i < n; ++i )
	{
		MQUES_chat_record_info_t& info = this->lst[i];
		cdr.write_4(info.id);
		cdr.write_2(info.sync_msg_count);
		cdr.write_2(info.offline_msg_count);
	}

	return cdr.get_curr_pos();
}

/* - * - */

template<class ALLOC>
aot_uint32_t MQUES_read_ent_dep_list_offline_msg_t<ALLOC>::cdr_read_serv_id(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ){
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf + 8, len);

	return cdr.read_4();
}

template<class ALLOC>
aot_uint32_t MQUES_read_ent_dep_list_offline_msg_t<ALLOC>::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ){
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);

	this->tata_id = cdr.read_4();
	this->ent_id = cdr.read_4();
	this->serv_id = cdr.read_4();
	this->ims_id = cdr.read_4();
	this->how_read = cdr.read_1();
	this->how_write = cdr.read_1();

	cdr.read_date_time(&this->tm);

	aot_uint32_t n = cdr.read_4();
	lst.init(n + 2); /// 预留几个
	for( aot_uint32_t i = 0; i < n; ++i )
	{
		this->lst.push_back(cdr.read_4());
	}
	return cdr.get_curr_pos();
}

template<class ALLOC>
aot_uint32_t MQUES_read_ent_dep_list_offline_msg_t<ALLOC>::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() )
	{
		assert(0);
		return 0;
	}

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);

	cdr.write_4(this->tata_id);
	cdr.write_4(this->ent_id);
	cdr.write_4(this->serv_id);
	cdr.write_4(this->ims_id);
	cdr.write_1(this->how_read);
	cdr.write_1(this->how_write);

	cdr.write_date_time(&this->tm);

	aot_uint32_t n = this->lst.size();
	cdr.write_4(n);
	for( aot_uint32_t i = 0; i < n; ++i )
	{
		cdr.write_4(this->lst[i]);
	}
	return cdr.get_curr_pos();
}

/* - * - */

/* - * - ----------------------*/
template<class ALLOC>
aot_uint32_t MQUES_read_tribe_list_last_sync_msg_t<ALLOC>::cdr_size()
{ 
	aot_uint32_t r = 12 + aot::inet::date_time_t::size + 4 + 4;
	aot_uint32_t i = 0;
	aot_uint32_t n = this->lst.size();
	for( i = 0; i < n; ++i )
	{
		r += this->lst[i].cdr_size();
	}

	n = this->end_lst.size();
	for( i = 0; i < n; ++i )
	{
		r += this->end_lst[i].cdr_size();
	}

	return r;
}

template<class ALLOC>
aot_uint32_t MQUES_read_tribe_list_last_sync_msg_t<ALLOC>::cdr_read_serv_id(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ){
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf + 4, len);

	return cdr.read_4();
}

template<class ALLOC>
aot_uint32_t MQUES_read_tribe_list_last_sync_msg_t<ALLOC>::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ){
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);

	this->tata_id = cdr.read_4();
	this->serv_id = cdr.read_4();
	this->ims_id = cdr.read_4();

	cdr.read_date_time(&this->tm);

	aot_uint32_t i = 0;
	aot_uint32_t n = cdr.read_4();
	this->lst.init(n + 2); /// 预留几个
	for( i = 0; i < n; ++i )
	{
		MQUES_msg_key_info_t info;
		aot_uint32_t li_cdr_len = info.cdr_read(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
		if( li_cdr_len == 0 ) { assert(0); return 0; }

		li_cdr_len += cdr.get_curr_pos();
		cdr.set_curr_pos(li_cdr_len);
		this->lst.push_back(info);
	}

	n = cdr.read_4();
	this->end_lst.init(n + 2); /// 预留几个
	for( i = 0; i < n; ++i )
	{
		MQUES_msg_key_info_t info;
		aot_uint32_t li_cdr_len = info.cdr_read(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
		if( li_cdr_len == 0 ) { assert(0); return 0; }

		li_cdr_len += cdr.get_curr_pos();
		cdr.set_curr_pos(li_cdr_len);
		this->end_lst.push_back(info);
	}

	return cdr.get_curr_pos();
}

template<class ALLOC>
aot_uint32_t MQUES_read_tribe_list_last_sync_msg_t<ALLOC>::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() )
	{
		assert(0);
		return 0;
	}

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);

	cdr.write_4(this->tata_id);
	cdr.write_4(this->serv_id);
	cdr.write_4(this->ims_id);

	cdr.write_date_time(&this->tm);

	aot_uint32_t i = 0;
	aot_uint32_t n = this->lst.size();
	cdr.write_4(n);
	for( i = 0; i < n; ++i )
	{
		MQUES_msg_key_info_t& info = this->lst[i];
		aot_uint32_t li_cdr_len = info.cdr_write(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
		if( li_cdr_len == 0 ) { assert(0); return 0; }

		li_cdr_len += cdr.get_curr_pos();
		cdr.set_curr_pos(li_cdr_len);
	}

	n = this->end_lst.size();
	cdr.write_4(n);
	for( i = 0; i < n; ++i )
	{
		MQUES_msg_key_info_t& info = this->end_lst[i];
		aot_uint32_t li_cdr_len = info.cdr_write(buf + cdr.get_curr_pos(), len - cdr.get_curr_pos());
		if( li_cdr_len == 0 ) { assert(0); return 0; }

		li_cdr_len += cdr.get_curr_pos();
		cdr.set_curr_pos(li_cdr_len);
	}

	return cdr.get_curr_pos();
}

/* - * - */

template<class ALLOC>
aot_uint32_t MQUES_send_tribe_list_last_sync_msg_t<ALLOC>::cdr_size()
{ 
	aot_uint32_t r = 16 + 4;
	aot_uint32_t n = this->lst.size();
	for( aot_uint32_t i = 0; i < n; ++i )
	{
		r += this->lst[i].cdr_size();
	}
	return r;
}
template<class ALLOC>
aot_uint32_t MQUES_send_tribe_list_last_sync_msg_t<ALLOC>::cdr_read_ims_id(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf + 4, len - 4);

	return cdr.read_4();
}

template<class ALLOC>
aot_uint32_t MQUES_send_tribe_list_last_sync_msg_t<ALLOC>::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ) { assert(0); return 0; }

	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);

	this->tata_id = cdr.read_4();
	this->ims_id = cdr.read_4();
	this->msg_count = cdr.read_4();
	this->serv_id = cdr.read_4();

	aot_uint32_t n = cdr.read_4();
	lst.init(n + 2); /// 预留几个
	for( aot_uint32_t i = 0; i < n; ++i )
	{
		MQUES_chat_record_info_t info;
		info.id = cdr.read_4();
		info.sync_msg_count = cdr.read_2();
		info.offline_msg_count = cdr.read_2();
		this->lst.push_back(info);
	}

	return cdr.get_curr_pos();
}
template<class ALLOC>
aot_uint32_t MQUES_send_tribe_list_last_sync_msg_t<ALLOC>::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ){ assert(0); return 0; }

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);

	cdr.write_4(this->tata_id);
	cdr.write_4(this->ims_id);
	cdr.write_4(this->msg_count);
	cdr.write_4(this->serv_id);

	aot_uint32_t n = this->lst.size();
	cdr.write_4(n);
	for( aot_uint32_t i = 0; i < n; ++i )
	{
		MQUES_chat_record_info_t& info = this->lst[i];
		cdr.write_4(info.id);
		cdr.write_2(info.sync_msg_count);
		cdr.write_2(info.offline_msg_count);
	}

	return cdr.get_curr_pos();
}
/* - * - */

template<class ALLOC>
aot_uint32_t MQUES_read_tribe_list_offline_msg_t<ALLOC>::cdr_read_serv_id(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ){
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf + 4, len);

	return cdr.read_4();
}

template<class ALLOC>
aot_uint32_t MQUES_read_tribe_list_offline_msg_t<ALLOC>::cdr_read(char* buf, aot_uint32_t len)
{
	if( len < cdr_min_size() ){
		assert(0);
		return 0;
	}
	aot::inet::cdr_reader cdr;
	cdr.set_buf(buf, len);

	this->tata_id = cdr.read_4();
	this->serv_id = cdr.read_4();
	this->ims_id = cdr.read_4();
	this->how_read = cdr.read_1();
	this->how_write = cdr.read_1();

	cdr.read_date_time(&this->tm);

	aot_uint32_t n = cdr.read_4();
	lst.init(n + 2); /// 预留几个
	for( aot_uint32_t i = 0; i < n; ++i )
	{
		this->lst.push_back(cdr.read_4());
	}
	return cdr.get_curr_pos();
}

template<class ALLOC>
aot_uint32_t MQUES_read_tribe_list_offline_msg_t<ALLOC>::cdr_write(char* buf, aot_uint32_t len)
{
	if( len < cdr_size() )
	{
		assert(0);
		return 0;
	}

	aot::inet::cdr_writer cdr;
	cdr.set_buf(buf, len);

	cdr.write_4(this->tata_id);
	cdr.write_4(this->serv_id);
	cdr.write_4(this->ims_id);
	cdr.write_1(this->how_read);
	cdr.write_1(this->how_write);

	cdr.write_date_time(&this->tm);

	aot_uint32_t n = this->lst.size();
	cdr.write_4(n);
	for( aot_uint32_t i = 0; i < n; ++i )
	{
		cdr.write_4(this->lst[i]);
	}
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