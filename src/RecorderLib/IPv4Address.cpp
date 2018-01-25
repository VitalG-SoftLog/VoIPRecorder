// File IPv4Address.cpp
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <winsock2.h>
#include <string>
#include "IPv4Address.h"

#pragma comment(lib, "ws2_32.lib")

static const uint32	bad_ip		= 0xFFFFFFFF;	//  255.255.255.255
static const uint32 bad_port	= 0xFFFF;		//  255.255.255.255

CIPv4Address::CIPv4Address()
{
	Invalidate();
}

CIPv4Address::CIPv4Address(const CIPv4Address &a)
{
	Invalidate();
	IP(a.IP());
	Port(a.Port());
}

CIPv4Address::CIPv4Address(const char *a)
{
	Invalidate();
	IP(a);
}

CIPv4Address::CIPv4Address(const std::string &s)
{
	Invalidate();
	IP(s.c_str());
}


CIPv4Address::~CIPv4Address()
{
	Invalidate();
}

CIPv4Address &CIPv4Address::operator=(const CIPv4Address &a) 
{
	return (Assign(a));
}

CIPv4Address &CIPv4Address::Assign(const CIPv4Address &a) 
{
	IP(a.IP());
	Port(a.Port());
	return *this;
}

bool CIPv4Address::operator==(const CIPv4Address &a) const 
{
	return (m_uIP == m_uIP) && (m_uPort == m_uPort);
}

void CIPv4Address::Invalidate() 
{
	m_uIP = bad_ip;
	m_uPort = bad_port;
}

void CIPv4Address::Port(const char *port) 
{
	m_uPort = htons( (unsigned short)strtoul(port, 0, 0) );
}

void CIPv4Address::IP(const char *ip1) 
{
	Invalidate();
	char cip[1024]={0};
	::memset(cip, 0, sizeof(cip));

	unsigned i = 0;
	for (i=0;ip1[i] != ':' && ip1[i] != '\0'; i++)
		cip[i] = ip1[i]; 

	cip[i] = 0;

	m_uIP = ::inet_addr( cip );
	if (ip1[i] == ':') { Port( ip1 + i + 1 ); }
}


void CIPv4Address::IP(uint32 ip)	{m_uIP = ip;}
void CIPv4Address::Port(uint16 port)	{m_uPort = port;}

uint32 CIPv4Address::IP() const			{ return m_uIP;}
uint16 CIPv4Address::Port() const		{ return m_uPort;}

bool CIPv4Address::IsIP() const { return m_uIP!=bad_ip; }
bool CIPv4Address::IsPort() const { return m_uPort!=bad_port; }

std::string CIPv4Address::AsString() const
{
	char str[32];

	uint32 ip = m_uIP;
	uint32 port = ntohs( m_uPort );

	if (IsPort())
	{
		_snprintf_s(str, sizeof(str), sizeof(str) - 1, "%d.%d.%d.%d:%d", 
			((uint8 *)&ip)[0], ((uint8 *)&ip)[1],
			((uint8 *)&ip)[2], ((uint8 *)&ip)[3],
			port);
	}
	else
	{
		_snprintf_s(str, sizeof(str), sizeof(str) - 1, "%d.%d.%d.%d", 
			((uint8 *)&ip)[0], ((uint8 *)&ip)[1],
			((uint8 *)&ip)[2], ((uint8 *)&ip)[3]);
	}

	return std::string(str);
}
