#ifndef _SIP_PARSER_H_
#define _SIP_PARSER_H_

#include "sip.h"
#include "tbl_sip_tables.h"
#include "mvi_transport.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SIP_CONTENT_TYPE_SDP		"application/sdp"

#define SIP_CONTENT_TYPE_NEC_MULTI	"multipart/X-NECSIPEXT"
#define SIP_NEC_SEPARATOR			"--++"
#define SIP_NEC_SEPARATOR_END		"--++--"
#define SIP_TITLE_X_REGSVLISTNO		"x-regsvlistno"

#define	MSG_TRYING   "100 Trying"
#define	MSG_RINGING   "180 Ringing"
#define	MSG_OK200   "200 Ok"
#define	SIP_SEPARATOR   "\x0D\x0A"
#define SIP_VERSION   "SIP/2.0"
#define SIP_COM_INVITE_STR   "INVITE"
#define SIP_COM_ACK_STR   "ACK"
#define SIP_COM_BYE_STR   "BYE"
#define SIP_COM_CANCEL_STR   "CANCEL"
#define SIP_COM_REGISTER_STR   "REGISTER"
#define SIP_COM_OPTION_STR   "OPTIONS"
#define SIP_COM_INFO_STR   "INFO"
#define SIP_COM_PRACK_STR   "PRACK"
#define SIP_COM_UPDATE_STR   "UPDATE"
#define SIP_COM_SUBSCRIBE_STR   "SUBSCRIBE"
#define SIP_COM_NOTIFY_STR   "NOTIFY"
#define SIP_COM_REFER_STR   "REFER"
#define SIP_COM_MESSAGE_STR   "MESSAGE"
#define SIP_COM_PUBLISH_STR   "PUBLISH"
#define SIP_TITLE_CALL_ID   "call-id"
#define SIP_TITLE_CSEG   "cseq"
#define SIP_TITLE_VIA   "via"
#define SIP_TITLE_FROM   "from"
#define SIP_TITLE_TO   "to"
#define SIP_TITLE_CONTENT_LENGTH	"content-length"
#define SIP_TITLE_CONTENT_TYPE   "content-type"
#define SIP_TITLE_CONTENT_TYPE_ETALON	"Content-Type: "
#define SIP_TITLE_CONTACT   "contact"
#define SIP_TITLE_MAX_FORVARD   "max-forwards"
#define SIP_TITLE_MAX_FORVARD_ETALON	"Max-Forwards"
#define SIP_TITLE_RECORD_ROUTE   "record-route"
#define SIP_TITLE_ROUTE   "route"
#define SIP_TITLE_EXPIRES   "expires"
#define SIP_TITLE_REFER_TO		"refer-to"
#define SIP_TITLE_USER_AGENT	"user-agent"
#define SIP_TITLE_PROXY_AUTOR			"proxy-authorization"
#define SIP_TITLE_AUTOR					"authorization"
#define SIP_TITLE_PROXY_AUTH			"proxy-authenticate"
#define SIP_TITLE_WWW_AUTH				"www-authenticate"
#define SIP_TITLE_AUTH					"authenticate"
#define SIP_TITLE_ALLOW					"allow"


#define SIP_TITLE_EXPIRES_CONTACT   "expires="
#define SIP_TITLE_CALL_ID_S   "i"
#define SIP_TITLE_VIA_S   "v"
#define SIP_TITLE_FROM_S   "f"
#define SIP_TITLE_TO_S   "t"
#define SIP_TITLE_CONTENT_LENGTH_S   "l"
#define SIP_TITLE_CONTENT_TYPE_S   "c"
#define SIP_TITLE_CONTACT_S   "m"
#define SIP_TITLE_RECORD_ROUTE_ETALON   "Record-Route: "
#define SIP_TITLE_ROUTE_ETALON			"Route: "

#define SIP_TITLE_PROXY_AUTOR_ETALON	"Proxy-Authorization: "
#define SIP_TITLE_WWW_AUTHOR_ETALON		"WWW-Authorization: "
#define SIP_TITLE_AUTOR_ETALION			"Authorization: "
#define SIP_TITLE_REFER_TO_ETALION		"Refer-To: "
#define SIP_TITLE_AUTHENTICATE_ETALON	"Authenticate: "

