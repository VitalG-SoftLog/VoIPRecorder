#ifndef  _MVI_PTHREAD_H_
#define  _MVI_PTHREAD_H_

#include "os_include.h"
#include "deftype.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef CRITICAL_SECTION		pthread_mutex_t;
typedef int		pthread_mutexattr_t;

int pthread_mutex_init(pthread_mutex_t * mutex,const pthread_mutexattr_t * attr);
int pthread_mutex_lock(pthread_mutex_t * mutex);
int pthread_mutex_trylock(pthread_mutex_t * mutex);
int pthread_mutex_unlock (pthread_mutex_t * mutex);
int pthread_mutex_destroy(pthread_mutex_t * mutex);



#ifdef __cplusplus
}
#endif


#endif /* _MVI_PTHREAD_H_ */ 
