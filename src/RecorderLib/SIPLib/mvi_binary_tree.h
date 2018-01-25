#ifndef  _BINARY_TREE_H_
#define  _BINARY_TREE_H_

#include "mvi_pthread.h"
#include "hash_tables_define_constant.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct binary_tree {
	struct binary_tree*		p;		// parent
	struct binary_tree*		a;		// >
	struct binary_tree*		e;		// =
	struct binary_tree*		l;		// <

	int				count;				
	unsigned int	hash;			
	void*	obj;

} binary_tree_t;

typedef struct binary_tree_buf {
	binary_tree_t*		pfree;
	int					count;
	pthread_mutex_t		mutex;			
} binary_tree_buf_t;

void			binary_tree_init(void);	
binary_tree_t*  binary_tree_get(void);	
void			binary_tree_free(binary_tree_t* pbinary_tree);	
void			binary_tree_add(void);	



binary_tree_t*	binary_tree_insert(binary_tree_t** pproot,unsigned int hash);
void			binary_tree_delete(binary_tree_t** pproot,binary_tree_t* pbtt);
binary_tree_t*	binary_tree_search(binary_tree_t** pproot,unsigned int hash);
binary_tree_t*	binary_tree_search_insert(binary_tree_t** pproot,unsigned int hash);

binary_tree_t*	binary_tree_check_balance(binary_tree_t* proot);
binary_tree_t*	binary_tree_check_find(binary_tree_t** pproot,unsigned int hash,int* pbalance,int isAdd);
binary_tree_t*	binary_tree_rotation(binary_tree_t* proot,int* pbalance);

void binary_tree_print(binary_tree_t* proot,int kol);




#ifdef __cplusplus
}
#endif


#endif /* _BINARY_TREE_H_ */ 
