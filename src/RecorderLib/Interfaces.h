// Header: Interfaces.h
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "BaseTypes.h"
#include "UDPPacket.h"
#include "IPv4Address.h"
#include "RTPSessionParam.h"
#include "SIPCallParam.h"
#include "SIPPacket.h"

//////////////////////////////////////////////////////////////////////////
// Defines: Payload types for standard audio encodings
//////////////////////////////////////////////////////////////////////////

// Define: _RTP_PAYLOAD_TYPE_PCMU
// ITU G.711 PCM µ-Law Audio 64 kbit/s 8Khz Mono 
#define _RTP_PAYLOAD_TYPE_PCMU	0

// Define: _RTP_PAYLOAD_TYPE_PCMA
// ITU G.711 PCM A-Law Audio 64 kbit/s 8Khz Mono 
#define _RTP_PAYLOAD_TYPE_PCMA	8

// Define: _RTP_PAYLOAD_TYPE_GSM
// European GSM Full Rate Audio 13 kbit/s (GSM 06.10)
#define _RTP_PAYLOAD_TYPE_GSM	3

// Define: _RTP_PAYLOAD_TYPE_G722
// ITU G.722 Audio
#define _RTP_PAYLOAD_TYPE_G722	9

// Define: _RTP_PAYLOAD_TYPE_G729
// ITU G.729 and G.729a
#define _RTP_PAYLOAD_TYPE_G729	18

//////////////////////////////////////////////////////////////////////////
// Defines: Wave data formats
//////////////////////////////////////////////////////////////////////////

// Define: WAVE_FORMAT_UNKNOWN
// Unknown wave format
#ifndef WAVE_FORMAT_UNKNOWN
#define WAVE_FORMAT_UNKNOWN			0x0000
#endif//WAVE_FORMAT_UNKNOWN

// Define: WAVE_FORMAT_PCM
// linear PCM wave format
#ifndef WAVE_FORMAT_PCM
#define WAVE_FORMAT_PCM				0x0001
#endif//WAVE_FORMAT_PCM

// Define: WAVE_FORMAT_ALAW
// ITU G.711 PCM A-Law wave format
#ifndef WAVE_FORMAT_ALAW
#define WAVE_FORMAT_ALAW			0x0006
#endif//WAVE_FORMAT_ALAW

// Define: WAVE_FORMAT_MULAW
// ITU G.711 PCM µ-Law wave format
#ifndef WAVE_FORMAT_MULAW
#define WAVE_FORMAT_MULAW			0x0007
#endif//WAVE_FORMAT_MULAW

// Define: WAVE_FORMAT_GSM610
// European GSM Full Rate wave format
#ifndef WAVE_FORMAT_GSM610
#define WAVE_FORMAT_GSM610			0x0031
#endif//WAVE_FORMAT_GSM610

// Define: WAVE_FORMAT_G722_ADPCM
// ITU G.722 wave format
#ifndef WAVE_FORMAT_G722_ADPCM
#define WAVE_FORMAT_G722_ADPCM		0x0065
#endif//WAVE_FORMAT_G722_ADPCM

// Define: WAVE_FORMAT_SIPROLAB_G729
// ITU G.729 wave format
#ifndef WAVE_FORMAT_SIPROLAB_G729
#define WAVE_FORMAT_SIPROLAB_G729	0x0133
#endif//WAVE_FORMAT_SIPROLAB_G729

//////////////////////////////////////////////////////////////////////////
// Interface: IUDPPacketListener
// IUDPPacketListener interface lets you bypass an UDP packets to objects.
//////////////////////////////////////////////////////////////////////////
interface IUDPPacketListener
{
	// Method: OnUDPPacket
	// Call when UDP data is ready.
	//
	// Parameters:
	// packet - pointer to <CUDPPacket> object
	//
	// Returns: 
	// true if packet marked
	virtual bool OnUDPPacket(const CUDPPacket *packet) = 0;
};


//////////////////////////////////////////////////////////////////////////
// Interface: ISIPPacketListener
// ISIPPacketListener interface lets you bypass an SIP packets to objects.
//////////////////////////////////////////////////////////////////////////
interface ISIPPacketListener
{
	// Method: OnSIPPacket
	// Call when SIP packet is ready.
	//
	// Parameters:
	// packet - pointer to <CSIPPacket> object
	virtual void OnSIPPacket(const CSIPPacket *packet) = 0;
};


//////////////////////////////////////////////////////////////////////////
// Interface: ICallEvents
// ICallEvents interface lets you bypass an SIP calls events to objects.
//////////////////////////////////////////////////////////////////////////
interface ICallEvents
{
	// Method: OnNewRegister
	// Call when new IP terminal registered on PBX.
	//
	// Parameters:
	// ip - ip address of terminal (network order)
	// user - const reference to registered user name
	// ua_type - type of user agent
	virtual void OnNewRegister(uint32 ip, const std::string &user, int ua_type) = 0;

	// Method: OnEndRegister
	// Call when IP terminal unregistered or registration expired.
	//
	// Parameters:
	// ip - ip address of terminal (network order)
	// user - const reference to registered user name
	// ua_type - type of user agent
	virtual void OnEndRegister(uint32 ip, const std::string &user, int ua_type) = 0;

