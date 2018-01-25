// File: RecorderMain.cpp
//////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Debug.h"
#include "RecorderMain.h"
#include "UDPPacketFilter.h"
#include "TerminalManager.h"
#include "MessageQueue.h"
#include "NotifyThread.h"
#include "g711.h"
#include "Locker.h"
#include "Settings.h"
#include "StringUtils.h"
#include "MiniVersion.h"

// Globals
char	g_szSwVersion[_STATIC_STRING_BUFFER_SIZE];

const char	g_szSwType[]					= _SW_TYPE;
const char	g_szIfTypeNEC[]					= _IF_TYPE_NEC;
const char	g_szIfTypeSIP[]					= _IF_TYPE_OTHER;
const char	g_szIfTypeUnk[]					= _IF_TYPE_UNKNOWN;
const char	g_szEmptyStr[]					= "";

CTerminalManager	*g_pTerminalManager		= NULL;
CMessageQueue		*g_pMessageQueue		= NULL;
CNotifyThread		*g_pNotifyThread		= NULL;
CUDPPacketFilter	*g_pPacketFilter		= NULL;
bool				g_bFilterInitialized	= false;

critical_section	g_xApiLocker;

_MODULE_NAME(RecorderMainAPI)


BOOL APIENTRY DllMain(HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					  )
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			{
				_LOG("DLL_PROCESS_ATTACH");
				// retrieving DLL module version
				TCHAR szFileName[MAX_PATH + 1];
				GetModuleFileName(hModule, szFileName, MAX_PATH);
				CMiniVersion v(szFileName);

				WORD version[4];
				v.GetFileVersion(version);
				SAFE_SNPRINTF(g_szSwVersion, sizeof(g_szSwVersion), 
					sizeof(g_szSwVersion) - 1, "%02d.%02d.%02d", 
					version[0], version[1], version[2]);

				// creating notify message queue and notification thread
				g_pMessageQueue		= new CMessageQueue();
				g_pNotifyThread		= new CNotifyThread(g_pMessageQueue);
			}
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			{
				_LOG("DLL_PROCESS_DETACH");
				// stop notification thread
				if (g_pNotifyThread != NULL)
				{
					if (g_pNotifyThread->IsRunning())
					{
						_LOG("Stopping notification thread...");
						g_pNotifyThread->Stop();
						g_pNotifyThread->WaitForEnd();
						_LOG("Notification thread stopped.");
					}
					delete g_pNotifyThread;
					g_pNotifyThread = NULL;
				}

				// delete message queue
				if (g_pMessageQueue != NULL)
				{
					delete g_pMessageQueue;
					g_pMessageQueue = NULL;
				}
			}
			break;
	}
	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
// Functions which affect the entire DLL
//////////////////////////////////////////////////////////////////////////

// Function: MP_Open
//
// This function will attempt to open the link to the Telephone system. 
// Since it cannot immediately determine success or failure, this function 
// has no return value. The MP_GetStatus function should be called after 
// MP_Open to determine whether required devices are correctly connected.
void MP_Open(void)
{
	scope_locker ll(g_xApiLocker);

	_LOG("MP_Open()");

	Settings::LoadSettings();

	CIPv4Address pbx_ip(Settings::sPBXIP);
	pbx_ip.Port(htons(Settings::uSIPPort));

	g_pPacketFilter		= new CUDPPacketFilter();
	g_pTerminalManager	= new CTerminalManager(g_pMessageQueue, pbx_ip);
	
	// start notify thread
	g_pNotifyThread->Start();
	g_pNotifyThread->WaitForStart();

	// initialize packet filter
	g_bFilterInitialized = g_pPacketFilter->Initialize();
	
	if (!g_bFilterInitialized)
	{
		_WARNING("Can't initialize Packet Filter");
		return;
	}
	
	g_pPacketFilter->SetPBXIP(pbx_ip);
	_LOG("Set PBX ip:sip_port == " << pbx_ip.AsString());

	// setup UDP packet listener
	g_pPacketFilter->RegisterListener(g_pTerminalManager);

	if (!Settings::sDumpFile.empty())
	{
		_LOG("Set SIP dump file: " << Settings::sDumpFile);
		g_pPacketFilter->SaveMarkedPacketsToDump(Settings::sDumpFile);
	}

	// start capture packets...
	//std::vector<std::string> v = g_pPacketFilter->GetInterfaceList();
	if (!Settings::sCaptureFile.empty())
	{
		// capturing from file instead network
		_LOG("Start capturing from file: " << Settings::sCaptureFile);
		g_pPacketFilter->StartCapturingOffline(Settings::sCaptureFile);
	}
	else
	{
		// capturing from network interface
		_LOG("Start capturing from interface: " << Settings::sInterfaceName);
		g_pPacketFilter->StartCapturing(Settings::sInterfaceName);
	}
}

