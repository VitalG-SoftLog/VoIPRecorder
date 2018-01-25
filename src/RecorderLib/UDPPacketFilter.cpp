#include "StdAfx.h"
#include "UDPPacketFilter.h"
#include <algorithm>
#include "StringUtils.h"

using namespace StringUtils;


#define _load_pcap_func(X) \
	m_##X = (func_##X)::GetProcAddress(m_hDll, #X); \
	if (m_##X == NULL) \
		return false;

CUDPPacketFilter::CUDPPacketFilter(void)
{
	m_hDll = NULL;
}

CUDPPacketFilter::~CUDPPacketFilter(void)
{
	if (m_hDll != NULL)
	{
		//::FreeLibrary(m_hDll);
		m_hDll = NULL;
	}
}



bool CUDPPacketFilter::Initialize()
{
	m_hDll = ::LoadLibrary(_T("wpcap"));

	if (m_hDll == NULL)
	{
		DWORD dwError = GetLastError();
		return false;
	}

	_load_pcap_func(pcap_open_live);
	_load_pcap_func(pcap_datalink);
	_load_pcap_func(pcap_close);
	_load_pcap_func(pcap_compile);
	_load_pcap_func(pcap_setfilter);
	_load_pcap_func(pcap_breakloop);
	_load_pcap_func(pcap_findalldevs);
	_load_pcap_func(pcap_freealldevs);
	_load_pcap_func(pcap_loop);
	_load_pcap_func(pcap_dump_open);
	_load_pcap_func(pcap_dump_close);
	_load_pcap_func(pcap_dump);
	_load_pcap_func(pcap_open_offline);


	return true;
}

void CUDPPacketFilter::SaveMarkedPacketsToDump(const std::string &sDumpFile)
{
	m_sDumpFile = sDumpFile;
}

bool CUDPPacketFilter::StartCapturing(const std::string &sInterfaceName)
{
	char errbuf[PCAP_ERRBUF_SIZE] = {0};

	m_hPcapHandle = NULL;
	m_hPcapDumper = NULL;

	// Open the adapter
	if ((m_hPcapHandle = m_pcap_open_live(sInterfaceName.c_str(),// name of the device
							 65536,			// portion of the packet to capture. 
											// 65536 grants that the whole packet will be captured on all the MACs.
							 1,				// promiscuous mode (nonzero means promiscuous)
							 1000,			// read timeout
							 errbuf			// error buffer
							 )) == NULL)
	{
		// Unable to open the adapter. %s is not supported by WinPcap
		return false;
	}
	

	if (!m_sDumpFile.empty())
	{
		m_hPcapDumper = m_pcap_dump_open(m_hPcapHandle, m_sDumpFile.c_str());
	}



	// Check the link layer. We support only Ethernet for simplicity.
	if(m_pcap_datalink(m_hPcapHandle) != DLT_EN10MB)
	{
		// This program works only on Ethernet networks.
		m_pcap_close(m_hPcapHandle);
		return false;
	}
	
	// If the interface is without addresses we suppose to be in a C class network
	u_int netmask=0xffffff; 

	struct bpf_program fcode = {0};

	// compile the filter
	if (m_pcap_compile(m_hPcapHandle, &fcode, _UDP_FILTER, 1, netmask) <0 )
	{
		// Unable to compile the packet filter. Check the syntax.
		m_pcap_close(m_hPcapHandle);
		return false;
	}
	
	// set the filter
	if (m_pcap_setfilter(m_hPcapHandle, &fcode)<0)
	{
		// Error setting the filter.
		m_pcap_close(m_hPcapHandle);
		return false;
	}
	
	Start();
	WaitForStart();
	
	return true;
}


bool CUDPPacketFilter::StartCapturingOffline(const std::string &sFileName)
{
	char errbuf[PCAP_ERRBUF_SIZE] = {0};

	m_hPcapDumper = NULL;
	m_hPcapHandle = NULL;

	// Open the adapter
	if ((m_hPcapHandle = m_pcap_open_offline(sFileName.c_str(),errbuf)) == NULL)
	{
		// Unable to open the file. %s is not supported by WinPcap
		return false;
	}

	// Check the link layer. We support only Ethernet for simplicity.
	if(m_pcap_datalink(m_hPcapHandle) != DLT_EN10MB)
	{
		// This program works only on Ethernet networks.
		m_pcap_close(m_hPcapHandle);
		return false;
	}

	// If the interface is without addresses we suppose to be in a C class network
	u_int netmask=0xffffff; 

	struct bpf_program fcode = {0};

	// compile the filter
	if (m_pcap_compile(m_hPcapHandle, &fcode, _UDP_FILTER, 1, netmask) <0 )
	{
		// Unable to compile the packet filter. Check the syntax.
		m_pcap_close(m_hPcapHandle);
		return false;
	}

	// set the filter
	if (m_pcap_setfilter(m_hPcapHandle, &fcode)<0)
	{
		// Error setting the filter.
		m_pcap_close(m_hPcapHandle);
		return false;
	}

	Start();
	WaitForStart();

	return true;
}


void CUDPPacketFilter::StopCapturing()
{
	if (m_hPcapHandle && IsRunning())
	{
		m_pcap_breakloop(m_hPcapHandle);
	}
}


std::vector<std::string> CUDPPacketFilter::GetInterfaceList(bool only_desc/*=false*/)
{
	std::vector<std::string> vReturn;

	pcap_if_t *alldevs	= NULL;
	pcap_if_t *d		= NULL;

	char errbuf[PCAP_ERRBUF_SIZE];
	
	// Retrieve the device list
	if(m_pcap_findalldevs(&alldevs, errbuf) == -1)
	{
		return vReturn;
	}
	
	// Fill the list
	if (only_desc)
	{
		for(d=alldevs; d; d=d->next)
		{
			std::string dev_name;

			if (d->description != NULL)
			{
				dev_name = Trim(d->description);
				if (dev_name.empty())
					dev_name = d->name;
			}
			else 
			{
				dev_name = d->name;
			}
			vReturn.push_back(dev_name);
		}
	}
	else
	{
		for(d=alldevs; d; d=d->next)
			vReturn.push_back(d->name);
	}

	m_pcap_freealldevs(alldevs);

	return vReturn;
}

void CUDPPacketFilter::RegisterListener(IUDPPacketListener *pListener)
{
	if (pListener != NULL)
	{
		std::list<IUDPPacketListener*>::iterator it = 
			std::find(m_xListeners.begin(), m_xListeners.end(), pListener);

		if (it == m_xListeners.end())
		{
			m_xListeners.push_back(pListener);
		}
	}
}

void CUDPPacketFilter::UnRegisterListener(IUDPPacketListener *pListener)
{
	if (pListener != NULL)
	{
		std::list<IUDPPacketListener*>::iterator it = 
			std::find(m_xListeners.begin(), m_xListeners.end(), pListener);

		if (it != m_xListeners.end())
		{
			m_xListeners.erase(it);
		}
	}
}

void CUDPPacketFilter::Run()
{
	m_pcap_loop(m_hPcapHandle, 0, PcapHandler, (u_char*)this);

	if (m_hPcapDumper != NULL)
	{
		m_pcap_dump_close(m_hPcapDumper);
		m_hPcapDumper = NULL;
	}
	
	m_pcap_close(m_hPcapHandle);
	m_hPcapHandle = NULL;
}

void CUDPPacketFilter::PcapHandler(u_char *user, const struct pcap_pkthdr *pkt_header, const u_char *pkt_data)
{
	if (user)
	{
		CUDPPacketFilter *pThis = (CUDPPacketFilter *)user;
		CUDPPacket packet(pkt_header, pkt_data);
		
		// write to dump 
		if (pThis->m_hPcapDumper)
		{
			if (packet.Dst().IP() == pThis->m_xPbxIP.IP() || 
				packet.Src().IP() == pThis->m_xPbxIP.IP())
			{
				if (packet.Dst().Port() == pThis->m_xPbxIP.Port() || 
					packet.Src().Port() == pThis->m_xPbxIP.Port())
				//if (packet.SaveToDump())
				{
					pThis->m_pcap_dump((u_char *)pThis->m_hPcapDumper, pkt_header, pkt_data);
				}
			}
		}

		pThis->OnPcapPacket(&packet);
	}
}

void CUDPPacketFilter::OnPcapPacket(CUDPPacket *pPacket)
{
	if (pPacket == NULL)
		return;

	// notificate all listeners
	std::list<IUDPPacketListener*>::iterator it = m_xListeners.begin();
	for (;it != m_xListeners.end(); it++)
	{
		if (*it != NULL)
		{
			if ((*it)->OnUDPPacket(pPacket))
			{
				pPacket->SaveToDump(true);
			}
		}
	}
}
