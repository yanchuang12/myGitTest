#include "nd_core.h"

#define __nd_heap_parent(x) ( (x) == 0 ? 0 : (((x) - 1) / 2) )
#define __nd_heap_lchild(x) (((x)+(x))+1)

static ND_INLINE void
__nd_heap_copy(nd_min_heap_t *q, nd_uint_t slot, nd_heap_node_t *moved_node)
{
	q->heap[slot] = moved_node;
	q->ids[moved_node->id] = (nd_int_t)slot;
}

static nd_int_t
__pop_free_list(nd_min_heap_t *q)
{
	while (q->ids_curr < q->max_size && 
			(q->ids[q->ids_curr] >= 0 || q->ids[q->ids_curr] == -2) )
	{
		q->ids_curr++;
	}

	if (q->ids_curr == q->max_size) {
		q->ids_curr = q->ids_min_free;
		q->ids_min_free = q->max_size;
	}
	return q->ids_curr++;
}

static void 
__push_freelist (nd_min_heap_t *q, nd_int_t old_id)
{
	if (q->ids[old_id] == -2) {
		--q->cur_limbo;
	}
	else {
		--q->cur_size;
	}

	q->ids[old_id] = -1;

	if ((nd_uint_t)old_id < q->ids_min_free && 
		(nd_uint_t)old_id <= q->ids_curr ) {
		q->ids_min_free = old_id;
	}
}

static nd_err_t 
__insert (nd_min_heap_t *q, nd_heap_node_t *tn);

static nd_err_t 
__grow_heap(nd_min_heap_t *q);

static void
__reheap_up(nd_min_heap_t *q, nd_heap_node_t *moved_node, nd_uint_t slot, nd_uint_t parent);


static void
__reheap_down (nd_min_heap_t *q, nd_heap_node_t *moved_node, nd_uint_t slot, nd_uint_t child);


static nd_err_t 
__insert(nd_min_heap_t *q, nd_heap_node_t *node)
{
	if (q->cur_size + q->cur_limbo + 2 >= q->max_size) {
		if (ND_RET_OK != __grow_heap(q)) {
			return ND_RET_ERROR;
		}
	}

	__reheap_up(q, node, q->cur_size, __nd_heap_parent(q->cur_size));
	q->cur_size++;

	return ND_RET_OK;
}

static nd_err_t 
__grow_heap(nd_min_heap_t *q)
{
	nd_uint_t new_size, i;
	nd_heap_node_t **new_heap;
	nd_int_t *new_ids;

	new_size = q->max_size * 2;
	new_heap = (nd_heap_node_t**)malloc(sizeof(nd_heap_node_t*) * new_size);
	if (!new_heap) {
		return ND_RET_ERROR;
	}

	new_ids = malloc(sizeof(nd_int_t) * new_size);
	if (!new_ids) {
		free((void*)new_heap);
		return ND_RET_ERROR;
	}

	memcpy(new_heap, q->heap, q->max_size * sizeof(nd_heap_node_t*));
	free((void*)q->heap);
	q->heap = new_heap;

	memcpy(new_ids, q->ids, q->max_size * sizeof(nd_int_t));
	free(q->ids);
	q->ids = new_ids;

	for (i = q->max_size; i < new_size; i++) {
		q->ids[i] = -((nd_int_t)i + 1);
	}

	q->max_size = new_size;
	q->ids_min_free = new_size;

	return ND_RET_OK;
}

static void
__reheap_up(nd_min_heap_t *q, nd_heap_node_t *moved_node, nd_uint_t slot, nd_uint_t parent)
{
	while (slot > 0) {
		if (moved_node->key < q->heap[parent]->key) {
			__nd_heap_copy(q, slot, q->heap[parent]);
			slot = parent;
			parent = __nd_heap_parent(slot);
		}
		else
			break;
	}
	__nd_heap_copy(q, slot, moved_node);
}

