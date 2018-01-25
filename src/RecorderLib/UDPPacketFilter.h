// Header: UDPPacketFilter.h
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "Interfaces.h"
#include "SimplyThread.h"
#include "IPv4Address.h"
#include <string>
#include <list>
#include <vector>

// Define: _UDP_FILTER
// filter string for WinPcap
#define _UDP_FILTER "ip and udp"

//////////////////////////////////////////////////////////////////////////
// Typedefs for pcap interface functions
//////////////////////////////////////////////////////////////////////////
typedef pcap_t*(__cdecl *func_pcap_open_live)(const char *, int, int, int, char *);
typedef int(__cdecl *func_pcap_datalink)(pcap_t *);
typedef void(__cdecl *func_pcap_close)(pcap_t *);
typedef int(__cdecl *func_pcap_compile)(pcap_t *, struct bpf_program *, const char *, int, bpf_u_int32);
typedef int(__cdecl *func_pcap_setfilter)(pcap_t *, struct bpf_program *);
typedef void(__cdecl *func_pcap_breakloop)(pcap_t *);
typedef int(__cdecl *func_pcap_findalldevs)(pcap_if_t **, char *);
typedef void(__cdecl *func_pcap_freealldevs)(pcap_if_t *);
typedef int(__cdecl *func_pcap_loop)(pcap_t *, int, pcap_handler, u_char *);
typedef pcap_dumper_t*(__cdecl *func_pcap_dump_open)(pcap_t *, const char *);
typedef void(__cdecl *func_pcap_dump_close)(pcap_dumper_t *);
typedef void(__cdecl *func_pcap_dump)(u_char *, const struct pcap_pkthdr *, const u_char *);
typedef pcap_t*(__cdecl *func_pcap_open_offline)(const char *, char *);



//////////////////////////////////////////////////////////////////////////
// Class: CUDPPacketFilter
// An encapsulation of the WinPcap library
//////////////////////////////////////////////////////////////////////////
class CUDPPacketFilter 
	: public CSimplyThread
{
public:
	
	// Constructor: CUDPPacketFilter()
	// Default constructor
	CUDPPacketFilter();

	// Destructor: ~CUDPPacketFilter()
	// Default destructor	
	virtual ~CUDPPacketFilter();

	// Method: Initialize
	// Loading winpcap.dll and initialize dll exported functions
	//
	// Returns:
	// true if dll successfully loaded.
	bool Initialize();

	//
	void SetPBXIP(const CIPv4Address &addr) {m_xPbxIP = addr;}


	// Method: StartCapturing
	//	Start capture UDP packets from specified interface
	//
	// Parameters:
	//	sInterfaceName - network interface name
	//
	// Returns:
	//	true if capturing started and false if error occurred	
	bool StartCapturing(const std::string &sInterfaceName);
	
	// Method: StartCapturingOffline
	//	Start capture UDP packets from specified .pcap file
	//
	// Parameters:
	//	sFileName - full name of .pcap file
	//
	// Returns:
	//	true if capturing started and false if error occurred
	bool StartCapturingOffline(const std::string &sFileName);

	// Method: StopCapturing
	//	Stop capturing UDP packets
	void StopCapturing();
	
	// Method: GetInterfaceList
	//	Return installed network interfaces names
	//
	// Returns:
	//	vector of strings that contain names of network interfaces
	std::vector<std::string> GetInterfaceList(bool only_desc=false);
	
	// Method: RegisterListener
	//	Register new listener of UDP packets
	//
	// Parameters:
	//	pListener - pointer to <IUDPPacketListener> interface
	void RegisterListener(IUDPPacketListener *pListener);

	// Method: UnRegisterListener
	//	UnRegister previous registered listener of UDP packets
	//
	// Parameters:
	//	pListener - pointer to <IUDPPacketListener> interface
	void UnRegisterListener(IUDPPacketListener *pListener);


	void SaveMarkedPacketsToDump(const std::string &sDumpFile);


protected:

	// pcap API functions pointers
	func_pcap_open_live		m_pcap_open_live;
	func_pcap_datalink		m_pcap_datalink;
	func_pcap_close			m_pcap_close;
	func_pcap_compile		m_pcap_compile;
	func_pcap_setfilter		m_pcap_setfilter;
	func_pcap_breakloop		m_pcap_breakloop;
	func_pcap_findalldevs	m_pcap_findalldevs;
	func_pcap_freealldevs	m_pcap_freealldevs;
	func_pcap_loop			m_pcap_loop;
	func_pcap_dump_open		m_pcap_dump_open;
	func_pcap_dump_close	m_pcap_dump_close;
	func_pcap_dump			m_pcap_dump;
	func_pcap_open_offline	m_pcap_open_offline;

	// winpcap.dll handle
	HMODULE					m_hDll;

	static void PcapHandler(u_char *user, const struct pcap_pkthdr *pkt_header, const u_char *pkt_data);
	void OnPcapPacket(CUDPPacket *pPacket);
	virtual void Run();
	std::list<IUDPPacketListener *> m_xListeners;

	pcap_t			*m_hPcapHandle;
	pcap_dumper_t	*m_hPcapDumper;
	std::string		m_sDumpFile;
	CIPv4Address	m_xPbxIP;
};
