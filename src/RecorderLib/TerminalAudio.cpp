#include "StdAfx.h"
#include "TerminalAudio.h"
#include "StringUtils.h"
#include "Settings.h"
#include "MultiXtR.h"

_MODULE_NAME(CTerminalAudio)

const int IRD_MAX_AMP		= 32767;
const int IRD_ACCEPT_AMP	= 30000;

CTerminalAudio::CTerminalAudio(int iTerminalID, 
							   CMessageQueue *pMessageQueue, 
							   IRTPRecorder *pRecorder)
							   : CStreamMixer()
{
	m_iTerminalID		= iTerminalID;
	m_pNotifyQueue		= pMessageQueue;
	m_pRTPRecorder		= pRecorder;
	m_bRecording		= false;
	
	m_pLocalDataOut		= dynamic_cast<CMixerInputStream*>(CreateInput(0));
	m_pLocalDataOut->GetData()->SetBufferLength(
		m_pLocalDataOut->GetData()->GetBufferLength() * 2
	);
	//	m_pLocalDataOut->GetData()->SetOverrunHandler(StaticOverrunHandler, this);

	m_pRemoteDataOut	= dynamic_cast<CMixerInputStream*>(CreateInput(1));
	m_pRemoteDataOut->GetData()->SetBufferLength(
		m_pRemoteDataOut->GetData()->GetBufferLength() * 2
	);
	//m_pRemoteDataOut->GetData()->SetOverrunHandler(StaticOverrunHandler, this);

	// realloc inner buffers
	delete []m_pMixedBuffer;
	m_pMixedBuffer		= new uint8[m_pRemoteDataOut->GetData()->GetBufferLength()];

	delete []m_pCurrentBuffer;
	m_pCurrentBuffer	= new uint8[m_pRemoteDataOut->GetData()->GetBufferLength()];

	m_xMixerRemote.SetOutStream(m_pRemoteDataOut);
	m_xMixerLocal.SetOutStream(m_pLocalDataOut);

	m_iFlushCnt = 0;

	m_iGainLocal		= 256;
	m_iGainRemote		= 256;

	ResetAGC();
}

CTerminalAudio::~CTerminalAudio(void)
{
}

void CTerminalAudio::ResetAGC()
{
	m_nTCLocal			= 0;
	m_nTCRemote			= 0;
	m_fAGCLocal			= 1.0;
	m_fAGCRemote		= 1.0;
}

void CTerminalAudio::StaticOverrunHandler(int Length, void *pUserData)
{
	if (pUserData)
	{
		CTerminalAudio *pThis = (CTerminalAudio *)pUserData;
	}
}

void CTerminalAudio::OverrunHandler(int Length)
{
	_WARNING("Buffer overrun!");
}

void CTerminalAudio::Notify(int iEvent, int iData)
{
	if (m_pNotifyQueue)
		m_pNotifyQueue->Push(m_iTerminalID, iEvent, iData);
}

void CTerminalAudio::RecordStream(bool bEnabled, uint32 uID, bool bRemote)
{
	if (bEnabled)
	{// enable recording
		IDataStream *stream = NULL;
		if (bRemote) {
			stream = m_xMixerRemote.CreateInput(uID);
		} else {
			stream = m_xMixerLocal.CreateInput(uID);
		}
		m_pRTPRecorder->StartRecord(uID, stream);
	}
	else
	{// disable recording
		if (bRemote) {
			m_pRTPRecorder->StopRecord(uID);
			m_xMixerRemote.DeleteInput(uID);
		} else {
			m_pRTPRecorder->StopRecord(uID);
			m_xMixerLocal.DeleteInput(uID);
		}
	}
}

void CTerminalAudio::StartRecord()
{
	if (!m_bRecording)
	{
		_LOG("Recording enabled");
		m_bRecording = true;
		// start recording all stored sessions
		streams_list::iterator it = m_xStreams.begin();
		for(;it != m_xStreams.end(); it++)
		{
			RecordStream(true, it->first, it->second);
		}
	}
}

void CTerminalAudio::StopRecord()
{
	if (m_bRecording)
	{
		_LOG("Recording disbaled");
		m_bRecording = false;
		// stop recording all stored sessions
		streams_list::iterator it = m_xStreams.begin();
		for(;it != m_xStreams.end(); it++)
		{
			RecordStream(false, it->first, it->second);
		}
	}
}

void CTerminalAudio::AddNewStream(uint32 id, bool bRemote)
{
	_LOG("Added new stream ID: " << StringUtils::IntToString(id) << " Remote: " << bRemote ? "TRUE" : "FALSE");
	m_xStreams.push_back(std::pair<uint32, bool>(id, bRemote));
	// if recording is enabled, then start recording on new stream
	if (m_bRecording) 
	{
		RecordStream(true, id, bRemote);
	}
}

