/** Copyright (c) 2015-2016
 * All rights reserved.
 * 
 * �ļ�����:	aot_hashmap.h  
 * ժ	 Ҫ:	�������ݽṹ��װ,�ȱ�׼����.��
 * 
 * ��ǰ�汾��	1.0
 * ��	 ��:	������(FTT)
 * ��	 ��:	�½�
 * �������:	2015��05��11��
*/

#ifndef __AOT_STD_HASHMAP_201505114_H__
#define __AOT_STD_HASHMAP_201505114_H__

#include "aot_std_typedef.h"
#include "aot_alloc.h"
#include "aot_pair.h"

namespace aot_std{

/// BKDR�㷨: 1
inline aot_uint_t BKDR_hash(const char *str)
{
	aot_uint_t seed = 131; // 31 131 1313 13131 131313 etc..
	aot_uint_t hash = 0;

	while( *str )
	{
		hash = hash * seed + (*str++);
	}

	return (hash & 0x7FFFFFFF);
}

/// AP�㷨
inline aot_uint_t AP_hash(const char *str)
{
	aot_uint_t hash = 0;
	int i;

	for( i = 0; *str; i++ )
	{
		if( (i & 1) == 0 ) {
			hash ^= ((hash << 7) ^ (*str++) ^ (hash >> 3));
		}
		else{
			hash ^= (~((hash << 11) ^ (*str++) ^ (hash >> 5)));
		}
	}

	return (hash & 0x7FFFFFFF);
}

template<class KEY, class VAL>
struct hash_map_node_t
{
	hash_map_node_t() : next(NULL)
	{
		;
	}
	hash_map_node_t(const KEY& k, const VAL& v) : next(NULL), key(k), val(v)
	{
		;
	}
	hash_map_node_t*	next;
	VAL		val;
	KEY		key;
};


/*	
	�����ṩ����Ĭ�Ͽɹ�ѡ��ķ���:
	1. struct __default_hashmap_fun_group_t:		��ʹ���ڴ��: __alloc_node() �� __dealloc_node()
	2. struct __default_pool_hashmap_fun_group_t:	ʹ��Ĭ�ϵ��ڴ��: __alloc_node_with_pool() �� __dealloc_node_with_pool()
		a. ��ʹ��free����������(��Ϊhash_map�ڲ�����ֻ�з��� node_t ����, ��������ڴ��������Ԥ����ȹ���)
		b. ���Ӧ��ȷʵ��ҪԤ��������������ڴ��������, ������ʵ��֮.

	ע��: ɾ��node_t�ڵ�ʱ, ����ⲿ�������@KEY ���� @VAL ����ר�ŵ��ͷŲ���(��VALͨ��������һ��ָ��), 
		  ������ ���� __dealloc_node() ���� __dealloc_node_with_pool()ǰ������֮, �������Լ���Ӧ�ͷŴ��뼰�߼�
*/

/* ������ʵ�����º���:
	1. ��ϣ����				aot_uint_t	hash(const KEY& k)
	2. key�ıȽϺ���		bool		key_cmp(const KEY& k1, const KEY& k2)
	3. ����һ��node_t		node_t*		alloc_node(const KEY& k, const VAL& v)
	4. �ͷ�node_t*			void		dealloc_node(node_t* p)
	5. �ͷ�@KEY, @VAL		void		destroy(KEY* k, VAL* v)
*/


template<class KEY, class VAL>
struct __default_hashmap_fun_group_t
{
	typedef hash_map_node_t<KEY, VAL> node_t;
	inline node_t* __alloc_node(const KEY& k, const VAL& v) { return new node_t(k, v); }
	inline void __dealloc_node(node_t* p) { if( p ) { delete p; } }
	/*
	aot_uint_t	hash(const KEY& k)
	bool		key_cmp(const KEY& k1, const KEY& k2)
	node_t*		alloc_node(const KEY& k, const VAL& v)
	void		dealloc_node(node_t* p)
	void		destroy(KEY* k, VAL* v)
	*/
};

template<class KEY, class VAL>
struct __default_pool_hashmap_fun_group_t
{
	typedef hash_map_node_t<KEY, VAL> node_t;
	__default_pool_hashmap_fun_group_t() : free_(NULL)
	{;}
	/*
	aot_uint_t	hash(const KEY& k)
	bool		key_cmp(const KEY& k1, const KEY& k2)
	node_t*		alloc_node(const KEY& k, const VAL& v)
	void		dealloc_node(node_t* p)
	void		destroy(KEY* k, VAL* v)
	*/

