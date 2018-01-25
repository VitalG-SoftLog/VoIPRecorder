#include "StdAfx.h"
#include "SIPManager.h"


CSIPManager::CSIPManager(uint32 uPbxIP)
{
	m_uLastSessionID = 1;

	sip_init(htonl(uPbxIP));
	
	// install callback handlers
	set_on_register(cbOnRegister, this);
	set_on_update_register(cbOnRegister, this);
	set_on_end_register(cbOnEndRegister, this);
	set_on_call_start(cbOnCallStart, this);
	set_on_call_end(cbOnCallEnd, this);
	set_on_new_session(cbOnNewSession, this);
	set_on_end_session(cbOnEndSession, this);
	set_on_update_session(cbOnUpdateSession, this);
}

CSIPManager::~CSIPManager(void)
{
	// uninstall callback handlers
	set_on_register(NULL, this);
	set_on_update_register(NULL, this);
	set_on_end_register(NULL, this);
	set_on_call_start(NULL, this);
	set_on_call_end(NULL, this);
	set_on_new_session(NULL, this);
	set_on_end_session(NULL, this);
	set_on_update_session(NULL, this);
	sip_destroy();
}

bool CSIPManager::OnUDPPacket(const CUDPPacket *packet)
{
	return sip_worker_parse((u_char *)packet->RawPacket(), packet->RawPacketSize()) == 1;
}

void CSIPManager::SetCallEventsListener(ICallEvents *pCallEvents)
{
	m_pCallEventsListener = pCallEvents;
}

void CSIPManager::AddNewCall(const CSIPCallParam &call)
{
	m_xCalls[call.GetCallID()] = call;
	if (m_pCallEventsListener)
		m_pCallEventsListener->OnNewCall(call.GetCallID());
}

void CSIPManager::DeleteCall(const std::string &call_id)
{
	std::map<std::string, CSIPCallParam>::iterator it = m_xCalls.find(call_id);

	if (it != m_xCalls.end())
	{
		CSIPCallParam params = it->second;
		std::vector<uint32> xSessions = params.GetSessionsIDs();

		// remove sessions
		for (unsigned i = 0; i < xSessions.size(); i++)
			DeleteSession(params.GetCallID(), xSessions[i]);

		if (m_pCallEventsListener)
			m_pCallEventsListener->OnEndCall(call_id);

		m_xCalls.erase(it);
	}
}

void CSIPManager::AddSession(const std::string &call_id, uint32 session_id, const CRTPSessionParam &session)
{
	m_xSessionsParams[session_id]	= session;
	m_xSessionsToCalls[session_id]	= call_id;
	m_xSessionsTasks[session.GetUserData()] = session_id;
	if (m_pCallEventsListener)
		m_pCallEventsListener->OnNewSession(session_id, session);
}

void CSIPManager::DeleteSession(const std::string &call_id, uint32 session_id)
{
	std::map<uint32, CRTPSessionParam>::iterator it = m_xSessionsParams.find(session_id);
	if (it != m_xSessionsParams.end())
	{
		if (m_pCallEventsListener)
			m_pCallEventsListener->OnStopSession(session_id);

		std::map<void*, uint32>::iterator it_ut = m_xSessionsTasks.find(it->second.GetUserData());
		if (it_ut != m_xSessionsTasks.end())
		{
			m_xSessionsTasks.erase(it_ut);
		}

		m_xSessionsToCalls.erase(m_xSessionsToCalls.find(session_id));
		m_xSessionsParams.erase(it);
	}
}

// ICallManager interface
void CSIPManager::GetSessionsByCallID(const std::string &call_id, std::vector<uint32> &sessions)
{
	sessions.clear();
	std::map<std::string, CSIPCallParam>::iterator i = m_xCalls.find(call_id);
	if (i != m_xCalls.end())
	{
		sessions = i->second.GetSessionsIDs();
	}
}

uint32 CSIPManager::GetCallCount()
{
	return (uint32)m_xCalls.size();
}

bool CSIPManager::GetCallParams(const std::string &call_id, CSIPCallParam &params)
{
	std::map<std::string, CSIPCallParam>::iterator i = m_xCalls.find(call_id);
	if (i != m_xCalls.end())
	{
		params = i->second;
		return true;
	}

	return false;
}

std::string CSIPManager::GetCallByIndex(int index)
{
	return "";
}

std::string CSIPManager::GetCallBySession(uint32 sess_id)
{
	std::map<uint32, std::string>::iterator it = m_xSessionsToCalls.find(sess_id);
	if (it != m_xSessionsToCalls.end())
	{
		return it->second;
	}
	return "";
}

