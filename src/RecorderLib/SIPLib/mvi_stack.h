#ifndef  _MVI_STACK_H_
#define  _MVI_STACK_H_

#include "os_include.h"
#include "deftype.h"
#include "mvi_pthread.h"

#ifdef __cplusplus
extern "C" {
#endif


BOOL stack_pop_fifo(int* pbegin,int* pend,int* pmax,int* pnow); 
BOOL stack_push_fifo(int* pbegin,int* pend,int* pmax,int* pnow); 
BOOL stack_check_fifo(int* pbegin,int* pend,int* pmax,int* pnow);

BOOL stack_pop_lifo(int* pmax,int* pnow); 
BOOL stack_push_lifo(int* pmax,int* pnow); 





#ifdef __cplusplus
}
#endif


#endif /* _MVI_STACK_H_ */ 
