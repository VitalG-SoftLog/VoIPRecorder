// Header: SIPContentSDP.h
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "RTPSessionParam.h"

//////////////////////////////////////////////////////////////////////////
// Class: CSIPContentSDP
// Parser of SIP packets body.
//////////////////////////////////////////////////////////////////////////
class CSIPContentSDP
{
public:
	// Constructor: CSIPContentSDP
	// Default constructor.
	//
	// Parameters:
	// sContentType - [in] const reference to content-type string.
	// sSipBody - [in] const reference to SIP packet body.
	CSIPContentSDP(const std::string &sContentType, const std::string &sSipBody);

	// Destructor: 
	// Default destructor.
	virtual ~CSIPContentSDP();

	const CRTPSessionParam &GetSessionParams() const;
	bool IsValid() const;

protected:
	bool ParseSDP(const std::string &sSipBody);
	bool ParseMultiPart(const std::string &sSipBody);

	CRTPSessionParam m_xRtpSessionParams;
	bool m_bValid;
};
