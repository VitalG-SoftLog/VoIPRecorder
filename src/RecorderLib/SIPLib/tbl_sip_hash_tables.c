#include "tbl_sip_hash_tables.h"
#include "mvi_hash_functions.h"
#include "mvi_log.h"
#include "mvi_hash_functions.h"
//#include "sip_attribute_parse.h"
#include "sip_parser.h"
#include "sip_worker_parse.h"

hash_table_t* pht_registry;	
hash_table_t* pht_active_session;	

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void sip_init_tables(void)
{
	binary_tree_init();
	free_buffers_init();	
	user_registry_init();	
	active_session_init();

	pht_registry		=	hash_table_create(CFG_SIP_REG_HASHTABLE_SIZE,hash_function,hi_registry,hs_registry,hd_registry);
	pht_active_session	=	hash_table_create(CFG_SIP_SESSIONS_HASHTABLE_SIZE,hash_function,hi_active_session,hs_active_session,hd_active_session);
}


/////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
unsigned int hash_function(void* k)
{
	char* tmp;
	tmp = k;
	return SDBMHash(tmp,strlen(tmp));
}


//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
user_registry_t* sip_registry_insert(char* puri,sip_t* psip,mvi_transport_t* pmvi_transport)	
{
	user_registry_t*	purt = NULL;
	binary_tree_t*		pbtt;
	void*	pobj[3];

	pobj[0] = puri;
	pobj[1] = psip;
	pobj[2] = pmvi_transport;

	pbtt = hash_table_insert(pht_registry,(void**)&pobj,3);
	if ( pbtt )  purt = (user_registry_t*) pbtt->obj;
	return purt;
}
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void set_table_registry(sip_t* psip,user_registry_t* purt)
{
	psip_url_t	url;

	purt->obj.expires = psip->titles.expires_v;
	if ( purt->obj.expires )	purt->obj.status = ST_REGISTER;
	else						purt->obj.status = ST_END_USER_REGISTER;

	if ( purt->obj.pcall_id ) return;

	purt->obj.pcall_id	= malloc_char(&purt->pibt,(char*)psip->all_titles[psip->titles.call_id].value.p,&psip->status);
	if ( psip->status != Ok || !purt->obj.puser_name ) {
		psip->status = ER_SIP_PARSE_URL; 
		return;
	}
	purt->obj.cseg = psip->titles.cseg_v;

	psip->status =  psip_set_struct_url1((char*)psip->all_titles[psip->titles.contact].value.p,&url);
	if ( psip->status != Ok || !url.pdomen || !url.pname ) {
		psip->status = ER_SIP_PARSE_URL; 
		return ;
	}
	purt->obj.pcontact = malloc_char(&purt->pibt,url.pname,&psip->status);
	psip_back_struct_url(&url);
	if ( psip->status != Ok ) {
		psip->status = ER_SIP_PARSE_URL; 
		return ;
	}

	psip->status =  psip_set_struct_url1((char*)psip->all_titles[psip->titles.from].value.p,&url);
	if ( psip->status != Ok || !url.pdomen || !url.pname ) {
		psip->status = ER_SIP_PARSE_URL; 
		return ;
	}
	purt->obj.pfrom = malloc_char(&purt->pibt,url.pname,&psip->status);
	psip_back_struct_url(&url);
	if ( psip->status != Ok ) {
		psip->status = ER_SIP_PARSE_URL; 
		return ;
	}
}

