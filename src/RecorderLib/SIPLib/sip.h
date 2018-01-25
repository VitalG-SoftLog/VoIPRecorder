#ifndef _SIP_H_
#define _SIP_H_

#include "mvi_error_message.h"
#include "mvi_malloc.h"
#include "sip_parser_api.h"
//#include "rtp_map.h"


#ifdef __cplusplus
extern "C" {
#endif


#define MAX_LEN_SIP_ATTR	255 

#define STR_EMPTY	""

#define is_sip_command(com)  ( com > SIP_MESSAGE_6XX ) 

typedef struct str_item {
  const   u_char* p;
  int     l;
} str_item_t;

typedef struct sip_item {
  str_item_t   name;
  str_item_t   value;
  int          is_not_send;
} sip_item_t;

#define MAX_TITLES		128
#define MAX_VIA			20
#define MAX_CRYPTO		3

typedef struct sip_titles {
  sip_method_t command;
  sip_method_t command_start;
  int	mess;

  int   content_length_v;
  int   max_forward_v;
  int   cseg_v;
  int   expires_v;

  int   expires;
  int   cseg;
  int   call_id;
  int   from;
  int   to;
  int   contact;
  int   content_length;
  int   content_type;
  int   max_forward;
  int   refer_to;
  int   allow;
  int   user_agent;
  int	x_regsvlistno;


  int   route[MAX_VIA];
  int   col_route;
  int   record_route[MAX_VIA];
  int   col_record_route;
  int   proxy_authorization[MAX_VIA];
  int   col_proxy_authorization;
  int   via[MAX_VIA];
  int   col_via;
} sip_titles_t;

typedef struct sip_media {
  int   m;
  int   c;
  int   crypto[MAX_CRYPTO];
  int   col_crypto;
} sip_media_t;

#define	MAX_ITEN_ATTR	128
#define	MAX_ROUTE_ATTR	512

#define CFG_RTP_PROXY_MEDIA_DESCRIPTORS_MAX 8
#ifndef DISABLE_SRTP
  #define SRTP_MAX_CRYPTO_KEYLEN 46
#endif

typedef struct sip {
  mvi_status_t	  status;
  
  sip_titles_t    titles;
  sip_item_t      all_titles[MAX_TITLES];
  int             now_titles;

//////////    OLD  //////////////////////////
  sip_item_t      all_messages[MAX_TITLES];
  int			  col_messages;
  sip_media_t	  sip_media[CFG_RTP_PROXY_MEDIA_DESCRIPTORS_MAX];
  int			  col_media;

  const char*	  pall_buf_sdp;
/////////////////////////////////////////////

  const char*		pbuf_content;
  int				buf_content_len;
  const char*		pcontent_type;

  item_buffer_t*	pibt;		// allocation memory 

  char*			  buf;
  int			  len;	
} sip_t;

typedef struct sip_url {
  char    url[MAX_LEN_SIP_ATTR];
  char*   ptitle;
  char*   pname;
  char*   pdomen;
  char*   pport;
  char	  ch_end;	
} sip_url_t;

typedef struct psip_url {
  char*   purl;
  char*   ptitle;
  char*   pname;
  char*   pdomen;
  char*   pport;
  char	  ch_end;	
} psip_url_t;


#ifdef __cplusplus
}
#endif

#endif		/* _SIP_H_ */
