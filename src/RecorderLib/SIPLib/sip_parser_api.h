#ifndef  _SIP_PARSER_API_H_
#define  _SIP_PARSER_API_H_

#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <io.h>

#include "deftype.h"

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct sip_task {
	uint32		addr;
	uint16		port;
    uint8		mac[6];
} sip_task_t;

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct rtp_task {
	uint32		addr;
	uint16		port;
	uint32		rtpmap_time;
	BOOL		is_active;
	uint8		codec;
} rtp_task_t;


////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////  TABLE REGISTRY  /////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

typedef enum user_status {
	ST_RECEIVE_NOTIFY = 0,
	ST_RECEIVE_REGISTER_EXPIRES,
	ST_REGISTER,
	ST_END_USER_REGISTER ,
	ST_END_EXPIRY_REGISTER ,
	ST_ERROR_REGISTER 
} user_status_t;

typedef enum phone_type {
	PHONE_TYPE_UNDEFINED = 0,
	PHONE_TYPE_NEC,
	PHONE_TYPE_OTHER,
} phone_type_t;


typedef struct tbl_registry {
	char*			pcontact;		// user_name@domen
	char*			pfrom;			// user_name@domen	
	char*			request_uri;	// domen 
	char*			puser_name;		// user_name
	char*			px_regsvlistno; // NEC UA field (unique)

	char*			pcall_id;
	int				cseg;	
	int				expires;
	time_t			last_message;

	phone_type_t		phone_type;
	user_status_t		status;
	sip_task_t			task_ua;
	sip_task_t			task_pbx;

} tbl_registry_t;

typedef enum {
  SIP_MESSAGE_1XX   = 0,
  SIP_MESSAGE_2XX,
  SIP_MESSAGE_3XX,
  SIP_MESSAGE_4XX,
  SIP_MESSAGE_5XX,
  SIP_MESSAGE_6XX,
  SIP_COM_INVITE,
  SIP_COM_ACK,
  SIP_COM_BYE,
  SIP_COM_CANCEL,
  SIP_COM_REGISTER,
  SIP_COM_OPTION,
  SIP_COM_INFO,
  SIP_COM_PRACK,
  SIP_COM_UPDATE,
  SIP_COM_SUBSCRIBE,
  SIP_COM_NOTIFY,
  SIP_COM_REFER,
  SIP_COM_MESSAGE,
  SIP_COM_PUBLISH
} sip_method_t;

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////  TABLE ACTIVE SESSION  ///////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
typedef enum {
	ST_NOT_DEFINED = 0,
	ST_INVITE_START,
	ST_INVITE_BELL,
	ST_INVITE_STOP,
	ST_CLOSE_TRANSPORT
} session_status_t;

typedef struct map_param {
	sip_task_t		sip_task;
	rtp_task_t		rtp_task;
	rtp_task_t		rtp_task_old;
	tbl_registry_t*		ptbl_registry;
}map_param_t;

typedef struct tbl_session {
	char*	pcall_id;			
	char*	pfrom;				// user_name@domen
	char*	pto;					// user_name@domen
	char*	prequest_uri;		// user_name@domen
	int		cseg;	

	sip_method_t	command;

	map_param_t		map_param_0; // UAC description params of the first SIP command created this session
	map_param_t		map_param_1; // UAS

	map_param_t*	pmap_param_recv; // points to the side (map_param_0|1) which sent the SIP command being processed
	map_param_t*	pmap_param_send; // the opposite side

	session_status_t	status;
	time_t  last_message;
	time_t  begin_call;
} tbl_session_t;

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

void sip_init(uint32 ipaddr);
void sip_destroy(void);
int sip_worker_parse(u_char* pbuf,int len);
void sip_close_session(char* pcall_id);


////  
void set_on_register(void (*func)(void* pctx,const tbl_registry_t* ptbl_registry),void* pctx);
void set_on_update_register(void (*func)(void* pctx,const tbl_registry_t* ptbl_registry),void* pctx);
void set_on_end_register(void (*func)(void* pctx,const tbl_registry_t* ptbl_registry),void* pctx);
void set_on_call_start(void (*func)(void* pctx,const tbl_session_t* ptbl_session),void* pctx);
void set_on_call_end(void (*func)(void* pctx,const tbl_session_t* ptbl_session),void* pctx);
void set_on_new_session(void (*func)(void* pctx,const rtp_task_t* prtp_task,const tbl_session_t* ptbl_session),void* pctx);
void set_on_end_session(void (*func)(void* pctx,const rtp_task_t* prtp_task,const tbl_session_t* ptbl_session),void* pctx);
void set_on_update_session(void (*func)(void* pctx,const rtp_task_t* prtp_task,const tbl_session_t* ptbl_session),void* pctx);
////

#ifdef __cplusplus
}
#endif

#endif /* _SIP_PARSER_API_ */ 