// Function: MP_Close
//
// This function will close the link to the Telephone system. It will 
// release the ports and all allocated software devices, returning them 
// to the Windows system.
void MP_Close(void)
{
	scope_locker ll(g_xApiLocker);
	_LOG("MP_Close()");

	Settings::SaveSettings();

	// delete packet filter
	if (g_pPacketFilter != NULL)
	{
		_LOG("Stoping capturing...");
		g_bFilterInitialized = false;
		// stop capturing
		g_pPacketFilter->StopCapturing();
		// wait while capturing thread is stopping
		g_pPacketFilter->WaitForEnd();
		_LOG("Capture stopped.");
		// unregister listener
		g_pPacketFilter->UnRegisterListener(g_pTerminalManager);
		// delete packet filter
		delete g_pPacketFilter;
		g_pPacketFilter = NULL;
	}

	// delete terminal manager
	if (g_pTerminalManager != NULL)
	{
		delete g_pTerminalManager;
		g_pTerminalManager = NULL;
	}

	// stop notification thread
	if (g_pNotifyThread != NULL)
	{
		_LOG("Stopping notification thread...");
		g_pNotifyThread->Stop();
		g_pNotifyThread->WaitForEnd();
		_LOG("Notification thread stopped.");
	}

	// cleanup message queue
	if (g_pMessageQueue != NULL)
	{
		g_pMessageQueue->Clear();
	}
}

// Function: MP_GetSwType
//
// This function will return the version type of the current software library. 
// This is a Intelligent Recording product code, which is a string containing 
// manufacturer, hardware interface and audio encoding type. This string consists 
// of 5 characters e.g. AVfUS, TOfUK. Currently supported types are as follows. 
// There may be more in the future.
LPCSTR MP_GetSwType(void)
{
	_LOG("MP_GetSwType(): " << g_szSwType);
	return g_szSwType;
}

// Function: MP_GetSwVersion
//
//This function will return the version number of the current software library. 
//This is an ASCII string of the format “nn.nn.nn”
LPCSTR MP_GetSwVersion(void)
{
	_LOG("MP_GetSwVersion(): " << g_szSwVersion);
	return g_szSwVersion;
}

// Function: MP_SetCallback
//
//This function sets a callback which will be used by the library to identify 
//significant events taking place on the telephone lines. The events provided 
//to the callback are described below. MyProc may be NULL, which disables the 
//further sending of callbacks. Note that callbacks should come from an 
//independent thread.
//
// Parameters:
// MyProc - Callback function. MyProc may be NULL, which disables the 
// further sending of callbacks.
void MP_SetCallback( MULTIXTRCALLBACK *MyProc )
{
	scope_locker ll(g_xApiLocker);

	_LOG("MP_SetCallback(" << StringUtils::PtrToHex(MyProc) << ")");

	if (g_pNotifyThread)
	{
		g_pNotifyThread->SetCallBack(MyProc);
	}
}