bool CSIPManager::GetSessionParams(uint32 sess_id, CRTPSessionParam &sess_params)
{
	std::map<uint32, CRTPSessionParam>::iterator i = m_xSessionsParams.find(sess_id);
	if (i != m_xSessionsParams.end())
	{
		sess_params = i->second;
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
// SIP Lib helpers
//////////////////////////////////////////////////////////////////////////
void CSIPManager::RTPTask2Param(const rtp_task_t* prtp_task, CRTPSessionParam &sess_params)
{
	if (prtp_task)
	{
		CIPv4Address ip;
		ip.IP(htonl(prtp_task->addr));
		ip.Port(htons(prtp_task->port));
		sess_params.SetDst(ip);
		sess_params.SetPayloadType(prtp_task->codec);
		sess_params.SetUserData((void *)prtp_task);
	}
}

uint32 CSIPManager::GetSessionID(const rtp_task_t* prtp_task)
{
	std::map<void*, uint32>::iterator it = m_xSessionsTasks.find((void *)prtp_task);
	if (it != m_xSessionsTasks.end())
	{
		return it->second;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// SIP Lib callbacks
//////////////////////////////////////////////////////////////////////////
void CSIPManager::cbOnRegister(void* pctx,const tbl_registry_t* ptbl_registry)
{
	CSIPManager *pThis = (CSIPManager *)pctx;
	if (pThis)
	{
		scope_locker ll(pThis->m_xLocker);
		if (ptbl_registry && ptbl_registry->puser_name)
		{
			if (((ptbl_registry->status == ST_REGISTER) || 
				(ptbl_registry->status == ST_RECEIVE_NOTIFY)) &&
				ptbl_registry->phone_type != PHONE_TYPE_UNDEFINED)
			{

				if (pThis->m_pCallEventsListener)
				{
					pThis->m_pCallEventsListener->OnNewRegister(
						htonl(ptbl_registry->task_ua.addr), 
						ptbl_registry->puser_name, ptbl_registry->phone_type);
				}
			}
		}
	}
}

void CSIPManager::cbOnEndRegister(void* pctx,const tbl_registry_t* ptbl_registry)
{
	CSIPManager *pThis = (CSIPManager *)pctx;
	if (pThis)
	{
		scope_locker ll(pThis->m_xLocker);

		if (ptbl_registry && ptbl_registry->puser_name)
		{
			if (ptbl_registry->status == ST_END_USER_REGISTER || 
				ptbl_registry->status == ST_END_EXPIRY_REGISTER)
			{

				if (pThis->m_pCallEventsListener)
				{
					pThis->m_pCallEventsListener->OnEndRegister(
						htonl(ptbl_registry->task_ua.addr), 
						ptbl_registry->puser_name, ptbl_registry->phone_type);
				}
			}
		}
	}
}

void CSIPManager::cbOnCallStart(void* pctx,const tbl_session_t* ptbl_session)
{
	CSIPManager *pThis = (CSIPManager *)pctx;
	if (pThis && ptbl_session)
	{
		scope_locker ll(pThis->m_xLocker);
		CSIPCallParam call;
		if (ptbl_session->pcall_id)
		{
			call.SetCallID(ptbl_session->pcall_id);
			call.SetCSeq(ptbl_session->cseg);
			call.SetUserFrom(ptbl_session->pfrom);
			call.SetUserTo(ptbl_session->pto);

			CIPv4Address ip;
			ip.IP(htonl(ptbl_session->map_param_0.sip_task.addr));
			call.SetFrom(ip);
			ip.IP(htonl(ptbl_session->map_param_1.sip_task.addr));
			call.SetTo(ip);

			if ((ptbl_session->map_param_0.ptbl_registry == NULL) ||
				(ptbl_session->map_param_1.ptbl_registry != NULL))
			{
				call.SetExternal(true);
			}
			else
			{
				call.SetExternal(false);
			}

			pThis->AddNewCall(call);
		}
	}
}

void CSIPManager::cbOnCallEnd(void* pctx,const tbl_session_t* ptbl_session)
{
	CSIPManager *pThis = (CSIPManager *)pctx;
	if (pThis && ptbl_session)
	{
		scope_locker ll(pThis->m_xLocker);
		if (ptbl_session->pcall_id)
			pThis->DeleteCall(ptbl_session->pcall_id);
	}
}

void CSIPManager::cbOnNewSession(void* pctx, const rtp_task_t* prtp_task,const tbl_session_t* ptbl_session)
{
	CSIPManager *pThis = (CSIPManager *)pctx;
	if (pThis && prtp_task && ptbl_session)
	{
		scope_locker ll(pThis->m_xLocker);
		if (ptbl_session->pcall_id)
		{
			CRTPSessionParam session_params;
			RTPTask2Param(prtp_task, session_params);
			pThis->AddSession(ptbl_session->pcall_id, pThis->m_uLastSessionID, session_params);
			pThis->m_uLastSessionID++;
		}
	}
}

void CSIPManager::cbOnEndSession(void* pctx, const rtp_task_t* prtp_task,const tbl_session_t* ptbl_session)
{
	CSIPManager *pThis = (CSIPManager *)pctx;
	if (pThis && prtp_task && ptbl_session)
	{
		scope_locker ll(pThis->m_xLocker);
		if (ptbl_session->pcall_id)
		{
			uint32 session_id = pThis->GetSessionID(prtp_task);
			if (session_id)
			{
				pThis->DeleteSession(ptbl_session->pcall_id, session_id);
			}
		}
	}
}

void CSIPManager::cbOnUpdateSession(void* pctx, const rtp_task_t* prtp_task, const tbl_session_t* ptbl_session)
{
	CSIPManager *pThis = (CSIPManager *)pctx;
	if (pThis && prtp_task && ptbl_session)
	{
		scope_locker ll(pThis->m_xLocker);
		if (ptbl_session->pcall_id)
		{
			uint32 session_id = pThis->GetSessionID(prtp_task);
			if (session_id && pThis->m_pCallEventsListener)
			{
				CRTPSessionParam session_params;
				RTPTask2Param(prtp_task, session_params);
				pThis->m_pCallEventsListener->OnSessionModify(session_id, session_params);
			}
		}
	}
}


