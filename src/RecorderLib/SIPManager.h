// Header: SIPManager.h
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "Interfaces.h"
#include "Locker.h"
#include <sip_parser_api.h>


//////////////////////////////////////////////////////////////////////////
// Class: CSIPManager
// C++ wrapper around plain C SIPLib
//////////////////////////////////////////////////////////////////////////
class CSIPManager : 
	public IUDPPacketListener,
	public ICallManager

{
public:
	// Constructor: CSIPManager
	// Default constructor.
	//
	// Parameters:
	// uPbxIP - [in] IP address of the main device in network order.
	CSIPManager(uint32 uPbxIP);

	// Destructor: CSIPManager
	// Default destructor.
	virtual ~CSIPManager();

	// 
	void SetCallEventsListener(ICallEvents *pCallEvents);

	// IUDPPacketListener interface
	virtual bool OnUDPPacket(const CUDPPacket *packet);

	// ICallManager interface
	virtual void			GetSessionsByCallID(const std::string &call_id, std::vector<uint32> &sessions);
	virtual uint32			GetCallCount();
	virtual bool			GetCallParams(const std::string &call_id, CSIPCallParam &params);
	virtual std::string 	GetCallByIndex(int index);
	virtual bool			GetSessionParams(uint32 sess_id, CRTPSessionParam &sess_params);
	virtual std::string 	GetCallBySession(uint32 sess_id);

protected:
	static void cbOnRegister(void* pctx,const tbl_registry_t* ptbl_registry);
	static void cbOnEndRegister(void* pctx,const tbl_registry_t* ptbl_registry);
	static void cbOnCallStart(void* pctx,const tbl_session_t* ptbl_session);
	static void cbOnCallEnd(void* pctx,const tbl_session_t* ptbl_session);
	static void cbOnNewSession(void* pctx,const rtp_task_t* prtp_task,const tbl_session_t* ptbl_session);
	static void cbOnEndSession(void* pctx,const rtp_task_t* prtp_task,const tbl_session_t* ptbl_session);
	static void cbOnUpdateSession(void* pctx,const rtp_task_t* prtp_task,const tbl_session_t* ptbl_session);

	static void RTPTask2Param(const rtp_task_t* prtp_task, CRTPSessionParam &sess_params);
	uint32 GetSessionID(const rtp_task_t* prtp_task);


	void		AddNewCall(const CSIPCallParam &call);
	void		DeleteCall(const std::string &call_id);
	void		AddSession(const std::string &call_id, uint32 session_id, const CRTPSessionParam &session);
	void		DeleteSession(const std::string &call_id, uint32 session_id);
	void		UpdateSession(const std::string &call_id, uint32 session_id, const CRTPSessionParam &session);

	std::map<std::string, CSIPCallParam>	m_xCalls;			// call-ID ==> Call object map
	std::map<void*, uint32>					m_xSessionsTasks;	// rtp_task_t pointer ==> session id
	std::map<uint32, CRTPSessionParam>		m_xSessionsParams;	// session id ==> CRTPSessionParam object
	std::map<uint32, std::string>			m_xSessionsToCalls;	// sessions ==> call-ID

	ICallEvents								*m_pCallEventsListener;	// pointer to Call Events Listener
	uint32									m_uLastSessionID;	// last session ID
	critical_section						m_xLocker;			// containers locker
};
