// Header: TerminalManager.h
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "Interfaces.h"
#include "Terminal.h"
#include "Locker.h"
#include <map>
#include <vector>
#include <string>
#include "SIPManager.h"
#include "RTPRecorder.h"
#include "MessageQueue.h"


//////////////////////////////////////////////////////////////////////////
// Class: CTerminalManager
// Manager of CTerminal objects
//////////////////////////////////////////////////////////////////////////
class CTerminalManager 
	:	public IUDPPacketListener,
		public ICallEvents
{
public:
	// Constructor: CTerminalManager
	// Default constructor.
	//
	// Parameters:
	// pMessageQueue - [in] pointer to <CMessageQueue> object
	// pbx - [in] const reference to <CIPv4Address> object that contain
	// main device ip and sip port.
	CTerminalManager(CMessageQueue *pMessageQueue, 
						const CIPv4Address &pbx);

	// Destructor: ~CTerminalManager
	// Default destructor.
	virtual ~CTerminalManager();
	
	// IUDPPacketListener
	virtual bool OnUDPPacket(const CUDPPacket *packet);
	
	// API
	CTerminal	*GetTerminalByID(int iSerial);
	int			 GetTerminalIDByIndex(int iVirtual);
	int			 GetTerminalCount();

	// interface ICallEvents
	virtual void OnNewSession(uint32 session_id, const CRTPSessionParam &params);
	virtual void OnSessionModify(uint32 session_id, const CRTPSessionParam &params);
	virtual void OnStopSession(uint32 sess_id);
	virtual void OnNewCall(const std::string &call_id);
	virtual void OnEndCall(const std::string &call_id);
	virtual void OnNewRegister(uint32 ip, const std::string &user, int type);
	virtual void OnEndRegister(uint32 ip, const std::string &user, int type);

protected:
	void		DeleteTerminal(uint32 dst_ip, int terminal_id);

//	void		CreateTerminal(const CSIPPacket *packet);
//	int			GetDeviceID(const CSIPPacket *packet);

	void		CreateTerminal(uint32 dst_ip, std::string from_user, int type);
	int			GetDeviceID(uint32 dst_ip, std::string from_user);


	CTerminal	*GetDeviceByIP(const CIPv4Address &addr);
	CTerminal	*GetDeviceByCallID(const std::string &call_id);

	// devices by id
	std::map<int, CTerminal*>		m_xTerminalsByID;
	// devices by IP
	std::map<uint32, CTerminal*>	m_xTerminalsByIP;
	// devices by index
	std::vector<CTerminal*>			m_xDevices;

	CIPv4Address m_xPbxIP;	// PBX ip, SIP port

protected:
	// sip call manager
	CSIPManager			*m_pCallManager;
	// rtp sessions recorder
	CRTPRecorder		*m_pRTPRecorder;
	// notification message queue
	CMessageQueue		*m_pMessageQueue;

	// buffers and containers locker
	critical_section	m_cs;
};
