// Header: RecorderMain.h
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "MultiXtR.h"
#include <sip_parser_api.h>

// Define: _MAX_PORTS_NUMBER
// Maximum connected ports
#define _MAX_PORTS_NUMBER		512

// Define: _IF_TYPE_NEC
// Type string for NEC-SIP terminals
#define _IF_TYPE_NEC			"NEiSIP"

// Define: _IF_TYPE_OTHER
// Type string for none-NEC SIP terminals
#define _IF_TYPE_OTHER			"UNiSIP"

// Define: _IF_TYPE_UNKNOWN
// Type string for unknown SIP terminals
#define _IF_TYPE_UNKNOWN		"UNiSIP"

// Define: _ALLOWED_PHONE_TYPE
// Allowed by DLL phone types (by default PHONE_TYPE_NEC)
#define _ALLOWED_PHONE_TYPE		PHONE_TYPE_NEC	

// Define: _SW_TYPE
// Version type of the current software library. This is a Intelligent Recording 
// product code, which is a string containing manufacturer, 
// hardware interface and audio encoding type.
#define _SW_TYPE				_IF_TYPE_NEC


#ifdef COMVURGENTTEST
const char	*MP_GetPortIP(int Port);
unsigned int	MP_GetRecordTime(int Port);
#define MP_CALL_RECORD_END	10

#endif//COMVURGENTTEST
