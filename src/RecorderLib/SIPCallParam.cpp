#include "StdAfx.h"
#include "SIPCallParam.h"
#include "StringUtils.h"

using namespace StringUtils;

_MODULE_NAME(CSIPCallParam);

CSIPCallParam::CSIPCallParam()
{
	Invalidate();
}

CSIPCallParam::CSIPCallParam(const CSIPCallParam &c)
{
	operator=(c);
}

CSIPCallParam::~CSIPCallParam()
{
	Invalidate();
}

void CSIPCallParam::Invalidate()
{
	m_xSessionsID.clear();
	m_sCallID.clear();
	m_xFromIP.Invalidate();
	m_xToIP.Invalidate();
	m_uCSeq = 0;
	m_bExternal = false;
	m_bOutgoing = false;
	m_sUserFrom.clear();
	m_sUserTo.clear();
}


CSIPCallParam &CSIPCallParam::operator=(const CSIPCallParam &c)
{
	m_xSessionsID.clear();
	m_xSessionsID = c.m_xSessionsID;
	m_sCallID	= c.m_sCallID;
	m_xFromIP	= c.m_xFromIP;
	m_xToIP		= c.m_xToIP;
	m_uCSeq		= c.m_uCSeq;
	m_bExternal = c.m_bExternal;
	m_bOutgoing = c.m_bOutgoing;
	m_sUserFrom = c.m_sUserFrom;
	m_sUserTo	= c.m_sUserTo;
	return *this;
}


const std::string &CSIPCallParam::GetUserFrom() const
{
	return m_sUserFrom;
}

void CSIPCallParam::SetUserFrom(const std::string &user)
{
	m_sUserFrom = user;
}

const std::string &CSIPCallParam::GetUserTo() const
{
	return m_sUserTo;
}

void CSIPCallParam::SetUserTo(const std::string &user)
{
	m_sUserTo = user;
}


bool CSIPCallParam::GetOutgoing() const
{
	return m_bOutgoing;
}

void CSIPCallParam::SetOutgoing(bool bOut)
{
	m_bOutgoing = bOut;
}

bool CSIPCallParam::GetExternal() const
{
	return m_bExternal;
}

void CSIPCallParam::SetExternal(bool bExt)
{
	m_bExternal = bExt;
}

const CIPv4Address &CSIPCallParam::GetFrom() const 
{
	return m_xFromIP;
}

void CSIPCallParam::SetFrom(const CIPv4Address &ip)
{
	m_xFromIP = ip;
}

const CIPv4Address &CSIPCallParam::GetTo() const
{
	return m_xToIP;
}

void CSIPCallParam::SetTo(const CIPv4Address &ip)
{
	m_xToIP = ip;
}

const std::string &CSIPCallParam::GetCallID() const
{
	return m_sCallID;
}

void CSIPCallParam::SetCallID(const std::string &call_id)
{
	m_sCallID = call_id;
}

const std::vector<uint32> &CSIPCallParam::GetSessionsIDs() const
{
	return m_xSessionsID;
}

void CSIPCallParam::SetSessionsIDs(const std::vector<uint32> &ids)
{
	m_xSessionsID.clear();
	m_xSessionsID = ids;
}

void CSIPCallParam::AddNewSessionsID(uint32 sess_id)
{
	m_xSessionsID.push_back(sess_id);
}

uint32 CSIPCallParam::GetCSeq() const
{
	return m_uCSeq;
}

void CSIPCallParam::SetCSeq(uint32 cseq)
{
	m_uCSeq = cseq;
}


#ifdef _DEBUG_LOG
void CSIPCallParam::Dump() const
{
	_LOG("Call-ID: " << m_sCallID);
	_LOG("CSeq: " << IntToString(m_uCSeq));
	_LOG("FromIP: " << m_xFromIP.AsString());
	_LOG("ToIP: " << m_xToIP.AsString());
	_LOG("Outgoing: " << m_bOutgoing ? "TRUE" : "FALSE");
	_LOG("External: " << m_bExternal ? "TRUE" : "FALSE");

	std::string sOut;
	for (unsigned i = 0; i < m_xSessionsID.size(); i++)
	{
		if (!sOut.empty())
			sOut += ", ";
		sOut += IntToHex(m_xSessionsID[i], 8);
	}
	_LOG("Sessions: " << sOut);
}
#endif//_DEBUG_LOG