static void
__reheap_down(nd_min_heap_t *q, nd_heap_node_t *moved_node, nd_uint_t slot, nd_uint_t child)
{
	while (child < q->cur_size) {
		if (child + 1 < q->cur_size && 
			q->heap[child + 1]->key < q->heap[child]->key)
		{
			child++;
		}

		if (q->heap[child]->key < moved_node->key) {
			__nd_heap_copy(q, slot, q->heap[child]);
			slot = child;
			child = __nd_heap_lchild(child);
		}
		else {
			break;
		}
	}
	__nd_heap_copy(q, slot, moved_node);
}

static nd_heap_node_t*
__remove(nd_min_heap_t *q, nd_uint_t slot)
{
	nd_heap_node_t *n, *n2;
	nd_uint_t parent;

	n = q->heap[slot];

	--q->cur_size;

	if (slot < q->cur_size) {

		n2 = q->heap[q->cur_size];
		__nd_heap_copy(q, slot, n2);

		parent = __nd_heap_parent(slot);

		if (n2->key >= q->heap[parent]->key) {
			__reheap_down(q, n2, slot, __nd_heap_lchild(slot));
		}
		else {
			__reheap_up (q, n2, slot, parent);
		}
	}

	q->ids[n->id] = -1;
	++q->cur_limbo;
	return n;
}

nd_min_heap_t*
nd_min_heap_create(nd_uint_t max_size)
{
	nd_min_heap_t *q;
	nd_uint_t i;

	q = malloc(sizeof(nd_min_heap_t));
	if (!q) {
		return NULL;
	}

	memset(q, 0, sizeof(nd_min_heap_t));

	q->max_size = max_size;

	q->heap = (nd_heap_node_t**)malloc(sizeof(nd_heap_node_t*) * max_size);
	if (!q->heap) {
		goto do_failed;
	}

	q->ids = malloc(sizeof(nd_int_t) * max_size );
	if (!q->ids) {
		goto do_failed;
	}

	for (i = 0; i < max_size; i++) {
		q->ids[i] = -1;
	}

	return q;

do_failed:
	if (q) {
		if (q->heap) {
			free((void*)q->heap);
		}
		if (q->ids) {
			free(q->ids);
		}
		free(q);
	}
	return NULL;
}

void
nd_min_heap_destroy(nd_min_heap_t *q)
{
	nd_uint_t n;

	if (q) {
		n = q->cur_size;
		if (q->heap) {
			/*for (i = 0; i < n; i++) {
				free(q->heap[i]);
			}*/
			free(q->heap);
		}
		free(q->ids);
		free(q);
	}
}

nd_err_t
nd_min_heap_insert(nd_min_heap_t *q, nd_heap_node_t *node)
{
	if (q->cur_size + q->cur_limbo < q->max_size) {
		node->id = __pop_free_list(q);
		__insert(q, node);
		return ND_RET_OK;
	}
	return ND_RET_ERROR;
}

nd_err_t 
nd_min_heap_remove(nd_min_heap_t *q, nd_heap_node_t *node)
{
	nd_int_t slot;

	/// id begin from 0
	if (node->id < 0 || (nd_uint_t)node->id >= q->max_size) {
		return ND_RET_ERROR;
	}

	slot = q->ids[node->id];

	if(slot < 0) {
		return ND_RET_ERROR;
	}

	if(node->id != q->heap[slot]->id){
		return ND_RET_ERROR;
	}
	else {
		nd_heap_node_t *rn;
		rn = __remove(q, slot);
		nd_assert(node == rn);
	}

	return ND_RET_OK;
}

nd_heap_node_t*
nd_min_heap_remove2(nd_min_heap_t *q, nd_int_t id)
{
	nd_int_t slot;

	if (id < 0 || (nd_uint_t)id >= q->max_size) {
		return NULL;
	}

	slot = q->ids[id];

	if(slot < 0) {
		return NULL;
	}

	if(id != q->heap[slot]->id){
		return NULL;
	}
	else {
		nd_heap_node_t *rn;
		rn = __remove(q, slot);
		return rn;
	}
}

nd_heap_node_t*
nd_min_heap_remove_first(nd_min_heap_t *q)
{
	if (q->cur_size)
		return __remove (q, 0);
	else
		return NULL;
}