#include "os_include.h"
#include "sip_worker_parse.h"
#include "mvi_log.h"
#include "sip_parser.h"
#include "mvi_file.h"
#include "mvi_transport.h"
#include "tbl_sip_tables.h"
#include "tbl_sip_hash_tables.h"

uint32 ipaddr_pbx;
DWORD thread_reglament = 0;
#define MVI_END_THREAD		WM_USER + 10

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
DWORD WINAPI SipReglamentThread(void* pv)
{
	int fl;
	MSG	msg;
	(void)pv;

	thread_reglament =  GetCurrentThreadId();
	LOG_debug("Start SipReglamentThread = %u",thread_reglament);
	while (1) {
		fl = PeekMessage(&msg,NULL,0,0,PM_REMOVE); 
		if ( fl == -1 )							{ thread_reglament = 0; break; }
		if ( msg.message == MVI_END_THREAD )	{ thread_reglament = 0; break; }
		_sleep(10000);
		fl = PeekMessage(&msg,NULL,0,0,PM_REMOVE); 
		if ( fl == -1 )							{ thread_reglament = 0; break; }
		if ( msg.message == MVI_END_THREAD )	{ thread_reglament = 0; break; }
		user_registry_reglament();
		active_session_reglament();
	}
	LOG_debug("Stop SipReglamentThread = %u",thread_reglament);
	return 0;
}

