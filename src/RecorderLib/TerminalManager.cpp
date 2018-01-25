#include "StdAfx.h"
#include "TerminalManager.h"
#include "Terminal.h"
#include "StringUtils.h"

using namespace StringUtils;

_MODULE_NAME(CTerminalManager);

CTerminalManager::CTerminalManager(CMessageQueue *pMessageQueue, const CIPv4Address &pbx)
{
	m_pMessageQueue = pMessageQueue;
	m_pCallManager = new CSIPManager(pbx.IP());
	//m_pCallManager = new CSIPCallManager;
	m_pCallManager->SetCallEventsListener(this);
	m_pRTPRecorder = new CRTPRecorder;
	m_pRTPRecorder->Initialize();
	m_xPbxIP = pbx;
}

CTerminalManager::~CTerminalManager()
{
	for (unsigned i = 0; i < m_xDevices.size(); i++)
		m_xDevices[i]->Release();

	m_pRTPRecorder->UnInitialize();

	delete m_pCallManager;
	delete m_pRTPRecorder;
}

/*
int CTerminalManager::GetDeviceID(const CSIPPacket *packet)
{
	// extract extension number from packet
	int iNumber = StringToInt(packet->FromUser());

	if (iNumber != 0)
	{
		std::map<uint32, int>::iterator it = m_xIP2ID.find(packet->DstIP().IP());
		if (it == m_xIP2ID.end())
		{
			m_xIP2ID[packet->DstIP().IP()] = iNumber;
			return iNumber;
		}
		else
		{
			return it->second;
		}
	}
	return 0;
}
*/


void CTerminalManager::DeleteTerminal(uint32 dst_ip, int terminal_id)
{
	scope_locker ll(m_cs);

	_LOG("Delete device: " << IntToString(terminal_id));

	CTerminal *d = NULL;

	std::map<int, CTerminal*>::iterator it = m_xTerminalsByID.find(terminal_id);
	if (it != m_xTerminalsByID.end())
	{// found device!
		d = it->second;
		m_xTerminalsByID.erase(it);
	}

	if (d != NULL)
	{
		std::map<uint32, CTerminal*>::iterator itip = m_xTerminalsByIP.find(dst_ip);
		if (itip != m_xTerminalsByIP.end())
		{// found device!
			m_xTerminalsByIP.erase(itip);
		}
	}
/*
	if (d != NULL)
	{
		std::map<uint32, int> itid = m_xIP2ID.find(dst_ip);
		if (itid != m_xIP2ID.end())
		{// found device!
			m_xIP2ID.erase(itid);
		}
	}
*/
	if (d != NULL)
	{
		std::vector<CTerminal*>::iterator itv = std::find(m_xDevices.begin(), m_xDevices.end(), d);
		if (itv != m_xDevices.end())
		{// found device!
			m_xDevices.erase(itv);
		}
	}
	
	if (d != NULL)
		d->Release();
}


int CTerminalManager::GetDeviceID(uint32 dst_ip, std::string from_user)
{
	// extract extension number from packet
	int iNumber = StringToInt(from_user);
/*
	if (iNumber != 0)
	{
		std::map<uint32, int>::iterator it = m_xIP2ID.find(dst_ip);
		if (it == m_xIP2ID.end())
		{
			m_xIP2ID[dst_ip] = iNumber;
			return iNumber;
		}
		else
		{
			return it->second;
		}
	}
	return 0;
*/
	return iNumber;
}