	// Method: OnNewSession
	// Call when detected a new RTP session.
	//
	// Parameters:
	// session_id - RTP session ID (not SSRC!).
	// params - Parameters of new session. Const reference to 
	// <CRTPSessionParam> object. 
	virtual void OnNewSession(uint32 session_id, const CRTPSessionParam &params) = 0;

	// Method: OnSessionModify
	// Call when detected a new RTP session parameters modification.
	//
	// Parameters:
	// session_id - RTP session ID (not SSRC!).
	// params - New parameters of defined session. Const reference to 
	// <CRTPSessionParam> object. 
	virtual void OnSessionModify(uint32 session_id, const CRTPSessionParam &params) = 0;

	// Method: OnStopSession
	// Call when session deleted.
	//
	// Parameters:
	// session_id - RTP session ID (not SSRC!).
	virtual void OnStopSession(uint32 sess_id) = 0;

	// Method: OnNewCall
	// Call when new SIP call detected.
	//
	// Parameters:
	// call_id - const reference to call_id string.
	virtual void OnNewCall(const std::string &call_id) = 0;

	// Method: OnEndCall
	// Call when SIP call ended.
	//
	// Parameters:
	// call_id - const reference to call_id string.
	virtual void OnEndCall(const std::string &call_id) = 0;

};

//////////////////////////////////////////////////////////////////////////
// Interface: ICallManager
// ICallManager interface allow to obtain parameters associated with the 
// specified call object, or session.
//////////////////////////////////////////////////////////////////////////
interface ICallManager
{
	// Method: GetSessionsByCallID
	// Returns sessions IDs associated with specified call ID
	//
	// Parameters:
	// call_id - [in] const reference to call_id string.
	// sessions - [out] const reference to vector of uint32 which contain id of RTP sessions
	virtual void		GetSessionsByCallID(const std::string &call_id, std::vector<uint32> &sessions) = 0;

	// Method: GetCallParams
	// Return <CSIPCallParam> object associated with specified call ID.
	//
	// Parameters:
	// call_id - [in] const reference to call_id string.
	// params - [out] reference to <CSIPCallParam> object.
	virtual bool		GetCallParams(const std::string &call_id, CSIPCallParam &params) = 0;

	// Method: GetSessionParams
	// Return <CRTPSessionParam> object associated with specified session ID.
	//
	// Parameters:
	// sess_id - [in] session ID.
	// sess_params - [out] reference to <CRTPSessionParam> object.
	virtual bool		GetSessionParams(uint32 sess_id, CRTPSessionParam &sess_params) = 0;

	// Method: GetCallBySession
	// Return call ID associated with specified session.
	//
	// Parameters:
	// sess_id - [in] session ID.
	//
	// Returns:
	// std::string with call ID.
	virtual std::string GetCallBySession(uint32 sess_id) = 0;
};

//////////////////////////////////////////////////////////////////////////
// Interface: IDataStream
// IDataStream interface lets you bypass audio data to object.
//////////////////////////////////////////////////////////////////////////
interface IDataStream 
{
	// Method: SetID
	// Set ID for data stream object
	//
	// Parameters:
	// uID - [in] data stream ID.
	virtual void	SetID(uint32 uID) = 0;

	// Method: GetID
	// Get data stream ID associated with current object.
	//
	// Returns:
	// data stream ID.
	virtual uint32	GetID() = 0;

	// Method: AddData
	// Append new data to stream.
	//
	// Parameters:
	// data - [in] pointer to data.
	// size - [in] size of data.
	// data_type - [int] type of added data.
	virtual void	AddData(void *data, uint32 size, uint32 data_type) = 0;

	// Method: Flush
	// Flushing inner stream buffers.
	virtual void	Flush() = 0;
};

//////////////////////////////////////////////////////////////////////////
// Interface: IRTPRecorder
// IRTPRecorder interface lets you start or stop recording RTP sessions.
//////////////////////////////////////////////////////////////////////////
interface IRTPRecorder : public ICallEvents
{
	// Method: StartRecord
	// Start recording specified session.
	//
	// Parameters:
	// sess_id - [in] ID of session for recording.
	// pStream - [in] pointer to <IDataStream> which will receive the decoded audio
	// data associated with selected RTP session.
	virtual void StartRecord(unsigned int sess_id, IDataStream *pStream) = 0; 

	// Method: StopRecord
	// Stop recording specified session.
	//
	// Parameters:
	// sess_id - [in] ID of session for stopping recording
	virtual void StopRecord(unsigned int sess_id) = 0;
};

//////////////////////////////////////////////////////////////////////////
// Interface: IStreamer
// IStreamer interface lets you start or stop recording RTP sessions.
//////////////////////////////////////////////////////////////////////////
interface IStreamer
{
	// return number of streamed samples
	virtual unsigned int OnPayloadData(uint32 session_id, uint32 payload_type,
		void *payload_data, uint32 payload_size, uint32 silence_samples) = 0;

	// flush streams buffers
	virtual void Flush(int iMS) = 0;

	// create/remove streams
	virtual void	BindStream(IDataStream *pStream, uint32 session_id) = 0;
	virtual void	UnBindStream(uint32 session_id) = 0;

};
