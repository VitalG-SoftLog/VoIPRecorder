#include "StdAfx.h"
#include "NotifyThread.h"

CNotifyThread::CNotifyThread(CMessageQueue *queue)
: CSimplyThread()
{
	m_pMessageQueue = queue;
	m_pCallBack		= NULL;
	m_hStopEvent	= ::CreateEvent(NULL, FALSE, FALSE, NULL);
}

CNotifyThread::~CNotifyThread()
{
	::CloseHandle(m_hStopEvent);
}

void CNotifyThread::SetCallBack(MULTIXTRCALLBACK *call_back)
{
	m_pCallBack = call_back;
}

void CNotifyThread::Stop()
{
	::SetEvent(m_hStopEvent);
	CSimplyThread::Stop();
}


void CNotifyThread::Run()
{
	HANDLE handles[2] = {0};

	handles[0] = m_pMessageQueue->GetEvent();
	handles[1] = m_hStopEvent;
	
	while (IsRunning())
	{
		DWORD dwWaitResult = ::WaitForMultipleObjects(2, handles, FALSE, INFINITE);

		if (dwWaitResult == WAIT_OBJECT_0 + 0)
		{// queue event signaled
			TNotificationMessage msg;

			// processing all messages in queue
			while (m_pMessageQueue->Pop(msg))
			{
				if (m_pCallBack != NULL)
				{
					(*m_pCallBack)(msg.iSerialNumber, msg.iEvent, msg.iData);
				}
			}
		}
		else if (dwWaitResult == WAIT_OBJECT_0 + 1)
		{// stop event signaled
			break;// break loop
		}
		else
		{// error
			break;// break loop
		}
	}
}