/*
void CTerminalManager::CreateTerminal(const CSIPPacket *packet)
{
	if (packet->DstIP().IP() == m_xPbxIP.IP())
		return;

	// create a new registered device
	CTerminal *d = NULL;

	m_cs.enter(); // enter CS

	int iID = GetDeviceID(packet);

	if (iID != 0)
	{
		d = new CTerminal(iID, packet->DstIP().IP(), 
								m_pCallManager, 
								m_pRTPRecorder, 
								m_pMessageQueue,
								true);
		d->AddRef();

		_LOG("Created new registered device: " << IntToString(d->ID()));
		_LOG("Device IP: " << packet->DstIP().AsString());

		m_xTerminalsByID[d->ID()] = d;
		m_xTerminalsByIP[packet->DstIP().IP()] = d;
		m_xDevices.push_back(d);
	}

	m_cs.leave(); // leave CS

	if (d != NULL)
	{
		if (m_pMessageQueue)
			m_pMessageQueue->Push(d->ID(), MP_STATUS, d->GetStatus());
	}
}
*/

void CTerminalManager::CreateTerminal(uint32 dst_ip, std::string from_user, int type)
{
	if (dst_ip == m_xPbxIP.IP())
		return;

	// create a new registered device
	CTerminal *d = NULL;

	m_cs.enter(); // enter CS

	int iID = GetDeviceID(dst_ip, from_user);

	if (iID != 0)
	{
		// check if device already created...
		std::map<int, CTerminal*>::iterator itid = m_xTerminalsByID.find(iID);

		if (itid != m_xTerminalsByID.end())
			return;// already registered



		d = new CTerminal(iID, dst_ip, 
			m_pCallManager, m_pRTPRecorder, 
//			m_pMessageQueue,type);
			m_pMessageQueue, type); // BUGBUG: SIPManager not return valid type on onNotifyRegister
		d->AddRef();

#ifdef _DEBUG_LOG
		CIPv4Address dst; dst.IP(dst_ip);
		_LOG("Created new registered device: " << IntToString(d->ID()));
		_LOG("Device IP: " << dst.AsString());
#endif//_DEBUG_LOG

		m_xTerminalsByID[d->ID()] = d;
		m_xTerminalsByIP[dst_ip] = d;
		m_xDevices.push_back(d);
	}

	m_cs.leave(); // leave CS

	if (d != NULL)
	{
		if (m_pMessageQueue)
			m_pMessageQueue->Push(d->ID(), MP_STATUS, d->GetStatus());
	}
}

bool CTerminalManager::OnUDPPacket(const CUDPPacket *packet)
{
	if (packet)
	{
		uint32 pbx_ip		= m_xPbxIP.IP();
		uint16 sip_port		= m_xPbxIP.Port();

		bool is_sip_packet	= false;

		if ((packet->Dst().IP() == pbx_ip || packet->Src().IP() == pbx_ip) &&
			(packet->Dst().Port() == sip_port || packet->Src().Port() == sip_port))
		{
			CSIPPacket sip_packet(*packet);
			if (sip_packet.IsValid())
			{
				is_sip_packet = true;
				std::map<uint32, CTerminal*>::const_iterator i = m_xTerminalsByIP.end();
/*
				// check if this success response to REGISTER or NOTIFY request
				if (sip_packet.IsResponse() && sip_packet.StatusCode() == 200)
				{
					switch(sip_packet.CSeqMethod())
					{
					case(CSIPPacket::REGISTER):
						{
							// check if device already registered
							i = m_xTerminalsByIP.find(sip_packet.DstIP().IP());
							if (i == m_xTerminalsByIP.end())
							{// create a new device
								CreateTerminal(&sip_packet);
							}
							break;
						}
					case(CSIPPacket::NOTIFY):
						{
							// check if device already registered
							i = m_xTerminalsByIP.find(sip_packet.DstIP().IP());
							if (i == m_xTerminalsByIP.end())
							{// create a new device
								CreateTerminal(&sip_packet);
							}
							break;
						}
					default:
						break;
					}
				}
*/
				// find inbound device for packet...
				if (i == m_xTerminalsByIP.end())
					i = m_xTerminalsByIP.find(sip_packet.DstIP().IP());
				if (i != m_xTerminalsByIP.end())
					i->second->OnSIPPacket(&sip_packet, false);

				// find outbound device for packet...
				i = m_xTerminalsByIP.find(sip_packet.SrcIP().IP());
				if (i != m_xTerminalsByIP.end())
					i->second->OnSIPPacket(&sip_packet, true);

				// bypass packet to call manager
				//m_pCallManager->OnSIPPacket(&sip_packet);
				m_pCallManager->OnUDPPacket(packet);
			}
		}

		if (!is_sip_packet)
		{// bypass UDP packet to RTP recorder
			m_pRTPRecorder->OnUDPPacket(packet);
		}

		return is_sip_packet;
	}
	return false;
}

