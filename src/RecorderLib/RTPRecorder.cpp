#include "StdAfx.h"
#include "RtpRecorder.h"
#include "StringUtils.h"

#pragma comment(lib, "Winmm.lib")

using namespace StringUtils;

_MODULE_NAME(CRTPRecorder);

CRTPRecorder::CRTPRecorder(void)
: CSimplyThread()
{
	m_hStopEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
}

CRTPRecorder::~CRTPRecorder(void)
{
	// delete all sessions
	std::map<unsigned int, CRTPRecorderSession*>::iterator it = m_xSessionsByID.begin();
	for (;it != m_xSessionsByID.end(); it++)
	{
		CRTPRecorderSession *pSession = it->second;
		delete pSession;
	}

	m_xSessionsByID.clear();
	CloseHandle(m_hStopEvent);
}

void CRTPRecorder::Initialize()
{
	// starting thread
	Start();
	WaitForStart();
}

void CRTPRecorder::UnInitialize()
{
	// stopping thread
	SetEvent(m_hStopEvent);
	WaitForEnd();
}

// incoming UDP packet callback
bool CRTPRecorder::OnUDPPacket(const CUDPPacket *packet)
{
	scope_locker sl(m_xCS);
	
// test packets drop
//	if ((rand()%4) == 0)
//		return;

	if (!packet)
		return false;

	unsigned __int64 dst = packet->Dst().IP();
	dst <<= 16;
	dst |= packet->Dst().Port();


	std::multimap<unsigned __int64, unsigned int>::iterator it = m_xDstIpPorts.find(dst);
	if (it != m_xDstIpPorts.end())
	{
		std::multimap<unsigned __int64, unsigned int>::iterator last;
		last = m_xDstIpPorts.upper_bound(dst);

		do {
			std::map<unsigned int, CRTPRecorderSession *>::iterator it_sess = m_xSessionsByID.find(it->second);

			if (it_sess != m_xSessionsByID.end())
			{
				CRTPRecorderSession *pSession = it_sess->second;
				if (pSession)
				{
					pSession->OnUDPPacket(packet);
				}
			}
			it++;

		} while (it != last);
	}

	return false;
}

// thread routine
void CRTPRecorder::Run()
{
	TIMECAPS		timeCaps = {0};
	LARGE_INTEGER	llCounter = {0};
	LARGE_INTEGER	llCounterFreq = {0};

	::timeGetDevCaps(&timeCaps, sizeof(timeCaps));
	::timeBeginPeriod(timeCaps.wPeriodMin);

	::QueryPerformanceFrequency(&llCounterFreq);
	::QueryPerformanceCounter(&llCounter);

	DWORD dwWaitTimeout		= _RTP_JITTER_PACKET;
	int64 dwLastQueueStamp	= 0;
	DWORD dwWaitResult		= ::WaitForSingleObject(m_hStopEvent, dwWaitTimeout);

	while (dwWaitResult == WAIT_TIMEOUT)
	{
		LARGE_INTEGER llCurrentCounter = {0};
		::QueryPerformanceCounter(&llCurrentCounter);

		int64 iRealWaitTimeMS = dwWaitTimeout;
		if (llCounterFreq.QuadPart / 1000) {
			iRealWaitTimeMS = (llCurrentCounter.QuadPart - llCounter.QuadPart) 
								/ (llCounterFreq.QuadPart / 1000);
		}

		m_xCS.enter();

		if (dwLastQueueStamp >= _RTP_QUEUE_PROCESS_TIMEOUT)
		{
			// processing all sessions instances
			std::map<unsigned int, CRTPRecorderSession*>::iterator i = m_xSessionsByID.begin();
			for (;i != m_xSessionsByID.end();i++) 
			{
				CRTPRecorderSession *pSession = i->second;
				pSession->ProcessReceivedPakets(&m_xStreamer, dwLastQueueStamp);
			}
			dwLastQueueStamp = 0;
		}
		
		m_xStreamer.Flush(iRealWaitTimeMS);

		m_xCS.leave();

		dwLastQueueStamp += iRealWaitTimeMS;				
		dwWaitTimeout = _RTP_JITTER_PACKET;

		llCounter.QuadPart = llCurrentCounter.QuadPart;
		dwWaitResult = ::WaitForSingleObject(m_hStopEvent, dwWaitTimeout);
	}

	::timeEndPeriod(timeCaps.wPeriodMin);
}

// 
void CRTPRecorder::OnNewCall(const std::string &call_id)
{
}

