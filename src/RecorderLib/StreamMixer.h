#pragma once

#include <list>
#include "Interfaces.h"
#include "DataBuffer.h"
#include "g711.h"
#include "Locker.h"
#include "IRCircularBuffer.h"

#define _BUFFER_MS	100

class CStreamMixer
{
	friend class CMixerInputStream;
public:
	CStreamMixer();
	virtual ~CStreamMixer(void);

	IDataStream *CreateInput(uint32 uID = 0);

	void DeleteInput(uint32 uID);
	void DeleteInput(IDataStream *pStream);
	IDataStream *GetInput(uint32 uID);

	void SetOutStream(IDataStream *stream);

protected:
	uint8	*m_pMixedBuffer;
	uint8	*m_pCurrentBuffer;
	bool	IsAllStreamsProcessed();
	void	SetAllStreamsProcessed(bool bProcessed);

	virtual void Poll(int iOverrun = 0);
	
	// mixing two buffers
	void MixPcmBuffers(int iDataLen);
	std::list<IDataStream*>	 m_xInputStreams;
	IDataStream				*m_pOutStream;

	// locker for containers
	critical_section		m_xCS;
};

class CMixerInputStream : public IDataStream
{
public:
	CMixerInputStream(CStreamMixer *parent);
	virtual ~CMixerInputStream();
	CIRCircularBuffer *GetData();

	// IDataStream
	virtual void	SetID(uint32 uID) {m_uStreamID = uID;}
	virtual uint32	GetID() {return m_uStreamID;}
	virtual void	AddData(void *data, uint32 size, uint32 type);
	virtual void	Flush();
	bool			IsProcessed();
	void			Processed(bool bProcessed);

protected:
	static void StaticOverrunHandler(int Length, void *pUserData);
	void OverrunHandler(int Length);

	uint32				m_uStreamID;
	CIRCircularBuffer	*m_pBuffer;
	CStreamMixer		*m_pParent;
	bool				m_bIsProcessed;
};
