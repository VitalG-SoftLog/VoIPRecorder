
#ifndef  _SIP_ATTRIBUTE_PARSE_H_
#define  _SIP_ATTRIBUTE_PARSE_H_

#include "sip.h"

#ifdef __cplusplus
extern "C" {
#endif

#define is_OCTET(ptr)	( ptr[0] >= 0x00 && ptr[0] <= 0xFF )
#define is_CHAR(ptr)	( ptr[0] >= 0x01 && ptr[0] <= 0x7F )
#define is_VCHAR(ptr)	( ptr[0] >= 0x21 && ptr[0] <= 0x7F )
#define is_ALPHA(ptr)	( (ptr[0] >= 0x41 && ptr[0] <= 0x5A) || (ptr[0] >= 0x61 && ptr[0] <= 0x7A) )
#define is_DIGIT(ptr)	( ptr[0] >= 0x30 && ptr[0] <= 0x39 )
#define is_CTL(ptr)		( (ptr[0] >= 0x00 && ptr[0] <= 0x1F) || ptr[0] == 0x7F )
#define is_HTAB(ptr)	( ptr[0] == 0x09 )
#define is_LF(ptr)		( ptr[0] == 0x0A )
#define is_CR(ptr)		( ptr[0] == 0x0D )
#define is_SP(ptr)		( ptr[0] == 0x20 )
#define is_DQUOTE(ptr)	( ptr[0] == 0x22 )
#define is_BIT(ptr)		( ptr[0] == '0' || ptr[0] == '1' )
#define is_HEXDIG(ptr)	( is_DIGIT(ptr) || ptr[0] == 'A' || ptr[0] == 'B' || ptr[0] == 'C' || ptr[0] == 'D' || ptr[0] == 'E' || ptr[0] == 'F' \
										|| ptr[0] == 'a' || ptr[0] == 'b' || ptr[0] == 'c' || ptr[0] == 'd' || ptr[0] == 'e' || ptr[0] == 'f' )
#define is_LHEX(ptr)	( is_DIGIT(ptr) || (ptr[0] >= 0x61 && ptr[0] >= 0x66) )
#define is_CRLF(ptr)	( ptr[0] == 0x0D && ptr[1] == 0x0A )
#define is_WSP(ptr)		( is_SP(ptr) || is_HTAB(ptr) )

#define is_ALPHANUM(ptr)	( is_ALPHA(ptr) || is_DIGIT(ptr) ) 
#define is_SEPARATORS(ptr)	\
	(	ptr[0] == '(' || ptr[0] == ')' || ptr[0] == '<' || ptr[0] == '>'  || ptr[0] == '@' || \
		ptr[0] == ',' || ptr[0] == ';' || ptr[0] == ':' || ptr[0] == '\\' || ptr[0] == '/' || \
		ptr[0] == '[' || ptr[0] == ']' || ptr[0] == '?' || ptr[0] == '='  || ptr[0] == '{' || ptr[0] == '}' || \
		is_DQUOTE(ptr) || is_SP(ptr) || is_HTAB(ptr) )
#define is_MARK(ptr)	\
	(	ptr[0] == '-' || ptr[0] == '_' || ptr[0] == '.' || ptr[0] == '!'  || ptr[0] == '~' || \
		ptr[0] == '*' || ptr[0] == '\'' || ptr[0] == '(' || ptr[0] == ')' )
#define is_RESERVED(ptr)	\
	(	ptr[0] == ';' || ptr[0] == '/' || ptr[0] == '?' || ptr[0] == ':'  || ptr[0] == '@' || \
		ptr[0] == '&' || ptr[0] == '=' || ptr[0] == '+' || ptr[0] == '$' || ptr[0] == ',' )
#define is_UNRESERVED(ptr)	( is_ALPHANUM(ptr) || is_MARK(ptr) )
#define is_ESCAPED(ptr)		( ptr[0] == '%' )
#define is_TOKEN(ptr)	\
	(	ptr[0] == '-' || ptr[0] == '.' || ptr[0] == '!' || ptr[0] == '%'  || ptr[0] == '*' || \
		ptr[0] == '_' || ptr[0] == '+' || ptr[0] == '`' || ptr[0] == '\'' || ptr[0] == '~' ||  is_ALPHANUM(ptr) )

