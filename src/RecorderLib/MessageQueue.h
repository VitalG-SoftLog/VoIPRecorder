// Header: MessageQueue.h
//////////////////////////////////////////////////////////////////////////
#pragma once

#include <queue>
#include "Locker.h"

//////////////////////////////////////////////////////////////////////////
// Struct: TNotificationMessage
//////////////////////////////////////////////////////////////////////////
struct TNotificationMessage
{
	// Variable: iSerialNumber
	// Port serial number.
	int iSerialNumber;

	// Variable: iEvent
	// Event ID.
	int iEvent;

	// Variable: iData
	// Event data.
	int iData;
};

//////////////////////////////////////////////////////////////////////////
// Class: CMessageQueue
// Thread safe notification message queue
//////////////////////////////////////////////////////////////////////////
class CMessageQueue
{
public:
	// Constructor: CMessageQueue
	// Default constructor.
	CMessageQueue();

	// Destructor: CMessageQueue
	// Default destructor.
	virtual ~CMessageQueue();
	
	// Method: Push
	// Push message to queue
	//
	// Parameters:
	// iSerial - [in] Port serial
	// iEvent - [in] Event
	// iData - [in] Message data
	//
	void Push(int iSerial, int iEvent, int iData);

	// Method: Pop
	// Pop notification message from queue
	//
	// Parameters:
	// msg - [out] Reference to <TNotificationMessage> struct
	//
	// Returns:
	// return true if message dequeue, or false if queue is empty
	bool Pop(TNotificationMessage &msg);

	// Method: Clear
	// Delete all messages from queue
	void Clear();

	// Method: GetEvent
	// Return handle to event what signaled when new data pushed
	//
	// Returns:
	// HANDLE to Event object
	HANDLE GetEvent() {return m_hEvent;}

protected:
	HANDLE m_hEvent;
	std::queue<TNotificationMessage> m_xQueue;
	critical_section m_xQueueLocker;
};
