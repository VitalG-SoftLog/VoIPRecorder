#pragma once
#include "Interfaces.h"
#include "MessageQueue.h"
#include "IRCircularBuffer.h"
#include "StreamMixer.h"

#define _ENABLE_AGC 1

////////////////////////////////////////////
//                   ^                    //
//                   | Merged Data        //
//           *----------------*           //
//           | CTerminalAudio |           //
//           *----------------*           //
//               ^         ^              //
//    Phone Data |         | PBX Data     //
// *----------------*  *----------------* //
// | m_xMixerLocal  |  | m_xMixerRemote | //
// *----------------*  *----------------* //
//   ^ ^ ^ ^ ^ ^ ^       ^ ^ ^ ^ ^ ^ ^    //
//   | | | | | | |       | | | | | | |    //
//   Local streams       Remote streams   //
////////////////////////////////////////////

class CTerminalAudio : public CStreamMixer
{
public:
	CTerminalAudio(int iTerminalID, 
					CMessageQueue *pMessageQueue, 
					IRTPRecorder *pRecorder);

	virtual ~CTerminalAudio();

	void StartRecord();
	void StopRecord();

	void AddNewStream(uint32 id, bool bRemote);
	void DeleteStream(uint32 id);

	int	GetAudioData(int MaxLength, 
						unsigned char *pPBXData, 
						unsigned char *pPhoneData, 
						unsigned char *pMergedData);

	void SetAudioGain(BOOL Remote, int Gain);
	
	void ResetAGC();

protected:
	static void StaticOverrunHandler(int Length, void *pUserData);
	void OverrunHandler(int Length);

	void Notify(int iEvent, int iData);
	void RecordStream(bool bEnabled, uint32 uID, bool bRemote);
	virtual void Poll(int iOverrun);
	void	AplyGainToBuffer(unsigned char *buffer, int size, int gain, float *fAGC, int *pTC);
	int16	AGC(float* pfAGC, float fAGC, int* piTC, int16 sValue);

	void PoolMin();
	void PoolOverride(int iOverride);
	void MixAndNotify(bool bRemoteMixing, int iSize);

protected:
	CMessageQueue			*m_pNotifyQueue;
	IRTPRecorder			*m_pRTPRecorder;
	int						m_iTerminalID;
	bool					m_bRecording;

	// all stored streams
	typedef std::list<std::pair<uint32, bool>> streams_list;
	streams_list m_xStreams;

	// output buffers
	CIRCircularBuffer		m_xDataPBX;
	CIRCircularBuffer		m_xDataPhone;
	CIRCircularBuffer		m_xDataMerged;
	
	// streams mixers
	CStreamMixer			m_xMixerLocal;
	CStreamMixer			m_xMixerRemote;

	CMixerInputStream		*m_pLocalDataOut;
	CMixerInputStream		*m_pRemoteDataOut;

	// gain control variables
	int						m_iGainLocal;
	int						m_iGainRemote;
	float					m_fAGCLocal;
	float					m_fAGCRemote;
	int						m_nTCLocal;
	int						m_nTCRemote;
	int						m_iFlushCnt;
};