#define is_HNV_UNRESERVED(ptr)	\
	(	ptr[0] == '[' || ptr[0] == ']' || ptr[0] == '/' || ptr[0] == '?'  || ptr[0] == ':' || \
		ptr[0] == '+' || ptr[0] == '$' )
#define is_PARAM_UNRESERVED(ptr)	\
	(	ptr[0] == '[' || ptr[0] == ']' || ptr[0] == '/' || ptr[0] == ':'  || ptr[0] == '&' || \
		ptr[0] == '+' || ptr[0] == '$' )
#define is_USER_UNRESERVED(ptr)	\
	(	ptr[0] == '&' || ptr[0] == '=' || ptr[0] == '+' || ptr[0] == '$'  || ptr[0] == ',' || \
		ptr[0] == ';' || ptr[0] == '?' || ptr[0] == '/' )
#define is_PASSWORD_UNRESERVED(ptr)	\
	(	ptr[0] == '&' || ptr[0] == '=' || ptr[0] == '+' || ptr[0] == '$'  || ptr[0] == ',' )

#define is_LWS(ptr)	is_WSP(ptr)		//LWS	=  	[*WSP   CRLF]   1*WSP   ?????????

#define is_UTF8_CONT(ptr)	( ptr >= 0x80 && ptr <= 0xBF ) 
#define is_UTF8_NONASCII_1(ptr)	( ptr[0] >= 0xC0 && ptr[0] <= 0xDF  &&	is_UTF8_CONT(ptr[1]) ) 
#define is_UTF8_NONASCII_2(ptr)	( ptr[0] >= 0xE0 && ptr[0] <= 0xEF  &&	is_UTF8_CONT(ptr[1]) &&	is_UTF8_CONT(ptr[2]) ) 
#define is_UTF8_NONASCII_3(ptr)	( ptr[0] >= 0xF0 && ptr[0] <= 0xF7  &&	is_UTF8_CONT(ptr[1]) &&	is_UTF8_CONT(ptr[2]) &&	is_UTF8_CONT(ptr[3]) ) 
#define is_UTF8_NONASCII_4(ptr)	( ptr[0] >= 0xF8 && ptr[0] <= 0xFB  &&	is_UTF8_CONT(ptr[1]) &&	is_UTF8_CONT(ptr[2]) &&	is_UTF8_CONT(ptr[3]) &&	is_UTF8_CONT(ptr[4]) ) 
#define is_UTF8_NONASCII_5(ptr)	( ptr[0] >= 0xFC && ptr[0] <= 0xFD  &&	is_UTF8_CONT(ptr[1]) &&	is_UTF8_CONT(ptr[2]) &&	is_UTF8_CONT(ptr[3]) &&	is_UTF8_CONT(ptr[4]) &&	is_UTF8_CONT(ptr[4]) ) 
#define is_UTF8_NONASCII(ptr)	( is_UTF8_NONASCII_1(ptr) || is_UTF8_NONASCII_2(ptr) || is_UTF8_NONASCII_3(ptr) || is_UTF8_NONASCII_4(ptr) || is_UTF8_NONASCII_5(ptr) )
#define is_QUOTED_PAIR(ptr)	\
	(	ptr[0] == '\\' && ( (ptr[1] >= 0x00 && ptr[1] <= 0x09) || (ptr[1] >= 0x0B && ptr[1] <= 0x0C) || (ptr[1] >= 0x0E && ptr[1] <= 0x7F) ) )
#define is_QDTEXT(ptr)	\
	(	is_LWS(ptr) || ptr[0] == 0x21  || (ptr[0] >= 0x23 && ptr[0] <= 0x5B) || (ptr[0] >= 0x5D && ptr[0] <= 0x7E) )

#define skip_UTF8_NONASCII(ptr) \
	while (1) {														\
		if ( is_UTF8_NONASCII_1(ptr) ) { ptr += 2; continue; }		\
		if ( is_UTF8_NONASCII_2(ptr) ) { ptr += 3; continue; }		\
		if ( is_UTF8_NONASCII_3(ptr) ) { ptr += 4; continue; }		\
		if ( is_UTF8_NONASCII_4(ptr) ) { ptr += 5; continue; }		\
		if ( is_UTF8_NONASCII_5(ptr) ) { ptr += 6; continue; }		\
		break;														\
	}
