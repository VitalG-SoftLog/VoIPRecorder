#include "StdAfx.h"
#include "UDPPacket.h"
#include <iostream>

CUDPPacket::CUDPPacket(void)
{
	Init();
}

CUDPPacket::CUDPPacket(const CUDPPacket &p)
{
	Init();
	operator=(p);
}

CUDPPacket::CUDPPacket(const struct pcap_pkthdr *header, const u_char *pkt_data)
{

	ip_header *ih	= NULL;
	udp_header *uh	= NULL;
	u_int ip_len	= 0;

	Init();

	m_pRawData = pkt_data;

	/* retireve the position of the ip header */
	ih = (ip_header *) (pkt_data + 14); //length of ethernet header

	/* retireve the position of the udp header */
	ip_len = (ih->ver_ihl & 0xf) * 4;
	uh = (udp_header *) ((u_char*)ih + ip_len);

#ifdef _UDP_NOALLOC
	m_pIPHeader		= ih;
	m_pUDPHeader	= uh;
#else
	m_pIPHeader = new ip_header;
	memcpy(m_pIPHeader, ih, sizeof(ip_header));
	m_pUDPHeader = new udp_header;
	memcpy(m_pUDPHeader, uh, sizeof(udp_header));
#endif//_UDP_NOALLOC

	m_nDataLen = ntohs(uh->len);
	m_nDataLen -= sizeof(udp_header);

#ifdef _UDP_NOALLOC
	m_pData = (u_char*)uh + sizeof(udp_header);
#else//_UDP_NOALLOC
	m_pData = new u_char[m_nDataLen];
	memcpy(m_pData, (u_char*)uh + sizeof(udp_header), m_nDataLen);
#endif//_UDP_NOALLOC

	m_iRawDataSize = 14; // ethernet frame
	m_iRawDataSize += ip_len; // ip header
	m_iRawDataSize += m_pUDPHeader->len; // udp len

	m_ipSrc.IP(*(uint32 *)&m_pIPHeader->saddr);
	m_ipDst.IP(*(uint32 *)&m_pIPHeader->daddr);
	m_ipSrc.Port(m_pUDPHeader->sport);
	m_ipDst.Port(m_pUDPHeader->dport);
}


CUDPPacket::~CUDPPacket(void)
{
	Free();
}

void CUDPPacket::Init()
{
	m_pIPHeader		= NULL;
	m_pUDPHeader	= NULL;
	m_pData			= NULL;
	m_nDataLen		= 0;
	m_bSaveToDump	= false;
	m_pRawData		= NULL;
	m_iRawDataSize	= 0;
}

void CUDPPacket::SaveToDump(bool bSaveToDump)
{
	m_bSaveToDump = bSaveToDump;
}

bool CUDPPacket::SaveToDump() const
{
	return m_bSaveToDump;
}

const CIPv4Address	&CUDPPacket::Src() const
{
	return m_ipSrc;
}

const CIPv4Address &CUDPPacket::Dst() const
{
	return m_ipDst;
}

void CUDPPacket::Free()
{
#ifdef _UDP_NOALLOC
	m_pIPHeader		= NULL;
	m_pUDPHeader	= NULL;
	m_pData			= NULL;
#else//_UDP_NOALLOC
	if (m_pIPHeader != NULL)
	{
		delete m_pIPHeader;
		m_pIPHeader		= NULL;
	}

	if (m_pUDPHeader != NULL)
	{
		delete m_pUDPHeader;
		m_pUDPHeader	= NULL;
	}

	if (m_pData != NULL)
	{
		delete[] m_pData;
		m_pData	= NULL;
	}
#endif//_UDP_NOALLOC
	m_nDataLen		= 0;
	m_pRawData = NULL;
	m_iRawDataSize = 0;
}

const CUDPPacket &CUDPPacket::operator=(const CUDPPacket &p)
{
	Free();

	if (p.IPHeader())
	{
#ifdef _UDP_NOALLOC
		m_pIPHeader = p.m_pIPHeader;
#else//_UDP_NOALLOC
		m_pIPHeader = new ip_header;
		memcpy(m_pIPHeader, p.m_pIPHeader, sizeof(ip_header));
#endif//_UDP_NOALLOC
	}

	if (p.UDPHeader())
	{
#ifdef _UDP_NOALLOC
		m_pUDPHeader = p.m_pUDPHeader;
#else//_UDP_NOALLOC
		m_pUDPHeader = new udp_header;
		memcpy(m_pUDPHeader, p.m_pUDPHeader, sizeof(udp_header));
#endif//_UDP_NOALLOC
	}

	if (p.Data() && p.DataLen())
	{
#ifdef _UDP_NOALLOC
		m_pData = p.m_pData;
#else//_UDP_NOALLOC
		m_pData = new u_char[p.m_nDataLen];
		memcpy(m_pData, p.m_pData, p.m_nDataLen);
#endif//_UDP_NOALLOC
	}
	
	m_ipSrc			= p.m_ipSrc;
	m_ipDst			= p.m_ipDst;
	m_pRawData		= p.m_pRawData;
	m_iRawDataSize	= p.m_iRawDataSize;

	return *this;
}

const ip_header	 *CUDPPacket::IPHeader() const
{
	return m_pIPHeader;
}

const udp_header *CUDPPacket::UDPHeader() const
{
	return m_pUDPHeader;
}

const char *CUDPPacket::Data() const
{
	return (const char *)m_pData;
}

const int CUDPPacket::DataLen() const
{
	return m_nDataLen;
}

const u_char *CUDPPacket::RawPacket() const
{
	return m_pRawData;
}

int	CUDPPacket::RawPacketSize() const
{
	return m_iRawDataSize;
}