// Function: MP_GetDataFormat
//
// This function will return the data format being used by the attached telephone 
// system. Current supported values are WAVE_FORMAT_MULAW for American µ-law or 
// WAVE_FORMAT_ALAW for European A-law.
//
// Returns: data format being used by the attached telephone system.
int MP_GetDataFormat(void)
{
	//_LOG("MP_GetDataFormat(): " << StringUtils::IntToHex(Settings::iOutDataFormat, 2));
	return Settings::iOutDataFormat;
}

// Function: MP_ConvertData
//
// This function converts received telephone data into 16 bit data suitable for 
// use in Windows WAV files, or by a soundcard. This function works correctly regardless 
// of whether the data received from the telephone is in European A-law or American µ-law.
//
// Parameters:
// Data8bit - [in] Encoded 8 bit sample.
//
// Returns: 16 bit PCM sample.
short MP_ConvertData(const char Data8bit)
{
	if (Settings::iOutDataFormat == WAVE_FORMAT_ALAW)
		return alaw2linear(Data8bit);
	else if (Settings::iOutDataFormat == WAVE_FORMAT_MULAW)
		return ulaw2linear(Data8bit);

	return Data8bit;
}

//////////////////////////////////////////////////////////////////////////
// Functions to determine Port numbers of connected devices
//////////////////////////////////////////////////////////////////////////

// Function: MP_GetMaxNumPorts
//
// This function will return the maximum number of connected devices 
// connected to the PC. Not necessarily the number which currently 
// are connected.
//
// Returns: Maximum number of connected devices.
int MP_GetMaxNumPorts(void)
{
	scope_locker ll(g_xApiLocker);

	int iReturn = 0;
	if (g_pTerminalManager != NULL)
	{
		iReturn =  g_pTerminalManager->GetTerminalCount();
	}

	_LOG("MP_GetMaxNumPorts(): " << StringUtils::IntToString(iReturn));

	return iReturn;
}

// Function: MP_GetMaxNumPorts
//
// This function will return the serial number of the connected virtual device.
// If no device is present in this slot it returns 0. Even if the device has a 
// serial number, it might still be faulty. You will need to check the status to 
// ensure that the device is working.
//
// Returns: Serial number of the connected virtual device. 
// 0 - if no device connected to this slot.
int MP_GetIfSerialNo( int PortNumber )
{
	scope_locker ll(g_xApiLocker);

	int iReturn = 0;

	if (g_pTerminalManager != NULL)
	{
		iReturn = g_pTerminalManager->GetTerminalIDByIndex(PortNumber);
	}
	
	_LOG("MP_GetIfSerialNo(" << 
		StringUtils::IntToString(PortNumber) 
			<< "): " << StringUtils::IntToString(iReturn));

	return iReturn;
}

//////////////////////////////////////////////////////////////////////////
// Functions which affect a single connected port
//////////////////////////////////////////////////////////////////////////
int MP_GetStatus( int Port )
{
	scope_locker ll(g_xApiLocker);

	if (g_pPacketFilter == NULL || 
		g_pTerminalManager == NULL ||
		g_pMessageQueue == NULL)
	{
		_LOG("MP_GetStatus(" << StringUtils::IntToString(Port) 
			<< "): " << "STATUS_NOTINITIALISED");
		return STATUS_NOTINITIALISED;
	}

	if (!g_bFilterInitialized)
	{
		_LOG("MP_GetStatus(" << StringUtils::IntToString(Port) 
			<< "): " << "STATUS_INTERNALERROR");
		return STATUS_INTERNALERROR;
	}
	
	CTerminal *pPort = g_pTerminalManager->GetTerminalByID(Port);
	if (pPort == NULL)
	{
		_LOG("MP_GetStatus(" 
			<< StringUtils::IntToString(Port) 
			<< "): " << "STATUS_NOTPRESENT");
		return STATUS_NOTPRESENT;
	}

	int iStatus = pPort->GetStatus();
	pPort->Release();

	_LOG("MP_GetStatus(" 
		<< StringUtils::IntToString(Port) 
		<< "): " << StringUtils::IntToString(iStatus));

	return iStatus;
}

