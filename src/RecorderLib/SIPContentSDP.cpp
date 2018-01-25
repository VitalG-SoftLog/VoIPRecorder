// File: SDPContent.cpp
//////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "SIPContentSDP.h"
#include "StringUtils.h"

#include <map>
using namespace std;
using namespace StringUtils;

#define _LINE_END			"\r\n"
#define _SDP_KEY_VAL_SEP	"="
#define _SDP_CONTENT_TYPE	"application/sdp"

CSIPContentSDP::CSIPContentSDP(const std::string &sContentType, const std::string &sSipBody)
{
	m_bValid = false;
	
	string content_type = ToLowerCase(sContentType);

	if (content_type == _SDP_CONTENT_TYPE)
		m_bValid = ParseSDP(sSipBody);

	if (content_type.find("multipart/") != string::npos)
		m_bValid = ParseMultiPart(sSipBody);
}

CSIPContentSDP::~CSIPContentSDP(void)
{
}

const CRTPSessionParam &CSIPContentSDP::GetSessionParams() const 
{
	return m_xRtpSessionParams;
}

bool CSIPContentSDP::IsValid() const 
{
	return m_bValid;
}

bool CSIPContentSDP::ParseSDP(const std::string &sSipBody)
{
	list<string> vTokens = Explode(sSipBody, _LINE_END);
	list<string>::iterator i = vTokens.begin();

	multimap<string, string> xKeyValues;

	for (;i != vTokens.end(); i++)
	{
		string sLine = Trim(*i);
		list<string> vKeyValue = Explode(sLine, _SDP_KEY_VAL_SEP);

		if (vKeyValue.size() == 2)
		{
			list<string>::iterator k = vKeyValue.begin();
			string key = *k; k++; string value = *k;
			xKeyValues.insert(pair<string, string>(key, value));
		}
	}

	// parse o=
	multimap<string, string>::iterator mi = xKeyValues.find("o");
	if (mi == xKeyValues.end())
		return false;

	vTokens = Explode(mi->second, " ");
	if (vTokens.size() != 6)
		return false;

	list<string>::reverse_iterator ri = vTokens.rbegin();
	m_xRtpSessionParams.SetDst(*ri);

	// parse m=
	mi = xKeyValues.find("m");
	if (mi == xKeyValues.end())
		return false;

	vTokens = Explode(mi->second, " ");
	if (vTokens.size() < 4)
		return false;

	i = vTokens.begin();

	if (*i != "audio") 
		return false;

	i++; 
	CIPv4Address dst = m_xRtpSessionParams.GetDst();
	dst.Port(htons(StringToInt(*i)));
	m_xRtpSessionParams.SetDst(dst);

	i++; 
	if (*i != "RTP/AVP") 
		return false;

	i++; 
	m_xRtpSessionParams.SetPayloadType(StringToInt(*i));

	return true;
}

bool CSIPContentSDP::ParseMultiPart(const std::string &sSipBody)
{
	string ct = "content-type: " _SDP_CONTENT_TYPE _LINE_END;
	string lowercase_body = ToLowerCase(sSipBody);

	size_t start = lowercase_body.find(ct);

	if (start == string::npos)
		return false;
	
	size_t sdp_start = sSipBody.find(_LINE_END _LINE_END, start);

	if (sdp_start == string::npos)
		return false;

	sdp_start += strlen(_LINE_END _LINE_END);
	
	size_t sdp_end = sSipBody.find(_LINE_END _LINE_END, sdp_start);

	if (sdp_end == string::npos)
		return false;

	return ParseSDP(sSipBody.substr(sdp_start, sdp_end - sdp_start));
}
