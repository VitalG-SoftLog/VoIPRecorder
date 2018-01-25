#include "StdAfx.h"
#include "StreamMixer.h"
#include "Settings.h"
#include "g711.h"
#include <algorithm>
#include "StringUtils.h"

_MODULE_NAME(CStreamMixer)

//////////////////////////////////////////////////////////////////////////
// CMixerInputStream class
//////////////////////////////////////////////////////////////////////////
CMixerInputStream::CMixerInputStream(CStreamMixer *parent)
{
	m_uStreamID = 0;
	m_pParent = parent;
	m_pBuffer = new CIRCircularBuffer((_BUFFER_MS * 8));
	m_pBuffer->SetOverrunHandler(StaticOverrunHandler, this);
	m_bIsProcessed = false;
}

CMixerInputStream::~CMixerInputStream()
{
	delete m_pBuffer;
}

bool CMixerInputStream::IsProcessed() {
	return m_bIsProcessed;
}

void CMixerInputStream::Processed(bool bProcessed) {
	m_bIsProcessed = bProcessed;
}

void CMixerInputStream::StaticOverrunHandler(int Length, void *pUserData)
{
	if (pUserData)
	{
		CMixerInputStream *pThis = (CMixerInputStream *)pUserData;
		pThis->OverrunHandler(Length);
	}
}

void CMixerInputStream::OverrunHandler(int Length)
{
/*
	Processed(true);
	if (m_pParent)
	{
		m_pParent->Poll(Length);
	}
*/
}

// IDataStream
void CMixerInputStream::AddData(void *data, uint32 size, uint32 type)
{
	if (m_pParent)
	{
		m_pBuffer->AddData((uint8 *)data, size);
/*
		Processed(true);
		m_pParent->Poll();
*/
	}
}

void CMixerInputStream::Flush()
{

	Processed(true);
	if (m_pParent)
	{
		m_pParent->Poll();
	}
}

CIRCircularBuffer *CMixerInputStream::GetData() {return m_pBuffer;}


//////////////////////////////////////////////////////////////////////////
// CStreamMixer class
//////////////////////////////////////////////////////////////////////////
CStreamMixer::CStreamMixer()
{
	m_pMixedBuffer		= new uint8[(_BUFFER_MS * 8)];
	m_pCurrentBuffer	= new uint8[(_BUFFER_MS * 8)];
}

CStreamMixer::~CStreamMixer(void)
{
	std::list<IDataStream*>::iterator i = m_xInputStreams.begin();
	for (;i != m_xInputStreams.end(); i++)
		delete (*i);

	m_xInputStreams.clear();

	delete [] m_pMixedBuffer;
	delete [] m_pCurrentBuffer;
}

IDataStream *CStreamMixer::CreateInput(uint32 uID /*= 0*/)
{
	scope_locker sl(m_xCS);

	CMixerInputStream *s = new CMixerInputStream(this);
	s->SetID(uID);
	m_xInputStreams.push_back(s);
	return s;
}

void CStreamMixer::DeleteInput(uint32 uID)
{
	scope_locker sl(m_xCS);

	std::list<IDataStream*>::iterator i = m_xInputStreams.begin();
	for (;i != m_xInputStreams.end(); i++)
	{
		if ((*i)->GetID() == uID)
		{
			delete (*i);
			m_xInputStreams.erase(i);
			break;
		}
	}
}

void CStreamMixer::DeleteInput(IDataStream *pStream)
{
	scope_locker sl(m_xCS);

	std::list<IDataStream*>::iterator i = m_xInputStreams.begin();
	for (;i != m_xInputStreams.end(); i++)
	{
		if ((*i) == pStream)
		{
			delete (*i);
			m_xInputStreams.erase(i);
			break;
		}
	}
}

IDataStream *CStreamMixer::GetInput(uint32 uID)
{
	scope_locker sl(m_xCS);
	std::list<IDataStream*>::iterator i = m_xInputStreams.begin();
	for (;i != m_xInputStreams.end(); i++)
	{
		if ((*i)->GetID() == uID)
		{
			return (*i);
		}
	}
	return NULL;
}

void CStreamMixer::SetOutStream(IDataStream *stream)
{
	m_pOutStream = stream;
}

