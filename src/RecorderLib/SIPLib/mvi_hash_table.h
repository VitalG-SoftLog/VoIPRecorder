#ifndef  _HASH_TABLE_H_
#define  _HASH_TABLE_H_

#include "mvi_binary_tree.h"
#include "hash_tables_define_constant.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct hash_table {
	binary_tree_t**	pproots;
	int				size;
	pthread_mutex_t	mutex;			

    unsigned int	(*hashfunc)(void* k);
    binary_tree_t*	(*insertfunc)(binary_tree_t* pbtt,void** ppobj,int i);
    binary_tree_t*	(*searchfunc)(binary_tree_t* pbtt,void** ppobj,int i);
    binary_tree_t*	(*deletefunc)(binary_tree_t* pbtt,void** ppobj,int i);

} hash_table_t;


hash_table_t*	hash_table_create(int size,unsigned int (*hashfunc)(void *k),
								binary_tree_t*	(*insertfunc)(binary_tree_t* pbtt,void** ppobj,int i),
								binary_tree_t*	(*findfunc)(binary_tree_t* pbtt,void** ppobj,int i),
								binary_tree_t*	(*deletefunc)(binary_tree_t* pbtt,void** ppobj,int i));

binary_tree_t*	hash_table_insert(hash_table_t* phtt,void** ppobj,int i);
binary_tree_t*	hash_table_search(hash_table_t* phtt,void** ppobj,int i);
binary_tree_t*	hash_table_search_insert(hash_table_t* phtt,void** ppobj,int i);
void			hash_table_delete(hash_table_t* phtt,void** ppobj,int i);

//void	hash_table_get_info(hash_table_t* phtt,int* pmax,int* pmin,int* pcount,int* pnull);



#ifdef __cplusplus
}
#endif


#endif /* _HASH_TABLE_H_ */ 
