#ifndef  _SIP_TABLES_H_
#define  _SIP_TABLES_H_

#include "sip_parser_api.h"
#include "os_include.h"
#include "mvi_hash_table.h"
#include "mvi_malloc.h"
#include "sip.h"
#include "mvi_transport.h"

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////  TABLE REGISTRY  /////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct user_registry {
	tbl_registry_t		obj;

	item_buffer_t*			pibt;
	struct user_registry*	next;
	int		col_session;				
} user_registry_t;

typedef struct free_user_registry {
	user_registry_t*	pfree;
	int					count_all;
	int					count_free;
	int					count_registry;
	int					tek;
	pthread_mutex_t		mutex;			
} free_user_registry_t;

user_registry_t*	user_registry_get_first(void);
user_registry_t*	user_registry_get_next(void);
void				user_registry_reglament(void);	
int					user_registry_get_count(void);
void				user_registry_init(void);	
user_registry_t*	user_registry_get(void);	
void				user_registry_free(user_registry_t* puser_registry);

void				user_registry_print(const char* pmsg,const tbl_registry_t* ptbl_registry);
void				user_registry_print_all(void);

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////  TABLE ACTIVE SESSION  ///////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct active_session {
	tbl_session_t	obj;

	user_registry_t*	puser_registry0;
	user_registry_t*	puser_registry1;

	unsigned		is_send_start:1;
	unsigned		is_send_stop:1;
	unsigned		is_send_new_session0:1;
	unsigned		is_send_end_session0:1;
	unsigned		is_send_new_session1:1;
	unsigned		is_send_end_session1:1;

	item_buffer_t*			pibt;
	struct active_session*	next;
	int		col_session;				
} active_session_t;

typedef struct free_active_session {
	active_session_t*	pfree;
	int					count_all;
	int					count_free;
	int					tek;
	pthread_mutex_t		mutex;			
} free_active_session_t;

active_session_t*	active_session_get_first(void);
active_session_t*	active_session_get_next(void);
void				active_session_reglament(void);	
int					active_session_get_count(void);
void				active_session_init(void);	
active_session_t*	active_session_get(void);	
void				active_session_free(active_session_t* purt);	

void active_session_print(const char* pmsg,const tbl_session_t* ptbl_session);


#ifdef __cplusplus
}
#endif


#endif /* _SIP_TABLES_H_ */ 
