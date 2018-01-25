#include "StdAfx.h"
#include <ShlObj.h>
#include "Settings.h"
#include "XmlUtils.h"
#include "UDPPacketFilter.h"
#include "g711.h"

// set defaults...
eSipMode Settings::eMode			= modeDefault;
uint16 Settings::uSIPPort			= 5060;
int Settings::iOutDataFormat		= WAVE_FORMAT_ALAW;
uint8 Settings::uSilenceSample		= 0x55;
std::string Settings::sInterfaceName= "";
std::string Settings::sPBXIP		= "127.0.0.1";
std::string Settings::sDumpFile		= "";
std::string Settings::sCaptureFile	= "";
std::string Settings::sLogFile		= "";


std::string Settings::GetXmlPath()
{
	std::string strXmlPath;
	char szBuffer[MAX_PATH+1] = {0};
	ZeroMemory(szBuffer, sizeof(szBuffer));
	SHGetSpecialFolderPathA(NULL, szBuffer, CSIDL_COMMON_APPDATA, FALSE);
	strXmlPath = szBuffer;
	strXmlPath += "\\";
	strXmlPath += _REL_SETTINGS_PATH;
	strXmlPath += "\\";
	strXmlPath += _CONFIG_NAME;
//	strXmlPath += ".xml";
	return strXmlPath;
}

void Settings::LoadSettings()
{
	// load XML
	TiXmlDocument doc(GetXmlPath().c_str());

	if (doc.LoadFile())
	{
		// loading settings section...
		TiXmlElement *pRoot = doc.FirstChildElement("engine");
		if (!pRoot)
			return;

		const char *pGuid = pRoot->Attribute("guid");
		if (!pGuid)
			return;

		// check if our config...
		if (std::string(_MODULE_ID) != pGuid)
			return;
		
		// loading settings section
		TiXmlElement *pSection = pRoot->FirstChildElement("settings");

		if (!pSection)
			return;
		
		XmlUtils::LoadValStr("capture-device", sInterfaceName, pSection);
		XmlUtils::LoadValStr("pbx-ip", sPBXIP, pSection);
		XmlUtils::LoadValInt("sip-port", uSIPPort, pSection);
		std::string sTmp;
		XmlUtils::LoadValStr("out-format", sTmp, pSection);

		if (sTmp == "UK") {
			iOutDataFormat = WAVE_FORMAT_ALAW;
			uSilenceSample = linear2alaw(0);
		} else if (sTmp == "US") {
			iOutDataFormat = WAVE_FORMAT_MULAW;
			uSilenceSample = linear2ulaw(0);
		}

		XmlUtils::LoadValStr("mode", sTmp, pSection);

		if (sTmp == "default") {
			eMode = modeDefault;
		} else if (sTmp == "nsip") {
			eMode = modeNSIP;
		}

		// loading debug settings
		pSection = pRoot->FirstChildElement("debug");

		if (!pSection) 
			return;

		XmlUtils::LoadValStr("capture-from-file", sCaptureFile, pSection);
		XmlUtils::LoadValStr("write-raw-sip", sDumpFile, pSection);
		XmlUtils::LoadValStr("debug-log", sLogFile, pSection);
	}
}

void Settings::SaveSettings()
{
	char szBuffer[MAX_PATH+1] = {0};
	ZeroMemory(szBuffer, sizeof(szBuffer));
	SHGetSpecialFolderPathA(NULL, szBuffer, CSIDL_COMMON_APPDATA, FALSE);
	std::string strPath = szBuffer;
	strPath += "\\";
	strPath += _REL_SETTINGS_PATH;
	CreateDirectoryA(strPath.c_str(), NULL);
	

	TiXmlDocument Doc(Settings::GetXmlPath().c_str());
	TiXmlDeclaration* pDecl = new TiXmlDeclaration( "1.0", "", "" );  
	Doc.LinkEndChild(pDecl); 	
	TiXmlElement * pRoot = new TiXmlElement("engine");
	pRoot->SetAttribute("guid", _MODULE_ID);
	Doc.LinkEndChild(pRoot);

	// save "settings" section
	TiXmlElement *pSection = new TiXmlElement("settings");
	pRoot->LinkEndChild(pSection);

	XmlUtils::SaveValStr("capture-device", sInterfaceName, pSection);
	XmlUtils::SaveValStr("pbx-ip", sPBXIP, pSection);
	XmlUtils::SaveValInt("sip-port", uSIPPort, pSection);
	XmlUtils::SaveValStr("out-format", (iOutDataFormat == WAVE_FORMAT_ALAW) ? "UK" : "US", pSection);
	
	std::string sMode = "default";

	switch (eMode) {
		case(modeDefault)	: sMode="default"; break;
		case(modeNSIP)		: sMode="nsip"; break;
	}

	XmlUtils::SaveValStr("mode", sMode, pSection);

	// save "debug" section...
	if (!sDumpFile.empty() || !sCaptureFile.empty() || !sLogFile.empty())
	{
		pSection = new TiXmlElement("debug"); pRoot->LinkEndChild(pSection);
		XmlUtils::SaveValStr("capture-from-file", sCaptureFile, pSection);
		XmlUtils::SaveValStr("write-raw-sip", sDumpFile, pSection);
		XmlUtils::SaveValStr("debug-log", sLogFile, pSection);
	}

	// save devices...
	CUDPPacketFilter filter; 
	if (filter.Initialize())
	{
		std::vector<std::string> xNames = filter.GetInterfaceList(false);
		std::vector<std::string> xDesc	= filter.GetInterfaceList(true);

		pSection = new TiXmlElement("devices"); pRoot->LinkEndChild(pSection);

		for (unsigned i = 0; i < xNames.size(); i++)
		{
			TiXmlElement *pDevice = new TiXmlElement("device");
			pDevice->SetAttribute("name", xNames[i].c_str());
			pDevice->SetAttribute("description", xDesc[i].c_str());
			pDevice->SetAttribute("index", i);
			pSection->LinkEndChild(pDevice);
		}
	}
	
	Doc.SaveFile();
}
