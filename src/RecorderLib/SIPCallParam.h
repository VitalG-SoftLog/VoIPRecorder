#pragma once

#include "IPv4Address.h"
#include "RTPSessionParam.h"
#include <vector>

class CSIPCallParam
{
public:
	CSIPCallParam();
	CSIPCallParam(const CSIPCallParam &c);

	virtual ~CSIPCallParam();

	CSIPCallParam &operator=(const CSIPCallParam &c);

	void Invalidate();

	const CIPv4Address &GetFrom() const;
	void SetFrom(const CIPv4Address &ip);

	const CIPv4Address &GetTo() const;
	void SetTo(const CIPv4Address &ip);

	uint32 GetCSeq() const;
	void SetCSeq(uint32 cseq);

	const std::string &GetCallID() const;
	void SetCallID(const std::string &call_id);

	const std::string &GetUserFrom() const;
	void SetUserFrom(const std::string &user);

	const std::string &GetUserTo() const;
	void SetUserTo(const std::string &user);

	const std::vector<uint32> &GetSessionsIDs() const;
	void SetSessionsIDs(const std::vector<uint32> &ids);

	void AddNewSessionsID(uint32 sess_id);

	bool GetOutgoing() const;
	void SetOutgoing(bool bOut);

	bool GetExternal() const;
	void SetExternal(bool bExt);

#ifdef _DEBUG_LOG
	void Dump() const;
#endif//_DEBUG_LOG

protected:
	std::string			m_sUserFrom;
	std::string			m_sUserTo;
	std::string			m_sCallID;
	CIPv4Address		m_xFromIP;
	CIPv4Address		m_xToIP;
	std::vector<uint32>	m_xSessionsID;
	uint32				m_uCSeq;

	bool				m_bOutgoing;
	bool				m_bExternal;
};
