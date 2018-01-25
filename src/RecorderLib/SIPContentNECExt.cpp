#include "StdAfx.h"
#include "SIPContentNECExt.h"
#include "StringUtils.h"

#include <map>
using namespace std;
using namespace StringUtils;

#define _NS_LINE_END			"\r\n"
#define _NS_KEY_VAL_SEP			"="
#define _NS_CONTENT_TYPE		"application/x-necsipext2mlv1"


#define _NS_EVENT_HANDSET		"event-handset"
#define _NS_DISP_LINE			"ind-displinen"
#define _NS_INFO_CURRNO			"info-currentno2"
#define _NS_INFO_DIRECTIN		"info-directin2"

CSIPContentNECExt::CSIPContentNECExt(const std::string &sContentType, const std::string &sSipBody)
{
	m_bValid = false;

	string content_type = ToLowerCase(sContentType);

	if (content_type == _NS_CONTENT_TYPE)
		m_bValid = ParseNecSipExt(sSipBody);

	if (content_type.find("multipart/") != string::npos)
		m_bValid = ParseMultiPart(sSipBody);

}

CSIPContentNECExt::~CSIPContentNECExt(void)
{
}

bool CSIPContentNECExt::IsNecHeaderPresent(const std::string &sHeader) const
{
	multimap<string, string>::const_iterator i = m_xData.find(sHeader);
	return i != m_xData.end();
}


bool CSIPContentNECExt::IsHandsetEvent() const
{
	return IsNecHeaderPresent(_NS_EVENT_HANDSET);
}

bool CSIPContentNECExt::IsHandset() const
{
	multimap<string, string>::const_iterator i = m_xData.find(_NS_EVENT_HANDSET);
	if (i != m_xData.end())
		return i->second == "on";

	return true;	
}


std::vector<std::string> CSIPContentNECExt::ExplodeHeader(
	const std::string &sHeaderName, const std::string &sep, bool bLast /*= false*/) const
{
	std::vector<std::string> vReturn;
	multimap<string, string>::const_iterator i = m_xData.find(sHeaderName);
	if (i != m_xData.end()) {
		if (bLast) {
			i = m_xData.upper_bound(sHeaderName);
			i--;
		}
		vReturn = StringUtils::VExplode(i->second, sep);
	}

	return vReturn;
}


bool CSIPContentNECExt::IsDispLineEvent(int iDispLineNumber) const
{
	std::vector<std::string> params = ExplodeHeader(_NS_DISP_LINE, ":");
	if (params.size() >= 2) {
		// check if our line...7
		return (params[0] == StringUtils::IntToString(iDispLineNumber));
	}
	return false;
}

bool CSIPContentNECExt::IsDispLineClear(int iDispLineNumber) const
{
	std::vector<std::string> params = ExplodeHeader(_NS_DISP_LINE, ":");
	if (params.size() >= 2) {
		// check if our line...7
		if (params[0] == StringUtils::IntToString(iDispLineNumber)) {
			return params[1] == "clr";
		}
	}

	return false;
}

std::string CSIPContentNECExt::DispLineText(int iDispLineNumber) const
{
	std::string sReturn;
	std::vector<std::string> params = ExplodeHeader(_NS_DISP_LINE, ":");
	if (params.size() == 3) {
		// check if our line...
		if (params[0] == StringUtils::IntToString(iDispLineNumber)) {
			// our line
			if (params[1] == "dsp") {
				return params[2];
			}
		}
	}
	return sReturn;
}

bool CSIPContentNECExt::IsInfoCurrentNoEvent() const
{
	return (ExplodeHeader(_NS_INFO_CURRNO, " ").size() != 0);
}

std::string CSIPContentNECExt::InfoCurrentNo() const
{
	std::vector<std::string> params = ExplodeHeader(_NS_INFO_CURRNO, " ", true);
	if (params.size()) {

		std::string sNoPair;
		// try to find "no:12345"
		for (unsigned i = 0; i < params.size(); i++) {
			if (params[i].find("no:") == 0) {
				sNoPair = params[i];
				break;
			}
		}

		if (!sNoPair.empty()) {
			params = StringUtils::VExplode(sNoPair, ":");
			if (params.size() == 2) {
				return StringUtils::Trim(params[1]);
			}
		}
	}

	return "";
}

// Info-DirectIn2: events
bool CSIPContentNECExt::IsInfoDirectIn() const
{
	return (ExplodeHeader(_NS_INFO_DIRECTIN, " ").size() != 0);
}

// Info-DirectIn2=unknown ans no:22518
std::string CSIPContentNECExt::InfoDirectIn() const
{
	std::vector<std::string> params = ExplodeHeader(_NS_INFO_DIRECTIN, " ", true);
	if (params.size()) {

		std::string sNoPair;
		// try to find "no:12345"
		for (unsigned i = 0; i < params.size(); i++) {
			if (params[i].find("no:") == 0) {
				sNoPair = params[i];
				break;
			}
		}

		if (!sNoPair.empty()) {
			params = StringUtils::VExplode(sNoPair, ":");
			if (params.size() == 2) {
				return StringUtils::Trim(params[1]);
			}
		}
	}

	return "";
}



bool CSIPContentNECExt::ParseNecSipExt(const std::string &sSipBody)
{
	list<string> vTokens = Explode(sSipBody, _NS_LINE_END);
	list<string>::iterator i = vTokens.begin();

	for (;i != vTokens.end(); i++)
	{
		string sLine = Trim(*i);
		list<string> vKeyValue = Explode(sLine, _NS_KEY_VAL_SEP);

		if (vKeyValue.size() == 2)
		{
			list<string>::iterator k = vKeyValue.begin();
			string key = ToLowerCase(*k); 
			k++; 
			string value = ToLowerCase(*k);
			m_xData.insert(pair<string, string>(key, value));
		}
	}

	return !m_xData.empty();
}

bool CSIPContentNECExt::ParseMultiPart(const std::string &sSipBody)
{
	string ct = "content-type: " _NS_CONTENT_TYPE _NS_LINE_END;
	string lowercase_body = ToLowerCase(sSipBody);

	size_t start = lowercase_body.find(ct);

	if (start == string::npos)
		return false;

	size_t sdp_start = sSipBody.find(_NS_LINE_END _NS_LINE_END, start);

	if (sdp_start == string::npos)
		return false;

	sdp_start += strlen(_NS_LINE_END _NS_LINE_END);

	size_t sdp_end = sSipBody.find(_NS_LINE_END _NS_LINE_END, sdp_start);

	if (sdp_end == string::npos)
		return false;

	return ParseNecSipExt(sSipBody.substr(sdp_start, sdp_end - sdp_start));
}
