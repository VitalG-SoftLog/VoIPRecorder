#include "StdAfx.h"
#include "MessageQueue.h"

CMessageQueue::CMessageQueue(void)
{
	m_hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
}

CMessageQueue::~CMessageQueue(void)
{
	::CloseHandle(m_hEvent);
}


void CMessageQueue::Clear()
{
	m_xQueueLocker.enter();

	while(!m_xQueue.empty())
	{
		m_xQueue.pop();
	}

	m_xQueueLocker.leave();
}

void CMessageQueue::Push(int iSerial, int iEvent, int iData)
{
	m_xQueueLocker.enter();
	TNotificationMessage msg = {iSerial, iEvent, iData};
	m_xQueue.push(msg);
	m_xQueueLocker.leave();
	::SetEvent(m_hEvent);
}

bool CMessageQueue::Pop(TNotificationMessage &msg)
{
	bool bReturn = false;
	m_xQueueLocker.enter();

	if (!m_xQueue.empty())
	{
		msg = m_xQueue.front();
		m_xQueue.pop();
		bReturn = true;
	}

	m_xQueueLocker.leave();
	return bReturn;
}
