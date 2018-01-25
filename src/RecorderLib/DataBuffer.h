//////////////////////////////////////////////////////////////////////////
// Header: DataBuffer.h
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "BaseTypes.h"

// Define: _ALLOC_BUCKET_SIZE
// default memory grown size
#define _ALLOC_BUCKET_SIZE 512

//////////////////////////////////////////////////////////////////////////
// Class: CDataBuffer
// Simple data buffer class automatically allocate ad reallocate memory
//////////////////////////////////////////////////////////////////////////
class CDataBuffer
{
public:
	
	// Constructor: CDataBuffer
	// Default constructor	
	CDataBuffer();
	
	// Constructor: CDataBuffer
	// allocate buffer	
	CDataBuffer(int size);

	// Constructor: CDataBuffer
	// Copy constructor	
	CDataBuffer(const CDataBuffer &ab);
	
	// Destructor: ~CDataBuffer
	// Default destructor
	virtual ~CDataBuffer(void);

	
	// Method: AddData
	// Add data chunk to buffer
	//
	// Parameters:
	// data - pointer to data
	// size - size of data
	void	AddData(const uint8 *data, uint32 size);

	
	// Method: Reset
	// Delete buffer and reset all states
	void	Reset();

	
	// Method: GetDataSize
	// Size of stored data
	//
	// Returns:
	// Return size of stored data	
	uint32	GetDataSize() const;

	// Operator: LPBYTE
	// Return pointer of LPBYTE type
	operator LPBYTE() {return m_pData;}

	// Operator: LPCBYTE
	// Return pointer of const LPBYTE type
	operator LPCBYTE() const {return m_pData;}

	// Operator: LPVOID
	// Return pointer of LPVOID type
	operator LPVOID() {return m_pData;}

	// Operator: LPCVOID
	// Return pointer of const LPVOID type
	operator LPCVOID() const {return m_pData;}

	// Operator: []
	// Return data byte by index
	uint8 operator [](uint32 index) const;

	// Operator: =
	// Assign data to object
	CDataBuffer &operator=(const CDataBuffer &ab);

	// Operator: +=
	// Add data to object data
	CDataBuffer &operator+=(const CDataBuffer &ab);

	// Operator: +=
	// Add 16 bit integer data value to buffer
	CDataBuffer &operator+=(const int16 data);

	// Operator: <<
	// Add 16 bit integer data value to buffer
	CDataBuffer &operator<<(const int16 data) {return operator+=(data);}

protected:
	uint32	m_uRealSize;
	uint32	m_uDataSize;
	uint8	*m_pData;
};
