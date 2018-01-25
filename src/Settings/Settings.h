
// Settings.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CSettingsApp:
// See Settings.cpp for the implementation of this class
//

class CSettingsApp : public CWinAppEx
{
public:
	CSettingsApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CSettingsApp theApp;