#define VALUE_PHONE_TYPE_NEC_DPV	"NEC-DPV"
#define VALUE_PHONE_TYPE_NEC_DPT	"NEC-DPT"
#define VALUE_PHONE_TYPE_NEC_DPE	"NEC-DPE"
#define VALUE_PHONE_TYPE_NEC_DPD	"NEC-DPD"
#define VALUE_PHONE_TYPE_NEC_DPS	"NEC-DPS"
#define VALUE_PHONE_TYPE_NEC_SPT	"NEC-SPT"
#define VALUE_PHONE_TYPE_NEC_WPT	"NEC-WPT"
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
phone_type_t get_phone_type(sip_t* psip)
{
	if ( !psip->titles.user_agent ) return PHONE_TYPE_UNDEFINED;
	if ( !memcmp(psip->all_titles[psip->titles.user_agent].value.p,VALUE_PHONE_TYPE_NEC_DPV,strlen(VALUE_PHONE_TYPE_NEC_DPV)) ) return PHONE_TYPE_NEC;
	if ( !memcmp(psip->all_titles[psip->titles.user_agent].value.p,VALUE_PHONE_TYPE_NEC_DPT,strlen(VALUE_PHONE_TYPE_NEC_DPT)) ) return PHONE_TYPE_NEC;
	if ( !memcmp(psip->all_titles[psip->titles.user_agent].value.p,VALUE_PHONE_TYPE_NEC_DPE,strlen(VALUE_PHONE_TYPE_NEC_DPE)) ) return PHONE_TYPE_NEC;
	if ( !memcmp(psip->all_titles[psip->titles.user_agent].value.p,VALUE_PHONE_TYPE_NEC_DPD,strlen(VALUE_PHONE_TYPE_NEC_DPD)) ) return PHONE_TYPE_NEC;
	if ( !memcmp(psip->all_titles[psip->titles.user_agent].value.p,VALUE_PHONE_TYPE_NEC_DPS,strlen(VALUE_PHONE_TYPE_NEC_DPS)) ) return PHONE_TYPE_NEC;
	if ( !memcmp(psip->all_titles[psip->titles.user_agent].value.p,VALUE_PHONE_TYPE_NEC_SPT,strlen(VALUE_PHONE_TYPE_NEC_SPT)) ) return PHONE_TYPE_NEC;
	if ( !memcmp(psip->all_titles[psip->titles.user_agent].value.p,VALUE_PHONE_TYPE_NEC_WPT,strlen(VALUE_PHONE_TYPE_NEC_WPT)) ) return PHONE_TYPE_NEC;
	return PHONE_TYPE_OTHER;
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
binary_tree_t*	hi_registry(binary_tree_t* pbtt,void** ppobj,int i)
{
	char* puri;
	sip_t* psip;
	mvi_transport_t* pmvi_transport;
	user_registry_t* purt; 

	uint32* paddr_ua;	
	uint16* pport_ua;	
	uint8*	pmac_ua;
	uint32* paddr_pbx;	
	uint16* pport_pbx;	
	uint8*	pmac_pbx;

	switch (i) {
	case 3:
		break;
	default:
		return NULL;
	}
	puri			= ppobj[0];
	psip			= ppobj[1];
	pmvi_transport	= ppobj[2];

	if ( psip )				psip->status = ER_SMALL_BUFFER; 
	if ( pmvi_transport )	psip->status = ER_SMALL_BUFFER; 


	if ( !puri || !psip || !pmvi_transport ) return NULL;
	if ( !pbtt ) return pbtt;

	purt = user_registry_get();
	if ( !purt )  return NULL; 
	purt->obj.status	= ST_ERROR_REGISTER;
	time(&purt->obj.last_message);

	purt->obj.phone_type = get_phone_type(psip);

	purt->obj.puser_name	= malloc_char(&purt->pibt,puri,&psip->status);
	if ( psip->status != Ok || !purt->obj.puser_name ) {
		user_registry_free(purt);
		psip->status = ER_SIP_PARSE_URL; 
		return NULL;
	}
	if ( psip->titles.command_start == SIP_COM_REGISTER ) {
		set_table_registry(psip,purt);
		if ( psip->status != Ok ) {
			user_registry_free(purt);
			return NULL;
		}
		paddr_ua	= &pmvi_transport->pip_header->dst_addr;	
		pport_ua	= &pmvi_transport->pudp_header->dst_port;	
		pmac_ua		= pmvi_transport->peth_header->dst_mac;
		paddr_pbx	= &pmvi_transport->pip_header->src_addr;	
		pport_pbx	= &pmvi_transport->pudp_header->src_port;	
		pmac_pbx	= pmvi_transport->peth_header->src_mac;
	}
	else {
		if ( ntohl(pmvi_transport->pip_header->src_addr) == ipaddr_pbx ) {
			paddr_ua	= &pmvi_transport->pip_header->dst_addr;	
			pport_ua	= &pmvi_transport->pudp_header->dst_port;	
			pmac_ua		= pmvi_transport->peth_header->dst_mac;
			paddr_pbx	= &pmvi_transport->pip_header->src_addr;	
			pport_pbx	= &pmvi_transport->pudp_header->src_port;	
			pmac_pbx	= pmvi_transport->peth_header->src_mac;
		}
		else {
			paddr_ua	= &pmvi_transport->pip_header->src_addr;	
			pport_ua	= &pmvi_transport->pudp_header->src_port;	
			pmac_ua		= pmvi_transport->peth_header->src_mac;
			paddr_pbx	= &pmvi_transport->pip_header->dst_addr;	
			pport_pbx	= &pmvi_transport->pudp_header->dst_port;	
			pmac_pbx	= pmvi_transport->peth_header->dst_mac;
		}
		purt->obj.status	= ST_RECEIVE_NOTIFY;
	}

	purt->obj.task_ua.addr = ntohl(*paddr_ua); 
	purt->obj.task_ua.port = ntohs(*pport_ua);
	memcpy(purt->obj.task_ua.mac,pmac_ua,sizeof(purt->obj.task_ua.mac));
	purt->obj.task_pbx.addr = ntohl(*paddr_pbx); 
	purt->obj.task_pbx.port = ntohs(*pport_pbx);
	memcpy(purt->obj.task_pbx.mac,pmac_pbx,sizeof(purt->obj.task_pbx.mac));

	purt->col_session	++;
	psip->status = Ok;

	pbtt->obj = purt;
	return pbtt;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
user_registry_t* sip_registry_search(char* puri,sip_t* psip,mvi_transport_t* pmvi_transport)	
{
	user_registry_t*	purt = NULL;
	binary_tree_t*		pbtt;
	void*	pobj[4];

	pobj[0] = puri;
	pobj[1] = psip;
	pobj[2] = pmvi_transport;

	pbtt = hash_table_search(pht_registry,(void**)&pobj,3);
	if ( pbtt )  purt = (user_registry_t*) pbtt->obj;
	return purt;
}



//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
binary_tree_t*	hs_registry(binary_tree_t* pbtt,void** ppobj,int i)
{
	char* puri;
	sip_t* psip;
	mvi_transport_t* pmvi_transport;
	user_registry_t* purt; 
	psip_url_t	url;


	switch (i) {
	case 3:
		break;
	default:
		return NULL;
	}
	puri			= ppobj[0];
	psip			= ppobj[1];
	pmvi_transport	= ppobj[2];

	if ( !puri || !psip || !pmvi_transport ) return NULL;
	if ( !pbtt ) return pbtt;

	while ( pbtt )   {
		purt = pbtt->obj;
		if ( purt ) if ( purt->obj.puser_name ) {
			if ( !strcmp(purt->obj.puser_name,puri) ) break;
		}
		pbtt = pbtt->e;
	}
	if ( !pbtt ) return NULL;

	purt = pbtt->obj;

	if ( purt->obj.phone_type == PHONE_TYPE_UNDEFINED ) {
		purt->obj.phone_type = get_phone_type(psip);
		if ( purt->obj.phone_type != PHONE_TYPE_UNDEFINED && func_on_update_register ) 
			func_on_update_register(pctx_on_update_register,&purt->obj);
	}

	if ( psip->titles.command == SIP_COM_REGISTER || psip->titles.command_start == SIP_COM_REGISTER ) {
		time(&purt->obj.last_message);
		if ( psip->titles.command == SIP_COM_REGISTER ) {
			if ( psip->titles.x_regsvlistno && !purt->obj.px_regsvlistno )
				purt->obj.px_regsvlistno = malloc_char(&purt->pibt,(char*)psip->all_titles[psip->titles.x_regsvlistno].value.p,&psip->status);

			psip->status =  psip_set_struct_url1((char*)psip->all_titles[0].value.p,&url);
			if ( psip->status == Ok && url.pdomen ) {
				purt->obj.request_uri = malloc_char(&purt->pibt,url.pdomen,&psip->status);
				psip_back_struct_url(&url);
			}
			purt->obj.status = ST_RECEIVE_REGISTER_EXPIRES;
		}
		else {
			if ( psip->titles.command == SIP_MESSAGE_2XX && psip->titles.command_start == SIP_COM_REGISTER ) {
				set_table_registry(psip,purt);
			}
		}
	}
	purt->col_session ++;
	return pbtt;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
void sip_registry_delete(user_registry_t* purt)	
{
	void* pvoid[3];
	if ( func_on_end_register ) func_on_end_register(pctx_on_end_register,&purt->obj);

	if ( !purt->obj.puser_name ) return;
	pvoid[0] = purt->obj.puser_name;
	pvoid[1] = purt;
	hash_table_delete(pht_registry,pvoid,2);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
void sip_registry_free(user_registry_t* purt)	
{
	if ( purt->col_session <= 0 ) {
		LOG_error("internal_error: col_session sip_registry_free [%d]", purt->col_session);
		return;
	}
	if ( pthread_mutex_lock(&pht_registry->mutex) ) {
		LOG_error("internal_error: pthread_mutex_lock sip_registry_free [%d]", purt->col_session);
		return;
	}
	purt->col_session--;
	pthread_mutex_unlock(&pht_registry->mutex);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
BOOL sip_registry_inc(user_registry_t* purt)
{
	BOOL ret = FALSE;
	if ( pthread_mutex_lock(&pht_registry->mutex) ) {
		LOG_error("internal_error: pthread_mutex_lock sip_registry_inc [%d]", purt->col_session);
		return ret;
	}
	if ( purt->obj.puser_name ) {
		purt->col_session ++;
		ret = TRUE;
	} else {
		LOG_debug("internal_error: NULL puser_name");
	}
	pthread_mutex_unlock(&pht_registry->mutex);
	return ret;
}	


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
binary_tree_t*	hd_registry(binary_tree_t* pbtt,void** ppobj,int i)
{
	user_registry_t* purt; 

	(void)ppobj;(void)i;

	if ( !pbtt ) return pbtt;
	purt = pbtt->obj;
	if ( !purt ) return pbtt;

	if ( purt->col_session > 0 ) return NULL;

	user_registry_free(purt);
	return pbtt;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
active_session_t* sip_active_session_insert(char* pcall_id,sip_t* psip,mvi_transport_t* pmvi_transport)	
{
	active_session_t*	patt = NULL;
	binary_tree_t*		pbtt;
	void*	pobj[3];

	pobj[0] = pcall_id;
	pobj[1] = psip;
	pobj[2] = pmvi_transport;

	pbtt = hash_table_insert(pht_active_session,(void**)&pobj,3);
	if ( pbtt )  patt = (active_session_t*) pbtt->obj;
	return patt;
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
binary_tree_t*	hi_active_session(binary_tree_t* pbtt,void** ppobj,int i)
{
	char* pcall_id;
	sip_t* psip;
	active_session_t* past; 
	mvi_status_t ret;	
	mvi_transport_t* pmvi_transport;
	psip_url_t	url;

	if ( i !=  3 ) return NULL;
	pcall_id			= ppobj[0];
	psip				= ppobj[1];
	pmvi_transport		= ppobj[2];
	if ( !pcall_id || !psip || !pmvi_transport ) return NULL;
	if ( !pbtt ) return pbtt;

	past = active_session_get();
	if ( !past ) { psip->status = ER_SMALL_BUFFER; return NULL; }

	past->obj.pcall_id		= malloc_char(&past->pibt,pcall_id,&ret);

	psip->status =  psip_set_struct_url1((char*)psip->all_titles[psip->titles.from].value.p,&url);
	if ( psip->status != Ok || !url.pdomen || !url.pname ) {
		active_session_free(past);
		psip->status = ER_SIP_PARSE_URL; 
		return NULL;
	}
	past->obj.pfrom = malloc_char(&past->pibt,url.pname,&psip->status);
	psip_back_struct_url(&url);
	if ( psip->status != Ok ) {
		active_session_free(past);
		psip->status = ER_SIP_PARSE_URL; 
		return NULL;
	}

	psip->status =  psip_set_struct_url1((char*)psip->all_titles[psip->titles.to].value.p,&url);
	if ( psip->status != Ok || !url.pdomen || !url.pname ) {
		active_session_free(past);
		psip->status = ER_SIP_PARSE_URL; 
		return NULL;
	}
	past->obj.pto = malloc_char(&past->pibt,url.pname,&psip->status);
	psip_back_struct_url(&url);
	if ( psip->status != Ok ) {
		active_session_free(past);
		psip->status = ER_SIP_PARSE_URL; 
		return NULL;
	}

	psip->status =  psip_set_struct_url1((char*)psip->all_titles[0].value.p,&url);
	if ( psip->status != Ok || !url.pdomen || !url.pname ) {
		active_session_free(past);
		psip->status = ER_SIP_PARSE_URL; 
		return NULL;
	}
	past->obj.prequest_uri = malloc_char(&past->pibt,url.pname,&psip->status);
	psip_back_struct_url(&url);
	if ( psip->status != Ok ) {
		active_session_free(past);
		psip->status = ER_SIP_PARSE_URL; 
		return NULL;
	}
	time(&past->obj.last_message);
	past->obj.cseg = psip->titles.cseg_v;
	past->obj.map_param_0.sip_task.addr = ntohl(pmvi_transport->pip_header->src_addr); 
	past->obj.map_param_0.sip_task.port = ntohs(pmvi_transport->pudp_header->src_port);
	memcpy(past->obj.map_param_0.sip_task.mac,pmvi_transport->peth_header->src_mac,sizeof(past->obj.map_param_0.sip_task.mac));
	past->obj.map_param_1.sip_task.addr = ntohl(pmvi_transport->pip_header->dst_addr); 
	past->obj.map_param_1.sip_task.port = ntohs(pmvi_transport->pudp_header->dst_port);
	memcpy(past->obj.map_param_1.sip_task.mac,pmvi_transport->peth_header->dst_mac,sizeof(past->obj.map_param_1.sip_task.mac));

	past->obj.pmap_param_recv = &past->obj.map_param_0;
	past->obj.pmap_param_send = &past->obj.map_param_1;
	// SDP
	if ( psip->col_media ) {
		if ( !media2task(psip,0,&past->obj.pmap_param_recv->rtp_task) )
			memcpy(&past->obj.pmap_param_recv->rtp_task,&past->obj.pmap_param_recv->rtp_task_old,sizeof(rtp_task_t));
	}

	past->obj.status = ST_INVITE_START;

	past->col_session	++;
	pbtt->obj = past;
	return pbtt;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
active_session_t* sip_active_session_search(char* pcall_id,sip_t* psip,mvi_transport_t* pmvi_transport)	
{
	active_session_t*	past = NULL;
	binary_tree_t*		pbtt;
	void*	pobj[3];

	pobj[0] = pcall_id;
	pobj[1] = psip;
	pobj[2] = pmvi_transport;
	pbtt = hash_table_search(pht_active_session,(void**)&pobj,3);
	if ( pbtt )  past = (active_session_t*) pbtt->obj;
	return past;
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
binary_tree_t*	hs_active_session(binary_tree_t* pbtt,void** ppobj,int i)
{
	char* pcall_id;
	sip_t* psip;
	active_session_t*	past = NULL;
	mvi_transport_t* pmvi_transport;

	if ( i !=  3 ) return NULL;
	pcall_id	= ppobj[0];
	psip		= ppobj[1];
	pmvi_transport = ppobj[2];

	if ( !pcall_id || !psip || !pmvi_transport ) return NULL;
	if ( !pbtt ) return pbtt;

	while ( pbtt )   {
		past = pbtt->obj;
		if ( past ) if ( past->obj.pcall_id ) if ( !memcmp(past->obj.pcall_id,pcall_id,strlen(pcall_id)) ) break;
		pbtt = pbtt->e;
	}
	if ( pbtt ) {
		if ( !memcmp(past->obj.map_param_0.sip_task.mac,pmvi_transport->peth_header->src_mac,sizeof(past->obj.map_param_0.sip_task.mac)) ) {
			past->obj.pmap_param_recv = &past->obj.map_param_0;
			past->obj.pmap_param_send = &past->obj.map_param_1;
		}
		else {
			past->obj.pmap_param_recv = &past->obj.map_param_1;
			past->obj.pmap_param_send = &past->obj.map_param_0;
		}
		if ( psip->col_media ) {
			memcpy(&past->obj.pmap_param_recv->rtp_task_old,&past->obj.pmap_param_recv->rtp_task,sizeof(rtp_task_t));
			if ( !media2task(psip,0,&past->obj.pmap_param_recv->rtp_task) )
				memcpy(&past->obj.pmap_param_recv->rtp_task,&past->obj.pmap_param_recv->rtp_task_old,sizeof(rtp_task_t));
		}

		time(&past->obj.last_message);
		if ( is_sip_command(psip->titles.command) ) {
			switch (psip->titles.command) {
			case SIP_COM_INVITE:
				past->obj.status = ST_INVITE_START;
				past->obj.cseg = psip->titles.cseg_v;
				break;
			case SIP_COM_ACK:
				if ( past->obj.status == ST_INVITE_START ) {
					if ( past->obj.command != SIP_MESSAGE_2XX )	past->obj.status = ST_INVITE_STOP;
					else {
						past->obj.status = ST_INVITE_BELL;
						time(&past->obj.begin_call);
					}
				}
				break;
			case SIP_COM_BYE:
			case SIP_COM_CANCEL:
				past->obj.status = ST_INVITE_STOP;	
				break;
			}
		}
		past->obj.command = psip->titles.command;
		past->col_session ++;
	}
	return pbtt;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
void sip_active_session_free(active_session_t* past)	
{
	if ( !past->is_send_stop && past->obj.status == ST_INVITE_STOP ) {
		past->is_send_stop = 1;
		if ( func_on_end_session ) {
			func_on_end_session(pctx_on_end_session,&past->obj.map_param_0.rtp_task,&past->obj);
			func_on_end_session(pctx_on_end_session,&past->obj.map_param_1.rtp_task,&past->obj);
		}
		if ( func_on_call_end )	func_on_call_end(pctx_on_call_end,&past->obj);
	}
	if ( pthread_mutex_lock(&pht_active_session->mutex) ) {
		LOG_error("internal_error: pthread_mutex_lock sip_active_session_free [%d]", past->col_session);
		return;
	}
	past->col_session--;
	pthread_mutex_unlock(&pht_active_session->mutex);
}



//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
BOOL sip_active_session_inc(active_session_t* past)
{
	BOOL ret = FALSE;
	if ( pthread_mutex_lock(&pht_active_session->mutex) ) {
		LOG_error("internal_error: pthread_mutex_lock sip_active_session_inc [%d]", past->col_session);
		return ret;
	}
	if ( past->obj.status ) {
		past->col_session ++;
		ret = TRUE;
	}
	pthread_mutex_unlock(&pht_active_session->mutex);


	return ret;
}	

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
void sip_active_session_delete(active_session_t* past)	
{
	void* pvoid[3];
	if ( !past->is_send_stop ) {
		past->is_send_stop = 1;
		if ( func_on_end_session ) {
			func_on_end_session(pctx_on_end_session,&past->obj.map_param_0.rtp_task,&past->obj);
			func_on_end_session(pctx_on_end_session,&past->obj.map_param_1.rtp_task,&past->obj);
		}
		if ( func_on_call_end )	func_on_call_end(pctx_on_call_end,&past->obj);
	}

	pvoid[0] = past->obj.pcall_id;
	pvoid[1] = past;

	hash_table_delete(pht_active_session,pvoid,2);
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void sip_close_session(char* pcall_id)
{
	void* pvoid[3];

	pvoid[0] = pcall_id;
	pvoid[1] = NULL;

	hash_table_delete(pht_active_session,pvoid,2);

}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
binary_tree_t*	hd_active_session(binary_tree_t* pbtt,void** ppobj,int i)
{
	active_session_t* past = NULL; 

	(void)ppobj;(void)i;

	if ( !pbtt ) return pbtt;
	if ( ppobj[1] ) {
		while ( pbtt ) { 
			past = pbtt->obj;
			if ( !past ) return NULL;
			if ( past == ppobj[1] ) break;
			pbtt = pbtt->e; 
		}
	}
	else {
		while ( pbtt ) { 
			past = pbtt->obj;
			if ( !past ) return NULL;
			if ( !memcmp(past->obj.pcall_id,(char*)ppobj[0],strlen(past->obj.pcall_id)) ) break;
			pbtt = pbtt->e; 
		}
	}
	if ( !pbtt ) return pbtt;

	past->obj.status = ST_INVITE_STOP;
	if ( past->col_session > 0 ) return NULL;

	if ( past->puser_registry0 ) {
		sip_registry_free(past->puser_registry0);
		past->puser_registry0 = NULL;
		past->obj.map_param_0.ptbl_registry = NULL;
	}
	if ( past->puser_registry1 ) {
		sip_registry_free(past->puser_registry1);
		past->puser_registry1 = NULL;
		past->obj.map_param_1.ptbl_registry = NULL;
	}

	active_session_free(past);
	return pbtt;
}


