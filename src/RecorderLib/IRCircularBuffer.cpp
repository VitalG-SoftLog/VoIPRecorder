#include "StdAfx.h"
#include "IRCircularBuffer.h"

_MODULE_NAME(CIRCircularBuffer)

const int IRD_DEFAULTSIZE = 8192;

CIRCircularBuffer::CIRCircularBuffer()
{
	m_Size = IRD_DEFAULTSIZE;
	m_Count = 0;
	m_pData = new BYTE[m_Size];
	m_pGetPtr = m_pPutPtr = m_pData;
	m_pOverrun = (typeOVERRUNHANDLER)-1;
	m_pOverrun2 = (typeOVERRUNHANDLER2)-1;
	m_pOverrunData = NULL;	
}

CIRCircularBuffer::CIRCircularBuffer(int Size)
{
	m_Size = Size;
	m_Count = 0;
	m_pData = new BYTE[ m_Size ];
	m_pGetPtr = m_pPutPtr = m_pData;
	m_pOverrun = (typeOVERRUNHANDLER)-1;
	m_pOverrun2 = (typeOVERRUNHANDLER2)-1;
	m_pOverrunData = NULL;	
}

CIRCircularBuffer::~CIRCircularBuffer()
{
	delete [] m_pData;
}

void CIRCircularBuffer::AddData(const BYTE* pData, int Length)
{
	if(pData != NULL && Length > 0)
	{
		int	Oversize = Length+GetDataLength()-m_Size;

		if( Oversize > 0 )
		{
			// uh oh. Data overrun
			Overrun( Oversize );
			// Now its just possible the overrun handler has taken
			// some data from the buffer, so lets check again
			Oversize = Length+GetDataLength()-m_Size;

			if( Oversize > 0 )
			{
				if( Length <= m_Size )
				{
					// Delete enough data to make room
					GetData( NULL, Oversize );
				}
				else
				{
					// Quite a problem. 
					// More than a bufferload in one write
					// Delete all but the end
					pData += (Length-m_Size);
					Length = m_Size;
					m_pGetPtr = m_pPutPtr = m_pData;
					m_Count = 0;
				}
			}
		}
		// Will it all fit in one go?
		int SpaceToEnd = static_cast<int>(m_pData+m_Size-m_pPutPtr);

		if( SpaceToEnd < Length )
		{
			// Bugger, not enough room. Do the first bit.
			CopyMemory( m_pPutPtr, pData, SpaceToEnd );
			Length -= SpaceToEnd;
			pData += SpaceToEnd;
			m_Count += SpaceToEnd;
			m_pPutPtr = m_pData;	// (Must have just wrapped to start)
		}
		
		// Last bit
		CopyMemory( m_pPutPtr, pData, Length );
		
		m_pPutPtr += Length;
		m_Count += Length;

		if( m_pPutPtr >= m_pData+m_Size )
		{
			m_pPutPtr = m_pData;	// Just wrapped.
		}	
	}
}

int CIRCircularBuffer::GetData(BYTE* pPut, int MaxLength, BOOL Peek )
{
	int	ReturnCount = 0;
	
	// Special case. Don't really want any data
	if( !MaxLength )
	{
		ReturnCount = GetDataLength();
	}
	else
	{		
		const BYTE*pTempGetPtr = m_pGetPtr;
		
		int Length = GetDataLength();
		
		if( Length > MaxLength )
		{
			Length = MaxLength;
		}

		// Can we do it all in one go.
		int DataToEnd = static_cast<int>(m_pData+m_Size-pTempGetPtr);

		if( DataToEnd < Length )
		{
			// Bugger, too much data. Do the first bit
			if( pPut && DataToEnd > 0)
			{
				CopyMemory( pPut, pTempGetPtr, DataToEnd );
				pPut += DataToEnd;
				ReturnCount += DataToEnd;
			}

			Length -= DataToEnd;
			pTempGetPtr = m_pData;	// (Must have just wrapped to start)
			
			if( !Peek )
			{
				m_Count -= DataToEnd;
			}
		}

		// Last bit.
		if( pPut && Length > 0) 
		{
			CopyMemory( pPut, pTempGetPtr, Length );
			ReturnCount += Length;
		}

		pTempGetPtr += Length;

		if( pTempGetPtr >= m_pData+m_Size ) 
		{
			pTempGetPtr = m_pData;	// Just wrapped
		}

		if( !Peek ) 
		{
			m_pGetPtr = pTempGetPtr;
			m_Count -= Length;
		}		
	}	

	return ReturnCount;
}

int CIRCircularBuffer::GetDataLength()
{
	return m_Count;
}

void CIRCircularBuffer::SetBufferLength( int NewLength )
{	
	// Make a new buffer
	BYTE*	pNewData = new BYTE [NewLength];

	// Copy all the old data into it.
	// We're only interested in current data, not the buffer length
	int OldLength = GetDataLength();

	// Check there is room
	int	ExcessData = OldLength - NewLength;

	if( ExcessData > 0 )
	{
		Overrun( ExcessData );
	
		// Check again in case overrun handler deleted any
		OldLength = GetDataLength();
		ExcessData = OldLength - NewLength;

		if( ExcessData > 0 )
		{
			// Read (and erase) excess data
			GetData( NULL, ExcessData );
			OldLength = NewLength;	// cos we just got rid of the excess
		}
	}

	// Read all the old data into the new buffer
	GetData( pNewData, OldLength );

	// Trash the old data
	delete [] m_pData;

	// Set the pointers to the new data
	m_pPutPtr = m_pData = pNewData;
	m_pGetPtr = m_pData + OldLength;
	m_Count = OldLength;
	m_Size = NewLength;	
}

int CIRCircularBuffer::GetBufferLength()
{
	return m_Size;
}

void CIRCircularBuffer::SetOverrunHandler( typeOVERRUNHANDLER2 pHandler, void *pData )
{
	m_pOverrun = NULL;
	m_pOverrun2 = pHandler;
	m_pOverrunData = pData;
}

void CIRCircularBuffer::SetOverrunHandler( typeOVERRUNHANDLER pHandler )
{
	m_pOverrun = pHandler;
	m_pOverrun2 = NULL;
	m_pOverrunData = NULL;
}

void CIRCircularBuffer::Overrun(int Length)
{
	// _LOG("Buffer Overrun!");
	if( m_pOverrun == (typeOVERRUNHANDLER)-1 )
	{
		// Too much data! Your CircularBuffer has overrun!
		// If you want to allow this to happen without warning,...
		// ...call SetOverrunHandler(NULL)		
	}
	else if( m_pOverrun!=NULL )	 
	{
		(*m_pOverrun)(Length);	// User action on overrun, NO data
	}
	else if( m_pOverrun2!=NULL ) 
	{
		(*m_pOverrun2)(Length,m_pOverrunData);	// User action on overrun, with data
	}
}
