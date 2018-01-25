#include "os_include.h"
#include "sip_parser.h"
#include "sip_attribute_parse.h"
#include "mvi_log.h"

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
mvi_status_t buf2sip(const char* pbuf,int* plen,sip_t* psip)
{
	mvi_status_t	ret = Ok;
	const char*	tmp = pbuf;
	int	startlen = *plen;
	int	len;
	int lenseparator;

	while ( !isalpha((u_char)tmp[0]) ) {	tmp++;	(*plen)--; if ( (*plen) <= 0 ) return ER_NOT_SIP_PROTOKOL; }

	len = find_str(tmp,SIP_SEPARATOR);
	if ( len <= 0 )		return ER_NOT_SIP_PROTOKOL;
	if ( len >= *plen ) return ER_NOT_SIP_PROTOKOL;
	ret = read_start_str(tmp,len,psip);
	if ( ret != Ok ) return ret;

	tmp		+= len;
	lenseparator = strlen(SIP_SEPARATOR);
	if ( memcmp(tmp,SIP_SEPARATOR,lenseparator) ) return ER_NOT_SIP_PROTOKOL;
	len		+= lenseparator;
	tmp		+= lenseparator;
	*plen	-= len;
	len		= *plen;

	ret = read_titles(tmp,&len,psip);
	if ( ret != Ok ) return ret;

	tmp		+= len;
	lenseparator = strlen(SIP_SEPARATOR);
	if ( memcmp(tmp,SIP_SEPARATOR,lenseparator) ) return ER_NOT_SIP_PROTOKOL;
	len		+= lenseparator;
	tmp		+= lenseparator;
	*plen	-= len;
	len		= *plen;
	while ( psip->titles.content_length_v ) {
		if ( psip->titles.content_length_v > *plen ) return ER_NOT_SIP_PROTOKOL;
		if ( psip->titles.content_type ) {
			check_nec(&tmp,plen,psip);
			if ( !is_sdp((char*)psip->all_titles[psip->titles.content_type].value.p,psip->all_titles[psip->titles.content_type].value.l) ) {
				psip->pall_buf_sdp = tmp;	
				*plen	-= psip->titles.content_length_v;
				break;
			}
		}
		switch ( psip->titles.command_start ) {
		case SIP_COM_OPTION:
		case SIP_COM_INFO:
		case SIP_COM_MESSAGE:
		case SIP_COM_NOTIFY:				
			psip->pall_buf_sdp = tmp;	
			*plen	-= psip->titles.content_length_v;
			break;
		default:	
			len = psip->titles.content_length_v;
			ret = read_messages(tmp,&len,psip);
			if ( ret != Ok ) return ret;
			*plen	-= len;
			break;
		}
		break;
	}

	ret = is_valid(psip);
	*plen = startlen - *plen; 
	set_null(psip);

	if ( ret == Ok )  check_contact_expires(psip);
	return ret;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
void check_nec(const char** ppbuf,int* plen,sip_t* psip)
{
	mvi_status_t ret;
	const char* tmp = *ppbuf;
	int len;
	int len_buf = *plen;
	int lenseparator = strlen(SIP_SEPARATOR);

	if ( memcmp((char*)psip->all_titles[psip->titles.content_type].value.p,SIP_CONTENT_TYPE_NEC_MULTI,strlen(SIP_CONTENT_TYPE_NEC_MULTI)) ) return;
	while ( len_buf > 0 ) {
		if ( !memcmp(tmp,SIP_NEC_SEPARATOR_END,strlen(SIP_NEC_SEPARATOR_END)) ) return;
		len = find_str(tmp,SIP_SEPARATOR);
		if ( len <= 0 )		return;
		if ( len >= *plen ) return;
		if ( memcmp(tmp,SIP_NEC_SEPARATOR,strlen(SIP_NEC_SEPARATOR)) ) return;
		tmp += len+lenseparator;
		len_buf -= len+lenseparator;
		len = len_buf;
		ret = read_titles(tmp,&len,psip);
		if ( ret != Ok ) return;
		tmp		+= len;
		if ( memcmp(tmp,SIP_SEPARATOR,lenseparator) ) return;
		len		+= lenseparator;
		tmp		+= lenseparator;
		len_buf	-= len;
		if ( is_sdp((char*)psip->all_titles[psip->titles.content_type].value.p,psip->all_titles[psip->titles.content_type].value.l) ) {
			*ppbuf = tmp;
			*plen = len_buf;
			break;
		}
		len_buf -= psip->titles.content_length_v + lenseparator;
		tmp += psip->titles.content_length_v + lenseparator;
	}
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
BOOL is_sdp(const char* pcontent_type,int len)
{
	char content_type[255];

	if ( (unsigned int)len > sizeof(content_type) -1 ) return FALSE;
	memset(content_type,0,sizeof(content_type));
	strncpy(content_type,pcontent_type,len);

	if ( !strncmp(content_type,SIP_CONTENT_TYPE_SDP,sizeof(SIP_CONTENT_TYPE_SDP)) ) return TRUE; 
	_strlwr(content_type);
	if ( !strncmp(content_type,SIP_CONTENT_TYPE_SDP,sizeof(SIP_CONTENT_TYPE_SDP)) ) return TRUE; 
	return FALSE;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
void set_null(sip_t* psip)
{
	int i;
	for ( i=0; i<psip->col_messages; i++ ) *(char*)(psip->all_messages[i].value.p + psip->all_messages[i].value.l) = 0;
	for ( i=0; i<psip->now_titles; i++ ) *(char*)(psip->all_titles[i].value.p + psip->all_titles[i].value.l) = 0;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
void set_not_null(sip_t* psip)
{
	int i;
	for ( i=0; i<psip->col_messages; i++ ) *(char*)(psip->all_messages[i].value.p + psip->all_messages[i].value.l) = 0x0D;
	for ( i=0; i<psip->now_titles; i++ ) *(char*)(psip->all_titles[i].value.p + psip->all_titles[i].value.l) = 0x0D;
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
mvi_status_t is_valid(sip_t* psip)
{
	if ( !psip->titles.call_id			||
		!psip->titles.from				||
		!psip->titles.to				||
		//	!psip->titles.content_length	||
		!psip->titles.via[0]			||
		!psip->titles.cseg ) return ER_NOT_VALID_SIP_PROTOKOL;

	if ( psip->titles.content_length_v && psip->titles.command == SIP_COM_INVITE ) {
		if ( !psip->sip_media[0].m || !psip->sip_media[0].c )  return ER_NOT_VALID_SIP_PROTOKOL;
	}
	return Ok;
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
mvi_status_t read_messages(const char* tmp,int* plen,sip_t* psip)
{
	mvi_status_t ret = Ok;
	int length = *plen;
	int lenstr;
	*plen = 0;
	while (1) {
		lenstr = find_str(tmp,SIP_SEPARATOR);
		if ( lenstr <= 1 ) break;
		if ( lenstr > length ) break;
		ret = read_message(tmp,lenstr,psip);
		if ( ret != Ok ) return ret;
		lenstr += 2; 
		*plen += lenstr;
		length -= lenstr;
		tmp += lenstr;
	}
	return ret;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
mvi_status_t read_message(const char* tmp,int len,sip_t* psip)
{
	mvi_status_t ret = Ok;
	int is_add;
	int lenname;

	ret = check_message(tmp,SIP_MEDIA_TYPE,tmp,len,psip,&is_add);
	if ( is_add ) {
		if ( !psip->col_media ) psip->col_media++;
		else {
			if ( psip->sip_media[psip->col_media-1].m ) {
				psip->col_media++;
				if ( psip->col_media > CFG_RTP_PROXY_MEDIA_DESCRIPTORS_MAX ) return ER_RTP_PARAMETERS;
			}
		}
		psip->sip_media[psip->col_media-1].m = psip->col_messages - 1;
		return ret;			
	}
	ret = check_message(tmp,SIP_MESSAGE_C_IP4,tmp,len,psip,&is_add);
#ifndef DISABLE_IPv6
	if ( !is_add ) ret = check_message(tmp,SIP_MESSAGE_C_IP6,tmp,len,psip,&is_add);
#endif
	if ( is_add ) {
		if ( !psip->col_media ) psip->col_media++;
		else {
			if ( psip->sip_media[psip->col_media-1].c ) {
				psip->col_media++;
				if ( psip->col_media > CFG_RTP_PROXY_MEDIA_DESCRIPTORS_MAX ) return ER_RTP_PARAMETERS;
			}
		}
		psip->sip_media[psip->col_media-1].c = psip->col_messages - 1;
		return ret;			
	}
	if ( psip->col_media ) {
		if ( psip->sip_media[psip->col_media-1].col_crypto < MAX_CRYPTO ) {
			ret = check_message(tmp,SIP_MESSAGE_A_CRYPTO,tmp,len,psip,&is_add);
			if ( is_add ) {
				psip->sip_media[psip->col_media-1].crypto[psip->sip_media[psip->col_media-1].col_crypto] = psip->col_messages - 1;
				psip->sip_media[psip->col_media-1].col_crypto ++;
				return ret;			
			}
		}
	}

	lenname = find_str(tmp,"=");
	if ( lenname <= 0 ) {
		lenname = find_str(tmp,":");
		if ( lenname <= 0 ) return ER_NOT_SIP_PROTOKOL;
	}
	lenname++;
	if ( lenname >= len ) {
		lenname = find_str(tmp,":");
		if ( lenname <= 0 ) return ER_NOT_SIP_PROTOKOL;
		if ( lenname >= len ) return ER_NOT_SIP_PROTOKOL;
	}
	len -= lenname;
	ret = add_sip_item_message(tmp,lenname,tmp + lenname,len,psip);

	if ( ret == Ok ) if ( !memcmp(tmp,SIP_MESSAGE_A_RTCP,strlen(SIP_MESSAGE_A_RTCP)) ) psip->col_messages --;  

	return ret;
}



//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
int get_length_title(const char* tmp,int* plenname)
{
	int len;
	BOOL isEnd = TRUE;

	*plenname = 0;

	if ( !strncmp(tmp,SIP_SEPARATOR,strlen(SIP_SEPARATOR)) ) return 0;
	len = find_str(tmp,":");
	if ( len <= 0 ) return 0;
	len++;

	while ( isEnd ) {
		switch (*(tmp+len) ) {
		case 0x20:
		case 0x09:
		case 0x0D:
		case 0x0A:
			len++;
			break;
		default:
			isEnd = FALSE;
			break;
		}
	} 

	*plenname = len;
	len += find_str(tmp+len,SIP_SEPARATOR);
	return len;
}



//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
mvi_status_t read_titles(const char* tmp,int* plen,sip_t* psip)
{
	mvi_status_t  ret = Ok;
	int length = *plen;
	int lenstr;
	int lenname;
	*plen = 0;
	while (1) {
		lenstr = get_length_title(tmp,&lenname);
		//	lenstr = find_str(tmp,SIP_SEPARATOR);
		if ( lenstr <= 1 ) break;
		if ( lenstr > length ) break;
		ret = read_title(tmp,lenstr,lenname,psip);
		if ( ret != Ok ) return ret;
		lenstr += 2; 
		*plen += lenstr;
		length -= lenstr;
		tmp += lenstr;
	}
	return ret;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
mvi_status_t read_title(const char* tmp,int len,int lenname,sip_t* psip)
{
	mvi_status_t  ret = Ok;
	int		length = len;
	int		is_add; 
	const char*	tmp1;
	const char*	tmp2;


	char Buf[255];
	if ( (unsigned int)length >= sizeof(Buf) ) length = sizeof(Buf) - 1;
	memset(Buf,0,sizeof(Buf));
	memcpy(Buf,tmp,length); 

	_strlwr(Buf);

	if ( psip->titles.col_via < MAX_VIA ) {
		ret = check_title(Buf,SIP_TITLE_VIA,tmp,len,lenname,psip,&is_add);
		if ( is_add ) {
			psip->titles.via[psip->titles.col_via] = psip->now_titles - 1;
			psip->titles.col_via++;
			while ( 1 ) {
				tmp1 = (char*)psip->all_titles[psip->titles.via[psip->titles.col_via-1]].value.p;
				len = psip->all_titles[psip->titles.via[psip->titles.col_via-1]].value.l;
				tmp2 = strstr(tmp1,",");
				if ( !tmp2 ) break;
				if ( tmp2 - tmp1 >= len ) break;
				psip->all_titles[psip->titles.via[psip->titles.col_via-1]].value.l = tmp2 - tmp1;
				len -= (tmp2 - tmp1);
				tmp2++;
				len--; 
				ret = add_sip_item_titles((char*)psip->all_titles[psip->titles.via[psip->titles.col_via-1]].name.p,
					psip->all_titles[psip->titles.via[psip->titles.col_via-1]].name.l,
					tmp2,len,psip);
				if ( ret != Ok ) return ret;
				psip->titles.via[psip->titles.col_via] = psip->now_titles - 1;
				psip->titles.col_via++;
			}
			return ret;			
		}
		ret = check_title(Buf,SIP_TITLE_VIA_S,tmp,len,lenname,psip,&is_add);
		if ( is_add ) {	
			psip->titles.via[psip->titles.col_via] = psip->now_titles - 1;
			psip->titles.col_via++;
			return ret;			
		}
	}

	if ( psip->titles.col_proxy_authorization < MAX_VIA ) {
		ret = check_title(Buf,SIP_TITLE_PROXY_AUTOR,tmp,len,lenname,psip,&is_add);
		if ( is_add ) {
			psip->titles.proxy_authorization[psip->titles.col_proxy_authorization] = psip->now_titles - 1;
			psip->titles.col_proxy_authorization++;
			return ret;			
		}
		ret = check_title(Buf,SIP_TITLE_AUTOR,tmp,len,lenname,psip,&is_add);
		if ( is_add ) {
			psip->titles.proxy_authorization[psip->titles.col_proxy_authorization] = psip->now_titles - 1;
			psip->titles.col_proxy_authorization++;
			return ret;			
		}
	}

	if ( !psip->titles.call_id ) {
		ret = check_title(Buf,SIP_TITLE_CALL_ID,tmp,len,lenname,psip,&is_add);
		if ( is_add ) {
			psip->titles.call_id = psip->now_titles - 1;
			return ret;			
		}
		ret = check_title(Buf,SIP_TITLE_CALL_ID_S,tmp,len,lenname,psip,&is_add);
		if ( is_add ) {
			psip->titles.call_id = psip->now_titles - 1;
			return ret;			
		}
	}
	if ( !psip->titles.from ) {
		ret = check_title(Buf,SIP_TITLE_FROM,tmp,len,lenname,psip,&is_add);
		if ( is_add ) {
			psip->titles.from = psip->now_titles - 1;
			return ret;			
		}
		ret = check_title(Buf,SIP_TITLE_FROM_S,tmp,len,lenname,psip,&is_add);
		if ( is_add ) {
			psip->titles.from = psip->now_titles - 1;
			return ret;			
		}
	}
	if ( !psip->titles.to ) {
		ret = check_title(Buf,SIP_TITLE_TO,tmp,len,lenname,psip,&is_add);
		if ( is_add ) {
			psip->titles.to = psip->now_titles - 1;
			return ret;			
		}
		ret = check_title(Buf,SIP_TITLE_TO_S,tmp,len,lenname,psip,&is_add);
		if ( is_add ) {
			psip->titles.to = psip->now_titles - 1;
			return ret;			
		}
	}
	if ( !psip->titles.contact ) {
		ret = check_title(Buf,SIP_TITLE_CONTACT,tmp,len,lenname,psip,&is_add);
		if ( is_add ) {
			psip->titles.contact = psip->now_titles -1;
			return ret;			
		}
		ret = check_title(Buf,SIP_TITLE_CONTACT_S,tmp,len,lenname,psip,&is_add);
		if ( is_add ) {
			psip->titles.contact = psip->now_titles -1;
			return ret;			
		}
	}

	//if ( !psip->titles.content_type ) {
	ret = check_title(Buf,SIP_TITLE_CONTENT_TYPE,tmp,len,lenname,psip,&is_add);
	if ( is_add ) {
		psip->titles.content_type = psip->now_titles -1;
		return ret;			
	}
	ret = check_title(Buf,SIP_TITLE_CONTENT_TYPE_S,tmp,len,lenname,psip,&is_add);
	if ( is_add ) {
		psip->titles.content_type = psip->now_titles -1;
		return ret;			
	}
	//}
	//if ( !psip->titles.content_length ) {
	ret = check_title(Buf,SIP_TITLE_CONTENT_LENGTH,tmp,len,lenname,psip,&is_add);
	if ( is_add ) {
		psip->titles.content_length = psip->now_titles - 1;
		psip->titles.content_length_v = atoi((char*)psip->all_titles[psip->titles.content_length].value.p);
		return ret;			
	}
	ret = check_title(Buf,SIP_TITLE_CONTENT_LENGTH_S,tmp,len,lenname,psip,&is_add);
	if ( is_add ) {
		psip->titles.content_length = psip->now_titles - 1;
		psip->titles.content_length_v = atoi((char*)psip->all_titles[psip->titles.content_length].value.p);
		return ret;			
	}
	//}
	if ( !psip->titles.expires ) {
		ret = check_title(Buf,SIP_TITLE_EXPIRES,tmp,len,lenname,psip,&is_add);
		if ( is_add ) {
			psip->titles.expires = psip->now_titles - 1;
			psip->titles.expires_v = atoi((char*)psip->all_titles[psip->titles.expires].value.p);
			return ret;			
		}
	}
	if ( !psip->titles.max_forward ) {
		ret = check_title(Buf,SIP_TITLE_MAX_FORVARD,tmp,len,lenname,psip,&is_add);
		if ( is_add ) {
			psip->titles.max_forward = psip->now_titles - 1;
			psip->titles.max_forward_v = atoi((char*)psip->all_titles[psip->titles.max_forward].value.p);
			return ret;			
		}
	}
	if ( psip->titles.col_record_route < MAX_VIA ) {
		ret = check_title(Buf,SIP_TITLE_RECORD_ROUTE,tmp,len,lenname,psip,&is_add);
		if ( is_add ) {
			psip->titles.record_route[psip->titles.col_record_route] = psip->now_titles - 1;
			psip->titles.col_record_route++;
			ret  = parse_title(Buf,psip,&is_add);
			if ( ret != Ok ) return ret;
			while ( is_add && psip->titles.col_record_route < MAX_VIA ) {
				psip->titles.record_route[psip->titles.col_record_route] = psip->now_titles - is_add;
				psip->titles.col_record_route++;
				is_add--;
			}
			return ret;			
		}
	}

	if ( psip->titles.col_route < MAX_VIA ) {
		ret = check_title(Buf,SIP_TITLE_ROUTE,tmp,len,lenname,psip,&is_add);
		if ( is_add ) {
			psip->titles.route[psip->titles.col_route] = psip->now_titles - 1;
			psip->titles.col_route++;
			ret  = parse_title(Buf,psip,&is_add);
			if ( ret != Ok ) return ret;
			while ( is_add && psip->titles.col_route < MAX_VIA ) {
				psip->titles.route[psip->titles.col_route] = psip->now_titles - is_add;
				psip->titles.col_route++;
				is_add--;
			}
			return ret;			
		}
	}

	if ( !psip->titles.cseg ) {
		ret = check_title(Buf,SIP_TITLE_CSEG,tmp,len,lenname,psip,&is_add);
		if ( is_add ) {
			psip->titles.cseg = psip->now_titles - 1;
			psip->titles.cseg_v = atoi((char*)psip->all_titles[psip->titles.cseg].value.p);
			tmp += lenname;
			len -= lenname;

			length=0;
			while ( isdigit(tmp[length++]) ) ;
			len -= length;
			tmp += length;
			if ( len <= 0 ) return ER_NOT_SIP_PROTOKOL;
			ret = get_command(tmp,len,&psip->titles.command_start,&len,psip);
			return ret;			
		}
	}
	if ( !psip->titles.refer_to ) {
		ret = check_title(Buf,SIP_TITLE_REFER_TO,tmp,len,lenname,psip,&is_add);
		if ( is_add ) {
			psip->titles.refer_to = psip->now_titles - 1;
			return ret;			
		}
	}

	if ( !psip->titles.user_agent ) {
		ret = check_title(Buf,SIP_TITLE_USER_AGENT,tmp,len,lenname,psip,&is_add);
		if ( is_add ) {
			psip->titles.user_agent = psip->now_titles - 1;
			return ret;			
		}
	}

	if ( !psip->titles.x_regsvlistno ) {
		ret = check_title(Buf,SIP_TITLE_X_REGSVLISTNO,tmp,len,lenname,psip,&is_add);
		if ( is_add ) {
			psip->titles.x_regsvlistno = psip->now_titles - 1;
			return ret;			
		}
	}



	length = find_str(tmp,":");
	if ( length <= 0 ) return Ok;
	length += 2;
	//if ( length >= len ) return ER_NOT_SIP_PROTOKOL;
	if ( length >= len ) return Ok;
	ret = add_sip_item_titles(tmp,length,tmp + length,len - length,psip);
	return ret;
}
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
mvi_status_t check_message(const char* pbuf,const char* ptitle,const char* tmp,int len,sip_t* psip,int* is_add)
{
	mvi_status_t ret = Ok;
	int	lentitle = strlen(ptitle);
	*is_add = 0;

	if ( !strncmp(pbuf,ptitle,lentitle) ) {
		*is_add = 1;
		if ( lentitle > len ) ret = ER_NOT_SIP_PROTOKOL;
		else {
			len -= lentitle;
			ret = add_sip_item_message(tmp,lentitle,tmp + lentitle,len,psip);
		}
	}
	return ret;
}
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
BOOL is_hcolon(const char* tmp,int len)
{
	int i;
	for (i=0; i<len; i++ ) {
		switch (*(tmp+i) ) {
		case ':':
		case 0x20:
		case 0x09:
		case 0x0D:
		case 0x0A:
			break;
		default:
			return FALSE;
		}
	}
	return TRUE;
}
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
mvi_status_t check_title(const char* pbuf,const char* ptitle,const char* tmp,int len,int lenname,sip_t* psip,int* is_add)
{
	mvi_status_t ret = Ok;
	int	lentitle = strlen(ptitle);
	*is_add = 0;
	if ( lenname < lentitle ) return ret;

	if ( !strncmp(pbuf,ptitle,lentitle) ) {
		if ( !is_hcolon(pbuf+lentitle,lenname - lentitle) ) return ret;
		*is_add = 1;
		if ( lentitle >= len ) ret = ER_NOT_SIP_PROTOKOL;
		else {
			//		len -= lentitle;
			//		ret = add_sip_item_titles(tmp,lentitle,tmp + lentitle,len,psip);
			len -= lenname;
			ret = add_sip_item_titles(tmp,lenname,tmp + lenname,len,psip);
		}
	}
	return ret;
}
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
mvi_status_t parse_title(const char* pbuf,sip_t* psip,int* pcol)
{
	mvi_status_t ret = Ok;

	const char*	tmp_name = (char*)psip->all_titles[psip->now_titles-1].name.p;
	int		len_name = psip->all_titles[psip->now_titles-1].name.l;
	const char*	tmp_value;
	int		len_value;
	*pcol = 0;

	tmp_value = pbuf;
	while ( ret == Ok ) {
		tmp_value = strstr(tmp_value,",");
		if ( !tmp_value ) break;
		(*pcol) ++;
		len_value = strlen(tmp_value);
		psip->all_titles[psip->now_titles-1].value.l -= len_value;
		tmp_value ++; 
		len_value --;
		ret = add_sip_item_titles(tmp_name,len_name,(char*)(psip->all_titles[psip->now_titles-1].value.p + psip->all_titles[psip->now_titles-1].value.l + 1),len_value,psip);
	}
	return ret;
}
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
mvi_status_t get_command(const char* tmp,int len,sip_method_t* psip_command,int* plensip_command,sip_t* psip)
{
	int mes;
	int i = SIP_MESSAGE_1XX;

	(void)len;

	switch (i)  {
	case SIP_MESSAGE_1XX:
		*plensip_command = strlen(SIP_VERSION);
		if ( !strncmp(tmp,SIP_VERSION,*plensip_command) ) {
			tmp += strlen(SIP_VERSION)+1;
			psip->titles.mess = atoi(tmp);
			mes = psip->titles.mess / 100;
			switch (mes) {
	case 1:
		*psip_command = SIP_MESSAGE_1XX;
		break;
	case 2:
		*psip_command = SIP_MESSAGE_2XX;
		break;
	case 3:
		*psip_command = SIP_MESSAGE_3XX;
		break;
	case 4:
		*psip_command = SIP_MESSAGE_4XX;
		break;
	case 5:
		*psip_command = SIP_MESSAGE_5XX;
		break;
	case 6:
		*psip_command = SIP_MESSAGE_6XX;
		break;
	default:
		return ER_NOT_SIP_PROTOKOL;
			}
			break;
		}
	case SIP_COM_INVITE:	
		*plensip_command = strlen(SIP_COM_INVITE_STR);
		if ( !strncmp(tmp,SIP_COM_INVITE_STR,*plensip_command) ) {
			*psip_command = SIP_COM_INVITE;
			break;
		}
	case SIP_COM_ACK:	
		*plensip_command = strlen(SIP_COM_ACK_STR);
		if ( !strncmp(tmp,SIP_COM_ACK_STR,*plensip_command) ) {
			*psip_command = SIP_COM_ACK;
			break;
		}
	case SIP_COM_BYE:	
		*plensip_command = strlen(SIP_COM_BYE_STR);
		if ( !strncmp(tmp,SIP_COM_BYE_STR,*plensip_command) ) {
			*psip_command = SIP_COM_BYE;
			break;
		}
	case SIP_COM_CANCEL:	
		*plensip_command = strlen(SIP_COM_CANCEL_STR);
		if ( !strncmp(tmp,SIP_COM_CANCEL_STR,*plensip_command) ) {
			*psip_command = SIP_COM_CANCEL;
			break;
		}
	case SIP_COM_REGISTER:	
		*plensip_command = strlen(SIP_COM_REGISTER_STR);
		if ( !strncmp(tmp,SIP_COM_REGISTER_STR,*plensip_command) ) {
			*psip_command = SIP_COM_REGISTER;
			break;
		}
	case SIP_COM_OPTION:	
		*plensip_command = strlen(SIP_COM_OPTION_STR);
		if ( !strncmp(tmp,SIP_COM_OPTION_STR,*plensip_command) ) {
			*psip_command = SIP_COM_OPTION;
			break;
		}
	case SIP_COM_INFO:	
		*plensip_command = strlen(SIP_COM_INFO_STR);
		if ( !strncmp(tmp,SIP_COM_INFO_STR,*plensip_command) ) {
			*psip_command = SIP_COM_INFO;
			break;
		}
	case SIP_COM_PRACK:	
		*plensip_command = strlen(SIP_COM_PRACK_STR);
		if ( !strncmp(tmp,SIP_COM_PRACK_STR,*plensip_command) ) {
			*psip_command = SIP_COM_PRACK;
			break;
		}
	case SIP_COM_UPDATE:	
		*plensip_command = strlen(SIP_COM_UPDATE_STR);
		if ( !strncmp(tmp,SIP_COM_UPDATE_STR,*plensip_command) ) {
			*psip_command = SIP_COM_UPDATE;
			break;
		}
	case SIP_COM_SUBSCRIBE:	
		*plensip_command = strlen(SIP_COM_SUBSCRIBE_STR);
		if ( !strncmp(tmp,SIP_COM_SUBSCRIBE_STR,*plensip_command) ) {
			*psip_command = SIP_COM_SUBSCRIBE;
			break;
		}
	case SIP_COM_NOTIFY:	
		*plensip_command = strlen(SIP_COM_NOTIFY_STR);
		if ( !strncmp(tmp,SIP_COM_NOTIFY_STR,*plensip_command) ) {
			*psip_command = SIP_COM_NOTIFY;
			break;
		}
	case SIP_COM_REFER:	
		*plensip_command = strlen(SIP_COM_REFER_STR);
		if ( !strncmp(tmp,SIP_COM_REFER_STR,*plensip_command) ) {
			*psip_command = SIP_COM_REFER;
			break;
		}
	case SIP_COM_MESSAGE:	
		*plensip_command = strlen(SIP_COM_MESSAGE_STR);
		if ( !strncmp(tmp,SIP_COM_MESSAGE_STR,*plensip_command) ) {
			*psip_command = SIP_COM_MESSAGE;
			break;
		}
	case SIP_COM_PUBLISH:	
		*plensip_command = strlen(SIP_COM_PUBLISH_STR);
		if ( !strncmp(tmp,SIP_COM_PUBLISH_STR,*plensip_command) ) {
			*psip_command = SIP_COM_PUBLISH;
			break;
		}
	default:
		return ER_NOT_SIP_PROTOKOL;
	}
	return Ok;
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
mvi_status_t read_start_str(const char* tmp,int len,sip_t* psip)
{
	int lencmp;
	mvi_status_t ret;

	ret = get_command(tmp,len,&psip->titles.command,&lencmp,psip);
	if ( ret != Ok ) return ret;

	if ( lencmp >= len ) return ER_NOT_SIP_PROTOKOL;
	len -= lencmp;
	return add_sip_item_titles(tmp,lencmp,tmp+lencmp,len,psip);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
mvi_status_t add_sip_item_titles(const char* pname,int lenname,const char* pvalue,int lenvalue,sip_t* psip)
{
	if ( psip->now_titles >= MAX_TITLES ) return ER_SMALL_BUFFER;
	psip->all_titles[psip->now_titles].name.p = (u_char*)pname;
	psip->all_titles[psip->now_titles].name.l = lenname;
	psip->all_titles[psip->now_titles].value.p = (u_char*)pvalue;
	psip->all_titles[psip->now_titles].value.l = lenvalue;
	psip->now_titles ++;
	return Ok;
}
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
mvi_status_t add_sip_item_message(const char* pname,int lenname,const char* pvalue,int lenvalue,sip_t* psip)
{
	if ( psip->col_messages >= MAX_TITLES ) return ER_SMALL_BUFFER;
	psip->all_messages[psip->col_messages].name.p = (u_char*)pname;
	psip->all_messages[psip->col_messages].name.l = lenname;
	psip->all_messages[psip->col_messages].value.p = (u_char*)pvalue;
	psip->all_messages[psip->col_messages].value.l = lenvalue;
	psip->col_messages ++;
	return Ok;
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
int find_str(const char* pstr,const char* ptemplate)
{
	char* tmp = strstr(pstr,ptemplate);
	if ( !tmp ) return 0;
	return (tmp - pstr);
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
mvi_status_t set_struct_url(psip_url_t* purl)
{
	char*	tmp;

	tmp = strstr(purl->purl,"<");
	if ( !tmp ) tmp = purl->purl;
	else		tmp++;

	while ( *tmp == ' ' )  tmp++;
	purl->ptitle = tmp;

	tmp = strstr(tmp,":");
	if ( !tmp ) return ER_SIP_PARSE_URL; 
	*tmp = 0;
	tmp++;
	purl->pname = tmp;


	tmp = strstr(tmp,"@");
	if ( !tmp ) {
		purl->pdomen = purl->pname;
		purl->pname--;
		tmp = purl->pdomen;
		purl->pname = NULL;
	}
	else {
		//	*tmp = 0;
		tmp++;
		purl->pdomen = tmp;
	}

	skip_IPV4ADDRESS((const u_char **)&tmp);
	while (1) {
		purl->ch_end = *tmp;
		if ( *tmp == 0 ) break;
		if ( *tmp == ' ' ) { *tmp = 0;	break; }
		if ( *tmp == ':' ) { *tmp = 0;	tmp++;	purl->pport = tmp;	continue; }
		if ( *tmp == '>' ) { *tmp = 0;	break; }
		if ( *tmp == ';' ) { *tmp = 0;	break; }
		tmp++;
	}
	if ( !strlen(purl->pdomen) ) return ER_SIP_PARSE_URL; 


	return Ok;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
mvi_status_t psip_set_struct_url1(char* psipurl,psip_url_t* purl)
{
	memset(purl,0,sizeof(psip_url_t));
	purl->purl = psipurl;

	return set_struct_url(purl);
}
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
mvi_status_t psip_set_struct_url(item_buffer_t** ppibt,const char* psipurl,psip_url_t* purl)
{
	mvi_status_t ret = Ok; 
	if ( !psipurl ) return ER_SIP_PARSE_URL; 
	memset(purl,0,sizeof(psip_url_t));
	purl->purl = malloc_char(ppibt,(char*)psipurl,&ret);
	if ( ret != Ok ) return ret;

	return set_struct_url(purl);
}
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
void psip_back_struct_url(psip_url_t* purl)
{
	if ( purl->ptitle ) purl->ptitle[strlen(purl->ptitle)] = ':'; 
	//if ( purl->pname )	purl->pname[strlen(purl->pname)] = '@'; 
	if ( purl->pport )	{
		purl->pdomen[strlen(purl->pdomen)] = ':';
		purl->pport[strlen(purl->pport)] = purl->ch_end;
	}
	else {
		purl->pdomen[strlen(purl->pdomen)] = purl->ch_end;
	}
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
mvi_status_t sip_set_struct_url(const char* psipurl,sip_url_t* psul)
{
	char*	tmp;
	if ( !psipurl ) return ER_SIP_PARSE_URL; 

	memset(psul,0,sizeof(sip_url_t));
	strncpy(psul->url,psipurl,sizeof(psul->url)-1);

	tmp = strstr(psul->url,"<");
	if ( !tmp ) tmp = psul->url,psipurl;
	else		tmp++;

	while ( *tmp == ' ' )  tmp++;
	psul->ptitle = tmp;

	tmp = strstr(tmp,":");
	if ( !tmp ) return ER_SIP_PARSE_URL; 
	*tmp = 0;
	tmp++;
	psul->pname = tmp;


	tmp = strstr(tmp,"@");
	//if ( !tmp ) return ER_SIP_PARSE_URL; 
	if ( !tmp ) {
		psul->pdomen = psul->pname;
		psul->pname--;
		tmp = psul->pdomen;
	}
	else {
		*tmp = 0;
		tmp++;
		psul->pdomen = tmp;
	}

	skip_IPV4ADDRESS((const u_char **)&tmp);
	while (1) {
		psul->ch_end = *tmp;
		if ( *tmp == 0 ) break;
		if ( *tmp == ' ' ) { *tmp = 0;	break; }
		if ( *tmp == ':' ) { *tmp = 0;	tmp++;	psul->pport = tmp;	continue; }
		if ( *tmp == '>' ) { *tmp = 0;	break; }
		if ( *tmp == ';' ) { *tmp = 0;	break; }
		tmp++;
	}
	if ( !strlen(psul->pdomen) ) return ER_SIP_PARSE_URL; 
	return Ok;
}




//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
BOOL check_contact_unregistry_all(const char* pcontact)
{
	int i = 0;

	while (1) {
		if ( pcontact[i] == '*' ) return TRUE;
		if ( pcontact[i] != ' ' ) break;
		i++;
	}
	return FALSE; 
}
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
BOOL skip_IPV4ADDRESS(const u_char** pptr)
{
	const u_char* tmp;
	int i;

	tmp = *pptr;

	for ( i=0; i<4; i++) {
		skip_DIGIT(tmp)
			if ( tmp - *pptr > 3 || tmp == *pptr ) return FALSE;
		if ( i == 3 ) {
			*pptr = tmp;	
			break;
		}
		if ( tmp[0] != '.' ) return FALSE;
		tmp ++;
		*pptr = tmp;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
BOOL media2task(sip_t* psip,int index,rtp_task_t* ptask)
{
	mvi_status_t ret;
	char* tmp;
	int i;

	if ( psip->col_media <= index ) return FALSE;
	if ( !psip->sip_media[index].c ) {
		LOG_error("not present sip_media[index].c");
		return FALSE;
	}
	if ( !psip->sip_media[index].m ) {
		LOG_error("not present sip_media[index].m");
		return FALSE;
	}
	ret = ipaddr2long((char*)psip->all_messages[psip->sip_media[index].c].value.p,&ptask->addr);
	if ( ret != Ok ) {
		LOG_error("parse IP Addr media.c");
		return FALSE;
	}
	tmp = strstr((char*)psip->all_messages[psip->sip_media[index].m].value.p," ");
	if ( !tmp ) {
		LOG_error("parse IP Addr media.m");
		return FALSE;
	}
	tmp++; 
	//tmp1 = tmp;
	//tmp = strstr(tmp," ");
	//if ( !tmp ) return ER_RTP_PARAMETERS;
	//*tmp = 0;
	//port = (uint16)atoi(tmp1);
	ptask->port = (uint16)atoi(tmp);
	if ( !ptask->port ) {
		LOG_error("parse IP Addr media.m  port = %s",tmp);
		return FALSE;
	}

	for ( i=0; i<2; i++ ) {
		tmp = strstr(tmp," ");
		if ( !tmp ) {
			LOG_error("parse Codec media.m");
			return FALSE;
		}
		tmp++;
	}
	ptask->codec = (uint8)atoi(tmp);

	ptask->rtpmap_time = 8000;
	for ( i=psip->sip_media[index].m + 1; i<psip->col_messages; i++ ) {
		if ( !memcmp(psip->all_messages[i].value.p,"rtpmap:",strlen("rtpmap:")) ) {
			tmp = strstr((char*)psip->all_messages[i].value.p,"/");
			tmp ++;
			if ( tmp ) 	ptask->rtpmap_time = atoi(tmp);
			break;
		}
	}
	ptask->is_active = TRUE;
	for ( ;i<psip->col_messages; i++ ) {
		if ( !memcmp(psip->all_messages[i].value.p,"inactive",strlen("inactive")) ) {
			ptask->is_active = FALSE;	
			break;
		}
		if ( !memcmp(psip->all_messages[i].value.p,"recvonly",strlen("recvonly")) ) {
			ptask->is_active = FALSE;	
			break;
		}
	}



	return TRUE;
}
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
void check_contact_expires(sip_t* psip)
{
	char* tmp = NULL;
	if ( psip->titles.command_start != SIP_COM_REGISTER ) return;
	if ( psip->titles.expires ) return;
	if ( psip->titles.contact ) {
		if ( !tmp ) tmp = (char*)psip->all_titles[psip->titles.contact].value.p;
		tmp = strstr(tmp,SIP_TITLE_EXPIRES_CONTACT);
		if ( tmp ) {
			tmp += strlen(SIP_TITLE_EXPIRES_CONTACT);
			psip->titles.expires_v = atoi(tmp);
			return;
		}
	}
}

