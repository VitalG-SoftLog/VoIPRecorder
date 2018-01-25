#ifndef  _FILE_H_
#define  _FILE_H_

#include "os_include.h"

#ifdef __cplusplus
extern "C" {
#endif

void read_file(const char* pname,char* pbuf,int* len);
void write_file(const char* pname,const char* pbuf,int len);
void write_file_flush(const char* pname,char* pbuf,int len,FILE** fl);

#ifdef __cplusplus
}
#endif


#endif /* _FILE_ */ 
