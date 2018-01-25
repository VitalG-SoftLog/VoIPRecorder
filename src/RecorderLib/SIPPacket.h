#pragma once

#include <string>
#include <map>
#include <list>
#include "UDPPacket.h"
#include "IPv4Address.h"

#define defineHeader(_NAME, _HEADER) const std::string &##_NAME##() const {return Header(_HEADER);}
#define defineHeaderNumeric(_NAME, _HEADER) int ##_NAME##() const {StrToInt(Header(_HEADER).c_str());}

#define _NEW_LINE		"\r\n"
#define _NEW_BODY_SEP	"\r\n\r\n"

class CSIPPacket
{
public:
	enum MethodTypes 
	{
		UNKNOWN,
		ACK,		// RFC 3261
		BYE,		// RFC 3261
		CANCEL,		// RFC 3261
		INVITE,		// RFC 3261
		REGISTER,	// RFC 3261
		OPTIONS,	// RFC 3261
		NOTIFY,		// RFC 3265
		SUBSCRIBE,	// RFC 3265
		REFER,		// RFC 3515
		INFO,		// RFC 2976
		PRACK,		// RFC 3262
		PUBLISH,	// RFC draft
		UPDATE,		// RFC 3311
	};

	struct _method_str_pair
	{
		MethodTypes method;
		const char *str;
		int strlen;
	};
	
	static _method_str_pair m_xMethodstable[];
	static std::string MethodToString(CSIPPacket::MethodTypes);
	static CSIPPacket::MethodTypes StringToMethod(const std::string &method);

public:
	CSIPPacket(void);	
	CSIPPacket(const CSIPPacket &packet);
	CSIPPacket(const CUDPPacket &packet);

	virtual ~CSIPPacket(void);

	bool Parse(const std::string &sPacket);

	const std::string &GetRawData() const {return m_sRawData;}

	bool IsRequest() const {return m_bIsRequest;}
	bool IsResponse() const {return m_bIsResponse;}
	bool IsValid() const;
	void Invalidate();

	const	CIPv4Address &SrcIP() const {return m_xSrcIP;}
	const	CIPv4Address &DstIP() const {return m_xDstIP;}

	MethodTypes Method() const {return m_eMethod;}
	const	std::string &Uri() const;
	const	std::string &RequestLine() const;
	
	int		StatusCode() const {return m_iStatusCode;}
	const	std::string &StatusLine() const;
	const	std::string &Reason() const;

	const	std::string &Body() const {return m_sBody;}
	const	std::string &Header(const std::string &sName) const;

	defineHeader(CallID,		"Call-ID");
	defineHeader(UserAgent,		"User-Agent");
	defineHeader(From,			"From");
	defineHeader(To,			"To");
	defineHeader(Via,			"Via");
	defineHeader(ContentType,	"Content-Type");
	defineHeader(CSeq,			"CSeq");

	int CSeqNumber() const;
	MethodTypes CSeqMethod() const;

	const std::string &FromTag() const {return m_sFromTag;}
	const std::string &FromAddress() const {return m_sFromAddress;}
	const std::string &FromUser() const {return m_sFromUser;}

	const std::string &ToTag() const {return m_sToTag;}
	const std::string &ToAddress() const {return m_sToAddress;}
	const std::string &ToUser() const {return m_sToUser;}

//	defineHeaderNumeric(Expires, "Expires");
protected:

	//
	void ParseTagsAddr(const std::string &sInput, 
						std::string &sAddr, 
						std::string &sTag, 
						std::string &sUser);

	bool m_bIsRequest;
	bool m_bIsResponse;

	// response members
	std::string m_sStatusLine;
	std::string m_sReason;

	CIPv4Address m_xSrcIP;
	CIPv4Address m_xDstIP;

	int			m_iCSeqNumber;
	MethodTypes m_eCSeqMethod;


	int			m_iStatusCode;


	// request members
	std::string m_sRequestLine;
	std::string m_sRequestUri;
	MethodTypes m_eMethod;


	std::string m_sFromTag;
	std::string m_sFromAddress;
	std::string m_sFromUser;

	std::string m_sToTag;
	std::string m_sToAddress;
	std::string m_sToUser;


	// headers
	std::map<std::string, std::string> m_xHeaders;

	// body
	std::string m_sBody;
	
	// unparsed raw data
	std::string m_sRawData;

};
