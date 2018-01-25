#pragma once
#include "Interfaces.h"
#include "MultiXtR.h"
#include "StreamMixer.h"
#include "Locker.h"
#include "IRCircularBuffer.h"
#include "MessageQueue.h"
#include "TerminalAudio.h"


class CTerminal
{
public:
	CTerminal(int iID, uint32 uIP, 
				ICallManager *pCallManager, 
				IRTPRecorder *pRecorder, 
				CMessageQueue *pMessageQueue,
				int iTerminalType);

	virtual ~CTerminal();

	// ref counter methods
	int AddRef();
	int Release();

	// ICallEvents
	virtual void OnNewSession(uint32 session_id, const CRTPSessionParam &params);
	virtual void OnSessionModify(uint32 session_id, const CRTPSessionParam &params);
	virtual void OnStopSession(uint32 sess_id);
	virtual void OnNewCall(const std::string &call_id);
	virtual void OnEndCall(const std::string &call_id);

	// inbound/outbound packets
	void OnSIPPacket(const CSIPPacket *packet, bool output);

	// device ID
	int ID() const {return m_iID;}

	// device status function
	int			GetCallState() {return m_iCallState;}
	int			GetStatus();
	const char *GetCallerID() {return m_sCallerID;}
	void		RecordAudio(bool bOnOf);
	void		RecordCtlData(bool bOnOf);
	void		AutoRecordAudio(bool bOnOf);
	void		AutoRecordCtlData(bool bOnOf);
	int			GetControlDataPBX(int MaxLength, unsigned char *pPBXData);
	int			GetControlDataPhone(int MaxLength, unsigned char *pPhoneData);

	int			GetAudioData(int MaxLength, 
						unsigned char *pPBXData, 
						unsigned char *pPhoneData, 
						unsigned char *pMergedData);

	void		SetAudioGain(BOOL Remote, int Gain);
	int			GetIfType() {return m_iTerminalType;}

	// for testing purposes
	const char	*GetIP() {return m_sIP;}
	unsigned int GetRecordTime();


protected:
	void OnSIPInNotify(const CSIPPacket *packet);
	void OnSIPInNotifyDefault(const CSIPPacket *packet);
	void OnSIPInNotifyNSIP(const CSIPPacket *packet);

	void Notify(int iEvent, int iData);
	void UpdateCallState(CSIPCallParam *pCallObject);
	void UpdateCallerID(CSIPCallParam *pCallObject);
	void UpdateState(int iState);

	int				m_iID;			// Terminal ID
	uint32			m_uDeviceIP;	// Terminal IP
	int				m_iCallState;	// Current call state

	bool			m_bAutoRecordAudio;	
	bool			m_bAutoRecordCtl;
	bool			m_bRecordCtlData;
	bool			m_bNECExt;

	ICallManager	*m_pCallManager;	// call manager
	IRTPRecorder	*m_pRecorder;		// RTP recorder
	CMessageQueue	*m_pMessageQueue;	// notification message queue

	std::map<std::string, CSIPCallParam*>	m_xCallObjects;
	std::vector<CSIPCallParam*>				m_xCalls;

	char			m_sCallerID[_STATIC_STRING_BUFFER_SIZE];
	char			m_sIP[_STATIC_STRING_BUFFER_SIZE];

protected:

	int					m_iTerminalType;
	int					m_bDirectInDetected;
	// record time 
	uint32				m_uRecordTime;

	// Ctrl Data buffers
	CIRCircularBuffer	m_xLocalCtlData;
	CIRCircularBuffer	m_xRemoteCtlData;
	CTerminalAudio		*m_pAudio;

	// locker
	critical_section m_xCS;

	// ref counter
	int	m_iRefCnt;

	// for nec phones display line 3
	std::string m_sNecDisplayLine;	
	// for nec phones info current no item
	std::string m_sNecInfoCurrentNo;	

};
