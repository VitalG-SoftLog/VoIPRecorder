#include "StdAfx.h"
#include "SIPPacket.h"
#include <Shlwapi.h>
#include "StringUtils.h"

#pragma comment(lib,"shlwapi.lib")

using namespace StringUtils;

#define _table_entry(_Method) {_Method, #_Method, sizeof(#_Method) - 1},

CSIPPacket::_method_str_pair 
	CSIPPacket::m_xMethodstable[] = 
{
	_table_entry(ACK)
	_table_entry(BYE)
	_table_entry(CANCEL)
	_table_entry(INVITE)
	_table_entry(REGISTER)
	_table_entry(OPTIONS)
	_table_entry(NOTIFY)
	_table_entry(SUBSCRIBE)
//	_table_entry(PEFER)
	_table_entry(INFO)
	_table_entry(PRACK)
	_table_entry(PUBLISH)
	_table_entry(UPDATE)
	{UNKNOWN,	NULL, 0}
};

std::string CSIPPacket::MethodToString(CSIPPacket::MethodTypes m)
{
	for (int i = 0; m_xMethodstable[i].str; i++)
	{
		if (m == m_xMethodstable[i].method)
		{
			return m_xMethodstable[i].str;
		}
	}
	return "";
}

CSIPPacket::MethodTypes CSIPPacket::StringToMethod(const std::string &method)
{
	for (int i = 0; m_xMethodstable[i].str; i++)
	{
		if (method.compare(0, m_xMethodstable[i].strlen, m_xMethodstable[i].str) == 0)
		{
			return m_xMethodstable[i].method;
		}
	}

	return UNKNOWN;
}

CSIPPacket::CSIPPacket(void)
{
	Invalidate();
}

CSIPPacket::~CSIPPacket(void)
{
	Invalidate();
}

CSIPPacket::CSIPPacket(const CSIPPacket &p)
{
	m_bIsRequest	= p.m_bIsRequest;
	m_bIsResponse	= p.m_bIsResponse;

	m_sStatusLine	= p.m_sStatusLine;
	m_sReason		= p.m_sReason;
	m_iStatusCode	= p.m_iStatusCode;

	m_sRequestLine	= p.m_sRequestLine;
	m_sRequestUri	= p.m_sRequestUri;
	m_eMethod		= p.m_eMethod;

	m_xHeaders		= p.m_xHeaders;
	m_sBody			= p.m_sBody;

	m_xSrcIP		= p.m_xSrcIP;
	m_xDstIP		= p.m_xDstIP;

	m_iCSeqNumber	= p.m_iCSeqNumber;
	m_eCSeqMethod	= p.m_eCSeqMethod;
	m_sRawData		= p.m_sRawData;

	m_sFromTag		= p.m_sFromTag;
	m_sFromAddress	= p.m_sFromAddress;
	m_sFromUser		= p.m_sFromUser;

	m_sToTag		= p.m_sToTag;
	m_sToAddress	= p.m_sToAddress;
	m_sToUser		= p.m_sToUser;
}

CSIPPacket::CSIPPacket(const CUDPPacket &packet)
{
	Invalidate();
	if (packet.DataLen() && packet.Data())
	{
		std::string sPacketBody;
		sPacketBody.assign(packet.Data(), packet.DataLen());
		if (Parse(sPacketBody))
		{
			m_xSrcIP = packet.Src();
			m_xDstIP = packet.Dst();
		}
	}
}

bool CSIPPacket::IsValid() const
{
	return 
		((m_bIsRequest && m_eMethod != CSIPPacket::UNKNOWN) || 
		(m_bIsResponse && m_iStatusCode != 0)) && 
		(m_eCSeqMethod != CSIPPacket::UNKNOWN) && 
		(m_iCSeqNumber != 0);
}

void CSIPPacket::Invalidate()
{
	m_bIsRequest = false;
	m_bIsResponse = false;

	m_sStatusLine.clear();
	m_sReason.clear();
	m_iStatusCode = 0;

	m_sRequestLine.clear();
	m_sRequestUri.clear();
	m_eMethod = CSIPPacket::UNKNOWN;

	m_xSrcIP.Invalidate();
	m_xDstIP.Invalidate();
	m_xHeaders.clear();
	m_sBody.clear();
	m_eCSeqMethod = CSIPPacket::UNKNOWN;
	m_iCSeqNumber = 0;
	m_sRawData.clear();

	m_sFromTag.clear();
	m_sFromAddress.clear();
	m_sFromUser.clear();

	m_sToTag.clear();
	m_sToAddress.clear();
	m_sToUser.clear();


}

