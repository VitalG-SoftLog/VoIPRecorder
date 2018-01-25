#ifndef  _SIP_HASC_TABLES_H_
#define  _SIP_HASC_TABLES_H_

#include "tbl_sip_tables.h"
#include "hash_tables_define_constant.h"
#include "mvi_transport.h"

#ifdef __cplusplus
extern "C" {
#endif

void sip_init_tables(void);


unsigned int	hash_function(void* k);
binary_tree_t*	hi_registry(binary_tree_t* pbtt,void** ppobj,int i);
binary_tree_t*	hs_registry(binary_tree_t* pbtt,void** ppobj,int i);
binary_tree_t*	hd_registry(binary_tree_t* pbtt,void** ppobj,int i);


user_registry_t* sip_registry_insert(char* puri,sip_t* psip,mvi_transport_t* pmvi_transport);	
user_registry_t* sip_registry_search(char* puri,sip_t* psip,mvi_transport_t* pmvi_transport);	
void sip_registry_delete(user_registry_t* purt);	
void sip_registry_free(user_registry_t* purt);	
BOOL sip_registry_inc(user_registry_t* purt);


binary_tree_t*	hi_active_session(binary_tree_t* pbtt,void** ppobj,int i);
binary_tree_t*	hs_active_session(binary_tree_t* pbtt,void** ppobj,int i);
binary_tree_t*	hd_active_session(binary_tree_t* pbtt,void** ppobj,int i);

active_session_t* sip_active_session_insert(char* pcall_id,sip_t* psip,mvi_transport_t* pmvi_transport);
active_session_t* sip_active_session_search(char* pcall_id,sip_t* psip,mvi_transport_t* pmvi_transport);	
void sip_active_session_delete(active_session_t* past);
void sip_active_session_free(active_session_t* past);	
BOOL sip_active_session_inc(active_session_t* past);


#ifdef __cplusplus
}
#endif


#endif /* _SIP_HASC_TABLES_H_ */ 
