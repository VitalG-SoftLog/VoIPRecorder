#include "StdAfx.h"
#include "Terminal.h"
#include "SIPContentNECExt.h"
#include "StringUtils.h"
#include "RecorderMain.h"
#include "Settings.h"

using namespace StringUtils;

_MODULE_NAME(CTerminal);

CTerminal::CTerminal(int iID, uint32 uIP,
					 ICallManager *pCallManager, IRTPRecorder *pRecorder, 
					 CMessageQueue *pMessageQueue, int iTerminalType)
{
	m_pMessageQueue		= pMessageQueue;
	m_iID				= iID;
	m_uDeviceIP			= uIP;
	m_iTerminalType		= iTerminalType;
	m_bDirectInDetected	= false;

	m_sNecInfoCurrentNo.clear();
	m_sNecDisplayLine.clear();
	m_sCallerID[0] = 0;

	CIPv4Address ip; 
	ip.IP(uIP);
	
	SAFE_STRNCPY(m_sIP, sizeof(m_sIP), ip.AsString().c_str(), sizeof(m_sIP) - 1);

	m_iRefCnt			= 0;
	m_iCallState		= CALL_IDLE;
	m_pCallManager		= pCallManager;
	m_pRecorder			= pRecorder;

	m_bAutoRecordAudio	= false;
	m_bAutoRecordCtl	= false;
	m_bRecordCtlData	= false;

	m_pAudio = new CTerminalAudio(m_iID, pMessageQueue, pRecorder);
}

CTerminal::~CTerminal(void)
{
	if (m_pAudio)
	{
		m_pAudio->StopRecord();
		delete m_pAudio;
	}

	for (unsigned i = 0; i < m_xCalls.size(); i++)
	{
		delete m_xCalls[i];
	}

	m_xCalls.clear();
	m_xCallObjects.clear();
}

int	CTerminal::GetAudioData(int MaxLength, 
				 unsigned char *pPBXData, 
				 unsigned char *pPhoneData, 
				 unsigned char *pMergedData)
{
	if (m_pAudio)
	{
		return m_pAudio->GetAudioData(MaxLength, pPBXData, pPhoneData, pMergedData);
	}
	return 0;
}

void CTerminal::SetAudioGain(BOOL Remote, int Gain)
{
	if (m_pAudio)
	{
		return m_pAudio->SetAudioGain(Remote, Gain);
	}
}

unsigned int CTerminal::GetRecordTime()
{
	if (m_uRecordTime != 0) 
	{
		return (uint32)time(NULL) - m_uRecordTime;
	}
	return 0;
}

void CTerminal::OnNewSession(uint32 session_id, const CRTPSessionParam &params)
{
	_LOG("Terminal " << IntToString(m_iID) << ", RTP session CREATED id: " << IntToHex(session_id, 8));

	std::string call_id = m_pCallManager->GetCallBySession(session_id);
	std::map<std::string, CSIPCallParam*>::iterator it = m_xCallObjects.find(call_id);
	CSIPCallParam *pCallObject = NULL;

	if (it != m_xCallObjects.end())
		pCallObject = it->second;

	if (pCallObject)
	{
		bool remote	= false;
		// first session is always inbound for this call object
		bool in_session = pCallObject->GetSessionsIDs().empty();

		// UpdateCallerID for active call
		if (!in_session && (pCallObject == m_xCalls[0]))
			UpdateCallerID(pCallObject);

		if (pCallObject->GetOutgoing())
			remote = in_session;				
		else
			remote = !in_session;				

		m_pAudio->AddNewStream(session_id, remote);
		pCallObject->AddNewSessionsID(session_id);
	}
}

void CTerminal::OnSessionModify(uint32 session_id, const CRTPSessionParam &params)
{
	_LOG("Terminal " << IntToString(m_iID) << ", RTP session MODIFY id: " << IntToHex(session_id, 8));
}

void CTerminal::OnStopSession(uint32 session_id)
{
	_LOG("Terminal " << IntToString(m_iID) << ", RTP session STOP id: " << IntToHex(session_id, 8));
	m_pAudio->DeleteStream(session_id);
}

