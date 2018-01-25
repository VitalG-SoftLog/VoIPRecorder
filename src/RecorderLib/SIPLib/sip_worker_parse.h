#ifndef  _SIP_WORKER_PARSE_H_
#define  _SIP_WORKER_PARSE_H_

#include "os_include.h"
#include "sip.h"
#include "sip.h"
#include "mvi_transport.h"

#ifdef __cplusplus
extern "C" {
#endif
extern uint32 ipaddr_pbx;

mvi_status_t sip_worker(sip_t* psip,mvi_transport_t* pmvi_transport);

void print_struct_sip_all(const char* pname_file,sip_t* psip);
void print_struct_sip_worker(const char* pname_file,sip_t* psip);
void print_struct_sip(const char* pname_file,sip_t* psip,FILE** fl);


extern void (*func_on_register)(void* pctx,const tbl_registry_t* ptbl_registry);
extern void* pctx_on_register;
extern void (*func_on_update_register)(void* pctx,const tbl_registry_t* ptbl_registry);
extern void* pctx_on_update_register;
extern void (*func_on_end_register)(void* pctx,const tbl_registry_t* ptbl_registry);
extern void* pctx_on_end_register;
extern void (*func_on_call_start)(void* pctx,const tbl_session_t* ptbl_session);
extern void* pctx_on_call_start;
extern void (*func_on_call_end)(void* pctx,const tbl_session_t* ptbl_session);
extern void* pctx_on_call_end;
extern void (*func_on_new_session)(void* pctx,const rtp_task_t* prtp_task,const tbl_session_t* ptbl_session);
extern void* pctx_on_new_session;
extern void (*func_on_end_session)(void* pctx,const rtp_task_t* prtp_task,const tbl_session_t* ptbl_session);
extern void* pctx_on_end_session;
extern void (*func_on_update_session)(void* pctx,const rtp_task_t* prtp_task,const tbl_session_t* ptbl_session);
extern void* pctx_on_update_session;


void on_register(void* pctx,const tbl_registry_t* ptbl_registry);
void on_end_register(void* pctx,const tbl_registry_t* ptbl_registry);
void on_call_start(void* pctx,const tbl_session_t* ptbl_session);
void on_call_end(void* pctx,const tbl_session_t* ptbl_session);
void on_new_session(void* pctx,const rtp_task_t* prtp_task,const tbl_session_t* ptbl_session);
void on_end_session(void* pctx,const rtp_task_t* prtp_task,const tbl_session_t* ptbl_session);
void on_update_session(void* pctx,const rtp_task_t* prtp_task,const tbl_session_t* ptbl_session);





#ifdef __cplusplus
}
#endif


#endif /* _SIP_WORKER_PARSE_ */ 
