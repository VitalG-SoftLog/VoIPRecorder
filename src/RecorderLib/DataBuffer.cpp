#include "StdAfx.h"
#include "DataBuffer.h"

CDataBuffer::CDataBuffer(void)
{
	m_uRealSize = 0;
	m_uDataSize	= 0;
	m_pData = NULL;
}

CDataBuffer::CDataBuffer(int size)
{
	m_uRealSize = size;
	m_uDataSize	= size;
	m_pData = new uint8[m_uRealSize];
}

CDataBuffer::CDataBuffer(const CDataBuffer &b)
{
	m_uRealSize = 0;
	m_uDataSize	= 0;
	m_pData = NULL;
	operator=(b);
}

CDataBuffer::~CDataBuffer(void)
{
	Reset();
}

void CDataBuffer::AddData(const uint8 *data, uint32 size)
{
	if (!data || !size)
		return;

	uint32 uLeftSpace = m_uRealSize - m_uDataSize;

	if (uLeftSpace < size)
	{// reallocate

		uint32 needed_space = size - uLeftSpace;
		needed_space = 
			((needed_space / _ALLOC_BUCKET_SIZE) * _ALLOC_BUCKET_SIZE) + _ALLOC_BUCKET_SIZE;

		m_uRealSize += needed_space;

		uint8 *pNewArray = new uint8[m_uRealSize];
		memcpy(pNewArray, m_pData, m_uDataSize);
		delete [] m_pData;
		m_pData = pNewArray;
	}
	
	//copy data
	memcpy(m_pData + m_uDataSize, data, size);
	m_uDataSize += size;
}

void CDataBuffer::Reset()
{
	if (m_pData)
	{
		delete [] m_pData;
		m_pData = NULL;
	}
	m_uRealSize = 0;
	m_uDataSize	= 0;
}

uint8 CDataBuffer::operator [](uint32 index) const
{
	if (index < m_uDataSize)
		return m_pData[index];

	return 0;
}

CDataBuffer &CDataBuffer::operator=(const CDataBuffer &ab)
{
	Reset();
	AddData((LPCBYTE)ab, ab.m_uDataSize);
	return *this;
}

CDataBuffer &CDataBuffer::operator+=(const CDataBuffer &ab)
{
	AddData((LPCBYTE)ab, ab.GetDataSize());
	return *this;
}

CDataBuffer &CDataBuffer::operator+=(const int16 data)
{
	AddData((const uint8*)&data, sizeof(int16));
	return *this;
}

uint32 CDataBuffer::GetDataSize() const
{
	return m_uDataSize;
}