#define skip_QDTEXT_ALL(ptr) \
	while (1) {																		\
		if  ( is_QDTEXT(ptr) )			{ptr++;  continue; }						\
		if  ( is_QUOTED_PAIR(ptr) )		{ptr += 2;  continue; }						\
		if ( is_UTF8_NONASCII(ptr) )	{ skip_UTF8_NONASCII(ptr) continue; }		\
		break;																		\
	}			


#define skip_CHAR(ptr)		while ( is_CHAR(ptr) ) ptr++;
#define skip_VCHAR(ptr)		while ( is_VCHAR(ptr) ) ptr++;
#define skip_ALPHA(ptr)		while ( is_ALPHA(ptr) ) ptr++;
#define skip_DIGIT(ptr)		while ( is_DIGIT(ptr) ) ptr++;
#define skip_CTL(ptr)		while ( is_CTL(ptr) ) ptr++;
#define skip_HEXDIG(ptr)	while ( is_HEXDIG(ptr) ) ptr++;
#define skip_LHEX(ptr)		while ( is_LHEX(ptr) ) ptr++;
#define skip_CRLF(ptr)		while ( is_CRLF(ptr) ) ptr += 2;
#define skip_WSP(ptr)		while ( is_WSP(ptr) ) ptr ++;
#define skip_LWSP(ptr)		if ( is_WSP(ptr) ) {				\
								ptr++;							\
								while (1) {						\
									if ( is_WSP(ptr) ) { ptr++; }	\
									else {						\
										if ( is_CRLF(ptr) ) { ptr+=2; if ( is_WSP(ptr) ) ptr ++;	else break;	} \
										else break;				\
									}							\
								}								\
							}

#define skip_LWS(ptr)		if is_WSP(ptr) {								\
								ptr++;										\
								if is_CRLF(ptr)  ptr+=2;					\
								while ( is_WSP(ptr) ) ptr++;				\
							}

#define skip_SWS(ptr)		skip_LWS(ptr)
#define skip_ALPHANUM(ptr)	while ( is_ALPHANUM(ptr) ) ptr++;
#define skip_ESCAPED(ptr)	if ( is_ESCAPED(ptr) ) { ptr++;	if ( is_HEXDIG(ptr) ) { ptr++; if ( is_HEXDIG(ptr) ) ptr++; }}
#define skip_TOKEN(ptr)		while ( is_TOKEN(ptr) ) ptr++;
#define skip_DQUOTE(ptr)	while ( is_DQUOTE(ptr) ) ptr++;

#define skip_UNRESERVED(ptr)			while ( is_UNRESERVED(ptr) ) ptr++;
#define skip_PARAM_UNRESERVED(ptr)		while ( is_PARAM_UNRESERVED(ptr) ) ptr++;
#define skip_USER_UNRESERVED(ptr)		while ( is_USER_UNRESERVED(ptr) ) ptr++;
#define skip_PASSWORD_UNRESERVED(ptr)	while ( is_PASSWORD_UNRESERVED(ptr) ) ptr++;
#define skip_HNV_UNRESERVED(ptr)		while ( is_HNV_UNRESERVED(ptr) ) ptr++;

const u_char* skip_HCOLON(const u_char** pptr);
void skip_QUOTED_STRING(const u_char** pptr);
const u_char* skip_SWS_BASED_DELIMITERS(const u_char** pptr,const u_char symbol);
const u_char* skip_SWS_BASED_DELIMITERS_L(const u_char** pptr,const u_char symbol);
const u_char* skip_SWS_BASED_DELIMITERS_R(const u_char** pptr,const u_char symbol);

BOOL skip_IPV4ADDRESS(const u_char** pptr);
#ifndef DISABLE_IPv6
BOOL skip_IPV6ADDRESS(const u_char** pptr);
#endif
BOOL skip_IPADDRESS(const u_char** pptr);