int MP_GetCallState( int Port )
{
	scope_locker ll(g_xApiLocker);

	if (g_pPacketFilter == NULL || 
		g_pTerminalManager == NULL ||
		g_pMessageQueue == NULL)
	{
		_LOG("MP_GetCallState(" 
			<< StringUtils::IntToString(Port) 
			<< "): " << "CALL_IDLE");
		return CALL_IDLE;
	}

	CTerminal *pPort = g_pTerminalManager->GetTerminalByID(Port);
	if (pPort == NULL)
	{
		_LOG("MP_GetCallState(" 
			<< StringUtils::IntToString(Port) 
			<< "): " << "CALL_IDLE");
		return CALL_IDLE;
	}

	int iStatus = pPort->GetCallState();
	pPort->Release();

	_LOG("MP_GetCallState(" 
		<< StringUtils::IntToString(Port) 
		<< "): " << StringUtils::IntToString(iStatus));

	return iStatus;
}

LPCSTR MP_GetIfType( int Port )
{
	scope_locker ll(g_xApiLocker);

	LPCSTR strReturn = g_szIfTypeUnk;

	if (g_pPacketFilter != NULL && g_pTerminalManager != NULL &&
		g_pMessageQueue != NULL && g_bFilterInitialized)
	{
		CTerminal *pPort = g_pTerminalManager->GetTerminalByID(Port);
		if (pPort != NULL)
		{
			switch(pPort->GetIfType())
			{
			case(PHONE_TYPE_NEC):
				strReturn = g_szIfTypeNEC;
				break;
			case(PHONE_TYPE_OTHER):
				strReturn = g_szIfTypeSIP;
				break;
			case(PHONE_TYPE_UNDEFINED):
				strReturn = g_szIfTypeUnk;
				break;
			default:
				break;
			}
			pPort->Release();
		}
	}

	_LOG("MP_GetIfType(" << StringUtils::IntToString(Port) << "): " << strReturn);
	return strReturn;
}

void MP_SetAudioGain( int Port, BOOL Remote, int Gain )		// Gain = fixed point / 256
{
	scope_locker ll(g_xApiLocker);

	_LOG("MP_SetAudioGain(" 
		<< StringUtils::IntToString(Port) 
		<< ", " << StringUtils::IntToString(Remote)
		<< ", " << StringUtils::IntToString(Gain) << ")"
	);

	if (g_pTerminalManager)
	{
		CTerminal *pPort = g_pTerminalManager->GetTerminalByID(Port);
		if (pPort)
		{
			pPort->SetAudioGain(Remote, Gain);
			pPort->Release();
		}
	}
}

int MP_GetAudioData( int Port, int MaxLength, char *pPBXData, char *pPhoneData, char *pMergedData )
{
	scope_locker ll(g_xApiLocker);
	
	int iReturn = 0;

	if (g_pTerminalManager)
	{
		CTerminal *pPort = g_pTerminalManager->GetTerminalByID(Port);
		if (pPort)
		{
			iReturn = pPort->GetAudioData(MaxLength, 
									(unsigned char *)pPBXData, 
									(unsigned char *)pPhoneData, 
									(unsigned char *)pMergedData);
			pPort->Release();
		}
	}

/*
	_LOG("MP_GetAudioData(" 
		<< StringUtils::IntToString(Port) 
		<< ", " << StringUtils::IntToString(MaxLength)
		<< ", " << StringUtils::PtrToHex(pPBXData)
		<< ", " << StringUtils::PtrToHex(pPhoneData)
		<< ", " << StringUtils::PtrToHex(pMergedData)
		<< "): " << StringUtils::IntToString(iReturn));
*/
	return iReturn;
}