// mixing two linear PCM buffers
void CStreamMixer::MixPcmBuffers(int iDataLen)
{
	int samples			= iDataLen;
	if (Settings::iOutDataFormat == WAVE_FORMAT_ALAW)
	{
		// a + b - ((a * b) / 65535)
		for(int i = 0; i < samples; i++)
		{
			int16 a = alaw2linear(m_pCurrentBuffer[i]);
			int16 b = alaw2linear(m_pMixedBuffer[i]);
			m_pMixedBuffer[i] = linear2alaw(a + b - ((a * b) / 65535));
		}

	}
	else if (Settings::iOutDataFormat == WAVE_FORMAT_MULAW)
	{
		// a + b - ((a * b) / 65535)
		for(int i = 0; i < samples; i++)
		{
			int16 a = ulaw2linear(m_pCurrentBuffer[i]);
			int16 b = ulaw2linear(m_pMixedBuffer[i]);
			m_pMixedBuffer[i] = linear2ulaw(a + b - ((a * b) / 65535));
		}
	}
}

bool CStreamMixer::IsAllStreamsProcessed()
{
	std::list<IDataStream*>::iterator i = m_xInputStreams.begin();
	for (;i != m_xInputStreams.end(); i++)
	{
		CMixerInputStream *pStream = (CMixerInputStream *)(*i);
		if (!pStream->IsProcessed())
			return false;
	}

	return true;
}

void CStreamMixer::SetAllStreamsProcessed(bool bProcessed)
{
	std::list<IDataStream*>::iterator i = m_xInputStreams.begin();
	for (;i != m_xInputStreams.end(); i++)
	{
		CMixerInputStream *pStream = (CMixerInputStream *)(*i);
		pStream->Processed(bProcessed);
	}
}

// start mixing linear PCM data
void CStreamMixer::Poll(int iOverrun/* = 0*/)
{
	scope_locker sl(m_xCS);

	if (iOverrun == 0) {
		if (!IsAllStreamsProcessed())
			return;
	}

	CIRCircularBuffer *pBufferMin = NULL;
	CIRCircularBuffer *pBufferMax = NULL;

	int iMixedLen	= 0;
	int iMinLen		= 0;
	int iMaxLen		= 0;

	std::list<IDataStream*>::iterator i = m_xInputStreams.begin();

	// find smallest and largest buffers
	for (;i != m_xInputStreams.end(); i++)
	{
		CMixerInputStream *pStream = (CMixerInputStream *)(*i);

		if (pBufferMin == NULL) {
			pBufferMin = pStream->GetData();
			iMinLen = pStream->GetData()->GetDataLength();
		}

		if (pBufferMax == NULL) {
			pBufferMax = pStream->GetData();
			iMaxLen = pStream->GetData()->GetDataLength();
		}

		
		if (iMaxLen < pStream->GetData()->GetDataLength())
		{
			iMaxLen		= pStream->GetData()->GetDataLength();
			pBufferMax	= pStream->GetData();
		}

		if (iMinLen > pStream->GetData()->GetDataLength())
		{
			iMinLen		= pStream->GetData()->GetDataLength();
			pBufferMin	= pStream->GetData();
		}
	}
	
	CIRCircularBuffer *pBaseBuffer = NULL;

	if (iOverrun != 0)
	{
		if (iOverrun > iMinLen)	{
			pBaseBuffer = pBufferMax;
			if (iOverrun > iMaxLen) {
				iMixedLen = iMaxLen;
			} else {
				iMixedLen = iOverrun;
			}
		} else {
			iMixedLen = iMinLen;
			pBaseBuffer = pBufferMin;
		}
	}
	else
	{
		iMixedLen = iMinLen;
		pBaseBuffer = pBufferMin;
	}

	if (pBaseBuffer && iMixedLen)
	{
		pBaseBuffer->GetData((LPBYTE)m_pMixedBuffer, iMixedLen);
		// start mixing streams...
		i = m_xInputStreams.begin();
		for (;i != m_xInputStreams.end(); i++)
		{
			CIRCircularBuffer *pcrBuffer = ((CMixerInputStream *)(*i))->GetData();
			if (pcrBuffer != pBaseBuffer)
			{
				int iDataLen = pcrBuffer->GetDataLength();

				if (iDataLen > iMixedLen) {
					iDataLen = iMixedLen;
				}
				
				if (iDataLen > 0) {
					pcrBuffer->GetData((LPBYTE)m_pCurrentBuffer, iDataLen);
				}

				// padding...
				for (int i = iDataLen; i < iMixedLen; i++)
					m_pCurrentBuffer[i] = Settings::uSilenceSample;

				MixPcmBuffers(iMixedLen);
			}
		}
	}

	if (m_pOutStream && iMixedLen)
	{
		m_pOutStream->AddData(m_pMixedBuffer, iMixedLen, Settings::iOutDataFormat);
		m_pOutStream->Flush();
	}

	SetAllStreamsProcessed(false);
}
