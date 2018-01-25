// Header: SimplyThread.h
//////////////////////////////////////////////////////////////////////////
#pragma once

//////////////////////////////////////////////////////////////////////////
// Class: CSimplyThread
// Encapsulate a system Thread object
//////////////////////////////////////////////////////////////////////////
class CSimplyThread
{
public:
	// Constructor: CSimplyThread
	// Default constructor.
	CSimplyThread();

	// Destructor: ~CSimplyThread
	// Default destructor.
	virtual ~CSimplyThread();
	
	// Method: Stop
	// Stop started thread
	virtual void Stop();

	// Method: Start
	// Start the thread
	virtual void Start();

	// return true if success
	bool WaitForStart(DWORD dwTimeout = INFINITE);

	bool WaitForEnd(DWORD dwTimeout = INFINITE);

	// set/get auto delete flag
	bool DeleteAfterEnd() const;
	void DeleteAfterEnd(bool bDelete);

	bool IsRunning();

protected:
	virtual void Run() = 0;

protected:
	// thread routine
	static DWORD WINAPI _RunThreadItem(LPVOID pParams);
	// running flag
	LONG	m_lRunning;
	// thread handle
	HANDLE	m_hThreadHandle;
	// manual reset state event handler
	HANDLE	m_hStateEvent;
	// thread id
	DWORD	m_dwThreadID;
	// if flag is set then thread object will be auto killed
	bool	m_bDeleteAfterEnd;
};