// create new session callback
void CRTPRecorder::OnNewSession(unsigned int session_id, const CRTPSessionParam &params)
{
	scope_locker sl(m_xCS);
	CreateSession(session_id, params);
}

// creating new session and add it to sessions map
void CRTPRecorder::CreateSession(unsigned int sess_id, const CRTPSessionParam &p)
{
	std::map<unsigned int, CRTPRecorderSession*>::iterator it = m_xSessionsByID.find(sess_id);

	if (it == m_xSessionsByID.end())
	{

#ifdef _DEBUG_LOG
		_LOG("Create RTP session " << IntToHex(sess_id, 8));
		p.Dump();
#endif

		// create new session
		CRTPRecorderSession *pSession = new CRTPRecorderSession(sess_id, p);

		m_xSessionsByID[sess_id] = pSession;

		// add to accepted ip map
		unsigned __int64 uIpPort1 = p.GetDst().IP();
		uIpPort1 <<= 16;
		uIpPort1 |= p.GetDst().Port();

		unsigned __int64 uIpPort2 = p.GetDst().IP();
		uIpPort2 <<= 16;
		uIpPort2 |= htons(ntohs(p.GetDst().Port()) + 1);
		
		m_xDstIpPorts.insert(std::pair<unsigned __int64, unsigned int>(uIpPort1, sess_id));
		m_xDstIpPorts.insert(std::pair<unsigned __int64, unsigned int>(uIpPort2, sess_id));
	}
}

// delete session
void CRTPRecorder::DeleteSession(unsigned int sess_id)
{
	std::map<unsigned int, CRTPRecorderSession*>::iterator it = m_xSessionsByID.find(sess_id);

	if (it != m_xSessionsByID.end())
	{
		_LOG("Delete RTP session " << IntToHex(sess_id, 8));

		CRTPRecorderSession *pSession = it->second;
		m_xSessionsByID.erase(it);
		
		CIPv4Address dst = pSession->GetParams().GetDst();
		DeleteIpFromMap(dst, sess_id);
		dst.Port(htons(ntohs(dst.Port()) + 1));
		DeleteIpFromMap(dst, sess_id);

		delete pSession;
	}
}

// delete listen IP:port from map
void CRTPRecorder::DeleteIpFromMap(const CIPv4Address &ip, unsigned int sess_id)
{
	unsigned __int64 uIpPort = ip.IP();
	uIpPort <<= 16;
	uIpPort |= ip.Port();

	std::multimap<unsigned __int64, unsigned int>::iterator ipIt = m_xDstIpPorts.find(uIpPort);
	if (ipIt != m_xDstIpPorts.end())
	{
		std::multimap<unsigned __int64, unsigned int>::iterator last;
		last = m_xDstIpPorts.upper_bound(uIpPort);
		do 
		{
			if (ipIt->second == sess_id)
			{
				m_xDstIpPorts.erase(ipIt);
				break;
			}
			ipIt++;
		} while(ipIt != last);
	}
}

// set "recording" flag of session
void CRTPRecorder::StartRecord(unsigned int sess_id, IDataStream *pStream)
{
	scope_locker sl(m_xCS);
	std::map<unsigned int, CRTPRecorderSession*>::iterator it = m_xSessionsByID.find(sess_id);
	if (it != m_xSessionsByID.end())
	{
		m_xStreamer.BindStream(pStream, sess_id);
		it->second->StartRecord();
	}
}

// unset "recording" flag
void CRTPRecorder::StopRecord(unsigned int sess_id)
{
	scope_locker sl(m_xCS);
	std::map<unsigned int, CRTPRecorderSession*>::iterator it = m_xSessionsByID.find(sess_id);
	if (it != m_xSessionsByID.end())
	{
		it->second->StopRecord();
		m_xStreamer.UnBindStream(sess_id);
	}
}

// modify session params callback
void CRTPRecorder::OnSessionModify(unsigned int session_id, const CRTPSessionParam &params)
{
	scope_locker sl(m_xCS);
	std::map<unsigned int, CRTPRecorderSession*>::iterator it = m_xSessionsByID.find(session_id);

	if (it != m_xSessionsByID.end())
	{
		// save recording status
		bool recording = it->second->IsRecording();
		DeleteSession(session_id);
		CreateSession(session_id, params);

		// restore recording status
		if (recording)
			m_xSessionsByID[session_id]->StartRecord();
	}
}

// remove session
void CRTPRecorder::OnStopSession(unsigned int sess_id)
{	
	scope_locker sl(m_xCS);
	StopRecord(sess_id);
	DeleteSession(sess_id);
}

void CRTPRecorder::OnEndCall(const std::string &call_id)
{
}