void CTerminal::UpdateCallerID(CSIPCallParam *pCallObject)
{
	if (pCallObject)
	{
		std::string sNewCallerID = pCallObject->GetOutgoing() ? pCallObject->GetUserTo() : pCallObject->GetUserFrom();
		// find @ char...
		size_t end = sNewCallerID.find('@');
		if (end != std::string::npos)
			sNewCallerID = sNewCallerID.substr(0, end);

		if (Settings::eMode == modeDefault) {
			// FIXME: Caller ID from displayed line 3...
			if ((GetIfType() == PHONE_TYPE_NEC) && 
				!m_sNecDisplayLine.empty() && 
				!StringUtils::IsAllDigits(sNewCallerID))
			{
				size_t start = m_sNecDisplayLine.rfind("  ");
				if (start != std::string::npos) 
				{
					std::string s = Trim(ToUpperCase(m_sNecDisplayLine.substr(start + 2))); 
					if (!s.empty())
					{
						sNewCallerID = s;
					}
				}
			}
		} 
		else if (Settings::eMode == modeNSIP) {
			// Caller ID from info-current-no2...
			if ((GetIfType() == PHONE_TYPE_NEC) && !m_sNecInfoCurrentNo.empty()) {
				sNewCallerID = m_sNecInfoCurrentNo;
			}	
		}

//		if (sNewCallerID != m_sCallerID)
		{
			SAFE_STRNCPY(m_sCallerID, sizeof(m_sCallerID), sNewCallerID.c_str(), sizeof(m_sCallerID) - 1);
			Notify(MP_CALLERIDUPDATE, 0);
		}
	}
	else
	{
		if (!m_sCallerID[0])
		{
			m_sCallerID[0] = 0;
			Notify(MP_CALLERIDUPDATE, 0);
		}
	}
}

void CTerminal::UpdateCallState(CSIPCallParam *pCallObject)
{
	if (pCallObject) {
		if ((Settings::eMode == modeNSIP) && (GetIfType() == PHONE_TYPE_NEC)) {
			UpdateState(CALL_ACTIVE);
		} else {
			UpdateState(pCallObject->GetOutgoing() ? CALL_OUTGOING : CALL_INCOMING);
		}
	} else {
		UpdateState(CALL_IDLE);
	}
	UpdateCallerID(pCallObject);
}

void CTerminal::OnNewCall(const std::string &call_id)
{
	_LOG("Terminal " << IntToString(m_iID) << ", SIP Call NEW id: " << call_id);

	CSIPCallParam call_param;
	if (m_pCallManager->GetCallParams(call_id, call_param))
	{
		call_param.SetOutgoing(m_uDeviceIP == call_param.GetFrom().IP());
		CSIPCallParam *pCallObject = new CSIPCallParam(call_param);
		m_xCallObjects[call_param.GetCallID()] = pCallObject;
		m_xCalls.push_back(pCallObject);
#ifdef _DEBUG_LOG
		pCallObject->Dump();
#endif//_DEBUG
		UpdateCallState(pCallObject);
		if (m_bAutoRecordAudio)
			RecordAudio(true);
	}
}

void CTerminal::OnEndCall(const std::string &call_id)
{
	_LOG("Terminal " << IntToString(m_iID) << ", SIP Call END id: " << call_id);

	std::map<std::string, CSIPCallParam*>::iterator it = m_xCallObjects.find(call_id);
	CSIPCallParam *pCallObject = NULL;
	if (it != m_xCallObjects.end())
		pCallObject = it->second;

	if (pCallObject)
	{
		// delete all streams from audio...
		std::vector<uint32> xSessions = pCallObject->GetSessionsIDs();
		for (unsigned i = 0; i < xSessions.size(); i++)
			m_pAudio->DeleteStream(xSessions[i]);
		
		// delete call object from vector...
		std::vector<CSIPCallParam*>::iterator vit = std::find(m_xCalls.begin(), m_xCalls.end(), pCallObject);
		if (vit != m_xCalls.end())
			m_xCalls.erase(vit);

		// delete call object from map
		m_xCallObjects.erase(it);

		// delete call object
		delete pCallObject;

		// update states...
		if (m_xCalls.size())
		{
			// we have more call objects
			pCallObject = m_xCalls[m_xCalls.size() - 1];
			// new active call object
			UpdateCallState(pCallObject);
		}
		else
		{
			UpdateCallState(NULL);
			RecordAudio(false);
		}
	}
}

