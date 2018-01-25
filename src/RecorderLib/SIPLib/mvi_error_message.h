#ifndef  _SIP_ERRR_MESSAGE_H_
#define  _SIP_ERRR_MESSAGE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define	BEGIN_RANGE_INFO	0
#define	END_RANGE_INFO		299

#define	BEGIN_RANGE_WARNING		300
#define	END_RANGE_WARNING		399

#define	BEGIN_RANGE_ERROR		400
#define	END_RANGE_ERROR			599

#define	BEGIN_RANGE_FATAL_ERROR		600
#define	END_RANGE_FATAL_ERROR		999

typedef enum {

	Ok							= 0,
	OK_REGISTRY_NOT_FOUND		= 1,

	ER_MAX_BUFFER				= 400,	
	ER_SMALL_BUFFER				= 401,
	ER_RTP_PARAMETERS			= 402,
	ER_NOT_SIP_PROTOKOL			= 403,
	ER_NOT_VALID_SIP_PROTOKOL	= 404,
	ER_SIP_PARSE_URL			= 405,
	ER_SIP_PARSE_IP_ADDR		= 406,
	ER_INIT_MUTEX				= 407,
	ER_MUTEX_LOCK				= 408,
	ER_MUTEX_UNLOCK				= 409,

	ER_END = 100000
} mvi_status_t;

#define	OK	Ok

typedef struct sip_error {
	mvi_status_t		id;
	char				name[128];
} mvi_error_t;

const char* mvi_get_error_msg(const mvi_status_t code);

#ifdef __cplusplus
}
#endif


#endif /* _SIP_ERRR_MESSAGE_H_  */ 
