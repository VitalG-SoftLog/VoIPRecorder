#include "os_include.h"
#include "tbl_sip_tables.h"
#include "mvi_log.h"
#include "tbl_sip_hash_tables.h"
#include "mvi_hash_functions.h"
#include "mvi_file.h"

//#include "sip_parser.h"
//#include "sip_transform.h"


////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////  TABLE REGISTRY  /////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
user_registry_t			user_registry[CFG_SIP_REG_TABLE_SIZE];
free_user_registry_t	free_user_registry;	
BOOL					is_lock_user_registry = FALSE;

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
user_registry_t* user_registry_get_first(void)
{
	free_user_registry.tek = 0;

	for ( free_user_registry.tek=0; free_user_registry.tek<CFG_SIP_REG_TABLE_SIZE; free_user_registry.tek++ ) {
		if ( user_registry[free_user_registry.tek].obj.puser_name ) {
			if ( sip_registry_inc(&user_registry[free_user_registry.tek]) ) {
				return &user_registry[free_user_registry.tek];	
			}
		}
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
user_registry_t* user_registry_get_next(void)
{
	free_user_registry.tek++;
	for (  ; free_user_registry.tek<CFG_SIP_REG_TABLE_SIZE; free_user_registry.tek++ ) {
		if ( user_registry[free_user_registry.tek].obj.puser_name ) {
			if ( sip_registry_inc(&user_registry[free_user_registry.tek]) ) {
				return &user_registry[free_user_registry.tek];	
			}
		}
	}
	return NULL;
}



//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void user_registry_reglament(void)
{
	int i;
	int count;
	time_t	tek_time;
	user_registry_t* puser_registry;

	count = user_registry_get_count();
	LOG_debug("===>Start REGLAMENT User Registry = (%d)",count);

	time(&tek_time);
	count = user_registry_get_count();

	for ( i=0; i<CFG_SIP_REG_TABLE_SIZE; i++ ) {
		if ( count <= 0 ) break;
		puser_registry = &user_registry[i];
		if ( puser_registry->obj.puser_name ) {
			count--;
			if ( !puser_registry->col_session ) {
				switch (puser_registry->obj.status) {
				case ST_END_USER_REGISTER:
				case ST_ERROR_REGISTER:
					sip_registry_delete(puser_registry);	
				case ST_REGISTER:
					if ( puser_registry->obj.expires + puser_registry->obj.last_message + MAX_TIME_REGISTRY < tek_time ) {
						puser_registry->obj.status = ST_END_EXPIRY_REGISTER;
						sip_registry_delete(puser_registry);	
					}			
				}
			}
		}
	}
	count = user_registry_get_count();
	LOG_debug("n===>Stop REGLAMENT User Registry = (%d)",count);
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
int user_registry_get_count(void)
{
	return free_user_registry.count_all - free_user_registry.count_free;
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void user_registry_init(void)
{
	user_registry_t* purt;
	int i;

	memset(user_registry,0,sizeof(user_registry));
	memset(&free_user_registry,0,sizeof(free_user_registry));

	if ( is_lock_user_registry ) if ( pthread_mutex_init(&free_user_registry.mutex,NULL) ) return ;

	if ( is_lock_user_registry ) if ( pthread_mutex_lock(&free_user_registry.mutex) ) return;
	free_user_registry.count_free = CFG_SIP_REG_TABLE_SIZE;
	free_user_registry.count_all = CFG_SIP_REG_TABLE_SIZE;
	free_user_registry.pfree = user_registry;
	purt = free_user_registry.pfree;
	for ( i=0; i<CFG_SIP_REG_TABLE_SIZE-1; i++ ) (purt+i)->next = (purt+i+1);
	if ( is_lock_user_registry ) pthread_mutex_unlock(&free_user_registry.mutex);
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
user_registry_t*  user_registry_get(void)
{
	user_registry_t* purt;

	if ( is_lock_user_registry ) if ( pthread_mutex_lock(&free_user_registry.mutex) ) return NULL;

	if ( !free_user_registry.pfree ) {
		if ( is_lock_user_registry ) pthread_mutex_unlock(&free_user_registry.mutex);
		return NULL;
	}
	purt = free_user_registry.pfree;
	free_user_registry.pfree = purt->next;
	purt->next = NULL;
	free_user_registry.count_free --;
	if ( is_lock_user_registry ) pthread_mutex_unlock(&free_user_registry.mutex);
	return purt;
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void  user_registry_free(user_registry_t* purt)
{
	if ( is_lock_user_registry ) if ( pthread_mutex_lock(&free_user_registry.mutex) ) return;
	if ( purt->pibt ) free_buffers_free(purt->pibt);
	memset(purt,0,sizeof(user_registry_t));
	purt->next = free_user_registry.pfree;
	free_user_registry.pfree = purt;
	free_user_registry.count_free ++;
	if ( is_lock_user_registry ) pthread_mutex_unlock(&free_user_registry.mutex);
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void user_registry_print(const char* pmsg,const tbl_registry_t* ptbl_registry)
{
#ifndef MVI_DEBUG_LOG
	(void)pmsg;
	(void)ptbl_registry;
	return;
#else
	static FILE*	fl = NULL;
	static u_int  i = 1; 
	char Buf[1024];
	snprintf(Buf,sizeof(Buf)-1,"\n(%u)=Status(%u)===(%s)=====%s"
		"UserName=%s\n"
		"Domen=%s\n"
		"Contact=%s\n"
		"From=%s\n"
		"CallId=%s  CSeg=%u Expires=%u\n"
		"UA  addr(%s:%u) mac(%02X:%02X:%02X:%02X:%02X:%02X)\n",i++,ptbl_registry->status,pmsg,
		ctime(&ptbl_registry->last_message),
		ptbl_registry->puser_name,ptbl_registry->request_uri,ptbl_registry->pcontact,ptbl_registry->pfrom,
		ptbl_registry->pcall_id,ptbl_registry->cseg,ptbl_registry->expires,
		ipaddr2char(htonl(ptbl_registry->task_ua.addr)),ptbl_registry->task_ua.port,
		ptbl_registry->task_ua.mac[0],ptbl_registry->task_ua.mac[1],ptbl_registry->task_ua.mac[2],
		ptbl_registry->task_ua.mac[3],ptbl_registry->task_ua.mac[4],ptbl_registry->task_ua.mac[5]);
	Buf[sizeof(Buf)-1] = 0;
	write_file_flush("table_registry.txt",Buf,strlen(Buf),&fl);

	snprintf(Buf,sizeof(Buf)-1,"PBX addr(%s:%u) mac(%02X:%02X:%02X:%02X:%02X:%02X)\n",
		ipaddr2char(htonl(ptbl_registry->task_pbx.addr)),ptbl_registry->task_pbx.port,
		ptbl_registry->task_pbx.mac[0],ptbl_registry->task_pbx.mac[1],ptbl_registry->task_pbx.mac[2],
		ptbl_registry->task_pbx.mac[3],ptbl_registry->task_pbx.mac[4],ptbl_registry->task_pbx.mac[5]);
	Buf[sizeof(Buf)-1] = 0;
	write_file_flush("table_registry.txt",Buf,strlen(Buf),&fl);
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
void user_registry_print_all(void)
{
	int count;
	int i;
	count = user_registry_get_count();

	for ( i=0; i<CFG_SIP_REG_TABLE_SIZE; i++ ) {
		if ( count <= 0 ) break;
		if ( user_registry[i].obj.puser_name ) {
			user_registry_print("All",&user_registry[i].obj);
			count--;
		}
	}
}




////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////  TABLE ACTIVE SESSION  ///////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
active_session_t			active_session[CFG_SIP_SESSIONS_TABLE_SIZE];
free_active_session_t		free_active_session;	
BOOL						is_lock_active_session = FALSE;
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
active_session_t* active_session_get_first(void)
{
	for ( free_active_session.tek=0; free_active_session.tek<CFG_SIP_SESSIONS_TABLE_SIZE; free_active_session.tek++ ) {
		if ( active_session[free_active_session.tek].obj.status ) {
			if ( sip_active_session_inc(&active_session[free_active_session.tek]) ) {
				return &active_session[free_active_session.tek];
			}
		}
	}
	return NULL;
}	

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
active_session_t* active_session_get_next(void)
{
	free_active_session.tek++;
	for ( ; free_active_session.tek<CFG_SIP_SESSIONS_TABLE_SIZE; free_active_session.tek++ ) {
		if ( active_session[free_active_session.tek].obj.status ) {
			if ( sip_active_session_inc(&active_session[free_active_session.tek]) ) {
				return &active_session[free_active_session.tek];
			}
		}
	}
	return NULL;
}	


/////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void active_session_reglament(void)
{
	int i;
	int count;
	time_t	tek_time;
	BOOL is_delete = FALSE;
	active_session_t* pactive_session;

	time(&tek_time);
	count = active_session_get_count();
	LOG_debug("===>Start REGLAMENT Active session = (%d)",count);
	for ( i=0; i<CFG_SIP_SESSIONS_TABLE_SIZE; i++ ) {
		if ( count <= 0 ) break;
		pactive_session = &active_session[i];
		if ( pactive_session->obj.status ) {
			is_delete = FALSE;
			count--;
			if ( !pactive_session->col_session ) {
				switch ( pactive_session->obj.status ) {
				case ST_NOT_DEFINED:
				case ST_INVITE_BELL:
					break;
				case ST_INVITE_START:
				case ST_INVITE_STOP:
					if ( pactive_session->obj.status == ST_INVITE_STOP ) {
						sip_active_session_delete(pactive_session);		
					}
					else {
						if ( pactive_session->obj.last_message + MAX_TIME_CONNECT < tek_time ) {
							pactive_session->obj.status = ST_INVITE_STOP;
							sip_active_session_delete(pactive_session);	
						}			
					}
					break;
				case ST_CLOSE_TRANSPORT:
					sip_active_session_delete(pactive_session);	
					break;
				default:
					LOG_debug("===>REGLAMENT NOT FOUND active_session[%d].status = %d",i,pactive_session->obj.status);
				}
			}
		}
	}
	count = active_session_get_count();
	LOG_debug("===>Stop REGLAMENT Active session = (%d)",count);
}	
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
int active_session_get_count(void)
{
	return free_active_session.count_all - free_active_session.count_free;
}


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void active_session_init(void)
{
	active_session_t* past;
	int i;


	memset(active_session,0,sizeof(active_session));
	memset(&free_active_session,0,sizeof(free_active_session));

	if ( is_lock_active_session ) if ( pthread_mutex_init(&free_active_session.mutex,NULL) ) return ;

	if ( is_lock_active_session ) if ( pthread_mutex_lock(&free_active_session.mutex) ) return;
	free_active_session.count_free = CFG_SIP_SESSIONS_TABLE_SIZE;
	free_active_session.count_all = CFG_SIP_SESSIONS_TABLE_SIZE;
	free_active_session.pfree = active_session;
	past = free_active_session.pfree;
	for ( i=0; i<CFG_SIP_SESSIONS_TABLE_SIZE-1; i++ ) (past+i)->next = (past+i+1);
	if ( is_lock_active_session ) pthread_mutex_unlock(&free_active_session.mutex);
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
active_session_t*  active_session_get(void)
{
	active_session_t* past;
	if ( is_lock_active_session ) if ( pthread_mutex_lock(&free_active_session.mutex) ) return NULL;

	if ( !free_active_session.pfree ) {
		if ( is_lock_active_session ) pthread_mutex_unlock(&free_active_session.mutex);
		return NULL;
	}
	past = free_active_session.pfree;
	free_active_session.pfree = past->next;
	past->next = NULL;

	free_active_session.count_free --;
	if ( is_lock_active_session ) pthread_mutex_unlock(&free_active_session.mutex);
	return past;
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void  active_session_free(active_session_t* past)
{
	if ( is_lock_active_session ) if ( pthread_mutex_lock(&free_active_session.mutex) ) return;
	if ( past->pibt ) free_buffers_free(past->pibt);
	memset(past,0,sizeof(active_session_t));
	past->next = free_active_session.pfree;
	free_active_session.pfree = past;
	free_active_session.count_free ++;
	if ( is_lock_active_session ) pthread_mutex_unlock(&free_active_session.mutex);
}


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void active_session_print(const char* pmsg,const tbl_session_t* ptbl_session)
{
#ifndef MVI_DEBUG_LOG
	(void)pmsg;
	(void)ptbl_session;
	return;
#else
	static FILE*	fl = NULL;
	static u_int  i = 1; 
	char Buf[1024];
	snprintf(Buf,sizeof(Buf)-1,"\n(%u)=Status(%u)=====(%s)=====%s"
		"BeginCall= %s"
		"RequestUri=%s\n"
		"From=%s\n"
		"To=%s\n"
		"CallId=%s  CSeg=%u\n"
		"ReceivePARAM=%s\n"
		"PARAM(0) sip addr(%s:%u) ",i++,ptbl_session->status,pmsg,
		ctime(&ptbl_session->last_message),
		ptbl_session->begin_call ? ctime(&ptbl_session->begin_call) : "null\n",
		ptbl_session->prequest_uri,ptbl_session->pfrom,ptbl_session->pto,ptbl_session->pcall_id,ptbl_session->cseg,	
		ptbl_session->pmap_param_recv == &ptbl_session->map_param_0 ? "0" : "1",	
		ipaddr2char(htonl(ptbl_session->map_param_0.sip_task.addr)),ptbl_session->map_param_0.sip_task.port);
	Buf[sizeof(Buf)-1] = 0;
	write_file_flush("table_active_session.txt",Buf,strlen(Buf),&fl);

	snprintf(Buf,sizeof(Buf)-1,"rtp addr(%s:%u) is_active(%d) (%d)(%d) registry - %s\n",
		ipaddr2char(htonl(ptbl_session->map_param_0.rtp_task.addr)),ptbl_session->map_param_0.rtp_task.port,
		ptbl_session->map_param_0.rtp_task.is_active,ptbl_session->map_param_0.rtp_task.codec,ptbl_session->map_param_0.rtp_task.rtpmap_time,
		ptbl_session->map_param_0.ptbl_registry ? ptbl_session->map_param_0.ptbl_registry->puser_name : "No");
	Buf[sizeof(Buf)-1] = 0;
	write_file_flush("table_active_session.txt",Buf,strlen(Buf),&fl);

	snprintf(Buf,sizeof(Buf)-1,"PARAM(1) sip addr(%s:%u) ",
		ipaddr2char(htonl(ptbl_session->map_param_1.sip_task.addr)),ptbl_session->map_param_1.sip_task.port);
	Buf[sizeof(Buf)-1] = 0;
	write_file_flush("table_active_session.txt",Buf,strlen(Buf),&fl);

	snprintf(Buf,sizeof(Buf)-1,"rtp addr(%s:%u) is_active(%d) (%d)(%d) registry - %s\n",
		ipaddr2char(htonl(ptbl_session->map_param_1.rtp_task.addr)),ptbl_session->map_param_1.rtp_task.port,
		ptbl_session->map_param_1.rtp_task.is_active,ptbl_session->map_param_1.rtp_task.codec,ptbl_session->map_param_1.rtp_task.rtpmap_time,
		ptbl_session->map_param_1.ptbl_registry ? ptbl_session->map_param_1.ptbl_registry->puser_name : "No");
	Buf[sizeof(Buf)-1] = 0;
	write_file_flush("table_active_session.txt",Buf,strlen(Buf),&fl);
#endif
}


