// Header: RTPSessionParam.h
//////////////////////////////////////////////////////////////////////////
#pragma once
#include "IPv4Address.h"

//////////////////////////////////////////////////////////////////////////
// Class: CRTPSessionParam
// Encapsulate params of RTP session.
//////////////////////////////////////////////////////////////////////////
class CRTPSessionParam
{
public:
	// Constructor: CRTPSessionParam
	// Default constructor.
	CRTPSessionParam();

	// Constructor: CRTPSessionParam
	// Copy constructor.
	//
	// Parameters:
	// s - [in] const reference to <CRTPSessionParam> object.
	CRTPSessionParam(const CRTPSessionParam &s);

	// Destructor: ~CRTPSessionParam
	// Default destructor.
	virtual ~CRTPSessionParam();

	// Operator: =
	// Copy one object data to another
	//
	// Parameters:
	// s - const reference to source <CRTPSessionParam> object
	//
	// Returns:
	// Reference to himself.
	CRTPSessionParam &operator =(const CRTPSessionParam &s);

	// Operator: ==
	// Compare two <CRTPSessionParam> objects
	//
	// Parameters:
	// s - const reference to <CRTPSessionParam> object
	//
	// Returns:
	// True if objects data is equal and False otherwise
	bool operator==(const CRTPSessionParam &s) const;


	const CIPv4Address &GetSrc() const;
	void SetSrc(const CIPv4Address &ip);

	const CIPv4Address &GetDst() const;
	void SetDst(const CIPv4Address &ip);

	unsigned int GetSSRC() const;
	void SetSSRC(unsigned int ssrc);

	unsigned int GetPayloadType() const;
	void SetPayloadType(unsigned int pt);

	unsigned int GetSamplesRate() const;
	void SetSamplesRate(unsigned int rate);

	void *GetUserData() const;
	void SetUserData(void *pdata);

#ifdef _DEBUG_LOG
	void Dump() const;
#endif

protected:
	unsigned int m_uSSRC;
	unsigned int m_uPayloadType;
	unsigned int m_uSamplesRate;
	CIPv4Address m_xSrc;
	CIPv4Address m_xDst;

	void *m_pUserData;
};
