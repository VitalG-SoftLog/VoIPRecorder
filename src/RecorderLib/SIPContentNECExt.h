#pragma once
#include <map>
#include <vector>

class CSIPContentNECExt
{
public:
	CSIPContentNECExt(const std::string &sContentType, const std::string &sSipBody);
	~CSIPContentNECExt(void);

	bool IsValid() const {return m_bValid;}

	const std::multimap<std::string, std::string> &GetData() const;

	bool IsHandsetEvent() const;
	bool IsHandset() const;


	// Info-CurrentNo2: events
	// Info-CurrentNo2=unknown no:0000000000 disp:stop
	bool IsInfoCurrentNoEvent() const;
	std::string InfoCurrentNo() const;


	// Info-DirectIn2: events
	// Info-DirectIn2=unknown ans no:22518
	bool IsInfoDirectIn() const;
	std::string InfoDirectIn() const;


	// Display Lines Notify Events
	bool IsDispLineClear(int iDispLineNumber) const;
	bool IsDispLineEvent(int iDispLineNumber) const;
	std::string DispLineText(int iDispLineNumber) const;

protected:
	std::vector<std::string> ExplodeHeader(const std::string &sHeaderName, const std::string &sep, bool bLast = false) const;
	bool IsNecHeaderPresent(const std::string &sHeader) const;
	bool ParseNecSipExt(const std::string &sSipBody);
	bool ParseMultiPart(const std::string &sSipBody);

	std::multimap<std::string, std::string> m_xData;
	bool m_bValid;
};