void CTerminal::UpdateState(int iState)
{
	int iOldState	= m_iCallState;
	m_iCallState	= iState;

	if (iOldState != iState)
	{// notify state changed

		if (iState == CALL_ACTIVE) {
			m_sCallerID[0] = 0;
			m_sNecInfoCurrentNo.clear();
			m_sNecDisplayLine.clear();
		}

		Notify(MP_CALLSTATE, iState);
		
#ifdef _DEBUG_LOG
		switch(iState)
		{
			case(CALL_IDLE):
				_LOG("Terminal " << IntToString(m_iID) << ", state changed: CALL_IDLE");
				break;
			case(CALL_ACTIVE):
				_LOG("Terminal " << IntToString(m_iID) << ", state changed: CALL_ACTIVE");
				break;
			case(CALL_INCOMING):
				_LOG("Terminal " << IntToString(m_iID) << ", state changed: CALL_INCOMING");
				break;
			case(CALL_OUTGOING):
				_LOG("Terminal " << IntToString(m_iID) << ", state changed: CALL_OUTGOING");
				break;
			case(CALL_INCOMING_EXTERNAL):
				_LOG("Terminal " << IntToString(m_iID) << ", state changed: CALL_INCOMING_EXTERNAL");
				break;
			case(CALL_INCOMING_INTERNAL):
				_LOG("Terminal " << IntToString(m_iID) << ", state changed: CALL_INCOMING_INTERNAL");
				break;
			case(CALL_OUTGOING_EXTERNAL):
				_LOG("Terminal " << IntToString(m_iID) << ", state changed: CALL_OUTGOING_EXTERNAL");
				break;
			case(CALL_OUTGOING_INTERNAL):
				_LOG("Terminal " << IntToString(m_iID) << ", state changed: CALL_OUTGOING_INTERNAL");
				break;
		}
#endif//_DEBUG_LOG
	}
}

int CTerminal::AddRef()
{
	m_iRefCnt++;
	return m_iRefCnt;
}

int CTerminal::Release()
{
	m_iRefCnt--;
	if (!m_iRefCnt)
	{
		delete this;
		return 0;
	}
	return m_iRefCnt;
}

// inbound/outbound packets
void CTerminal::OnSIPPacket(const CSIPPacket *packet, bool output)
{
	if (output)
	{// sended data
		if (m_bRecordCtlData)
		{
			m_xCS.enter(); // lock buffers			
			
			m_xLocalCtlData.AddData((const BYTE*)packet->GetRawData().c_str(), 
				(int)packet->GetRawData().size());
			int iDataSize = m_xLocalCtlData.GetDataLength();

			m_xCS.leave(); // unlock buffers
			
			Notify(MP_DCHANNELDATAPHONE, iDataSize);
		}
	}
	else
	{// received data
		if (packet->Method() == CSIPPacket::NOTIFY)
			OnSIPInNotify(packet);

		if (m_bRecordCtlData)
		{
			m_xCS.enter(); // lock buffers
			
			m_xRemoteCtlData.AddData((const BYTE *)packet->GetRawData().c_str(), 
				(int)packet->GetRawData().size());

			int iDataSize = m_xRemoteCtlData.GetDataLength();
			m_xCS.leave(); // unlock buffers
		
			Notify(MP_DCHANNELDATAPBX, iDataSize);
		}
	}
}

