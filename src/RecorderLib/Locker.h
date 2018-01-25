// Header: Locker.h
//////////////////////////////////////////////////////////////////////////
#pragma once

//////////////////////////////////////////////////////////////////////////
// Class: critical_section
// Encapsulate a system CRITICAL_SECTION object
//////////////////////////////////////////////////////////////////////////
class critical_section
{
public:
	// Constructor: critical_section
	// Default constructor.
	critical_section() {
		::InitializeCriticalSection(&m_lpCS);
	}

	// Destructor: ~critical_section
	// Default destructor.
	virtual ~critical_section() {
		::DeleteCriticalSection(&m_lpCS);
	}

	// Method: enter
	// Enter critical section
	void enter() {
		::EnterCriticalSection(&m_lpCS);
	}

	// Method: leave
	// Leave critical section
	void leave() {
		::LeaveCriticalSection(&m_lpCS);
	}

protected:
	CRITICAL_SECTION m_lpCS;
};


//////////////////////////////////////////////////////////////////////////
// Class: mutex
// Encapsulate a system MUTEX object
//////////////////////////////////////////////////////////////////////////
class mutex
{
public:
	// Constructor: mutex
	// Default constructor.
	mutex() {
		m_hMutex = ::CreateMutex(NULL, FALSE, NULL);
	}

	// Destructor: ~mutex
	// Default destructor.
	virtual ~mutex() {
		::CloseHandle(m_hMutex);
	}

	// Method: lock
	// Lock the mutex.
	void lock() {
		wait();
	}

	// Method: unlock
	// Release mutex.
	void unlock() {
		::ReleaseMutex(m_hMutex);
	}

	// Method: wait
	// Wait and lock mutex
	//
	// Parameters:
	// timeout - [in] wait timeout in ms. INFINITE by default.
	void wait(unsigned int timeout = INFINITE) {
		::WaitForSingleObject(m_hMutex, timeout);
	}

protected:
	HANDLE m_hMutex;
};


//////////////////////////////////////////////////////////////////////////
// Class: scope_locker
// Lock <critical_section> when created and unlock when instance deleted
//////////////////////////////////////////////////////////////////////////
class scope_locker
{
public:
	// Constructor: scope_locker
	// Default constructor.
	//
	// Parameters:
	// pCS - reference to <critical_section> object.
	scope_locker(critical_section &pCS) {
		m_pCS = &pCS; 
		m_pCS->enter();
	}

	// Destructor: ~scope_locker
	// Default destructor.
	virtual ~scope_locker() {
		m_pCS->leave();
	}

protected:
	critical_section *m_pCS;
};