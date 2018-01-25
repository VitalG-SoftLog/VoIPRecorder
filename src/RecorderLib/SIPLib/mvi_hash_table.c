#include "os_include.h"
#include "mvi_hash_table.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
hash_table_t*	hash_table_create(int size,unsigned int (*hashfunc)(void *k),
								  binary_tree_t*	(*insertfunc)(binary_tree_t* pbtt,void** ppobj,int i),
								  binary_tree_t*	(*searchfunc)(binary_tree_t* pbtt,void** ppobj,int i),
								  binary_tree_t*	(*deletefunc)(binary_tree_t* pbtt,void** ppobj,int i))
{
	hash_table_t* phtt; 	
	phtt = malloc(sizeof(hash_table_t));
	if ( !phtt ) return NULL;
	memset(phtt,0,sizeof(hash_table_t));

	if ( !size ) size = CFG_SIP_REG_HASHTABLE_SIZE;
	phtt->pproots = malloc(sizeof(phtt->pproots)*size);
	memset(phtt->pproots ,0,sizeof(phtt->pproots)*size);
	phtt->size			= size;
	phtt->hashfunc		= hashfunc;
	phtt->insertfunc	= insertfunc;
	phtt->searchfunc	= searchfunc;
	phtt->deletefunc	= deletefunc;
	if ( pthread_mutex_init(&phtt->mutex,NULL) ) return NULL;

	return phtt;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
binary_tree_t*	hash_table_insert(hash_table_t* phtt,void** ppobj,int i)
{
	unsigned int hash;
	unsigned int index;
	binary_tree_t*	pbtt;
	binary_tree_t*	pbtt1;
	binary_tree_t**	pproot;

	if ( !phtt || !ppobj || !i ) return NULL;

	if ( pthread_mutex_lock(&phtt->mutex) ) return NULL;

	if ( !phtt->hashfunc )  hash = *((int*)ppobj[0]);
	else					hash = (phtt->hashfunc)(ppobj[0]);

	index = hash % phtt->size;
	pproot = (phtt->pproots+index);
	pbtt = binary_tree_insert(pproot,hash);
	if ( pbtt ) if ( phtt->insertfunc ) {
		pbtt1 = (phtt->insertfunc)(pbtt,ppobj,i);
		if ( !pbtt1 ) binary_tree_delete(pproot,pbtt);
		pbtt = pbtt1;
	}

	pthread_mutex_unlock(&phtt->mutex);

	return pbtt;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
binary_tree_t*	hash_table_search(hash_table_t* phtt,void** ppobj,int i)
{
	unsigned int hash;
	unsigned int index;
	binary_tree_t*	pbtt;
	binary_tree_t**	pproot;

	if ( !phtt || !ppobj || !i ) return NULL;

	if ( pthread_mutex_lock(&phtt->mutex) ) return NULL;

	if ( !phtt->hashfunc )  hash = *((int*)ppobj[0]);
	else					hash = (phtt->hashfunc)(ppobj[0]);
	index = hash % phtt->size;
	pproot = (phtt->pproots+index);
	pbtt = binary_tree_search(pproot,hash);
	if ( pbtt ) if ( phtt->searchfunc ) pbtt = (phtt->searchfunc)(pbtt,ppobj,i);

	pthread_mutex_unlock(&phtt->mutex);
	return pbtt;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
binary_tree_t*	hash_table_search_insert(hash_table_t* phtt,void** ppobj,int i)
{
	unsigned int hash;
	unsigned int index;
	binary_tree_t*	pbtt;
	binary_tree_t**	pproot;

	if ( !phtt || !ppobj || !i ) return NULL;

	if ( pthread_mutex_lock(&phtt->mutex) ) return NULL;

	if ( !phtt->hashfunc )  hash = *((int*)ppobj[0]);
	else					hash = (phtt->hashfunc)(ppobj[0]);
	index = hash % phtt->size;
	pproot = (phtt->pproots+index);
	pbtt = binary_tree_search(pproot,hash);
	if ( pbtt ) {
		if ( phtt->searchfunc ) pbtt = (phtt->searchfunc)(pbtt,ppobj,i);	
	}
	if ( !pbtt ) {
		pbtt = binary_tree_insert(pproot,hash);
		if ( pbtt ) if ( phtt->insertfunc ) pbtt = (phtt->insertfunc)(pbtt,ppobj,i);	
	}
	pthread_mutex_unlock(&phtt->mutex);
	return pbtt;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
void hash_table_delete(hash_table_t* phtt,void** ppobj,int i)
{
	unsigned int hash;
	unsigned int index;
	binary_tree_t*	pbtt;
	binary_tree_t**	pproot;

	if ( !phtt || !ppobj || i<2 ) return;

	if ( pthread_mutex_lock(&phtt->mutex) ) return;

	if ( !phtt->hashfunc )  hash = *((int*)ppobj[0]);
	else					hash = (phtt->hashfunc)(ppobj[0]);
	index	= hash % phtt->size;
	pproot	= (phtt->pproots+index);
	pbtt	= binary_tree_search(pproot,hash);

	//while ( pbtt ) { if ( pbtt->obj == ppobj[1] ) break; pbtt = pbtt->e; }

	if ( pbtt ) if ( phtt->deletefunc ) pbtt = (phtt->deletefunc)(pbtt,ppobj,i);

	if ( pbtt ) binary_tree_delete(pproot,pbtt);

	pthread_mutex_unlock(&phtt->mutex);

}

/*
///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
void	hash_table_get_info(hash_table_t* phtt,int* pmax,int* pmin,int* pcount,int* pnull)
{
int index;
binary_tree_t* proot;

*pmax	= 0;
*pmin	= -1;
*pcount	= 0;
*pnull	= 0;

for ( index=0; index<phtt->size; index++ ) {
proot = *(phtt->pproots+index);
if ( proot )  {
if ( *pmax < proot->count ) *pmax = proot->count;
if ( *pmin == -1 )			*pmin = proot->count;
if ( *pmin > proot->count ) *pmin = proot->count;
*pcount += proot->count;
}
else *pnull	++;
}
}
*/