BOOL is_init = FALSE;
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
void sip_init(uint32 ipaddr)
{
	ipaddr_pbx = ipaddr;
	if ( is_init ) return;
	is_init = TRUE;
	LOG_init("log.txt");
	sip_init_tables();
	if ( !( CreateThread(NULL,0,&SipReglamentThread,NULL,0,NULL)) ) LOG_error("Create thread error"); 
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
void sip_destroy(void)
{
	func_on_register = NULL;
	pctx_on_register = NULL;

	func_on_end_register = NULL;
	pctx_on_end_register = NULL;

	func_on_call_start = NULL;
	pctx_on_call_start = NULL;

	func_on_call_end = NULL;
	pctx_on_call_end = NULL;

	func_on_new_session = NULL;
	pctx_on_new_session = NULL;

	func_on_end_session = NULL;
	pctx_on_end_session = NULL;

	func_on_update_session = NULL;
	pctx_on_update_session = NULL;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
int sip_worker_parse(u_char* pbuf,int len)
{
	static int sip_mesg = 0;
	static int not_sip_mesg = 0;
	sip_t sip;
	mvi_transport_t	mvi_transport; 
	mvi_status_t ret = Ok;
	char* pdata;
	int data_len;
	int ip_len;
	(void) len;


	mvi_transport.peth_header	= (eth_header_t*)pbuf;
	mvi_transport.pip_header	= (ip_header_t*) (pbuf+sizeof(eth_header_t));
	ip_len = (mvi_transport.pip_header->ver_ihl & 0xf) * 4;
	mvi_transport.pudp_header	= (udp_header_t*) ((u_char*)mvi_transport.pip_header + ip_len);
	pdata		= (char*)mvi_transport.pudp_header + sizeof(udp_header_t);
	data_len	= ntohs(mvi_transport.pudp_header->len) - sizeof(udp_header_t);

	memset(&sip,0,sizeof(sip));
	LOG_info("SIP message - (%u)  OTHER message - (%u)",sip_mesg,not_sip_mesg);
	sip.status = buf2sip(pdata,&data_len,&sip);
	if ( sip.status != Ok ) {
		not_sip_mesg ++;
		return 0;
	}
	sip_mesg++;

	print_struct_sip_all("sip_parser_all.txt",&sip);

	ret = sip_worker(&sip,&mvi_transport);
	LOG_debug("<sip_worker> - %s",mvi_get_error_msg(ret));
	return 1;
}


#define NEC_FIRST_REGISTRY_USER_NAME	"necsipuser"
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
user_registry_t* sip_check_registry(mvi_transport_t* pmvi_transport,sip_t* psip,sip_url_t* purl_from,sip_url_t* purl_to)
{
	user_registry_t* puser_registry;
	char* pname = NULL;

	if  ( psip->titles.command == SIP_COM_REGISTER || psip->titles.command_start == SIP_COM_REGISTER ) {
		if ( purl_from->pname ) pname = purl_from->pname;
		else {
			if ( purl_from->pname ) pname = purl_from->pname;
			else  { psip->status = ER_SIP_PARSE_URL; return NULL; }  
		}
		if ( !memcmp(pname,NEC_FIRST_REGISTRY_USER_NAME,strlen(NEC_FIRST_REGISTRY_USER_NAME)) ) {
			psip->status = OK_REGISTRY_NOT_FOUND;
			return NULL;
		}
	}

	if ( is_sip_command(psip->titles.command) ) {
		switch ( psip->titles.command ) {
		case SIP_COM_REGISTER:
			puser_registry = sip_registry_search(pname,psip,pmvi_transport);	
			if ( !puser_registry ) {
				psip->titles.command = SIP_COM_NOTIFY;
				psip->titles.command_start = SIP_COM_NOTIFY;
				puser_registry = sip_registry_insert(pname,psip,pmvi_transport);	
				if ( puser_registry && func_on_register ) func_on_register(pctx_on_register,&puser_registry->obj);
			}
			return puser_registry;
		default:
			return NULL;
		}
	}
	if  ( psip->titles.command != SIP_MESSAGE_2XX  ) return NULL;
	switch ( psip->titles.command_start ) {
	case SIP_COM_NOTIFY:
		if ( ntohl(pmvi_transport->pip_header->src_addr) == ipaddr_pbx ) {
			if ( !purl_from->pname ) return NULL;
			pname = purl_from->pname;
		}
		else {
			if ( ntohl(pmvi_transport->pip_header->dst_addr) == ipaddr_pbx ) {
				if ( !purl_to->pname ) return NULL;
				pname = purl_to->pname;
			}
			else return NULL;
		}
		break;
	case SIP_COM_REGISTER:
		break;
	default:
		return NULL;
	}
	puser_registry = sip_registry_search(pname,psip,pmvi_transport);	
	if ( !puser_registry ) {
		puser_registry = sip_registry_insert(pname,psip,pmvi_transport);	
		if ( puser_registry && func_on_register ) func_on_register(pctx_on_register,&puser_registry->obj);
	}
	return puser_registry;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
active_session_t* sip_check_active_session(mvi_transport_t* pmvi_transport,sip_t* psip,sip_url_t* purl_from,sip_url_t* purl_to)
{
	active_session_t* pactive_session = NULL;

	if ( is_sip_command(psip->titles.command) ) {
		switch ( psip->titles.command ) {
		case SIP_COM_INVITE:
			pactive_session = sip_active_session_search((char*)psip->all_titles[psip->titles.call_id].value.p,psip,pmvi_transport);
			if ( !pactive_session ){
				pactive_session = sip_active_session_insert((char*)psip->all_titles[psip->titles.call_id].value.p,psip,pmvi_transport);
			}
			if ( pactive_session ) {
				if ( !pactive_session->puser_registry0 && !pactive_session->puser_registry1 ) {
					if ( purl_from->pname )	{
						pactive_session->puser_registry0 = sip_registry_search(purl_from->pname,psip,pmvi_transport);	
						if ( pactive_session->puser_registry0 ) pactive_session->obj.map_param_0.ptbl_registry = &pactive_session->puser_registry0->obj;
					}
					if ( purl_to->pname )	{
						pactive_session->puser_registry1 = sip_registry_search(purl_to->pname,psip,pmvi_transport);	
						if ( pactive_session->puser_registry1 ) pactive_session->obj.map_param_1.ptbl_registry = &pactive_session->puser_registry1->obj;
					}
				}
				if ( !pactive_session->is_send_start && func_on_call_start )	{
					pactive_session->is_send_start = 1;				
					func_on_call_start(pctx_on_call_start,&pactive_session->obj);
				}
				break;
			}
			return NULL;
		case SIP_COM_BYE:
		case SIP_COM_ACK:
		case SIP_COM_REFER:
			pactive_session = sip_active_session_search((char*)psip->all_titles[psip->titles.call_id].value.p,psip,pmvi_transport);
			if ( pactive_session ) break;
			return NULL;
		default:
			return NULL;
		}
	}
	else {
		switch ( psip->titles.command_start ) {
		case SIP_COM_INVITE:
		case SIP_COM_BYE:
		case SIP_COM_ACK:
		case SIP_COM_CANCEL:
		case SIP_COM_REFER:
			pactive_session = sip_active_session_search((char*)psip->all_titles[psip->titles.call_id].value.p,psip,pmvi_transport);
			if ( pactive_session ) break;
			return NULL;
		default:
			return NULL;
		}
	}
	return pactive_session;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
mvi_status_t sip_worker(sip_t* psip,mvi_transport_t* pmvi_transport)
{
	user_registry_t* puser_registry = NULL;	 
	active_session_t* pactive_session = NULL;
	sip_url_t url_to;
	sip_url_t url_from;

	psip->status = sip_set_struct_url((char*)psip->all_titles[psip->titles.to].value.p,&url_to);
	if ( psip->status != Ok ) return psip->status;
	psip->status = sip_set_struct_url((char*)psip->all_titles[psip->titles.from].value.p,&url_from);
	if ( psip->status != Ok ) return psip->status;

	puser_registry = sip_check_registry(pmvi_transport,psip,&url_from,&url_to);
	if ( !puser_registry ) {
		if ( psip->status != Ok) return psip->status;
		pactive_session = sip_check_active_session(pmvi_transport,psip,&url_from,&url_to);
		if ( !pactive_session ) return psip->status;
	}


	print_struct_sip_worker("sip_parser_worker.txt",psip);
	if ( pactive_session ) {
		if ( memcmp(&pactive_session->obj.pmap_param_recv->rtp_task,
			&pactive_session->obj.pmap_param_recv->rtp_task_old,
			sizeof(pactive_session->obj.pmap_param_recv->rtp_task)) ) {
				if ( !pactive_session->obj.pmap_param_recv->rtp_task_old.port ) {
					if ( func_on_new_session )	func_on_new_session(pctx_on_new_session,&pactive_session->obj.pmap_param_recv->rtp_task,&pactive_session->obj);
				}
				else {
					if ( func_on_update_session )	func_on_update_session(pctx_on_update_session,&pactive_session->obj.pmap_param_recv->rtp_task,&pactive_session->obj);
				}
				memcpy(&pactive_session->obj.pmap_param_recv->rtp_task_old,
					&pactive_session->obj.pmap_param_recv->rtp_task,
					sizeof(pactive_session->obj.pmap_param_recv->rtp_task));
		}

		sip_active_session_free(pactive_session);	
	}
	if ( puser_registry ) {
		sip_registry_free(puser_registry);	
	}
	LOG_debug("table registry (%d) table session (%d)",user_registry_get_count(),active_session_get_count());
	return psip->status;
}




///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
void print_struct_sip_all(const char* pname_file,sip_t* psip)
{
	static FILE*	fl = NULL;
	print_struct_sip(pname_file,psip,&fl);
}
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
void print_struct_sip_worker(const char* pname_file,sip_t* psip)
{
	static FILE*	fl = NULL;
	print_struct_sip(pname_file,psip,&fl);
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
void print_struct_sip(const char* pname_file,sip_t* psip,FILE** fl)
{
#ifndef MVI_DEBUG_LOG
	(void)pname_file;
	(void)psip;
	(void)fl;
	return;
#else
	static u_int  i = 1; 
	char Buf[1024];
	snprintf(Buf,sizeof(Buf)-1,"\n(%u)===========================================================================\n%s\n%s\n%s\n%s\n%s\n",i++,
		(char*)psip->all_titles[0].name.p,	
		(char*)psip->all_titles[psip->titles.call_id].name.p,	
		(char*)psip->all_titles[psip->titles.from].name.p,	
		(char*)psip->all_titles[psip->titles.to].name.p,	
		(char*)psip->all_titles[psip->titles.cseg].name.p);
	Buf[sizeof(Buf)-1] = 0;
	write_file_flush(pname_file,Buf,strlen(Buf),fl);
#endif
}



void (*func_on_register)(void* pctx,const tbl_registry_t* ptbl_registry) = NULL;
void* pctx_on_register = NULL;
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
void set_on_register(void (*func)(void* pctx,const tbl_registry_t* ptbl_registry),void* pctx)
{
	func_on_register = func;
	pctx_on_register = pctx;
}

void (*func_on_update_register)(void* pctx,const tbl_registry_t* ptbl_registry) = NULL;
void* pctx_on_update_register = NULL;
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
void set_on_update_register(void (*func)(void* pctx,const tbl_registry_t* ptbl_registry),void* pctx)
{
	func_on_update_register = func;
	pctx_on_update_register = pctx;
}


void (*func_on_end_register)(void* pctx,const tbl_registry_t* ptbl_registry) = NULL;
void* pctx_on_end_register = NULL;
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
void set_on_end_register(void (*func)(void* pctx,const tbl_registry_t* ptbl_registry),void* pctx)
{
	func_on_end_register = func;
	pctx_on_end_register = pctx;
}

void (*func_on_call_start)(void* pctx,const tbl_session_t* ptbl_session) = NULL;
void* pctx_on_call_start = NULL;
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
void set_on_call_start(void (*func)(void* pctx,const tbl_session_t* ptbl_session),void* pctx)
{
	func_on_call_start = func;
	pctx_on_call_start = pctx;
}

void (*func_on_call_end)(void* pctx,const tbl_session_t* ptbl_session) = NULL;
void* pctx_on_call_end = NULL;
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
void set_on_call_end(void (*func)(void* pctx,const tbl_session_t* ptbl_session),void* pctx)
{
	func_on_call_end = func;
	pctx_on_call_end = pctx;
}

void (*func_on_new_session)(void* pctx,const rtp_task_t* prtp_task,const tbl_session_t* ptbl_session) = NULL;
void* pctx_on_new_session = NULL;
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
void set_on_new_session(void (*func)(void* pctx,const rtp_task_t* prtp_task,const tbl_session_t* ptbl_session),void* pctx)
{
	func_on_new_session = func;
	pctx_on_new_session = pctx;
}

void (*func_on_end_session)(void* pctx,const rtp_task_t* prtp_task,const tbl_session_t* ptbl_session) = NULL;
void* pctx_on_end_session = NULL;
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
void set_on_end_session(void (*func)(void* pctx,const rtp_task_t* prtp_task,const tbl_session_t* ptbl_session),void* pctx)
{
	func_on_end_session = func;
	pctx_on_end_session = pctx;
}

void (*func_on_update_session)(void* pctx,const rtp_task_t* prtp_task,const tbl_session_t* ptbl_session) = NULL;
void* pctx_on_update_session = NULL;
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
void set_on_update_session(void (*func)(void* pctx,const rtp_task_t* prtp_task,const tbl_session_t* ptbl_session),void* pctx)
{
	func_on_update_session = func;
	pctx_on_update_session = pctx;
}



///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
void on_register(void* pctx,const tbl_registry_t* ptbl_registry)
{
	(void) pctx;
	user_registry_print("on_register",ptbl_registry);
}
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
void on_end_register(void* pctx,const tbl_registry_t* ptbl_registry)
{
	(void) pctx;
	user_registry_print("on_end_register",ptbl_registry);
}
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
void on_call_start(void* pctx,const tbl_session_t* ptbl_session)
{
	(void) pctx;
	active_session_print("on_call_start",ptbl_session);
}
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
void on_call_end(void* pctx,const tbl_session_t* ptbl_session)
{
	(void) pctx;
	active_session_print("on_call_end",ptbl_session);
}
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
void on_new_session(void* pctx,const rtp_task_t* prtp_task,const tbl_session_t* ptbl_session)
{
	(void) pctx;
	(void) prtp_task;
	active_session_print("on_new_session",ptbl_session);
}
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
void on_end_session(void* pctx,const rtp_task_t* prtp_task,const tbl_session_t* ptbl_session)
{
	(void) pctx;
	(void) prtp_task;
	active_session_print("on_end_session",ptbl_session);
}
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
void on_update_session(void* pctx,const rtp_task_t* prtp_task,const tbl_session_t* ptbl_session)
{
	(void) pctx;
	(void) prtp_task;
	active_session_print("on_update_session",ptbl_session);
}



