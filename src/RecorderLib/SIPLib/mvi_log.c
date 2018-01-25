#include "mvi_log.h"
#include "mvi_file.h"
#include "mvi_pthread.h"


static const char *log_level_name[] = {
	"critical", // SAG_CRITICAL_LEVEL = 0,
	"error",    // SAG_ERROR_LEVEL,
	"warning",  // SAG_WARNING_LEVEL,
	"info",     // SAG_INFO_LEVEL,
	"debug"     // SAG_DEBUG_LEVEL
};

FILE*				pfile	  = NULL;

#define LOG_LOCKED

#ifdef  LOG_LOCKED
pthread_mutex_t		log_mutex;
#endif

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
void LOG_init(const char* ppath_file)
{
#ifndef MVI_DEBUG_LOG
	(void) ppath_file;
#else
	LOG_destroy();
	if ( ppath_file ) {
		pfile = fopen(ppath_file,"ab");
	}
#ifdef  LOG_LOCKED
	pthread_mutex_init(&log_mutex,NULL);
#endif
#endif
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
void LOG_destroy(void)
{
	if ( pfile ) {
		fclose(pfile);
		pfile = NULL;
	}
#ifdef  LOG_LOCKED
	pthread_mutex_destroy(&log_mutex);
#endif
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
void mvi_log(int level,char* file,int line,const char* format,...)
{
	va_list ap;
	va_start(ap,format);
	mvi_log1(level,file,line,format,&ap); 
	va_end(ap);
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
void mvi_log1(int level,char* file,int line,const char* format,va_list* pap)
{
#ifndef MVI_DEBUG_LOG
	(void)level;
	(void)file;
	(void)line;
	(void)format;
	(void)pap;
#else
	time_t ltime;
	char fmt[512];
	char* tmp;
	char* tmp1;

	memset(fmt,0,sizeof(fmt));
	if (level >= NONE_LEVEL || level < 0 )   return;

#ifdef  LOG_LOCKED
	if ( pthread_mutex_lock(&log_mutex) ) {
		printf("\nFATAL ERROR MUTEX_LOCK in write log file ");
		return;
	}
#endif

	tmp = file;
	while (tmp) {
		tmp1 = strstr(tmp,"\\");
		if ( !tmp1 ) break;
		tmp = tmp1+1;
	}

	time(&ltime);
	sprintf(fmt,"\n%.19s %s:%u %s:%s",ctime(&ltime)+4,tmp,line,log_level_name[level],format);
	if ( pfile ) {
		vfprintf(pfile,fmt,*pap);
		fflush(pfile);
#ifdef _DEBUG
		vprintf(fmt,*pap);
#endif 
	}
	else {
		vprintf(fmt,*pap);
	}
#ifdef  LOG_LOCKED
	pthread_mutex_unlock(&log_mutex);
#endif  
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
void add_log_sip_protokol(const char* file,const char* pname,const char* paddr,uint16 port,const char* pbuf,int len)
{
	char buftime[128];
	char bufdate[128];
	char bufout[1024];
	char filepatch[1024];

	sprintf(filepatch,"c:\\siplog\\%s",file);

	_strtime(buftime);
	_strdate(bufdate );
	sprintf(bufout,"--------%s ----------------\n %s:%u   %s %s \n----------------------------------------------\n",
		pname,paddr,port,bufdate,buftime);
	write_file(filepatch,bufout,strlen(bufout));



	write_file(filepatch,pbuf,len);
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
char* ipaddr2char(unsigned long ipAddr)
{
	struct in_addr InetAddress;
	InetAddress.s_addr = ipAddr;
	return (char *)inet_ntoa(InetAddress);
}
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
mvi_status_t ipaddr2long(const char* pip_char,u_int* paddr)
{
	if ( !pip_char ) return ER_SIP_PARSE_IP_ADDR;
	*paddr = inet_addr(pip_char);
	if ( *paddr == 0 || *paddr == 0xFFFFFFFF ) return ER_SIP_PARSE_IP_ADDR;
	*paddr = ntohl(*paddr);
	return Ok;
}