void CTerminal::OnSIPInNotify(const CSIPPacket *packet)
{
	switch (Settings::eMode)
	{
	case (modeDefault): 
		{
			OnSIPInNotifyDefault(packet);
			break;
		}
	case (modeNSIP): 
		{
			OnSIPInNotifyNSIP(packet);
			break;
		}
	}
}

void CTerminal::OnSIPInNotifyDefault(const CSIPPacket *packet)
{
	if (GetIfType() == PHONE_TYPE_NEC)
	{
		CSIPContentNECExt content(packet->ContentType(), packet->Body());
		if (content.IsDispLineEvent(3)) {
			if (!content.IsDispLineClear(3)) {
				m_sNecDisplayLine = content.DispLineText(3);
			} else {
				m_sNecDisplayLine.clear();
			}
		}
	}
}

void CTerminal::OnSIPInNotifyNSIP(const CSIPPacket *packet)
{
	if (GetIfType() == PHONE_TYPE_NEC) 
	{
		if (m_iCallState == CALL_ACTIVE) {

			CSIPContentNECExt content(packet->ContentType(), packet->Body());
			if (content.IsInfoDirectIn()) {
				// INCOMING CALL!!!
				UpdateState(CALL_INCOMING);
				m_sNecInfoCurrentNo = content.InfoDirectIn();
				if (!m_sNecInfoCurrentNo.empty() && 
					m_sNecInfoCurrentNo != m_sCallerID) {
					SAFE_STRNCPY(m_sCallerID, sizeof(m_sCallerID), m_sNecInfoCurrentNo.c_str(), sizeof(m_sCallerID) - 1);
					Notify(MP_CALLERIDUPDATE, 0);
				}
			} else if (content.IsInfoCurrentNoEvent()) {
				std::string sCurrentNo = content.InfoCurrentNo();
				if (m_sNecInfoCurrentNo == "0000000000") {
					m_sNecInfoCurrentNo = sCurrentNo;
					UpdateState(CALL_OUTGOING);
				} else {
					m_sNecInfoCurrentNo = sCurrentNo;
				}

				if (!m_sNecInfoCurrentNo.empty() && 
					m_sNecInfoCurrentNo != m_sCallerID) {
						SAFE_STRNCPY(m_sCallerID, sizeof(m_sCallerID), m_sNecInfoCurrentNo.c_str(), sizeof(m_sCallerID) - 1);
						Notify(MP_CALLERIDUPDATE, 0);
				}
			}
		}
	}
}

int CTerminal::GetControlDataPBX(int MaxLength, unsigned char *pPBXData)
{
	m_xCS.enter();// lock buffers
	int iReturn = m_xRemoteCtlData.GetData( pPBXData, MaxLength);
	m_xCS.leave();// unlock buffers
	return iReturn;
}

int CTerminal::GetControlDataPhone(int MaxLength, unsigned char *pPhoneData)
{
	m_xCS.enter();// lock buffers
	int iReturn = m_xLocalCtlData.GetData(pPhoneData, MaxLength);
	m_xCS.leave();// unlock buffers
	return iReturn;
}

void CTerminal::RecordAudio(bool bOnOf)
{
	if (bOnOf) {
		m_uRecordTime = (uint32)time(NULL);
		m_pAudio->StartRecord();
	} else {
		m_pAudio->StopRecord();
		m_uRecordTime = 0;
	}
}

void CTerminal::RecordCtlData(bool bOnOf)
{
}

void CTerminal::AutoRecordAudio(bool bOnOf)
{
	m_bAutoRecordAudio = bOnOf;
}

void CTerminal::AutoRecordCtlData(bool bOnOf)
{
	m_bAutoRecordCtl = bOnOf;
}

int CTerminal::GetStatus()
{
	if (m_iTerminalType == _ALLOWED_PHONE_TYPE)
	{
		return STATUS_GOOD;
	}
	else
	{
		return STATUS_UNLICENSED;
	}
}

void CTerminal::Notify(int iEvent, int iData)
{
	if (m_pMessageQueue) {
		m_pMessageQueue->Push(m_iID, iEvent, iData);
	}
}