#define skip_STAR(ptr)		skip_SWS_BASED_DELIMITERS(ptr,'*')
#define skip_SLASH(ptr)		skip_SWS_BASED_DELIMITERS(ptr,'/')
#define skip_EQUAL(ptr)		skip_SWS_BASED_DELIMITERS(ptr,'=')
#define skip_LPAREN(ptr)	skip_SWS_BASED_DELIMITERS(ptr,'(')
#define skip_RPAREN(ptr)	skip_SWS_BASED_DELIMITERS(ptr,')')
#define skip_COMMA(ptr)		skip_SWS_BASED_DELIMITERS(ptr,',')
#define skip_SEMI(ptr)		skip_SWS_BASED_DELIMITERS(ptr,';')
#define skip_COLON(ptr)		skip_SWS_BASED_DELIMITERS(ptr,':')
#define skip_LDQUOT(ptr)	skip_SWS_BASED_DELIMITERS_L(ptr,0x22)
#define skip_RDQUOT(ptr)	skip_SWS_BASED_DELIMITERS_R(ptr,0x22)
#define skip_LAQUOT(ptr)	skip_SWS_BASED_DELIMITERS_L(ptr,'<')
#define skip_RAQUOT(ptr)	skip_SWS_BASED_DELIMITERS_R(ptr,'>')


#define skip_ESCAPED(ptr)	if ( is_ESCAPED(ptr) ) { ptr++;	if ( is_HEXDIG(ptr) ) { ptr++; if ( is_HEXDIG(ptr) ) ptr++; }}


typedef str_item_t		str_t;
typedef sip_item_t		param_t;

#define	 FIELD_SIP_NAME_ADDR	str_t			display_name;	uri_t			uri;

#define	 MAX_SIP_PARAM		20	
#define	 START_SIP_PARAM_STRUCT		struct { param_t params[MAX_SIP_PARAM]; int now_params; str_t str_all;
#define	 STOP_SIP_PARAM_STRUCT		} parameters;
#define	 SIP_PARAM_STRUCT			START_SIP_PARAM_STRUCT STOP_SIP_PARAM_STRUCT
#define	 SIP_HOSTPORT_STRUCT		struct { uri_host_t	type; str_t host; str_t port; str_t str_all; } hostport; 

#define SIP_NAME_SIP	"sip"
#define SIP_NAME_SIPS	"sips"
///////////////////////////////////////////////////////////////////////////////////////
/////////////////////////// URI ///////////////////////////////////////////////////////
typedef enum uri_scheme {
	URI_SCHEME_OTHER = 0,
	URI_SCHEME_SIP,
	URI_SCHEME_SIPS,
} uri_scheme_t;

typedef enum uri_host {
	URI_HOST_UNDEFINED = 0,
	URI_HOST_DOMAIN,
	URI_HOST_IP4,
	URI_HOST_IP6
} uri_host_t;


#define	MAX_URI_PARAMETERS	20
#define	MAX_URI_HEADERS		10

typedef struct uri {
	uri_scheme_t	type;

	str_t			str_all;
	str_t			scheme;
	struct {
		str_t		str_all;
		str_t		user;
		str_t		password;
	} userinfo; 

	SIP_HOSTPORT_STRUCT

	START_SIP_PARAM_STRUCT	
		int			transport;
		int			user;
		int			method;
		int			ttl;
		int			maddr;
		int			lr;
		int			comp;
		int			target;
		int			cause;
		int			orig;
		int			gr;
	STOP_SIP_PARAM_STRUCT	

	struct {
		param_t		heads[MAX_URI_HEADERS];
		int			now_heads;
		str_t		str_all;
	} headers; 
} uri_t;

#define SIP_URI_PARAM_TRANSPORT		"transport="
#define SIP_URI_PARAM_USER			"user="
#define SIP_URI_PARAM_METHOD		"method="
#define SIP_URI_PARAM_TTL			"ttl="
#define SIP_URI_PARAM_MADDR			"maddr="
#define SIP_URI_PARAM_LR			"lr"
#define SIP_URI_PARAM_COMP			"comp="
#define SIP_URI_PARAM_TARGET		"target"
#define SIP_URI_PARAM_CAUSE			"cause"
#define SIP_URI_PARAM_ORIG			"orig"
#define SIP_URI_PARAM_GR			"gr"

typedef enum uri_param {
	URI_PARAM_UNDEFINED = 0,
	URI_PARAM_TRANSPORT,
	URI_PARAM_USER,
	URI_PARAM_METHOD,
	URI_PARAM_TTL,
	URI_PARAM_MADDR,
	URI_PARAM_LR,
	URI_PARAM_COMP,
	URI_PARAM_TARGET,
	URI_PARAM_CAUSE,
	URI_PARAM_ORIG,
	URI_PARAM_GR,
	URI_PARAM_OTHER,
} uri_param_t;