void CTerminalManager::OnNewRegister(uint32 ip, const std::string &user, int type)
{
	CreateTerminal(ip, user, type);
}

void CTerminalManager::OnEndRegister(uint32 ip, const std::string &user, int type)
{
	DeleteTerminal(ip, GetDeviceID(ip, user));
}

CTerminal *CTerminalManager::GetTerminalByID(int iSerial)
{
	scope_locker locker(m_cs);

	std::map<int, CTerminal*>::iterator i = m_xTerminalsByID.find(iSerial);
	if (i != m_xTerminalsByID.end())
	{
		i->second->AddRef();
		return i->second;
	}
	return NULL;
}

int CTerminalManager::GetTerminalIDByIndex(int iVirtual)
{
	scope_locker locker(m_cs);

	if (iVirtual >= 0 && iVirtual < (int)m_xDevices.size())
		return m_xDevices[iVirtual]->ID();

	return 0;
}

int CTerminalManager::GetTerminalCount()
{
	scope_locker locker(m_cs);
	return (int)m_xDevices.size();
}

CTerminal *CTerminalManager::GetDeviceByIP(const CIPv4Address &addr)
{
	std::map<uint32, CTerminal*>::iterator it = m_xTerminalsByIP.find(addr.IP());
	if (it != m_xTerminalsByIP.end())
		return it->second;
	return NULL;
}

CTerminal *CTerminalManager::GetDeviceByCallID(const std::string &call_id)
{
	CTerminal *pDevice = NULL;
	CSIPCallParam call_params;
	if (m_pCallManager->GetCallParams(call_id, call_params))
	{
		pDevice = GetDeviceByIP(call_params.GetFrom());
		if (pDevice == NULL)
			pDevice = GetDeviceByIP(call_params.GetTo());
	}

	return pDevice;
}

void CTerminalManager::OnNewSession(uint32 session_id, const CRTPSessionParam &params)
{
	m_pRTPRecorder->OnNewSession(session_id, params);
	std::string call_id = m_pCallManager->GetCallBySession(session_id);
	CTerminal *pDevice = GetDeviceByCallID(call_id);
	if (pDevice) {
		pDevice->OnNewSession(session_id, params);
	}
}

void CTerminalManager::OnSessionModify(uint32 session_id, const CRTPSessionParam &params)
{
	m_pRTPRecorder->OnSessionModify(session_id, params);
	std::string call_id = m_pCallManager->GetCallBySession(session_id);
	CTerminal *pDevice = GetDeviceByCallID(call_id);
	if (pDevice) {
		pDevice->OnSessionModify(session_id, params);
	}
}

void CTerminalManager::OnStopSession(uint32 session_id)
{
	m_pRTPRecorder->OnStopSession(session_id);
	std::string call_id = m_pCallManager->GetCallBySession(session_id);
	CTerminal *pDevice = GetDeviceByCallID(call_id);
	if (pDevice) {
		pDevice->OnStopSession(session_id);
	}
}

void CTerminalManager::OnNewCall(const std::string &call_id)
{
	m_pRTPRecorder->OnNewCall(call_id);
	CTerminal *pDevice = GetDeviceByCallID(call_id);
	if (pDevice) {
		pDevice->OnNewCall(call_id);
	}
}

void CTerminalManager::OnEndCall(const std::string &call_id)
{
	m_pRTPRecorder->OnNewCall(call_id);
	CTerminal *pDevice = GetDeviceByCallID(call_id);
	if (pDevice) {
		pDevice->OnEndCall(call_id);
	}
}
