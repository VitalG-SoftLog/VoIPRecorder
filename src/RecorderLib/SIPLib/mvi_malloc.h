#ifndef  _MVI_MALLOC_H_
#define  _MVI_MALLOC_H_

#include "mvi_pthread.h"
#include "mvi_error_message.h"
#include "deftype.h"
#include "hash_tables_define_constant.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CLEARBIT(a,b)	( a &= (~(1<<(b))) )
#define SETBIT(a,b)		( a |= (1<<(b)) )
#define TESTBIT(a,b)	( a & (1<<(b)) )

void clear_bits(char* pmask,int pos,int size);
void set_bits(char* pmask,int pos,int size);
void check_bits(char* pmask,int* ppos,int* psize);
BOOL find_bits(char* pmask,int* ppos,int size);

#define	MAX_SIZE_FREE_BUFFER		4096
#define	MIN_SIZE_ITEM				128
#define	COUNT_ITEM					32
#define	SIZE_MASK					4



typedef struct item_buffer_max {
	struct item_buffer_max*	next;
	struct item_buffer_max*	prev;
	char	mask[SIZE_MASK];
	char	buf[MAX_SIZE_FREE_BUFFER];
} item_buffer_max_t;

typedef struct item_buffer {
	struct item_buffer*	next;
	item_buffer_max_t*	proot;
	int		size;
	int		tek;
	int		index;
	char*	pbuf;
} item_buffer_t;


typedef struct free_buffers {
	item_buffer_max_t*	pfree[COUNT_ITEM];
	int					count[COUNT_ITEM];
	pthread_mutex_t		mutex;			
} free_buffers_t;

void			free_buffers_init(void);	
item_buffer_t*  free_buffers_get(int lendata);	
void			free_buffers_free(item_buffer_t* pibt);	
void			free_buffers_add(void);	
item_buffer_t*	free_buffers_find(item_buffer_t* pibt,int len);

char* malloc_char(item_buffer_t** ppibt,const char* obj,mvi_status_t* pstatus);
char* malloc_char_exp(item_buffer_t** ppibt,const char* pobj,int max_len,mvi_status_t* pstatus);
void* malloc_void(item_buffer_t** ppibt,int len,mvi_status_t* pstatus);

void  print_malloc(void);	 

#define MAX_BYTES_MALLOC	50000000

unsigned int get_bytes_malloc_yuri(void);

#ifdef __cplusplus
}
#endif


#endif /* MVI_MALLOC_H_ */ 