#define skip_URI_OTHER_PARAM(ptr)	\
									while (1) {																	\
										if ( is_UNRESERVED(ptr) ) { skip_UNRESERVED(ptr) }						\
										else {																	\
											if ( is_ESCAPED(ptr) ) { skip_ESCAPED(ptr) }						\
											else {																\
												if ( is_PARAM_UNRESERVED(ptr) ) { skip_PARAM_UNRESERVED(ptr) } else break; \
											}																	\
										}																		\
									}

#define skip_URI_USERINFO_USER(ptr)	\
									while (1) {																	\
										if ( is_UNRESERVED(ptr) ) { skip_UNRESERVED(ptr) }						\
										else {																	\
											if ( is_ESCAPED(ptr) ) { skip_ESCAPED(ptr) }						\
											else {																\
												if ( is_USER_UNRESERVED(ptr) ) { skip_USER_UNRESERVED(ptr) } else break; \
											}																	\
										}																		\
									}
#define skip_URI_USERINFO_PASSWORD(ptr)	\
									while (1) {																	\
										if ( is_UNRESERVED(ptr) ) { skip_UNRESERVED(ptr) }						\
										else {																	\
											if ( is_ESCAPED(ptr) ) { skip_ESCAPED(ptr) }						\
											else {																\
												if ( is_PASSWORD_UNRESERVED(ptr) ) { skip_PASSWORD_UNRESERVED(ptr) } else break; \
											}																	\
										}																		\
									}

#define skip_URI_HOSTPORT_DOMAIN(ptr)	\
									if ( is_ALPHANUM(ptr) ) {	 												\
										while (1) {																\
											ptr++;																\
											skip_ALPHANUM(ptr)													\
											if ( ptr[0] == '.' || ptr[0] == '-' ) continue;						\
											break;																\
										}																		\
									}

#define skip_URI_HEADERS(ptr)	\
									while (1) {																	\
										if ( is_UNRESERVED(ptr) ) { skip_UNRESERVED(ptr) }						\
										else {																	\
											if ( is_ESCAPED(ptr) ) { skip_ESCAPED(ptr) }						\
											else {																\
												if ( is_HNV_UNRESERVED(ptr) ) { skip_HNV_UNRESERVED(ptr) } else break; \
											}																	\
										}																		\
									}

int		sip_uri_parse(uri_t* puri,const u_char* ptr);
void	sip_uri_printf(uri_t* puri,const char* ptable);
int		sip_uri_print(uri_t* puri,char* pbuf,int len);

#define	MAX_FROM_PARAMETERS		MAX_SIP_PARAM
#define	MAX_TO_PARAMETERS		MAX_FROM_PARAMETERS
///////////////////////////////////////////////////////////////////////////////////////
/////////////////////// FROM  TO ///////////////////////////////////////////////////////
typedef struct from {
	str_t			str_all;
	FIELD_SIP_NAME_ADDR

	START_SIP_PARAM_STRUCT	
	int			tag;
	STOP_SIP_PARAM_STRUCT	

} from_t;
typedef from_t	to_t;

#define SIP_FROM_PARAMETERS_TAG		"tag"

int		sip_from_parse(from_t* pfrom,const u_char* ptr);
void	sip_from_printf(from_t* pfrom);
int		sip_from_print(from_t* pfrom,char* pbuf,int len);

#define sip_to_parse	sip_from_parse
#define sip_to_printf	sip_from_printf
#define sip_to_print	sip_from_print

///////////////////////////////////////////////////////////////////////////////////////
/////////////////////// CONTACT ///////////////////////////////////////////////////////
#define MAX_CONTACT_PARAMETERS		MAX_SIP_PARAM
typedef struct contact {
	str_t			str_all;
	FIELD_SIP_NAME_ADDR

	START_SIP_PARAM_STRUCT	
		int			expires;
	STOP_SIP_PARAM_STRUCT	

} contact_t;

#define SIP_CONTACT_PARAM_EXPIRES		"expires="

int		sip_contact_parse(contact_t* pcontact,const u_char* ptr);
void	sip_contact_printf(contact_t* pcontact);
int		sip_contact_print(contact_t* pcontact,char* pbuf,int len);


