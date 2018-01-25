#include "os_include.h"
#include "mvi_error_message.h"

const mvi_error_t error_msg[] = { 
{	Ok							,	  "Ok							"  },
{	OK_REGISTRY_NOT_FOUND		,     "OK_REGISTRY_NOT_FOUND	    "  },
{	ER_MAX_BUFFER				,     "ER_MAX_BUFFER				"  },
{	ER_SMALL_BUFFER				,     "ER_SMALL_BUFFER				"  },	 
{	ER_RTP_PARAMETERS			,     "ER_RTP_PARAMETERS			"  },
{	ER_NOT_SIP_PROTOKOL			,     "ER_NOT_SIP_PROTOKOL			"  },
{	ER_NOT_VALID_SIP_PROTOKOL	,     "ER_NOT_VALID_SIP_PROTOKOL	"  },
{	ER_SIP_PARSE_URL			,     "ER_SIP_PARSE_URL				"  },
{	ER_SIP_PARSE_IP_ADDR		,     "ER_SIP_PARSE_IP_ADDR			"  },
{	ER_INIT_MUTEX				,     "ER_INIT_MUTEX				"  },
{	ER_MUTEX_LOCK				,     "ER_MUTEX_LOCK				"  },
{	ER_MUTEX_UNLOCK				,     "ER_MUTEX_UNLOCK				"  },	 
                                                                     
{ ER_END,	"undefined error (%d)" }
};

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
const char* mvi_get_error_msg(const mvi_status_t code)
{
	static char buf[256];
	int i;

	for (i=0; error_msg[i].id != ER_END ;i++) if ( error_msg[i].id == code ) break;
	if ( error_msg[i].id == ER_END  ) {
		sprintf(buf,error_msg[i].name,code);
		return buf;
	}
	return error_msg[i].name;
}

