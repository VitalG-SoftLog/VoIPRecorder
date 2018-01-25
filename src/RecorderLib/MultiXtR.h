//
// FILENAME: MultiXtR.h
// PROJECT:  Xtension Recorder (All variants)
//
// PURPOSE:
//	This file is the include file specifying the interface for 
// the Xtension recorder Multiple interface software library.
//
// Copyright 2003 Comvurgent Ltd. Nottingham. UK.
// www.comvurgent.com
//
// HISTORY:
// 03-Nov-2003	Nigel Gaunt		File created.
//

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the LOWLEVEL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// LOWLEVEL_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef COMVURGENTTEST
#define	MULTIXTR_API
#else
#ifdef MULTIXTR_EXPORTS
#define MULTIXTR_API __declspec(dllexport)
#else
#define MULTIXTR_API __declspec(dllimport)
#endif
#endif

// Define standard types
#ifndef BOOLEAN
#define	BOOLEAN	int
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE	0
#endif

// Possible Callback Events
#define	MP_STATUS					1
#define	MP_BCHANNELDATA				2
#define	MP_DCHANNELDATAPBX			3
#define MP_DCHANNELDATAPHONE		4
#define MP_BCHANNELOVERFLOW			5
#define MP_DCHANNELOVERFLOWPBX		6
#define MP_DCHANNELOVERFLOWPHONE	7
#define MP_CALLSTATE				8
#define MP_CALLERIDUPDATE			9

// Possible status reports, either from GetStatus(), or a MP_STATUS callback
#define	STATUS_GOOD				 0		// Interface and telephone OK
#define STATUS_NOTPRESENT		-1		// Interface not connected
#define STATUS_UNLICENSED		-2		// The device is connected, but is for different keysets to the current software.
#define	STATUS_NOPHONE			-3		// Device present, Exchange detected, no response from phone
#define STATUS_NOEXCHANGE		-4		// Device present. Exchange not detected
#define STATUS_UNKNOWNERROR		-5		// Device present. No communication. Unable to determine if phone or exchange is at fault.
#define STATUS_NOTINITIALISED	-6		// You have not initialised the DLL
#define STATUS_INTERNALERROR	-7		// Failure during device initialisation
#define STATUS_TOOMANY			-8		// This device is connected, but cannot be used as there are too many devices connected
// May also be 1-100, initialising %, will count up as the device initialises

// Possible call states. Either from GetCallState(), or MP_CALLSTATE callback
// The DLL will give you as much information as possible
// Please handle all of these, as different phone types will give different levels of information
#define	CALL_IDLE				0
#define CALL_ACTIVE				1
#define CALL_INCOMING			2
#define CALL_OUTGOING			3
#define CALL_INCOMING_INTERNAL	4
#define CALL_INCOMING_EXTERNAL	5
#define CALL_OUTGOING_INTERNAL	6
#define CALL_OUTGOING_EXTERNAL	7
#define CALL_PAGE				8

// Determine what information is required from the audio or auto-audio
// You can always ask for everything, but asking for less may reduce
// the bandwidth used by the USB device.
#define	AUDIO_OFF		0
#define	AUDIO_MONO		1
#define	AUDIO_STEREO	2
#define	AUDIO_ON		3

// Typedef which may be used for callback functions
typedef void MULTIXTRCALLBACK( int Port, int Event, int Data );

// Interface Functions
// See Comvurgent API specification for details

// Functions which affect the entire DLL
MULTIXTR_API	void	MP_Open( void );
MULTIXTR_API	void	MP_Close( void );
MULTIXTR_API	LPCSTR  MP_GetSwType( void );
MULTIXTR_API	LPCSTR  MP_GetSwVersion( void );
MULTIXTR_API	void	MP_SetCallback( MULTIXTRCALLBACK *MyProc );
MULTIXTR_API	int		MP_GetDataFormat( void );
MULTIXTR_API	short	MP_ConvertData( const char Data8bit );

// Functions to determine Port numbers of connected devices
MULTIXTR_API	int		MP_GetMaxNumPorts( void );
MULTIXTR_API	int		MP_GetIfSerialNo( int PortNumber );	// 0=No device

// Functions which affect a single connected port
MULTIXTR_API	int		MP_GetStatus( int Port );
MULTIXTR_API	int		MP_GetCallState( int Port );
MULTIXTR_API	LPCSTR	MP_GetIfType( int Port );
MULTIXTR_API	void	MP_SetAudioGain( int Port, BOOL Remote, int Gain );		// Gain = fixed point / 256
MULTIXTR_API	int		MP_GetAudioData( int Port, int MaxLength, char *pPBXData, char *pPhoneData, char *pMergedData );
MULTIXTR_API	void	MP_AudioMonitor( int Port, BOOLEAN Active );
MULTIXTR_API	void	MP_AutoRecord( int Port, BOOLEAN ActivateAutoRecord );

MULTIXTR_API	void	MP_ControlMonitor( int Port, BOOLEAN Active );
MULTIXTR_API	int		MP_GetControlDataPBX( int Port, int MaxLength, char *pPBXData);
MULTIXTR_API	int		MP_GetControlDataPhone( int Port, int MaxLength, char *pPhoneData);
MULTIXTR_API	LPCSTR	MP_GetCallerID( int Port );