///////////////////////////////////////////////////////////////////////////////////////
/////////////////////// VIA ///////////////////////////////////////////////////////////
#define MAX_VIA_PARAMETERS		MAX_SIP_PARAM
typedef struct via {
	str_t			str_all;

	str_t			protocol_name;
	str_t			protocol_version;
	str_t			transport;

	SIP_HOSTPORT_STRUCT

	START_SIP_PARAM_STRUCT	
		int			ttl;
		int			maddr;
		int			received;
		int			branch;
		int			comp;
		int			rport;
	STOP_SIP_PARAM_STRUCT	
} via_t;

#define SIP_VIA_PARAMETERS_TTL		"ttl"
#define SIP_VIA_PARAMETERS_MADDR	"maddr"
#define SIP_VIA_PARAMETERS_RECEIVED	"received"
#define SIP_VIA_PARAMETERS_BRANCH	"branch"
#define SIP_VIA_PARAMETERS_COMP		"comp"
#define SIP_VIA_PARAMETERS_RPORT	"rport"

int		sip_via_parse(via_t* pvia,const u_char* ptr);
void	sip_via_printf(via_t* pvia);
int		sip_via_print(via_t* pvia,char* pbuf,int len);

///////////////////////////////////////////////////////////////////////////////////////
/////////////////////// ROUTE ///////////////////////////////////////////////////////
#define MAX_ROUTE_PARAMETERS			MAX_SIP_PARAM
#define MAX_RECORD_ROUTE_PARAMETERS		MAX_ROUTE_PARAMETERS
typedef struct route {
	str_t			str_all;
	FIELD_SIP_NAME_ADDR

	SIP_PARAM_STRUCT	
} route_t;

typedef route_t		record_route_t;

int		sip_route_parse(route_t* proute,const u_char* ptr);
void	sip_route_printf(route_t* proute);
int		sip_route_print(route_t* proute,char* pbuf,int len);

#define sip_record_route_parse		sip_route_parse
#define sip_record_route_printf		sip_route_printf
#define sip_record_route_print		sip_route_print

///////////////////////////////////////////////////////////////////////////////////////
/////////////////////// REQUEST_LINE  //////////////////////////////////////////////////
typedef struct request_line {
	str_t			str_all;
	str_t			method;
	uri_t			request_uri;
	str_t			sip_version;
} request_line_t;

int		sip_request_line_parse(request_line_t* prequest_line,const u_char* ptr);
void	sip_request_line_printf(request_line_t* prequest_line);
int		sip_request_line_print(request_line_t* prequest_line,char* pbuf,int len);

///////////////////////////////////////////////////////////////////////////////////////
/////////////////////// Authorization  ////////////////////////////////////////////////
typedef struct authorization {
	str_t			str_all;
	str_t			username;
	str_t			qop;
	str_t			cnonce;
	str_t			nc;
	str_t			response;
	str_t			realm;
	str_t			nonce;
	str_t			algorithm;
	str_t			opaque;
	str_t			auts;
	uri_t			uri;
} authorization_t;

#define SIP_AUTORIZATION_DIGEST			"Digest"
#define SIP_AUTORIZATION_USERNAME		"username"
#define SIP_AUTORIZATION_URI			"uri"
#define SIP_AUTORIZATION_QOP			"qop"
#define SIP_AUTORIZATION_CNONCE			"cnonce"
#define SIP_AUTORIZATION_NC				"nc"
#define SIP_AUTORIZATION_RESPONSE		"response"
#define SIP_AUTORIZATION_REALM			"realm"
#define SIP_AUTORIZATION_NONCE			"nonce"
#define SIP_AUTORIZATION_ALGORITHM		"algorithm"
#define SIP_AUTORIZATION_ALGORITHM_MD5	"MD5"
#define SIP_AUTORIZATION_OPAQUE			"opaque"
#define SIP_AUTORIZATION_AUTS			"auts"

int		sip_authorization_parse(authorization_t* pobj,const u_char* ptr);
void	sip_authorization_printf(authorization_t* pobj);
int		sip_authorization_print(authorization_t* pobj,char* pbuf,int len);



///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
typedef union sip_obj {
	from_t			from;
	to_t			to;
	contact_t		contact;
	via_t			via;
	route_t			route;
	record_route_t	record_route;
	request_line_t	request_line;
	authorization_t	authorization; 
} sip_obj_t;



#ifdef __cplusplus
}
#endif



#endif /* _SIP_ATTRIBUTE_PARSE_H_ */ 