const std::string &CSIPPacket::Header(const std::string &sName) const
{
	static const std::string sEmpty = "";
	
	std::map<std::string, 
		std::string>::const_iterator it;

	it = m_xHeaders.find(sName);
	return (m_xHeaders.end() == it) ? sEmpty : it->second;
}

int CSIPPacket::CSeqNumber() const
{
	return m_iCSeqNumber;
}

CSIPPacket::MethodTypes CSIPPacket::CSeqMethod() const
{
	return m_eCSeqMethod;
}

const std::string &CSIPPacket::RequestLine() const
{
	return m_sRequestLine;
}

const std::string &CSIPPacket::StatusLine() const
{
	return m_sStatusLine;
}


bool CSIPPacket::Parse(const std::string &sPacket)
{
	Invalidate();
	m_sRawData = sPacket;

	// check if response packet
	const char proto[] = "SIP/2.0 ";
	if (sPacket.compare(0, sizeof(proto)-1, proto) == 0)
	{
		m_bIsResponse = true;
	}
	
	if (!m_bIsResponse)
	{
		// check if request packet
		m_eMethod = StringToMethod(sPacket);
		if (m_eMethod != UNKNOWN)
		{
			m_bIsRequest = true;
		}
	}

	if (!m_bIsResponse && !m_bIsRequest)
	{	
		// not a SIP packet
		return false;
	}

	// tokenize packet body
	std::string sHeaders = sPacket;

	// trim leading spaces
	size_t endpos = sPacket.find(_NEW_BODY_SEP);
	if (std::string::npos != endpos)
	{
		sHeaders = sPacket.substr(0, endpos);

		if ((endpos + (sizeof(_NEW_BODY_SEP) - 1)) < sPacket.length())
			m_sBody = sPacket.substr(endpos + sizeof(_NEW_BODY_SEP)-1, -1);
	}	

	std::list<std::string> vLines = Explode(sHeaders, _NEW_LINE);

	if (vLines.empty())
	{
		Invalidate();
		return false;
	}

	std::list<std::string>::iterator it = vLines.begin();

	if (m_bIsRequest) {m_sRequestLine = *it;}
	else {m_sStatusLine = *it;}
	it++;

	for (;it != vLines.end(); ++it)
	{
		std::string sLine = Trim(*it);

		if (!sLine.empty())
		{
			std::string::size_type start = 0;
			std::string::size_type end = 0;

			if ((end = sLine.find(":", start)) != std::string::npos)
			{
				// header
				std::string sName = sLine.substr(start, end-start);
				std::string sValue = Trim(sLine.substr(end+1, -1));
				m_xHeaders[sName] = sValue;
			}
		}
	}

	if (m_bIsResponse)
	{
		// tokenize packet body
		vLines = Explode(m_sStatusLine, " ");
		if (vLines.size() > 2)
		{
			std::list<std::string>::iterator it = vLines.begin();
			it++;
			m_iStatusCode = StrToIntA((*it).c_str());
		}
	}

	// parse CSeq
	std::string sCSeq = CSeq();
	vLines = Explode(sCSeq, " ");
	if (vLines.size() == 2)
	{
		std::list<std::string>::iterator it = vLines.begin();
		m_iCSeqNumber = StrToIntA(it->c_str());
		it++;
		m_eCSeqMethod = StringToMethod(*it);
	}

	if (IsValid())
	{
		ParseTagsAddr(From(), m_sFromAddress, m_sFromTag, m_sFromUser);
		ParseTagsAddr(To(), m_sToAddress, m_sToTag, m_sToUser);
	}

	return IsValid();
}

void CSIPPacket::ParseTagsAddr(const std::string &sInput, std::string &sAddr, std::string &sTag, std::string &sUser)
{
	const char start_user_tag[] = "sip:";
	const char end_user_tag[]	= "@";
	
	// parse from tag
	sAddr = sInput;
	std::list<std::string> vLines = Explode(sInput, ";tag=");

	if (vLines.size())
	{
		sAddr = *(vLines.begin());
	}

	if (vLines.size() == 2)
	{
		sTag = *(++vLines.begin());
	}

	// parse from user
	if (!sAddr.empty())
	{
		size_t s = sAddr.find(start_user_tag);
		size_t e = sAddr.find(end_user_tag);

		if (s != std::string::npos && e != std::string::npos && s < e)
		{
			sUser = sAddr.substr(s+sizeof(start_user_tag)-1, e - (s+sizeof(start_user_tag)-1));
		}
	}
}