void CTerminalAudio::DeleteStream(uint32 id)
{
	_LOG("Delete stream ID: " << StringUtils::IntToString(id));

	// if recording is enabled, first stop recording on selected stream
	if (m_bRecording) 
	{
		// find stream...
		bool bRemote = false;
		streams_list::iterator it = m_xStreams.begin();
		for(;it != m_xStreams.end(); it++)
		{
			if (it->first == id)
			{
				bRemote = it->second;
				RecordStream(false, id, bRemote);
				// delete stream from list
				m_xStreams.erase(it);
				break;
			}
		}
	}
}

int	CTerminalAudio::GetAudioData(int MaxLength, 
				 unsigned char *pPBXData, 
				 unsigned char *pPhoneData, 
				 unsigned char *pMergedData)
{

	int iReturn = 0;
	
	scope_locker sl(m_xCS);

	if (m_bRecording)
	{
		m_xDataPBX.GetData(pPBXData, MaxLength);
		m_xDataPhone.GetData(pPhoneData, MaxLength);
		iReturn = m_xDataMerged.GetData(pMergedData, MaxLength);
	}

	return iReturn;
}

void CTerminalAudio::SetAudioGain(BOOL Remote, int Gain)
{
	scope_locker sl(m_xCS);

	if (Remote)
	{
		m_iGainRemote = Gain;
	}
	else
	{
		m_iGainLocal = Gain;
	}
}

void CTerminalAudio::AplyGainToBuffer(unsigned char *buffer, 
									  int size, int gain, 
									  float *fAGC, int *pTC)
{
	float k = 1.0f;
	if (gain != 256 || gain >= 0)
		k = (float)gain / 256.0f;

	float agc = *fAGC;

	if (Settings::iOutDataFormat == WAVE_FORMAT_ALAW)
	{
/*		_LOG("AplyGainToBuffer(), gain:" 
			<< StringUtils::IntToString(gain) 
			<< ", WAVE_FORMAT_ALAW" << ", size: " 
			<< StringUtils::IntToString(size));
*/
		for (int i = 0; i < size; i++)
		{
			int32 linear = alaw2linear(buffer[i]);

#ifdef _ENABLE_AGC
			linear = AGC(fAGC, agc, pTC, linear);
#endif//_ENABLE_AGC

			linear = int32((float)linear * k);
			if (linear < -IRD_MAX_AMP) {
				linear = -IRD_MAX_AMP;
			} else if (linear > IRD_MAX_AMP) {
				linear = IRD_MAX_AMP;
			}

			buffer[i] = linear2alaw((int16)linear);
		}
	}
	else if (Settings::iOutDataFormat == WAVE_FORMAT_MULAW)
	{
		for (int i = 0; i < size; i++)
		{
/*			_LOG("AplyGainToBuffer(), gain:" 
				<< StringUtils::IntToString(gain) 
				<< ", WAVE_FORMAT_MULAW" << ", size: " 
				<< StringUtils::IntToString(size));
*/
			int32 linear = ulaw2linear(buffer[i]);
#ifdef _ENABLE_AGC
			linear = AGC(fAGC, agc, pTC, linear);
#endif//_ENABLE_AGC
			linear = int32((float)linear * k);

			if (linear < -IRD_MAX_AMP) {
				linear = -IRD_MAX_AMP;
			} else if (linear > IRD_MAX_AMP) {
				linear = IRD_MAX_AMP;
			}

			buffer[i] = linear2ulaw((int16)linear);
		}
	}
}


// Perform the AGC algorithm on the supllied data 
// Return :		Result off the AGC
// Parameters:	float pfAGC - Place to save AGC value after the algorthim
//              float fAGC - AGC value to apply to the algorthim
//              short sValue - Data to apply the algorthim on

int16 CTerminalAudio::AGC(float* pfAGC, float fAGC, int* piTC, int16 sValue)
{
	// make all values positive
	int v2 = abs(sValue);	

	if (v2 * *pfAGC > IRD_MAX_AMP)
	{
		//over peak - lower the volume and set TC
		*pfAGC = static_cast<float>(IRD_MAX_AMP / v2);

		// 50 ms holdoff.
		(*piTC) = 400; 	
	}
	else if (v2 * fAGC > IRD_ACCEPT_AMP)
	{
		// not over range but big enough
		// don't ramp up for 400 samples
		*piTC = 400;
	}
	else
	{
		// under range check tc and raise gain if we can
		if (*piTC == 0)
		{
			// tc has expired, can we ramp up the gain any more
			if (*pfAGC < 10)
			{
				// yes, ramp it up a bit. (30db / sec) 
				// *1.00024 gives *10 after 9600 iterations
				// 9600 = 0.6 sec giving 30db/sec
				(*pfAGC) *= static_cast<float>(1.00024);
			}
		}
		else
		{
			// wait for time constant to expire
			(*piTC) --;
		}
	}

	int iVal = static_cast<int>(sValue * *pfAGC);

	if (iVal < - IRD_MAX_AMP)
	{
		iVal = -IRD_MAX_AMP;
	}

	if (iVal > IRD_MAX_AMP)
	{
		iVal = IRD_MAX_AMP;
	}

	return (static_cast<int16>(iVal));
}

