#include "StdAfx.h"
#include "RTPRecorderSession.h"
#include <rtppacket.h>
#include <rtpipv4address.h>
#include <rtpsessionparams.h>
#include <rtperrors.h>
#include <winsock2.h>
#include <rtpsourcedata.h>
#include "rtpfaketransmitter.h"
#include "StringUtils.h"

_MODULE_NAME(CRTPRecorderSession)

// constructor
CRTPRecorderSession::CRTPRecorderSession(unsigned int session_id, 
										 const CRTPSessionParam &params)
{
	m_uID			= session_id;
	m_xParams		= params;
	m_bRecording	= false;
	m_uNextTimeStamp= 0;

	// create and initialize fake transmitter for JRTPlib
	m_pRtpTransParams = new RTPFakeTransmissionParams;
	m_pRtpTransmitter = new RTPFakeTransmitter(NULL);

	m_pRtpTransParams->SetBindIP(ntohl(m_xParams.GetDst().IP()));
	m_pRtpTransParams->SetPortbase(ntohs(m_xParams.GetDst().Port()));

	std::list<uint32_t> local_ip;
	local_ip.push_back(ntohl(m_xParams.GetDst().IP()));
	m_pRtpTransParams->SetLocalIPList(local_ip);
	m_pRtpTransParams->SetPacketReadyCB(NULL);
	m_pRtpTransParams->SetPacketReadyCBData(NULL);

	m_pRtpTransmitter->Init(false);
	m_pRtpTransmitter->Create(RTPFAKETRANS_MAXPACKSIZE, m_pRtpTransParams);

	// create and initialize JRTPlib session
	RTPSessionParams xSessionParams;
	xSessionParams.SetOwnTimestampUnit(1.0/(double)params.GetSamplesRate());
	xSessionParams.SetAcceptOwnPackets(false);
	xSessionParams.SetReceiveMode(RTPTransmitter::AcceptAll);
	xSessionParams.SetUsePollThread(false);

	m_pRtpSession = new RTPSession();
	m_pRtpSession->Create(xSessionParams, m_pRtpTransmitter);
}

CRTPRecorderSession::~CRTPRecorderSession(void)
{
	if (m_pRtpSession)
	{
		delete m_pRtpSession;
		m_pRtpSession = NULL;
	}

	if (m_pRtpTransParams)
	{
		delete m_pRtpTransParams;
		m_pRtpTransParams = NULL;
	}

	// transmitter already deleted by RTPSession
/*
	if (m_pRtpTransmitter)
	{
		delete m_pRtpTransmitter;
		m_pRtpTransmitter = NULL;
	}
*/
}

const CRTPSessionParam &CRTPRecorderSession::GetParams() const
{
	return m_xParams;
}

unsigned int CRTPRecorderSession::GetID() const
{
	return m_uID;
}

void CRTPRecorderSession::StartRecord()
{
	m_bRecording = true;
}

void CRTPRecorderSession::StopRecord()
{
	m_bRecording = false;
}

bool CRTPRecorderSession::IsRecording() const
{
	return m_bRecording;
}

// IUDPPacketListener
bool CRTPRecorderSession::OnUDPPacket(const CUDPPacket *packet)
{
	if (packet && m_bRecording)
	{
		if (m_pRtpSession)
		{
			// bypass UDP packet to JRTPLib core
			m_pRtpTransParams->SetCurrentData((uint8_t *)packet->Data());
			m_pRtpTransParams->SetCurrentDataLen(packet->DataLen());
			m_pRtpTransParams->SetCurrentDataAddr(ntohl(packet->Src().IP()));
			m_pRtpTransParams->SetCurrentDataPort(ntohs(packet->Src().Port()));
			m_pRtpTransParams->SetPacketReadyCB(NULL);
			m_pRtpTransParams->SetPacketReadyCBData(NULL);
			m_pRtpTransParams->SetCurrentDataType(
				m_pRtpTransParams->GetPortbase() == ntohs(packet->Dst().Port()));

			m_pRtpSession->Poll();
		}
	}
	return false;
}

// processing incoming packets
void CRTPRecorderSession::ProcessReceivedPakets(IStreamer *streamer, 
												uint32 block_timeline)
{
	int processed_packets = 0;

	if (m_pRtpSession)
	{
		m_pRtpSession->BeginDataAccess();

		// check incoming packets
		if (m_pRtpSession->GotoFirstSourceWithData())
		{
			do
			{
				RTPPacket *pack;
				while ((pack = m_pRtpSession->GetNextPacket()) != NULL)
				{
					// SDP is not provide valid SSRC, so we get SSRC of first received RTP packet
					if (m_xParams.GetSSRC() == 0)
						m_xParams.SetSSRC(pack->GetSSRC());

					if (m_xParams.GetSSRC() == pack->GetSSRC() && streamer)
					{
						unsigned int silence = 0;

						if (m_uNextTimeStamp == 0)
						{// first packet
							unsigned int decoded_samples = 
								streamer->OnPayloadData(m_uID, pack->GetPayloadType(),
								pack->GetPayloadData(), (unsigned int)pack->GetPayloadLength(), 0);

							m_uNextTimeStamp = pack->GetTimestamp() + decoded_samples;
						}
						else
						{
							if (m_uNextTimeStamp < pack->GetTimestamp())
							{// calculate silence samples
								silence = pack->GetTimestamp() - m_uNextTimeStamp;
								//silence = 0;
/*								_LOG("Session: " 
									<< StringUtils::IntToString(m_uID) 
									<< ", Droped packets, silence: " << StringUtils::IntToString(silence) 
									<< " samples");
*/
								//printf("detect silence %d samples\r\n", silence);
							}

							unsigned int decoded_samples = 
								streamer->OnPayloadData(m_uID, pack->GetPayloadType(),
								pack->GetPayloadData(), (uint32)pack->GetPayloadLength(), silence);

							m_uNextTimeStamp += silence;
							m_uNextTimeStamp += decoded_samples;
						}
						processed_packets++;
					}
					m_pRtpSession->DeletePacket(pack);
				}
			} while (m_pRtpSession->GotoNextSourceWithData());
		}
		m_pRtpSession->EndDataAccess();
//		m_pRtpSession->Poll();
	}
/*
	if (!processed_packets && !m_uNextTimeStamp)
	{
		// no incoming packets captured
		// generate silence block
		uint32 silence_samples = block_timeline * (m_xParams.GetSamplesRate() / 1000);
//		uint32 silence_samples = 0;

		_LOG("Session: " 
			<< StringUtils::IntToString(m_uID) 
			<< ", No incoming packets silence block: " 
			<< StringUtils::IntToString(silence_samples) 
			<< " samples");

		if (streamer && silence_samples)
		{
			streamer->OnPayloadData(m_uID, 
				m_xParams.GetPayloadType(), NULL, 0, silence_samples);
		}

		 if (m_uNextTimeStamp)
			 m_uNextTimeStamp += silence_samples;
	}
*/
}


