#include "StdAfx.h"
#include "RtpSessionParam.h"
#include "Interfaces.h"
#include "StringUtils.h"

using namespace StringUtils;

_MODULE_NAME(CRTPSessionParam);

CRTPSessionParam::CRTPSessionParam()
{
	m_uSSRC = 0;
	m_uPayloadType = _RTP_PAYLOAD_TYPE_PCMU;
	m_uSamplesRate = 8000;
	m_xSrc.Invalidate();
	m_xDst.Invalidate();
	m_pUserData = NULL;
}

CRTPSessionParam::CRTPSessionParam(const CRTPSessionParam &s)
{
	operator=(s);
}

CRTPSessionParam::~CRTPSessionParam()
{
	m_uSSRC = 0;
	m_uPayloadType = 0;
	m_uSamplesRate = 0;
	m_xSrc.Invalidate();
	m_xDst.Invalidate();
}

bool CRTPSessionParam::operator==(const CRTPSessionParam &s) const
{
	return ((m_uSSRC == s.m_uSSRC) && (m_xSrc == s.m_xSrc) && 
		(m_xDst == s.m_xDst) && (m_uPayloadType == s.m_uPayloadType) &&
		(m_uSamplesRate == s.m_uSamplesRate));
}


CRTPSessionParam &CRTPSessionParam::operator=(const CRTPSessionParam &s)
{
	m_uSSRC = s.m_uSSRC;
	m_xSrc = s.m_xSrc;
	m_xDst = s.m_xDst;
	m_uPayloadType = s.m_uPayloadType;
	m_uSamplesRate = s.m_uSamplesRate;
	m_pUserData = s.m_pUserData;
	return *this;
}

const CIPv4Address &CRTPSessionParam::GetSrc() const
{
	return m_xSrc;
}

void CRTPSessionParam::SetSrc(const CIPv4Address &ip)
{
	m_xSrc = ip;
}

const CIPv4Address &CRTPSessionParam::GetDst() const
{
	return m_xDst;
}

void CRTPSessionParam::SetDst(const CIPv4Address &ip)
{
	m_xDst = ip;
}

unsigned int CRTPSessionParam::GetSSRC() const
{
	return m_uSSRC;
}

void CRTPSessionParam::SetSSRC(unsigned int ssrc)
{
	m_uSSRC = ssrc;
}

unsigned int CRTPSessionParam::GetPayloadType() const
{
	return m_uPayloadType;
}

void CRTPSessionParam::SetPayloadType(unsigned int pt)
{
	m_uPayloadType = pt;

	switch(m_uPayloadType)
	{
	case(_RTP_PAYLOAD_TYPE_PCMA):
	case(_RTP_PAYLOAD_TYPE_PCMU):
		SetSamplesRate(8000);
		break;
	case(_RTP_PAYLOAD_TYPE_G722):
		SetSamplesRate(16000);
		break;
	default:
		break;
	}
}

unsigned int CRTPSessionParam::GetSamplesRate() const
{
	return m_uSamplesRate;
}

void CRTPSessionParam::SetSamplesRate(unsigned int rate)
{
	m_uSamplesRate = rate;
}

void *CRTPSessionParam::GetUserData() const
{
	return m_pUserData;
}

void CRTPSessionParam::SetUserData(void *pdata)
{
	m_pUserData = pdata;
}


#ifdef _DEBUG_LOG
void CRTPSessionParam::Dump() const
{
	_LOG("SSRC: 0x" << IntToHex(m_uSSRC, 8));
	_LOG("PT: 0x" << IntToHex(m_uPayloadType, 2));
	_LOG("Rate: " << IntToString(m_uSamplesRate));
	_LOG("SrcIP: " << m_xSrc.AsString());
	_LOG("DstIP: " << m_xDst.AsString());
}
#endif
