#pragma once

#include "Interfaces.h"

#define _MODULE_ID			"6DDAC049-A676-4508-893D-DE181FB29127"
#define _REL_SETTINGS_PATH	"Intelligent recording\\Configuration"
#define _CONFIG_NAME		"VoIP.xml"

enum eSipMode{
	modeDefault,
	modeNSIP
};

namespace Settings
{
	extern uint8		uSilenceSample; // ulaw or mulaw silence sample depend on iOutDataFormat
	extern uint16		uSIPPort;
	extern eSipMode		eMode;
	extern int			iOutDataFormat;
	extern std::string	sInterfaceName;
	extern std::string	sPBXIP;
	extern std::string	sDumpFile;
	extern std::string	sCaptureFile;
	extern std::string	sLogFile;

	void LoadSettings();
	void SaveSettings();
	std::string GetXmlPath();
};