	inline node_t* __alloc_node(const KEY& k, const VAL& v)
	{
		node_t* p = NULL;
		if( this->free_ ) {
			p = this->free_;
			this->free_ = this->free_->next;
		}
		else {
			p = (node_t*)aot_std::default_alloc::alloc(sizeof(node_t));
			if( NULL == p ){ return NULL; }
		}
		new (p) node_t(k, v);
		return p;
	}
	inline void __dealloc_node(node_t* p)
	{
		if( p ){
			p->~node_t();
			p->next = this->free_; /// p���ڴ沢δ�����ͷ�, �����ʹ��p->next�ǰ�ȫ��
			this->free_ = p;
		}
	}		
public:
	node_t* free_;
};


template<class KEY, class VAL, class HASH_FUN_GROUP>
class hash_map
{
	typedef struct hash_map_node_t<KEY, VAL> node_t;
public:
	struct bucket_t
	{
		node_t*		node;
	};
	struct iterator
	{
		iterator(hash_map* m, node_t* d, aot_int_t idx) : map(m), curr(d), bkt_idx(idx)
		{;}
		KEY& key(){ return this->curr->key; }
		VAL& value(){ return this->curr->val; }
		void value(const VAL& v){ this->curr->val = v;}
	public:
		hash_map*	map;
		node_t*		curr;
		aot_int_t	bkt_idx;
	};
public:
	hash_map()
	{
		this->bkt_ = NULL;
		this->bkt_cnt_ = 0;
		this->size_ = 0;
	}

	~hash_map()
	{
		clear();
		if( this->bkt_ )
		{
			default_alloc::dealloc(this->bkt_);
		}
	}
public:
	iterator goto_begin()
	{
		for( aot_int_t i = 0; i < this->bkt_cnt_; ++i ) {
			if( this->bkt_[i].node ) {
				return iterator(this, this->bkt_[i].node, i);
			}
		}

		return iterator(this, NULL, 0);
	}
	iterator goto_next(iterator& it)
	{
		if( it.curr == NULL ){ return iterator(this, NULL, 0); }
		if( it.curr->next ) {
			return iterator(this, it.curr->next, it.bkt_idx);
		}

		for( aot_int_t i = it.bkt_idx + 1; i < this->bkt_cnt_; ++i ) {
			if( this->bkt_[i].node ) {
				return iterator(this, this->bkt_[i].node, i);
			}
		}
		
		return iterator(this, NULL, 0);
	}
	inline bool is_end(iterator& it)
	{
		return (it.curr == NULL);
	}
public:
	inline aot_uint_t size(){ return this->size_; }

	bool init(aot_int_t bkt_cnt)
	{
		assert( this->bkt_ == NULL );

		this->bkt_cnt_ = bkt_cnt;
		this->bkt_ = (bucket_t*)default_alloc::calloc(bkt_cnt * sizeof(bucket_t));

		if( NULL == this->bkt_ ) {
			assert(0);
			return false;
		}

		return true;
	}

	void clear()
	{
		if( this->size_ > 0 )
		{
			node_t* nd = NULL;
			for( aot_int_t i = 0; i < this->bkt_cnt_; ++i )
			{
				while( this->bkt_[i].node )
				{
					nd = this->bkt_[i].node;
					this->bkt_[i].node = this->bkt_[i].node->next;
					delete_node(nd);
				}
			}
			
			this->size_ = 0;
		}
	}

	inline VAL& operator[](const KEY& k) 
	{
		return find_or_insert(k, VAL()).curr->val;
	}

	iterator find(const KEY& k)
	{
		aot_uint_t idx = __get_bkt_idx(k);
		
		node_t* nd;
		for( nd = this->bkt_[idx].node; nd; nd = nd->next )
		{
			if( cmp_key(nd->key, k) ) {
				return iterator(this, nd, idx);
			}
		}
		return iterator(this, NULL, 0);
	}

	/// ����<prev, curr>, ��������ٵ�ɾ������
	pair_t<iterator, iterator> find2(const KEY& k)
	{
		aot_uint_t idx = __get_bkt_idx(k);

		node_t* nd = NULL;
		node_t* prev = NULL;

		for( nd = this->bkt_[idx].node; nd;  )
		{
			if( cmp_key(nd->key, k) ) {
				return pair_t<iterator, iterator>(iterator(this, prev, idx), iterator(this, nd, idx));
			}
			prev = nd;
			nd = nd->next;
		}
		return pair_t<iterator, iterator>(iterator(this, NULL, 0), iterator(this, NULL, 0));
	}

	/// �����Ԫ�ش��ڣ����ƶ���������
	iterator find_and_move_to_head(const KEY& k)
	{
		aot_uint_t idx = __get_bkt_idx(k);

		node_t* nd = NULL;
		node_t* prev = NULL;

		for( nd = this->bkt_[idx].node; nd;  )
		{
			if( cmp_key(nd->key, k) ) {
				if( NULL == prev ) {	/// �������Ǳ�ͷ
					assert(this->bkt_[idx].node == nd);
					return iterator(this, nd, idx);
				}
				prev->next = nd->next;
				nd->next = this->bkt_[idx].node;
				this->bkt_[idx].node = nd;
				return iterator(this, nd, idx);
			}

			prev = nd;
			nd = nd->next;
		}
		return iterator(this, NULL, 0);
	}

