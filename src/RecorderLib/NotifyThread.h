// Header: NotifyThread.h
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "simplythread.h"
#include "MultiXtR.h"
#include "MessageQueue.h"

//////////////////////////////////////////////////////////////////////////
// Class: CNotifyThread
// Fetch message from queue and call registered callback.
//////////////////////////////////////////////////////////////////////////
class CNotifyThread : public CSimplyThread
{
public:
	// Constructor: CNotifyThread
	// Default constructor.
	//
	// Parameters:
	// queue - [in] pointer to <CMessageQueue> object.
	CNotifyThread(CMessageQueue *queue);

	// Destructor: 
	// Default destructor.
	virtual ~CNotifyThread();

	// Method: Stop
	// Stop notification thread.
	virtual void Stop();

	// Method: SetCallBack
	// Sets a callback which will be used by the library to identify significant 
	// events taking place on the telephone lines.
	//
	// Parameters:
	// call_back - [in] Callback function. call_back may be NULL, which disables the 
	// further sending of callbacks.
	void SetCallBack(MULTIXTRCALLBACK *call_back);

protected:
	virtual void		Run();

	MULTIXTRCALLBACK	*m_pCallBack;
	CMessageQueue		*m_pMessageQueue;
	HANDLE				m_hStopEvent;
};