#define SIP_TITLE_CALL_ID_ETALON   "Call-Id: "
#define SIP_TITLE_CSEG_ETALON   "Cseq: "
#define SIP_TITLE_VIA_ETALON   "Via: "
#define SIP_TITLE_FROM_ETALON   "From: "
#define SIP_TITLE_TO_ETALON   "To: "
#define SIP_TITLE_CONTENT_LENGTH_ETALON   "Content-Length: "
#define SIP_TITLE_USER_AGENT_ETALON		"User-Agent: "
#define SIP_TITLE_SERVER_ETALON			"Server: "
#define SIP_TITLE_ALLOW_ETALON   "Allow: "
#define SIP_TITLE_CONTACT_ETALON   "Contact: "
#define SIP_TITLE_PROXY_AUTH_ETALON   "Proxy-Authenticate: "
#define SIP_TITLE_EXPIRES_ETALON   "Expires: "
#define SIP_VALUE_ALLOW   "INVITE, ACK, CANCEL, OPTIONS, BYE, REFER, SUBSCRIBE, NOTIFY"
#define SIP_TRANSPORT_TCP   "TCP"
#define SIP_TRANSPORT_UDP   "UDP"
#define SIP_TRANSPORT_TLS   "TLS"
#define SIP_MESSAGE_C_IP4   "c=IN IP4 "
#ifndef DISABLE_IPv6
#define SIP_MESSAGE_C_IP6   "c=IN IP6 "
#endif

#define SIP_MEDIA_TYPE				"m="
#define SIP_MEDIA_TYPE_AUDIO		"audio "
#define SIP_MEDIA_TYPE_VIDEO		"video "
#define SIP_MEDIA_TYPE_IMAGE		"image "
#define SIP_MEDIA_TYPE_TEXT			"text "
#define SIP_MEDIA_TYPE_MESSAGE		"message "
#define SIP_MEDIA_TYPE_APPLICATION	"application "
#define SIP_MEDIA_TYPE_DATA			"data "
#define SIP_MEDIA_TYPE_CONTROL		"control "

#define SIP_MESSAGE_A_CRYPTO   "a=crypto:"

#define SIP_MESSAGE_A_RTCP   "a=rtcp:"

#define SIP_TITLE_WARNING_ETALON   "Warning: "
#define SIP_TITLE_WARNING_MSG   "Request blocked by policy"

mvi_status_t buf2sip(const char* pbuf,int* plen,sip_t* psip);

int find_str(const char *pstr, const char *ptemplate);
mvi_status_t read_start_str(const char *tmp, int len, sip_t* psip);
mvi_status_t read_titles(const char *tmp, int *plen, sip_t *psip);
mvi_status_t read_messages(const char *tmp, int *plen, sip_t *psip);
mvi_status_t is_valid(sip_t *psip);
mvi_status_t read_title(const char *tmp, int len,int lenname,sip_t *psip);
mvi_status_t read_message(const char *tmp, int len, sip_t *psip);
mvi_status_t check_title(const char *pbuf, const char *ptitle, const char *tmp, int len, int lenname, sip_t *psip, int *is_add);
mvi_status_t parse_title(const char* pbuf,sip_t* psip,int* pcol);
mvi_status_t check_message(const char* pbuf,const char* ptitle,const char* tmp,int len,sip_t* psip,int* is_add);
mvi_status_t add_sip_item_message(const char *pname, int lenname, const char *pvalue, int lenvalue, sip_t *psip);
mvi_status_t add_sip_item_titles(const char *pname, int lenname, const char *pvalue, int lenvalue, sip_t *psip);
mvi_status_t get_command(const char* tmp,int len,sip_method_t* psip_command,int* plensip_command,sip_t* psip);
 
void psip_back_struct_url(psip_url_t* purl);
mvi_status_t set_struct_url(psip_url_t* purl);
mvi_status_t psip_set_struct_url1(char* psipurl,psip_url_t* psul);
mvi_status_t psip_set_struct_url(item_buffer_t** ppibt,const char* psipurl,psip_url_t* psul);
mvi_status_t sip_set_struct_url(const char* psipurl,sip_url_t* psul);
//mvi_status_t sip_get_struct_url(sip_url_t* psul);

void set_null(sip_t* psip);
void set_not_null(sip_t* psip);

BOOL is_sdp(const char* pcontent_type,int len);

BOOL check_contact_unregistry_all(const char* pcontact);

BOOL media2task(sip_t* psip,int index,rtp_task_t* ptask);

void check_nec(const char** ppbuf,int* plen,sip_t* psip);
void check_contact_expires(sip_t* psip);

#ifdef __cplusplus
}
#endif


#endif /* _SIP_PARSER_H_ */
