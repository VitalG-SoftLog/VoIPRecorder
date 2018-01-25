#ifndef  _MVI_TRANSPORT_H_
#define  _MVI_TRANSPORT_H_

#include "os_include.h"

#pragma pack(push)
#pragma pack(8)

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32;


typedef struct eth_header {
    uint8	dst_mac[6];
    uint8	src_mac[6];
    uint16	protocol;
} eth_header_t;

/* IPv4 header */
typedef struct ip_header {
	uint8		ver_ihl;		// Version (4 bits) + Internet header length (4 bits)
	uint8		tos;			// Type of service 
	uint16		tlen;			// Total length 
	uint16		identification; // Identification
	uint16		flags_fo;		// Flags (3 bits) + Fragment offset (13 bits)
	uint8		ttl;			// Time to live
	uint8		proto;			// Protocol
	uint16		crc;			// Header checksum
	uint32		src_addr;		// Source address
	uint32		dst_addr;		// Destination address
	int32		op_pad;			// Option + Padding
} ip_header_t;

/* UDP header*/
typedef struct udp_header {
	uint16 src_port;		// Source port
	uint16 dst_port;		// Destination port
	uint16 len;				// Datagram length
	uint16 crc;				// Checksum
} udp_header_t;

#pragma pack(pop)

typedef struct mvi_transport {
	eth_header_t*	peth_header;
	ip_header_t*	pip_header;
	udp_header_t*	pudp_header;
} mvi_transport_t;


#endif /* _MVI_TRANSPORT_ */ 