void CTerminalAudio::MixAndNotify(bool bRemoteMixing, int iSize)
{

	// add data to output buffers...
	if (bRemoteMixing)
	{// m_pMixedBuffer buffer == remote data
		AplyGainToBuffer(m_pMixedBuffer, iSize, m_iGainRemote, &m_fAGCRemote, &m_nTCRemote);
		AplyGainToBuffer(m_pCurrentBuffer, iSize, m_iGainLocal, &m_fAGCLocal, &m_nTCLocal);
		m_xDataPhone.AddData((LPBYTE)m_pCurrentBuffer, iSize);
		m_xDataPBX.AddData((LPBYTE)m_pMixedBuffer, iSize);
	}
	else
	{// m_pMixedBuffer buffer == local data
		AplyGainToBuffer(m_pMixedBuffer, iSize, m_iGainLocal, &m_fAGCLocal, &m_nTCLocal);
		AplyGainToBuffer(m_pCurrentBuffer, iSize, m_iGainRemote, &m_fAGCRemote, &m_nTCRemote);
		m_xDataPhone.AddData((LPBYTE)m_pMixedBuffer, iSize);
		m_xDataPBX.AddData((LPBYTE)m_pCurrentBuffer, iSize);
	}

	// mixing buffers
	MixPcmBuffers(iSize);

	// m_pMixedBuffer == merged data
	m_xDataMerged.AddData((LPBYTE)m_pMixedBuffer, iSize);

	// send notification...
	Notify(MP_BCHANNELDATA, m_xDataMerged.GetDataLength());
}


void CTerminalAudio::PoolMin()
{
	int iMixedLen		= 0;
	bool bRemoteMixing	= false;

	CIRCircularBuffer *pBufferMixed = NULL;
	CIRCircularBuffer *pBufferInput = NULL;

	if (m_pLocalDataOut->GetData()->GetDataLength() < 
		m_pRemoteDataOut->GetData()->GetDataLength())
	{
		pBufferMixed	= m_pLocalDataOut->GetData();
		pBufferInput	= m_pRemoteDataOut->GetData();
		iMixedLen		= pBufferMixed->GetDataLength();
		bRemoteMixing	= false;
	}
	else
	{
		pBufferMixed	= m_pRemoteDataOut->GetData();
		pBufferInput	= m_pLocalDataOut->GetData();
		iMixedLen		= pBufferMixed->GetDataLength();
		bRemoteMixing	= true;
	}

	if (iMixedLen > 0)
	{
		pBufferMixed->GetData((LPBYTE)m_pMixedBuffer, iMixedLen);
		pBufferInput->GetData((LPBYTE)m_pCurrentBuffer, iMixedLen);

		if (m_bRecording)
		{
			MixAndNotify(bRemoteMixing, iMixedLen);
		}
	}
}

void CTerminalAudio::PoolOverride(int iOverride)
{
	int iMixedLen		= 0;
	bool bRemoteMixing	= false;

	CIRCircularBuffer *pBufferMixed = NULL;
	CIRCircularBuffer *pBufferInput = NULL;

	if (m_pLocalDataOut->GetData()->GetDataLength() >
		m_pRemoteDataOut->GetData()->GetDataLength())
	{
		pBufferMixed	= m_pLocalDataOut->GetData();
		pBufferInput	= m_pRemoteDataOut->GetData();
		iMixedLen		= pBufferMixed->GetDataLength();
		bRemoteMixing	= false;
	}
	else
	{
		pBufferMixed	= m_pRemoteDataOut->GetData();
		pBufferInput	= m_pLocalDataOut->GetData();
		iMixedLen		= pBufferMixed->GetDataLength();
		bRemoteMixing	= true;
	}

	if (iMixedLen)
	{
		if (iMixedLen > iOverride)
			iMixedLen = iOverride;

		pBufferMixed->GetData((LPBYTE)m_pMixedBuffer, iMixedLen);
		int iMinSize = pBufferInput->GetDataLength();

		if (iMinSize) {
			pBufferInput->GetData((LPBYTE)m_pCurrentBuffer, iMixedLen);
		}
		
		if (m_bRecording)
		{
			// start adding silence samples...
			for (int i = iMinSize; i < iMixedLen; i++)
				m_pCurrentBuffer[i] = Settings::uSilenceSample;

			MixAndNotify(bRemoteMixing, iMixedLen);
		}
	}
}


// start mixing linear PCM data
void CTerminalAudio::Poll(int iOverrun)
{
	scope_locker sl(m_xCS);
	
	if (iOverrun) 
	{
		PoolOverride(iOverrun);
		SetAllStreamsProcessed(false);
	} 
	else 
	{
		if (!IsAllStreamsProcessed()) {return;}
		PoolMin();
		SetAllStreamsProcessed(false);
	}

//	m_iFlushCnt++;
//	if (m_iFlushCnt % 2)
//		return;

}
