#ifndef  _LOG_H_
#define  _LOG_H_

#include "os_include.h"
#include "deftype.h"
#include "mvi_error_message.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  CRITICAL_LEVEL = 0,
  ERROR_LEVEL,
  WARNING_LEVEL,
  INFO_LEVEL,
  DEBUG_LEVEL,

  NONE_LEVEL
} log_level_t;


void LOG_init(const char* ppath_file);
void LOG_destroy(void);


void add_log_sip_protokol(const char* file,const char* pname,const char* paddr,uint16 port,const char* pbuf,int len);

void mvi_log(int level,char* file,int line,const char* format,...);
void mvi_log1(int level,char* file,int line,const char* format,va_list* pap);

#define LOG(level,format, ...) mvi_log(level,__FILE__,__LINE__,format, ## __VA_ARGS__);

#define LOG_info(format,...)		LOG(INFO_LEVEL,format, __VA_ARGS__)
#define LOG_warning(format,...)		LOG(WARNING_LEVEL,format, __VA_ARGS__)
#define LOG_error(format,...)		LOG(ERROR_LEVEL,format, __VA_ARGS__)
#define LOG_debug(format,...)		LOG(DEBUG_LEVEL,format, __VA_ARGS__)
#define LOG_critical(format,...)	LOG(CRITICAL_LEVEL,format,__VA_ARGS__)


char* ipaddr2char(unsigned long ipAddr);
mvi_status_t ipaddr2long(const char* pip_char,u_int* paddr);

#ifdef __cplusplus
}
#endif


#endif /* _LOG_H_ */ 
