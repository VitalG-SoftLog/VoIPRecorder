#pragma once
#include "interfaces.h"
#include "IRCircularBuffer.h"
#include <hash_map>

#define _G722_MODE_64 1
#define _G722_MODE_56 2
#define _G722_MODE_48 3

#define _G722_OPERATION_MODE _G722_MODE_64

class CStreamer : public IStreamer
{
public:
	CStreamer();
	virtual ~CStreamer();

	// return number of streamed samples
	virtual unsigned int OnPayloadData(uint32 session_id, uint32 payload_type,
		void *payload_data, uint32 payload_size, uint32 silence_samples);

	// flush streams buffers
	virtual void Flush(int iMS);

	// create/remove streams
	virtual void BindStream(IDataStream *pStream, uint32 session_id);
	virtual void UnBindStream(uint32 session_id);

protected:

	struct TStreamStruct {
		uint32				uID;
		IDataStream			*pStream;
		CIRCircularBuffer	*pJBuffer;
		void				*pCTX;
	};

	typedef stdext::hash_map<uint32, TStreamStruct>			id2stream_map;
/*
	typedef stdext::hash_map<uint32, CIRCircularBuffer*>	id2buffers_map;
	typedef stdext::hash_map<uint32, IDataStream*>			id2stream_map;
	typedef stdext::hash_map<uint32, void*>					id2ctx_map;
*/
	void	AddSilenceToBuffer(CIRCircularBuffer *stream, int silence_samples);
	void	AddDataToBuffer(CIRCircularBuffer *stream, void *data, int size, int format);
/*
	void	AddSilenceToStream(CIRCircularBuffer *stream, int silence_samples);
	void	AddDataToStream(CIRCircularBuffer *stream, void *data, int size, int format);
*/
	// data stream id ==> data stream object
	id2stream_map	m_xStreams;
	
	// tmp buffer
	int16			*m_pPCMBuffer;
	int				m_iPCMBufferSize;

	int				Resample16KhzBuffer(int samples);	// resampling 16 khz to 8
	void			CompressPcmBuffer(int samples);		// compress out buffer
};
