#include "StdAfx.h"
#include "Streamer.h"
#include "g711.h"
#include "g722/g722.h"
#include <algorithm>
#include "Settings.h"


CStreamer::CStreamer(void)
{
	m_pPCMBuffer		= NULL;
	m_iPCMBufferSize	= 0;
}

CStreamer::~CStreamer(void)
{
	if (m_pPCMBuffer)
		delete [] m_pPCMBuffer;
	m_iPCMBufferSize = 0;
}

// return number of streamed samples
unsigned int CStreamer::OnPayloadData(uint32 session_id, uint32 payload_type,
									   void *payload_data, uint32 payload_size, 
									   uint32 silence_samples)
{
	CIRCircularBuffer	*jbuffer = NULL;

	id2stream_map::iterator it = m_xStreams.find(session_id);
	if (it != m_xStreams.end())
	{
		jbuffer = it->second.pJBuffer;
	}

	if (!jbuffer)
		return payload_size;

	if (payload_size)
	{
		if (m_pPCMBuffer == NULL)
		{
			// if need allocate new buffer for decoded data
			m_pPCMBuffer		= new int16[payload_size * 2];
			m_iPCMBufferSize	= payload_size * 2;
		}
		else if (m_iPCMBufferSize < (int32)payload_size * 2)
		{
			// if need reallocate the buffer
			delete [] m_pPCMBuffer;
			m_pPCMBuffer		= new int16[payload_size];
			m_iPCMBufferSize	= payload_size;
		}
	}

	switch(payload_type)
	{
	case(_RTP_PAYLOAD_TYPE_PCMU): // G711 ulaw
		{
			// add 16 bit PCM silence...
			AddSilenceToBuffer(jbuffer, silence_samples);
			if (payload_size)
			{
				if (Settings::iOutDataFormat == WAVE_FORMAT_MULAW) 
				{// check if output format is same
					AddDataToBuffer(jbuffer, payload_data, payload_size, Settings::iOutDataFormat);
				}
				else
				{
					// decoding g711 ulaw
					for (unsigned i = 0; i < payload_size; i++) {
						m_pPCMBuffer[i] = (int16)ulaw2linear(*((uint8 *)payload_data + i));
					}
					CompressPcmBuffer(payload_size);
					AddDataToBuffer(jbuffer, m_pPCMBuffer, payload_size, Settings::iOutDataFormat);
				}
				return payload_size;
			}
		}
		break;
	case(_RTP_PAYLOAD_TYPE_PCMA): // G711 alaw
		{
			// add 16 bit PCM silence...
			AddSilenceToBuffer(jbuffer, silence_samples);
			if (payload_size)
			{
				if (Settings::iOutDataFormat == WAVE_FORMAT_ALAW) 
				{// check if output format is same
					AddDataToBuffer(jbuffer, payload_data, payload_size, Settings::iOutDataFormat);
				}
				else
				{
					// decoding g711 alaw
					for (unsigned i = 0; i < payload_size; i++) {
						m_pPCMBuffer[i] = (int16)alaw2linear(*((uint8 *)payload_data + i));
					}
					CompressPcmBuffer(payload_size);
					AddDataToBuffer(jbuffer, m_pPCMBuffer, payload_size, Settings::iOutDataFormat);
				}
				return payload_size;
			}
		}
		break;
	case(_RTP_PAYLOAD_TYPE_G722): // G722
		{
			// because G722 is a 16khz codec, but recorder operates with 8khz audio
			// so we reduce by two times number of silence samples in stream
			AddSilenceToBuffer(jbuffer, silence_samples / 2);
			// decoding g722
			if (payload_size)
			{
				// find associated g722 decoder context...
				g722_state *g722decoder = (g722_state *)it->second.pCTX;

				if (!g722decoder)
				{
					// this is first frame, create and initialize new 
					// decoding context for this stream
					g722decoder = new g722_state;
					g722_reset_decoder(g722decoder);
					// add decoding context to map
					m_xStreams[session_id].pCTX = (void *)g722decoder;
				}
				
				// decoding frame...				
				int decoded_samples = g722_decode((int8 *)payload_data, 
											m_pPCMBuffer, _G722_OPERATION_MODE, 
											payload_size, g722decoder);

				if (decoded_samples == (payload_size * 2))
				{
					// because G722 is a 16khz codec, but recorder 
					// operates with 8khz audio so we must downsampling decoded audio
					int iLowSamples = Resample16KhzBuffer(decoded_samples);
					CompressPcmBuffer(iLowSamples);
					AddDataToBuffer(jbuffer, m_pPCMBuffer, iLowSamples, Settings::iOutDataFormat);
				}
				else
				{	
					// decoding error reset decoder state and add silence
					g722_reset_decoder(g722decoder);
					AddSilenceToBuffer(jbuffer, payload_size);
				}
				return payload_size * 2;
			}
		}
		break;
	default:
		break;
	}
	return 0;
}

