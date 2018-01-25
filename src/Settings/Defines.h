#ifndef _DEFINES_H_H__
#define _DEFINES_H_H__

#define _safe_delete(X) if(X){delete X; X = 0;}

#if defined(_MSC_VER) && (_MSC_VER >= 1400 )
// Microsoft visual studio, version 2005 and higher.
 #define SAFE_SNPRINTF						_snprintf_s
 #define SAFE_SSCANF						sscanf_s
 #define SAFE_STRDUP						_strdup
 #define SAFE_GMTIME(TM, TIME)				gmtime_s(&TM, TIME)
 #define SAFE_LOCALTIME(TM, TIME)			localtime_s(&TM, TIME)
 #define SAFE_STRNCPY(dst, s, src, n)		strncpy_s(dst, s, src, n)
 #define SAFE_SFOPEN(f, n, m)				{f = NULL; fopen_s(&f,n,m);}
#elif
// Microsoft visual studio, version 6 and higher.
 #define SAFE_SNPRINTF						snprintf
 #define SAFE_SSCANF						sscanf
 #define SAFE_STRDUP						_strdup
 #define SAFE_GMTIME(TM, TIME)				{struct tm *__tm__ = gmtime(TIME); if (__tm__) {memcpy(&TM, __tm__, sizeof(TM));}}
 #define SAFE_LOCALTIME(TM, TIME)			{struct tm *__tm__ = localtime(TIME); if (__tm__) {memcpy(&TM, __tm__, sizeof(TM));}}
 #define SAFE_STRNCPY						strncpy
 #define SAFE_SFOPEN(f, n, m)				f = fopen(n,m)
#endif

#define _STATIC_STRING_BUFFER_SIZE			128


#endif//_DEFINES_H_H__
