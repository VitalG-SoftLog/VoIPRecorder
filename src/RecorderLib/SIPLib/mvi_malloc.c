#include "os_include.h"
#include "mvi_malloc.h"
#ifndef linux
#include "mvi_log.h"
#else
// memset
#include <string.h>
// malloc
#include <stdlib.h>
// MAX_BYTES_MALLOC
#include "sag_config.h"
// LOG_*
//#include "sag_log.h"
#include "sag.h"

#endif

#define CLEARBIT(a,b)	( a &= (~(1<<(b))) )
#define SETBIT(a,b)		( a |= (1<<(b)) )
#define TESTBIT(a,b)	( a & (1<<(b)) )


unsigned int bytes_malloc = 0;


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void clear_bits(char* pmask,int pos,int size)
{
	int index = pos/8;
	pos -= index*8;

	while ( size ) {
		CLEARBIT(pmask[index],pos);
		pos++;
		if ( pos == 8 ) { pos=0; index ++; }
		size--;
	}
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void set_bits(char* pmask,int pos,int size)
{
	int index = pos/8;
	pos -= index*8;

	while ( size ) {
		SETBIT(pmask[index],pos);
		pos++;
		if ( pos == 8 ) { pos=0; index++; }
		size--;
	}
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void check_bits(char* pmask,int* ppos,int* psize)
{
	int index;
	int i;
	int	pos	= 0;
	int size	= 0;
	*ppos = 0;
	*psize = 0;

	for ( index=0; index<SIZE_MASK; index++ ) {
		for ( i=0; i<8; i++ ) {
			if ( !TESTBIT(pmask[index],i) ) {
				if ( !size ) pos = index*8+i;
				size++;
			}
			else {
				if ( *psize < size && size ) {
					*ppos = pos;
					*psize = size;
				}
				size = 0;
			}
		}
	}
	if ( *psize < size && size ) {
		*ppos = pos;
		*psize = size;
		size = 0;
	}
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
BOOL find_bits(char* pmask,int* ppos,int size)
{
	int index;
	int i;
	int	pos1	= 0;
	int size1	= 0;
	int size2	= 0;

	*ppos	= 0;

	for ( index=0; index<SIZE_MASK; index++ ) {
		for ( i=0; i<8; i++ ) {
			if ( !TESTBIT(pmask[index],i) ) {
				if ( !size1 ) pos1 = index*8+i;
				size1++;
			}
			else {
				if ( size1 == size ) {
					*ppos = pos1;			
					return TRUE;
				}
				if ( size1 > size ) {
					if ( !size2 ) {
						*ppos = pos1;
						size2 = size1;
					}
					else {
						if ( size2 > size1 ) {
							*ppos = pos1;
							size2 = size1;
						}
					}
				}
				size1 = 0;
			}
		}
	}
	if ( size1 == size ) {
		*ppos = pos1;			
		return TRUE;
	}
	if ( size1 > size ) {
		if ( !size2 ) {
			*ppos = pos1;
			size2 = size1;
		}
		else {
			if ( size2 > size1 ) {
				*ppos = pos1;
				size2 = size1;
			}
		}
	}
	if ( !size2 ) return FALSE;
	return TRUE;
}




free_buffers_t free_buffers;

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void	free_buffers_init(void)
{
	bytes_malloc = 0;
	memset(&free_buffers,0,sizeof(free_buffers));
	if ( pthread_mutex_init(&free_buffers.mutex,NULL) ) return ;
	if ( pthread_mutex_lock(&free_buffers.mutex) ) return;
	free_buffers_add();
	pthread_mutex_unlock(&free_buffers.mutex);
}


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void free_buffers_add(void)
{
	int i;
	item_buffer_max_t* pibtm;

	if ( free_buffers.pfree[COUNT_ITEM-1] ) return;
	if ( bytes_malloc >= MAX_BYTES_MALLOC )	return;
	free_buffers.pfree[COUNT_ITEM-1] = (item_buffer_max_t*) malloc(MAX_NEW_FREE_BUFFER*sizeof(item_buffer_max_t));
	if ( !free_buffers.pfree[COUNT_ITEM-1] ) return;
	bytes_malloc += MAX_NEW_FREE_BUFFER*sizeof(item_buffer_max_t);
	memset(free_buffers.pfree[COUNT_ITEM-1],0,MAX_NEW_FREE_BUFFER*sizeof(item_buffer_max_t));
	pibtm = free_buffers.pfree[COUNT_ITEM-1];
	for ( i=0; i<MAX_NEW_FREE_BUFFER-1; i++ ) {
		(pibtm+i)->next		= (pibtm+i+1);
		(pibtm+i+1)->prev	=  (pibtm+i);
	}
	free_buffers.count[COUNT_ITEM-1] =  MAX_NEW_FREE_BUFFER;
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
item_buffer_t*  free_buffers_get(int lendata)
{
	item_buffer_max_t*	pibtm = NULL;
	item_buffer_t*		pibt;
	int size;
	int i=0;
	int pos;
	int is_next = 0;

	lendata += sizeof(item_buffer_t) + 4;
	size = lendata/MIN_SIZE_ITEM;
	if ( size >= COUNT_ITEM ) return NULL;

	while ( !pibtm ) {
		for ( i=size; i<COUNT_ITEM; i++) {
			if ( free_buffers.pfree[i] ) {
				pibtm = free_buffers.pfree[i];
				free_buffers.pfree[i] = free_buffers.pfree[i]->next;
				if ( free_buffers.pfree[i] ) free_buffers.pfree[i]->prev = NULL; 
				free_buffers.count[i] --;
				break;
			}
		}
		if ( is_next ) break;
		if ( !pibtm ) free_buffers_add();
		is_next++;
	}
	if ( !pibtm ) return NULL;


	if ( !find_bits(pibtm->mask,&pos,size+1) ) return FALSE;
	set_bits(pibtm->mask,pos,size+1);

	pibt = (item_buffer_t* )(pibtm->buf + pos*MIN_SIZE_ITEM);
	pibt->proot = pibtm;
	pibt->size  = (size+1)*MIN_SIZE_ITEM - sizeof(item_buffer_t); 
	pibt->index = pos;
	pibt->pbuf  = &(pibtm->buf[pos*MIN_SIZE_ITEM + sizeof(item_buffer_t)]);

	pibtm->next = NULL;
	pibtm->prev = NULL;
	check_bits(pibtm->mask,&pos,&size);
	if ( size ) {
		pibtm->next = free_buffers.pfree[size-1];
		if ( free_buffers.pfree[size-1] ) free_buffers.pfree[size-1]->prev = pibtm; 
		free_buffers.pfree[size-1] = pibtm;
		free_buffers.count[size-1] ++;
	}
	return pibt;
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void free_buffers_free(item_buffer_t* pibt)
{
	item_buffer_max_t*	pibtm;
	int size;
	int pos;

	if ( !pibt ) return;

	if ( pibt->next ) free_buffers_free(pibt->next);
	if ( pthread_mutex_lock(&free_buffers.mutex) ) return;

	pibtm = pibt->proot;
	check_bits(pibtm->mask,&pos,&size);
	if ( size ) {
		if ( pibtm->next ) pibtm->next->prev = pibtm->prev;	
		if ( pibtm->prev ) pibtm->prev->next = pibtm->next;
		if ( pibtm == free_buffers.pfree[size-1] ) free_buffers.pfree[size-1] = pibtm->next;	
		free_buffers.count[size-1] --;
	}
	pos = pibt->index;
	size = pibt->size  + sizeof(item_buffer_t); 
	size /= MIN_SIZE_ITEM; 
	clear_bits(pibtm->mask,pos,size);
	memset(&pibtm->buf[pos*MIN_SIZE_ITEM],0,size*MIN_SIZE_ITEM);

	pibtm->next = NULL;
	pibtm->prev = NULL;
	check_bits(pibtm->mask,&pos,&size);
	if ( size ) {
		pibtm->next = free_buffers.pfree[size-1];
		if ( free_buffers.pfree[size-1] ) free_buffers.pfree[size-1]->prev = pibtm; 
		free_buffers.pfree[size-1] = pibtm;
		free_buffers.count[size-1] ++;
	}
	pthread_mutex_unlock(&free_buffers.mutex);
}


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
char* malloc_char(item_buffer_t** ppibt,const char* pobj,mvi_status_t* pstatus)
{
	char* tmp = NULL;
	int len = strlen(pobj);
	item_buffer_t* pibt;

	len++;
	if ( len >= (int)(MAX_SIZE_FREE_BUFFER + sizeof(item_buffer_t)) ) { if ( pstatus ) *pstatus = ER_MAX_BUFFER; return tmp; }

	if ( pthread_mutex_lock(&free_buffers.mutex) ) { if ( pstatus ) *pstatus = ER_MUTEX_LOCK; return tmp; }

	pibt = free_buffers_find(*ppibt,len);
	if ( !*ppibt ) *ppibt = pibt;
	if ( pibt ) {
		tmp = &pibt->pbuf[pibt->tek];
		memcpy(tmp,pobj,len);
		pibt->tek += len;
	}
	pthread_mutex_unlock(&free_buffers.mutex);
	if ( pstatus ) *pstatus = Ok;
	if ( !tmp ) if ( pstatus ) *pstatus = ER_SMALL_BUFFER;
	return tmp;
}
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
char* malloc_char_exp(item_buffer_t** ppibt,const char* pobj,int max_len,mvi_status_t* pstatus)
{
	char* tmp = NULL;
	int len = strlen(pobj);
	item_buffer_t* pibt;

	len++;
	if ( max_len < len ) max_len = len;
	if ( max_len >= (int)(MAX_SIZE_FREE_BUFFER + sizeof(item_buffer_t)) ) { if ( pstatus ) *pstatus = ER_MAX_BUFFER; return tmp; }

	if ( pthread_mutex_lock(&free_buffers.mutex) ) { if ( pstatus ) *pstatus = ER_MUTEX_LOCK; return tmp; }

	if ( !*ppibt )	pibt = free_buffers_find(*ppibt,max_len);
	else			pibt = free_buffers_find(*ppibt,len);
	if ( !*ppibt ) *ppibt = pibt;
	if ( pibt ) {
		tmp = &pibt->pbuf[pibt->tek];
		memcpy(tmp,pobj,len);
		pibt->tek += len;
	}
	pthread_mutex_unlock(&free_buffers.mutex);
	if ( pstatus ) *pstatus = Ok;
	if ( !tmp ) if ( pstatus ) *pstatus = ER_SMALL_BUFFER;
	return tmp;
}


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void* malloc_void(item_buffer_t** ppibt,int len,mvi_status_t* pstatus)
{
	void* tmp = NULL;
	item_buffer_t* pibt;

	if ( len >= (int)(MAX_SIZE_FREE_BUFFER + sizeof(item_buffer_t)) ) { if ( pstatus ) *pstatus = ER_MAX_BUFFER; return tmp; }

	if ( pthread_mutex_lock(&free_buffers.mutex) ) { if ( pstatus ) *pstatus = ER_MUTEX_LOCK; return tmp; }

	pibt = free_buffers_find(*ppibt,len+4);
	if ( !*ppibt ) *ppibt = pibt;
	if ( pibt ) {
		while ( (int)(&pibt->pbuf[pibt->tek]) % 4 ) pibt->tek++;
		tmp = &pibt->pbuf[pibt->tek];
		pibt->tek += len;
	}
	pthread_mutex_unlock(&free_buffers.mutex);

	if ( pstatus ) *pstatus = Ok;
	if ( !tmp ) if ( pstatus ) *pstatus = ER_SMALL_BUFFER;
	return tmp;
}


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
item_buffer_t* free_buffers_find(item_buffer_t* pibt,int len)
{
	item_buffer_t* pibt_tmp = pibt;

	while ( pibt_tmp ) {
		if ( len < pibt_tmp->size - pibt_tmp->tek ) return pibt_tmp;
		pibt_tmp = pibt->next;
		if ( pibt->next ) pibt = pibt->next;
	}
	pibt_tmp = free_buffers_get(len);
	if ( pibt ) pibt->next = pibt_tmp;

	return pibt_tmp;
}

/*
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
char* add_item_buffer_char(item_buffer_t** ppibt,const char* obj)
{
mvi_status_t ret;
return malloc_char(ppibt,obj,&ret);
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void* add_item_buffer_void(item_buffer_t** ppibt,int len)
{
mvi_status_t ret;
return malloc_void(ppibt,len,&ret);
}

*/


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void  print_malloc(void)
{
	int i;
	for(i=0; i<COUNT_ITEM; i++ ) { 
		if ( free_buffers.pfree[i] ) {
			LOG_debug("free_buffers.count[%d] = %d",i,free_buffers.count[i]);
		}
		//	else							printf("(%d.No %d) ",i,free_buffers.count[i]);
	}
}


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
unsigned int get_bytes_malloc_yuri(void)
{
	return bytes_malloc;
}