void CStreamer::AddDataToBuffer(CIRCircularBuffer *stream, void *data, int size, int format)
{
	if (stream && data)
	{
		stream->AddData((const BYTE*)data, size);
	}
}

// add silence to output stream
void CStreamer::AddSilenceToBuffer(CIRCircularBuffer *stream, int silence_samples)
{
	// prepare silence buffer
	uint8 sb[0x10]; 
	memset(sb, Settings::uSilenceSample, sizeof(sb));
	
	// add silence
	int i = 0;
	while(i < silence_samples)
	{
		int iDataSize = ((silence_samples - i) > sizeof(sb)) ? sizeof(sb) : silence_samples - i;
		AddDataToBuffer(stream, sb, iDataSize, Settings::iOutDataFormat);
		i += sizeof(sb);
	}
}

// resampling 16 khz PCM buffer to 8khz, return new buffer size
int CStreamer::Resample16KhzBuffer(int samples)
{
	int	index = 0;
	const int ratio = 2;
	int32 sum = 0;

	for (int i = 0; i < samples; i++)
	{
		sum += m_pPCMBuffer[i];
		
		if (((i + 1) % ratio) == 0)
		{
			m_pPCMBuffer[index] = sum / ratio;
			sum = 0;
			index++;
		}
	}

	return index;
}

void CStreamer::CompressPcmBuffer(int samples) // compress out buffer
{
	if (Settings::iOutDataFormat == WAVE_FORMAT_ALAW)
	{
		for (int i = 0; i < samples; i++) {
			((uint8*)m_pPCMBuffer)[i] = linear2alaw(m_pPCMBuffer[i]);
		}
	} 
	else if (Settings::iOutDataFormat == WAVE_FORMAT_MULAW)
	{
		for (int i = 0; i < samples; i++) {
			((uint8*)m_pPCMBuffer)[i] = linear2ulaw(m_pPCMBuffer[i]);
		}
	}
}

// flush streams buffers
void CStreamer::Flush(int iMS)
{
	int iDataSize = iMS * 8;
	uint8 *pBuffer = new uint8[iDataSize];

	id2stream_map::iterator it = m_xStreams.begin();
	for (;it != m_xStreams.end(); it++) 
	{
		TStreamStruct s(it->second);
		int iJBuffDataLen = s.pJBuffer->GetDataLength();

		if (iJBuffDataLen) {
			iJBuffDataLen = s.pJBuffer->GetData(pBuffer, iDataSize);
		}

		// padding...
		for (int i = iJBuffDataLen; i < iDataSize; i++)
			pBuffer[i] = Settings::uSilenceSample;

		s.pStream->AddData(pBuffer, iDataSize, Settings::iOutDataFormat);
	}
	
	delete [] pBuffer;

	it = m_xStreams.begin();
	for (;it != m_xStreams.end(); it++) {
		it->second.pStream->Flush();
	}
}

// create/remove streams
void CStreamer::BindStream(IDataStream *pStream, uint32 session_id)
{
	id2stream_map::iterator it = m_xStreams.find(session_id);
	if (it == m_xStreams.end())
	{
		TStreamStruct s;
		s.pCTX		= NULL;
		s.pJBuffer	= new CIRCircularBuffer();
		s.pStream	= pStream;
		s.uID		= session_id;

		m_xStreams[session_id] = s;
	}
}

void CStreamer::UnBindStream(uint32 session_id)
{
	id2stream_map::iterator it = m_xStreams.find(session_id);
	if (it != m_xStreams.end())
	{
		if (it->second.pCTX)
			delete it->second.pCTX;

		if (it->second.pJBuffer)
			delete it->second.pJBuffer;

		// remove stream from map
		m_xStreams.erase(it);
	}
}
