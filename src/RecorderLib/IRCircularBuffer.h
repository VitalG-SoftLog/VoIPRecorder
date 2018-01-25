#ifndef _IRCIRCULARBUFFER_H
#define _IRCIRCULARBUFFER_H

typedef	void (*typeOVERRUNHANDLER)(int Length);
typedef	void (*typeOVERRUNHANDLER2)(int Length,void *pUserData);

class CIRCircularBuffer  
{
public:
	CIRCircularBuffer();
	CIRCircularBuffer(int Size);
	virtual ~CIRCircularBuffer();

	// Add data to buffer
	void AddData(const BYTE* pData, int Length);	
		
	// Find how much data currently available
	int GetDataLength( void );	

	// Get data from buffer (Peek=TRUE does not delete it)
	int GetData( BYTE* pPut, int MaxLength, BOOL Peek = FALSE );	
	
	void SetBufferLength( int NewLength );
	int  GetBufferLength();

	void SetOverrunHandler( typeOVERRUNHANDLER pHandler);

	// pUserData is any user supplied data, returned to you on overrun
	void SetOverrunHandler( typeOVERRUNHANDLER2 pHandler, void *pUserData );	

private:	

	BYTE*       m_pData;
	int	        m_Size;
	int	        m_Count;
	const BYTE* m_pGetPtr;
	BYTE*       m_pPutPtr;

	void Overrun( int Length );
	void* m_pOverrunData;

	typeOVERRUNHANDLER m_pOverrun;
	typeOVERRUNHANDLER2 m_pOverrun2;
};

#endif
