// Header: RTPRecorder.h
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "Interfaces.h"
#include <string>
#include <map>
#include "SimplyThread.h"
#include "Locker.h"
#include "RTPRecorderSession.h"
#include "Streamer.h"

// Define: _RTP_QUEUE_PROCESS_TIMEOUT
// Timeout in ms, after which handled all incoming RTP packets
#define _RTP_QUEUE_PROCESS_TIMEOUT	200
#define _RTP_JITTER_PACKET			40

//////////////////////////////////////////////////////////////////////////
// Class: CRTPRecorder
// Manager of RTP sessions
//////////////////////////////////////////////////////////////////////////
class CRTPRecorder :
		public CSimplyThread,
		public IUDPPacketListener,
		public IRTPRecorder
{
public:
	// Constructor: CRTPRecorder
	// Default constructor.
	CRTPRecorder();

	// Destructor: ~CRTPRecorder
	// Default destructor.	
	virtual ~CRTPRecorder();
		
	// Method: Initialize
	// Initialize <CRTPRecorder> object and starting working thread. Must be 
	// called before any RTP data will be captured.
	void Initialize();
	
	// Method: UnInitialize
	// UnInitialize <CRTPRecorder> object and stopping working thread. Must 
	// be called before instance will be deleted.
	void UnInitialize();

	// Method: OnUDPPacket
	// Implementation of <IUDPPacketListener> interface.
	//
	// Parameters:
	// packet - pointer to <CUDPPacket> object.
	virtual bool OnUDPPacket(const CUDPPacket *packet);

	// ICallEvents
	virtual void OnNewRegister(uint32 ip, const std::string &user, int type) {}
	virtual void OnEndRegister(uint32 ip, const std::string &user, int type) {}

	// Method: OnNewCall
	// Implementation of <ICallEvents> interface. Call when new SIP call detected.
	//
	// Parameters:
	// call_id - const reference to call_id string.
	virtual void OnNewCall(const std::string &call_id);

	// Method: OnEndCall
	// Implementation of <ICallEvents> interface. Call when SIP call ended.
	//
	// Parameters:
	// call_id - const reference to call_id string.
	virtual void OnEndCall(const std::string &call_id);
	
	// Method: OnNewSession
	// Implementation of <ICallEvents> interface. Call when detected a new 
	// RTP session.
	//
	// Parameters:
	// session_id - RTP session ID (not SSRC!).
	// params - Parameters of new session. Const reference to 
	// <CRTPSessionParam> object. 
	virtual void OnNewSession(unsigned int session_id, const CRTPSessionParam &params);
	
	// Method: OnSessionModify
	// Implementation of <ICallEvents> interface. Call when detected a new 
	// RTP session parameters modification.
	//
	// Parameters:
	// session_id - RTP session ID (not SSRC!).
	// params - New parameters of defined session. Const reference to 
	// <CRTPSessionParam> object. 
	virtual void OnSessionModify(unsigned int session_id, const CRTPSessionParam &params);

	// Method: OnStopSession
	// Implementation of <ICallEvents> interface. Call when session deleted.
	//
	// Parameters:
	// session_id - RTP session ID (not SSRC!).	
	virtual void OnStopSession(unsigned int sess_id);

	// Method: StartRecord
	// Implementation of <IRTPRecorder> interface. Start recording specified session.
	//
	// Parameters:
	// sess_id - [in] ID of session for recording
	virtual void StartRecord(unsigned int sess_id, IDataStream *pStream); 

	// Method: StopRecord
	// Implementation of <IRTPRecorder> interface. Stop recording specified session.
	//
	// Parameters:
	// sess_id - [in] ID of session for stopping record
	virtual void StopRecord(unsigned int sess_id);

protected:
	void DeleteSession(unsigned int sess_id);
	void DeleteIpFromMap(const CIPv4Address &ip, unsigned int sess_id);
	void CreateSession(unsigned int sess_id, const CRTPSessionParam &p);

	// sessions map
	std::map<unsigned int, CRTPRecorderSession*> m_xSessionsByID;
	// allowed ip:port destination map
	std::multimap<unsigned __int64, unsigned int> m_xDstIpPorts;

protected:
	// thread routine
	virtual				void Run();
	CStreamer			m_xStreamer;
	HANDLE				m_hStopEvent;
	critical_section	m_xCS;
};
