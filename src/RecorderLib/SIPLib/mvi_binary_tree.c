#include "os_include.h"
#include "mvi_binary_tree.h"
#include "mvi_log.h"


binary_tree_buf_t binary_tree_buf;

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void	binary_tree_init(void)
{
	memset(&binary_tree_buf,0,sizeof(binary_tree_buf));
	if ( pthread_mutex_init(&binary_tree_buf.mutex,NULL) ) return ;
	binary_tree_add();
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void binary_tree_add(void)
{
	int i;
	binary_tree_t* pbtt;

	if ( binary_tree_buf.pfree ) return;
	//if ( !binary_tree_buf.mutex ) return;
	if ( pthread_mutex_lock(&binary_tree_buf.mutex) ) return;
	binary_tree_buf.pfree = malloc(MAX_NEW_BINARY_TREE*sizeof(binary_tree_t));
	memset(binary_tree_buf.pfree,0,MAX_NEW_BINARY_TREE*sizeof(binary_tree_t));
	pbtt = binary_tree_buf.pfree;
	for ( i=0; i<MAX_NEW_BINARY_TREE-1; i++ ) (pbtt+i)->e = (pbtt+i+1);
	binary_tree_buf.count += MAX_NEW_BINARY_TREE;
	if ( pthread_mutex_unlock(&binary_tree_buf.mutex) ) return;
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
binary_tree_t*  binary_tree_get(void)
{
	binary_tree_t* pbtt;

	if ( pthread_mutex_lock(&binary_tree_buf.mutex) ) return NULL;

	if ( !binary_tree_buf.pfree ) {
		if ( pthread_mutex_unlock(&binary_tree_buf.mutex) ) return NULL;
		binary_tree_add();
		if ( pthread_mutex_lock(&binary_tree_buf.mutex) ) return NULL;
		if ( !binary_tree_buf.pfree ) {
			pthread_mutex_unlock(&binary_tree_buf.mutex);
			return NULL;
		}
	}
	pbtt = binary_tree_buf.pfree;
	binary_tree_buf.pfree = pbtt->e;
	pbtt->e = NULL;
	pthread_mutex_unlock(&binary_tree_buf.mutex);
	return pbtt;
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void binary_tree_free(binary_tree_t* pbinary_tree)
{
	if ( pthread_mutex_lock(&binary_tree_buf.mutex) ) return;
	memset(pbinary_tree,0,sizeof(binary_tree_t));
	pbinary_tree->e = binary_tree_buf.pfree;
	binary_tree_buf.pfree = pbinary_tree;
	pthread_mutex_unlock(&binary_tree_buf.mutex);
}



//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
binary_tree_t*	binary_tree_check_balance(binary_tree_t* proot)
{
	int balance = 0;

	if ( proot->a ) balance = proot->a->count;
	if ( proot->l ) balance -= proot->l->count;
	switch (balance) {
	case 0:	case -1: case 1:
		return proot;		
	}

	if ( !balance ) return proot;
	return binary_tree_rotation(proot,&balance);
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
binary_tree_t*	binary_tree_rotation(binary_tree_t* proot,int* pbalance)
{
	binary_tree_t* pbbt;

	if ( *pbalance < 0 ) {
		pbbt = proot->l;
		proot->l = pbbt->a;
		if ( proot->l ) proot->l->p = proot;
		pbbt->p = proot->p;
		proot->p = pbbt;
		pbbt->a = proot;
	}
	else {
		pbbt = proot->a;
		proot->a = pbbt->l;
		if ( proot->a ) proot->a->p = proot;
		pbbt->p = proot->p;
		proot->p = pbbt;
		pbbt->l = proot;
	}
	if ( pbbt->p ) {
		if ( pbbt->p->a == proot ) pbbt->p->a = pbbt;
		if ( pbbt->p->l == proot ) pbbt->p->l = pbbt;
	}
	proot->count = 1;
	if ( proot->e ) proot->count += proot->e->count;
	if ( proot->a ) proot->count += proot->a->count;	
	if ( proot->l ) proot->count += proot->l->count;	

	pbbt->count = 1;
	if ( pbbt->e ) pbbt->count += pbbt->e->count;
	if ( pbbt->a ) pbbt->count += pbbt->a->count;	
	if ( pbbt->l ) pbbt->count += pbbt->l->count;	

	return pbbt;
}


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
binary_tree_t*	binary_tree_check_find(binary_tree_t** pproot,unsigned int hash,int* pbalance,int isAdd)
{
	binary_tree_t* pbtt;

	pbtt = *pproot;
	while (1) {
		if ( isAdd ) {
			pbtt = binary_tree_check_balance(pbtt);	
			pbtt->count ++;
		}
		if ( !pbtt->p ) *pproot = pbtt;
		if ( hash == pbtt->hash )	{
			*pbalance = 0;
		}
		else {
			if ( hash > pbtt->hash )	*pbalance = 1;
			else						*pbalance = -1;	
		}
		if ( !*pbalance ) break;
		if ( *pbalance < 0 ) {
			if ( !pbtt->l ) break;
			pbtt = pbtt->l;	
			//		if ( hash < pbtt->hash )	continue;
			//		if ( pbtt->a ) if ( hash > pbtt->a->hash )	break;
		} 
		else {
			if ( !pbtt->a ) break;
			pbtt = pbtt->a;	
			//		if ( hash > pbtt->hash )	continue;
			//		if ( pbtt->l ) if ( hash < pbtt->l->hash )	break;
		}
	}
	return pbtt;
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
binary_tree_t*	binary_tree_insert(binary_tree_t** pproot,unsigned int hash)
{
	binary_tree_t*	pbtt;
	binary_tree_t*	pbttnew;

	int				balance = 0;

	if ( !*pproot ) {
		*pproot = binary_tree_get();
		if ( *pproot ) {
			(*pproot)->count = 1;
			(*pproot)->hash = hash;
		}
		return *pproot;
	}

	pbtt = binary_tree_check_find(pproot,hash,&balance,TRUE);
	if ( !pbtt ) return pbtt;

	pbttnew = binary_tree_get();
	if ( !pbttnew ) return pbttnew;
	pbttnew->hash = hash;
	if ( !balance ) {
		pbttnew->p	= pbtt;
		pbttnew->e	= pbtt->e;
		pbtt->e		= pbttnew;
		if ( pbttnew->e ) {
			pbttnew->e->p = pbttnew;
			pbttnew->count = pbttnew->e->count;
		}
		pbttnew->count ++;
	} 
	else {
		if ( balance < 0 ) {
			pbttnew->p	= pbtt;
			pbttnew->l	= pbtt->l;
			pbtt->l		= pbttnew;	
			if ( pbttnew->l ) {
				pbttnew->l->p = pbttnew;
				pbttnew->count = pbttnew->l->count;
			}
			pbttnew->count ++;
		}
		else {
			pbttnew->p	= pbtt;
			pbttnew->a	= pbtt->a;
			pbtt->a		= pbttnew;	
			if ( pbttnew->a ) {
				pbttnew->a->p = pbttnew;
				pbttnew->count = pbttnew->a->count;
			}
			pbttnew->count ++;
		}
	}
	return pbttnew;
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
binary_tree_t*	binary_tree_search(binary_tree_t** pproot,unsigned int hash)
{
	binary_tree_t*	pbtt;
	int				balance;
	if ( !*pproot ) return NULL;
	pbtt = binary_tree_check_find(pproot,hash,&balance,FALSE);
	if ( !pbtt ) return pbtt;
	if ( !balance ) return pbtt;
	return NULL;
}
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
binary_tree_t*	binary_tree_search_insert(binary_tree_t** pproot,unsigned int hash)
{
	binary_tree_t*	pbtt;
	pbtt = binary_tree_search(pproot,hash);
	if ( !pbtt ) pbtt = binary_tree_insert(pproot,hash);
	return pbtt;
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void binary_tree_delete(binary_tree_t** pproot,binary_tree_t* pbtt)
{
	binary_tree_t* pparent;
	int balance;

	if ( !pbtt ) return;



	if ( pbtt->e ) {
		pbtt->e->l		= pbtt->l;
		if ( pbtt->l ) pbtt->l->p = pbtt->e;
		pbtt->e->a		= pbtt->a;
		if ( pbtt->a ) pbtt->a->p = pbtt->e;
		pbtt->e->p		= pbtt->p;
		pbtt->e->count	= pbtt->count -1;
		pparent = pbtt->p;
		if ( !pparent ) *pproot = pbtt->e;
		else {
			do {
				if ( pparent->l == pbtt ) {
					pparent->l = pbtt->e; 
					break;
				}
				if ( pparent->e == pbtt ) {
					pparent->e = pbtt->e; 
					break;
				}
				if ( pparent->a == pbtt ) {
					pparent->a = pbtt->e; 
					break;
				}
				LOG_critical("failed to create a binary tree");
				return;
			} while (FALSE);
			while ( pparent ) {
				pparent->count --;
				pparent = pparent->p;
			}
		}
		binary_tree_free(pbtt);	
		return;
	}

	pparent = pbtt->p;
	if ( pparent ) {
		if ( pparent->e == pbtt ) {
			pparent->e = NULL;
			while ( pparent ) {
				pparent->count --;
				pparent = pparent->p;
			}
			binary_tree_free(pbtt);	
			return;
		}
	}


	while ( 1 ) {
		if ( !pbtt->l || !pbtt->a ) break;
		if ( pbtt->l->count > pbtt->a->count )	balance = -1;
		else									balance = 1;
		pparent = binary_tree_rotation(pbtt,&balance);
	}

	if ( !pbtt->p && !pbtt->l && !pbtt->a ) {
		if ( *pproot != pbtt ) { LOG_critical("failed to create a binary tree object"); }
		else *pproot = NULL;

	}
	else {
		pparent = NULL;
		if ( pbtt->l ) {
			pbtt->l->p = pbtt->p;
			pparent = pbtt->l;
		}	 
		if ( pbtt->a ) {
			pbtt->a->p = pbtt->p;
			pparent = pbtt->a;
		}	 
		if ( !pbtt->p ) {
			*pproot = pparent;
		}
		else {
			if ( pbtt->p->a == pbtt ) pbtt->p->a = pparent;
			else {
				if ( pbtt->p->l == pbtt ) pbtt->p->l = pparent;
				else	LOG_critical("failed to create a binary tree");
			}
		}
	}

	pparent = pbtt->p;
	while ( pparent ) {
		pparent->count --;
		if ( !pparent->p ) *pproot = pparent;
		pparent = pparent->p;
	}
	binary_tree_free(pbtt);	
}


#define START	0
#define LEFT	1
#define RIGHT	2
#define PARENT	3

//////////////////////////////////////////////////////////
void binary_tree_print(binary_tree_t* node, int kol) 
{
	int* plevels = NULL;
	int	level=0;
	int state = START;
	int kol_items = 0;
	int equ_items = 0;

	if ( !node ) return;
	plevels = malloc(sizeof(int)*kol);
	if ( !plevels ) return;
	memset(plevels,0,sizeof(int)*kol); 


	do {
		switch (state) {
		case START:
			state = LEFT;
			break;
		case LEFT:
			if (node->l) {
				node = node->l;
				level++;
				state = LEFT;
			} 
			else {
				state = RIGHT;
			}
			break;
		case RIGHT:
			if (node->a) {
				node = node->a;
				level++;
				state = LEFT;
			} 
			else {
				state = PARENT;
			}
			break;
		case PARENT:
			if ( level >= kol ) {
				LOG_error("\n==>Error  binary_tree_print");
				return;
			}
			plevels[level] ++;
			kol_items  ++;
			if ( node->e ) {
				if ( node->e->count ) equ_items  += node->e->count;
			}

			if (node->p) {
				if (node->p->l == node) {
					node = node->p;
					level--;
					state = RIGHT;
				} 
				else {
					node = node->p;
					level--;
					state = PARENT;
				}
			} 
			else {
				state = START;
			}
			break;
		}
	} while(state != START);

	for (level=0; level<kol; level++ ) if ( plevels[level] > 0 ) printf("(Level[%04u] = %010u)",level,plevels[level]);

	LOG_info("\n\n===> kol_items (%d) equ_items (%d)",kol_items,equ_items);

	free(plevels); 
}
