#include "StdAfx.h"
#include "SimplyThread.h"
#include <crtdbg.h>

#ifdef _CRASH_REPORT
#include <CrashRpt.h>
#endif//_CRASH_REPORT

CSimplyThread::CSimplyThread()
{
	m_hThreadHandle = NULL;
	m_dwThreadID = 0;
	m_bDeleteAfterEnd = false;
	m_hStateEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	_ASSERT(m_hStateEvent);
}

CSimplyThread::~CSimplyThread()
{
	if (m_hStateEvent)
	{
		::CloseHandle(m_hStateEvent);
		m_hStateEvent = NULL;
	}

	if (m_hThreadHandle)
	{
		::CloseHandle(m_hThreadHandle);
		m_hThreadHandle = NULL;
	}
}


void CSimplyThread::Stop()
{
	if (IsRunning())
	{
		::ResetEvent(m_hStateEvent);
	}
}

void CSimplyThread::Start()
{
	// if open close old thread handle

	if (!IsRunning())
	{
		if (m_hThreadHandle)
		{
			::CloseHandle(m_hThreadHandle);
			m_hThreadHandle = NULL;
		}
		m_hThreadHandle = ::CreateThread( 
			NULL,              // default security attributes
			0,                 // use default stack size  
			_RunThreadItem,    // thread function 
			this,              // argument to thread function 
			0,                 // use default creation flags 
			&m_dwThreadID);    // returns the thread identifier 
	}
}

bool CSimplyThread::WaitForStart(DWORD dwTimeout/* = INFINITE*/)
{
	if (m_hStateEvent)
	{
		if (::WaitForSingleObject(m_hStateEvent, dwTimeout) == WAIT_OBJECT_0)
			return true;
	}

	return false;
}

bool CSimplyThread::WaitForEnd(DWORD dwTimeout/* = INFINITE*/)
{
	if (m_hThreadHandle)
	{
		if (::WaitForSingleObject(m_hThreadHandle, dwTimeout) == WAIT_OBJECT_0)
			return true;
	}

	return false;
}

bool CSimplyThread::DeleteAfterEnd() const
{
	return m_bDeleteAfterEnd;
}

void CSimplyThread::DeleteAfterEnd(bool bDelete)
{
	m_bDeleteAfterEnd = bDelete;
}


bool CSimplyThread::IsRunning()
{
	return (::WaitForSingleObject(m_hStateEvent, 0) == WAIT_OBJECT_0);
}

DWORD WINAPI CSimplyThread::_RunThreadItem(LPVOID pParams)
{
	CSimplyThread *t = (CSimplyThread *)pParams;

#ifdef _CRASH_REPORT
	crInstallToCurrentThread2(0);
#endif//_CRASH_REPORT

	// setting run event
	::SetEvent(t->m_hStateEvent);

	// run real thread routine
	t->Run();
	
	if (t->IsRunning())
		::ResetEvent(t->m_hStateEvent);

	if (t->DeleteAfterEnd())
		delete t;

#ifdef _CRASH_REPORT
	crUninstallFromCurrentThread();
#endif//_CRASH_REPORT

	return 0;
}


