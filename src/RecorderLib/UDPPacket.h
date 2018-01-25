#pragma once

#include "BaseTypes.h"
#include "IPv4Address.h"

#define _UDP_NOALLOC 1

#pragma pack(push)
#pragma pack(8)

/* 4 bytes IP address */
typedef struct ip_address
{
	uint8 byte1;
	uint8 byte2;
	uint8 byte3;
	uint8 byte4;
} ip_address;

/* IPv4 header */
typedef struct ip_header
{
	uint8		ver_ihl;		// Version (4 bits) + Internet header length (4 bits)
	uint8		tos;			// Type of service 
	uint16		tlen;			// Total length 
	uint16		identification; // Identification
	uint16		flags_fo;		// Flags (3 bits) + Fragment offset (13 bits)
	uint8		ttl;			// Time to live
	uint8		proto;			// Protocol
	uint16		crc;			// Header checksum
	ip_address		saddr;			// Source address
	ip_address		daddr;			// Destination address
	int32		op_pad;			// Option + Padding
}ip_header;

/* UDP header*/
typedef struct udp_header
{
	uint16 sport;			// Source port
	uint16 dport;			// Destination port
	uint16 len;			// Datagram length
	uint16 crc;			// Checksum
}udp_header;

#pragma pack(pop)

struct CUDPPacket
{
public:
	// default constructor
	CUDPPacket(void);
	// copy constructor
	CUDPPacket(const CUDPPacket &p);
	// construct object from pcap packet
	CUDPPacket(const struct pcap_pkthdr *header, const u_char *pkt_data);
	// default destructor
	virtual ~CUDPPacket(void);
	// assign operator
	const CUDPPacket& operator=(const CUDPPacket &p);
	
	// const reference to CIPv4Address source 
	const CIPv4Address		&Src() const;

	// const reference to CIPv4Address destination 
	const CIPv4Address		&Dst() const;

	// raw IP header
	const ip_header			*IPHeader() const;

	// raw UDP header
	const udp_header		*UDPHeader() const;

	// pointer to packet data
	const char				*Data() const;

	// size of packet data
	const int				DataLen() const;
	void					SaveToDump(bool bSaveToDump);
	bool					SaveToDump() const;

	const u_char			*RawPacket() const;
	int						RawPacketSize() const;

protected:
	void Init();
	void Free();

	ip_header			*m_pIPHeader;
	udp_header			*m_pUDPHeader;
	u_char				*m_pData;
	const u_char		*m_pRawData;
	int					m_iRawDataSize;
	u_short				m_nDataLen;
	CIPv4Address		m_ipSrc;
	CIPv4Address		m_ipDst;
	bool				m_bSaveToDump;
};