	/// ��������򷵻��Ѵ��ڵ�Ԫ�ص�����, ����, �����²���Ԫ�صĵ�����
	iterator find_or_insert(const KEY& k, const VAL& v)
	{
		aot_uint_t idx = __get_bkt_idx(k);

		node_t* nd;
		for( nd = this->bkt_[idx].node; nd; nd = nd->next )
		{
			if( cmp_key(nd->key, k) ) {
				return iterator(this, nd, idx);
			}
		}

		nd = alloc_node(k, v);
		nd->next = this->bkt_[idx].node;
		this->bkt_[idx].node = nd;
		++this->size_;
		return iterator(this, nd, idx);
	}

	/// ����, ���@key�Ѵ���, �򷵻�pair_t<false, iterator>, ����iterator Ϊ�Ѵ��ڵĵ�����
	pair_t<bool, iterator> insert_unique(const KEY& k, const VAL& v)
	{
		aot_uint_t idx = __get_bkt_idx(k);

		node_t* nd;
		for( nd = this->bkt_[idx].node; nd; nd = nd->next )
		{
			if( cmp_key(nd->key, k) ) {
				return pair_t(false, iterator(this, nd, idx));
			}
		}

		nd = alloc_node(k, v);
		nd->next = this->bkt_[idx].node;
		this->bkt_[idx].node = nd;
		++this->size_;
		return pair_t(true, iterator(this, nd, idx));
	}

	/// ����, ��������Ƿ������ͬ@key��Ԫ��
	inline iterator insert_unchk_unique(const KEY& k, const VAL& v)
	{
		aot_uint_t idx = __get_bkt_idx(k);

		node_t* nd = alloc_node(k, v);
		nd->next = this->bkt_[idx].node;
		this->bkt_[idx].node = nd;
		++this->size_;
		return iterator(this, nd, idx);
	}

	void remove(const KEY& k)
	{
		aot_uint_t idx = __get_bkt_idx(k);

		node_t* nd = NULL;
		node_t* prev = NULL;

		for( nd = this->bkt_[idx].node; nd;)
		{
			if( cmp_key(nd->key, k) )
			{
				--this->size_;

				if( NULL == prev ) /// �������Ǳ�ͷ
				{
					assert(this->bkt_[idx].node == nd);
					this->bkt_[idx].node = nd->next;
					delete_node(nd);
				}
				else
				{
					prev->next = nd->next;
					delete_node(nd);
				}
				break;
			}
			prev = nd;
			nd = nd->next;
		}
	}

	void remove_iter(iterator& it)
	{
		if( it.curr == NULL ) {
			assert(0);
			return;
		}

		node_t* nd = NULL;
		node_t* prev = NULL;

		for( nd = this->bkt_[it.bkt_idx].node; nd; )
		{
			if( nd == it.curr )
			{
				--this->size_;

				if( NULL == prev ) /// �������Ǳ�ͷ
				{
					assert(this->bkt_[it.bkt_idx].node == nd);
					this->bkt_[idx].node = nd->next;
					delete_node(nd);
				}
				else
				{
					prev->next = nd->next;
					delete_node(nd);
				}
				break;
			}

			prev = nd;
			nd = nd->next;
		}
	}

	inline void remove_iter2(const pair_t<iterator, iterator>& pr)
	{
		node_t* curr = pr.second.curr;
		node_t* prev = pr.first.curr;

		if( curr == NULL ) {
			assert(0);
			return;
		}

		--this->size_;

		if( NULL == prev ) /// �������Ǳ�ͷ
		{
			assert(this->bkt_[pr.second.bkt_idx].node == curr);

			this->bkt_[pr.second.bkt_idx].node = curr->next;
			delete_node(curr);
		}
		else
		{
			prev->next = curr->next;
			delete_node(curr);
		}
	}
private:
	inline bool cmp_key(const KEY& k1, const KEY& k2) { return this->fg_.cmp_key(k1, k2); }
	inline node_t* alloc_node(const KEY& k, const VAL& v) { return this->fg_.alloc_node(k, v); }
	inline void delete_node(node_t* p) { return this->fg_.dealloc_node(p); }
	inline aot_uint_t __get_bkt_idx(const KEY& key)
	{
		aot_uint_t h = this->fg_.hash(key);
		return ( h % ((aot_uint_t)this->bkt_cnt_) );
	}
private:
	aot_int_t	bkt_cnt_;
	bucket_t*	bkt_;
	HASH_FUN_GROUP	fg_;
	aot_uint_t	size_;
};


} /// end namespace aot_std

#endif /// __AOT_STD_HASHMAP_201505114_H__