void MP_AudioMonitor( int Port, BOOLEAN Active )
{
	scope_locker ll(g_xApiLocker);

	if (g_pTerminalManager)
	{
		CTerminal *pPort = g_pTerminalManager->GetTerminalByID(Port);
		if (pPort)
		{
			pPort->RecordAudio(Active != FALSE);
			pPort->Release();
		}
	}

	_LOG("MP_AudioMonitor(" 
		<< StringUtils::IntToString(Port) 
		<< ", " << StringUtils::IntToString(Active)
		<< ")");

}

void MP_AutoRecord( int Port, BOOLEAN ActivateAutoRecord )
{
	scope_locker ll(g_xApiLocker);

	if (g_pTerminalManager)
	{
		CTerminal *pPort = g_pTerminalManager->GetTerminalByID(Port);
		if (pPort)
		{
			pPort->AutoRecordAudio(ActivateAutoRecord != FALSE);
			pPort->Release();
		}
	}

	_LOG("MP_AutoRecord(" 
		<< StringUtils::IntToString(Port) 
		<< ", " << StringUtils::IntToString(ActivateAutoRecord)
		<< ")");
}

void MP_ControlMonitor( int Port, BOOLEAN Active )
{
	scope_locker ll(g_xApiLocker);

	if (g_pTerminalManager)
	{
		CTerminal *pPort = g_pTerminalManager->GetTerminalByID(Port);
		if (pPort)
		{
			pPort->RecordCtlData(Active != FALSE);
			pPort->Release();
		}
	}

	_LOG("MP_ControlMonitor(" 
		<< StringUtils::IntToString(Port) 
		<< ", " << StringUtils::IntToString(Active)
		<< ")");
}

int MP_GetControlDataPBX( int Port, int MaxLength, char *pPBXData)
{
	scope_locker ll(g_xApiLocker);

	if (g_pTerminalManager)
	{
		CTerminal *pPort = g_pTerminalManager->GetTerminalByID(Port);
		if (pPort != NULL)
		{
			int iReturn = pPort->GetControlDataPBX(MaxLength, (unsigned char *)pPBXData);
			pPort->Release();
			return iReturn;
		}
	}
	return 0;
}

int MP_GetControlDataPhone( int Port, int MaxLength, char *pPhoneData)
{
	scope_locker ll(g_xApiLocker);

	if (g_pTerminalManager)
	{
		CTerminal *pPort = g_pTerminalManager->GetTerminalByID(Port);
		if (pPort != NULL)
		{
			int iReturn = pPort->GetControlDataPhone(MaxLength, (unsigned char *)pPhoneData);
			pPort->Release();
			return iReturn;
		}
	}
	return 0;
}

LPCSTR	MP_GetCallerID( int Port )
{
	scope_locker ll(g_xApiLocker);

	LPCSTR lpCallerID = g_szEmptyStr;

	if (g_pTerminalManager)
	{
		CTerminal *pPort = g_pTerminalManager->GetTerminalByID(Port);
		if (pPort != NULL)
		{
			lpCallerID = pPort->GetCallerID();
			pPort->Release();
		}
	}

	_LOG("MP_GetCallerID(" 
		<< StringUtils::IntToString(Port) 
		<< "): " << lpCallerID);

	return lpCallerID;
}

#ifdef COMVURGENTTEST

const char *MP_GetPortIP(int Port)
{
	scope_locker ll(g_xApiLocker);

	if (g_pTerminalManager)
	{
		CTerminal *pPort = g_pTerminalManager->GetTerminalByID(Port);
		if (pPort != NULL)
		{
			const char *szStr = pPort->GetIP();
			pPort->Release();
			return szStr;
		}
	}
	return g_szEmptyStr;
}

unsigned int MP_GetRecordTime(int Port)
{
	scope_locker ll(g_xApiLocker);

	if (g_pTerminalManager)
	{
		CTerminal *pPort = g_pTerminalManager->GetTerminalByID(Port);
		if (pPort != NULL)
		{
			uint32 uReturn = pPort->GetRecordTime();
			pPort->Release();
			return uReturn;
		}
	}
	return 0;
}

#endif//COMVURGENTTEST
