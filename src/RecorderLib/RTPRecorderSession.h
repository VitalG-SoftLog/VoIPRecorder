// Header: RTPRecorderSession.h
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "Interfaces.h"
#include <rtpsession.h>
#include "RtpFakeTransmitter.h"
#include "RtpSessionParam.h"

//////////////////////////////////////////////////////////////////////////
// Class: CRTPRecorderSession
// Encapsulation of JRTPLib sessions.
//////////////////////////////////////////////////////////////////////////
class CRTPRecorderSession 
	: public IUDPPacketListener
{
public:
	// Constructor: CRTPRecorderSession
	// Default constructor.
	//
	// Parameters:
	// session_id - [in] Session ID.
	// params - [in] const reference to RTP session parameters.
	CRTPRecorderSession(unsigned int session_id, 
						const CRTPSessionParam &params);
	// Destructor: 
	// Default destructor.
	virtual ~CRTPRecorderSession(void);

	// Method: GetParams
	// Returns parameters of RTP session.
	//
	// Returns:
	// const reference to <CRTPSessionParam> object.
	const CRTPSessionParam &GetParams() const;
	
	// Method: GetID
	// Returns session id.
	//
	// Returns:
	// Returns session id.
	unsigned int GetID() const;

	// Method: StartRecord
	// Start streaming RTP payload data.
	void StartRecord();

	// Method: StopRecord
	// Stop streaming RTP payload data.
	void StopRecord();

	// Method: IsRecording
	// Return recording status
	//
	// Returns:
	// true if current session is streamed, false otherwise.
	bool IsRecording() const;

	// Method: ProcessReceivedPakets
	// Processing received RTP packets. Remove received packets from 
	// queue and bypass it to streamer.
	//
	// Parameters:
	// streamer - [in] pointer to <IStreamer> object.
	// block_timeline - [in] time in ms that elapsed since 
	// the last call o this method.
	void ProcessReceivedPakets(IStreamer *streamer, 
								uint32 block_timeline);

	// Method: OnUDPPacket
	// Implementation of <IUDPPacketListener> interface.
	// Call when UDP data is ready.
	//
	// Parameters:
	// packet - pointer to <CUDPPacket> object
	virtual bool OnUDPPacket(const CUDPPacket *packet);

protected:
	// session ID
	uint32					m_uID;
	// session parameters
	CRTPSessionParam		m_xParams;
	// recording flag
	bool					m_bRecording;

	// JRTPLib objects
	RTPSession					*m_pRtpSession;
	RTPFakeTransmitter			*m_pRtpTransmitter;
	RTPFakeTransmissionParams	*m_pRtpTransParams;

	// used for silent detection
	uint32					m_uNextTimeStamp;
};
