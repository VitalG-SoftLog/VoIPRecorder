
// SettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Settings.h"
#include "SettingsDlg.h"
#include <tinyxml.h>
#include "Defines.h"
#include "XmlUtils.h"
#include "../RecorderLib/MultiXtR.h"
#include "IPv4Address.h"

#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CSettingsDlg dialog




CSettingsDlg::CSettingsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSettingsDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSettingsDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CSettingsDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CSettingsDlg message handlers

BOOL CSettingsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	
	// create config directory...
	TCHAR szBuffer[MAX_PATH + 1];
	ZeroMemory(szBuffer, sizeof(szBuffer));
	SHGetSpecialFolderPath(NULL, szBuffer, CSIDL_COMMON_APPDATA, FALSE);
	CString strPath = szBuffer;
	CreateDirectory(strPath, NULL);
	strPath += _T("\\Intelligent recording");
	CreateDirectory(strPath, NULL);
	strPath += _T("\\Configuration");
	CreateDirectory(strPath, NULL);

	// save interfaces list and default settings...
	MP_Open();
	MP_Close();

	LoadInterfaceList();
	LoadSettings();

	// apply settings to UI..
	
	for (unsigned i = 0; i < m_xInterfaces.size(); i++)
	{
		if (m_sInterfaceName == m_xInterfaces[i])
		{
			CComboBox combo;
			combo.Attach(*GetDlgItem(IDC_COMBO_INTERFACE));
			combo.SetCurSel(i);
			combo.Detach();
			break;
		}
	}
	
	CIPv4Address ipAddr(m_sPBXIP);
	CIPAddressCtrl ipCtrl;
	ipCtrl.Attach(*GetDlgItem(IDC_IPADDRESS_PBX));
	ipCtrl.SetAddress(htonl(ipAddr.IP()));
	ipCtrl.Detach();

	CSpinButtonCtrl spin;
	spin.Attach(*GetDlgItem(IDC_SPIN1));
	spin.SetRange32(1, 0xffff);
	spin.SetPos32(m_uSIPPort);
	spin.Detach();

	UINT uCheck = 0;
	CButton Radio;
	if (m_iOutDataFormat == 6) {
		uCheck = IDC_RADIO_ALAW;
	} else {
		uCheck = IDC_RADIO_MULAW;
	}

	Radio.Attach(*GetDlgItem(uCheck));
	Radio.SetCheck(BST_CHECKED);
	Radio.Detach();

	uCheck = 0;
	if (m_sMode == "default") {
		uCheck = IDC_RADIO_MODE_DEFAULT;
	} else if (m_sMode == "nsip") {
		uCheck = IDC_RADIO_MODE_NSIP;
	}

	if (uCheck) {
		Radio.Attach(*GetDlgItem(uCheck));
		Radio.SetCheck(BST_CHECKED);
		Radio.Detach();
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSettingsDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSettingsDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSettingsDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CSettingsDlg::OnCbnSelchangeCombo1()
{
	// TODO: Add your control notification handler code here
}

std::string CSettingsDlg::GetConfigPath()
{
	std::string strXmlPath;
	char szBuffer[MAX_PATH+1] = {0};
	ZeroMemory(szBuffer, sizeof(szBuffer));
	SHGetSpecialFolderPathA(NULL, szBuffer, CSIDL_COMMON_APPDATA, FALSE);
	strXmlPath = szBuffer;
	strXmlPath += "\\Intelligent Recording\\Configuration\\VoIP.xml";
	return strXmlPath;
}

void CSettingsDlg::LoadSettings()
{
	// load XML
	TiXmlDocument doc(GetConfigPath().c_str());

	if (doc.LoadFile())
	{
		// loading settings section...
		TiXmlElement *pRoot = doc.FirstChildElement("engine");
		if (!pRoot)
			return;

		const char *pGuid = pRoot->Attribute("guid");
		if (!pGuid)
			return;

		// loading settings section
		TiXmlElement *pSection = pRoot->FirstChildElement("settings");

		if (!pSection)
			return;

		XmlUtils::LoadValStr("capture-device", m_sInterfaceName, pSection);
		XmlUtils::LoadValStr("pbx-ip", m_sPBXIP, pSection);
		XmlUtils::LoadValInt("sip-port", m_uSIPPort, pSection);
		std::string sOutFormat;
		XmlUtils::LoadValStr("out-format", sOutFormat, pSection);
		XmlUtils::LoadValStr("mode", m_sMode, pSection);

		if (sOutFormat == "UK") {
			m_iOutDataFormat = 0x0006;//WAVE_FORMAT_ALAW;
		} else if (sOutFormat == "US") {
			m_iOutDataFormat = 0x0007;//WAVE_FORMAT_MULAW;
		}
	}
}


void CSettingsDlg::SaveConfig()
{
	// modify XML file...
	TiXmlDocument Doc(GetConfigPath().c_str());

	if (Doc.LoadFile())
	{
		TiXmlElement * pRoot = Doc.FirstChildElement("engine");
		if (pRoot)
		{
			// prepare "settings" section
			TiXmlElement *pSection = new TiXmlElement("settings");

			TiXmlElement *entry = new TiXmlElement("entry");
			pSection->LinkEndChild(entry);
			entry->SetAttribute("name", "capture-device");
			entry->LinkEndChild(new TiXmlText(m_sInterfaceName.c_str()));

			entry = new TiXmlElement("entry");
			pSection->LinkEndChild(entry);
			entry->SetAttribute("name", "pbx-ip");
			entry->LinkEndChild(new TiXmlText(m_sPBXIP.c_str()));

			entry = new TiXmlElement("entry");
			pSection->LinkEndChild(entry);
			entry->SetAttribute("name", "sip-port");
			CStringA str; str.Format("%d", m_uSIPPort);
			entry->LinkEndChild(new TiXmlText(str));

			entry = new TiXmlElement("entry");
			pSection->LinkEndChild(entry);
			entry->SetAttribute("name", "out-format");
			
			if (m_iOutDataFormat == 6) {
				entry->LinkEndChild(new TiXmlText("UK"));
			} else {
				entry->LinkEndChild(new TiXmlText("US"));
			}

			entry = new TiXmlElement("entry");
			pSection->LinkEndChild(entry);
			entry->SetAttribute("name", "mode");
			entry->LinkEndChild(new TiXmlText(m_sMode.c_str()));

			// remove old Settings section....
			pRoot->RemoveChild(pRoot->FirstChildElement("settings"));
			// add new Settings section
			pRoot->LinkEndChild(pSection);
		}
		Doc.SaveFile();
	}
	else
	{
		MessageBox(_T("Can't find config file!"));
	}
}

void CSettingsDlg::LoadInterfaceList()
{
	CComboBox combo;
	combo.Attach(*GetDlgItem(IDC_COMBO_INTERFACE));

	m_xInterfaces.clear();
	TiXmlDocument Doc(GetConfigPath().c_str());

	if (Doc.LoadFile())
	{
		TiXmlElement * pRoot = Doc.FirstChildElement("engine");
		if (pRoot)
		{
			// prepare "settings" section
			TiXmlElement *pSection = pRoot->FirstChildElement("devices");
			if (pSection)
			{
				TiXmlElement *pDevice = pSection->FirstChildElement("device");
				for (;pDevice;pDevice=pDevice->NextSiblingElement("device"))
				{
					// read device name...
					const char *pName = pDevice->Attribute("name");
					const char *pDescription = pDevice->Attribute("description");
					if (pName)
					{
						m_xInterfaces.push_back(pName);
						CString strDescription(pName);

						if (pDescription)
						{
							strDescription = pDescription;
							if (strDescription.IsEmpty())
								strDescription = pName;
						}

						combo.AddString(strDescription);
					}
				}
			}

		}
	}
	combo.Detach();
}

void CSettingsDlg::OnBnClickedOk()
{
	// save settigs...
	CComboBox combo;
	combo.Attach(*GetDlgItem(IDC_COMBO_INTERFACE));
	int iSel = combo.GetCurSel();
	if (iSel != -1)
		m_sInterfaceName = m_xInterfaces[iSel];
	combo.Detach();


	CString strPort;
	GetDlgItem(IDC_EDIT_SIP_PORT)->GetWindowText(strPort);
	m_uSIPPort = StrToInt(strPort);
	
	CIPv4Address ip;

	CIPv4Address ipAddr(m_sPBXIP);
	CIPAddressCtrl ipCtrl;
	ipCtrl.Attach(*GetDlgItem(IDC_IPADDRESS_PBX));
	DWORD dwAddr;
	ipCtrl.GetAddress(dwAddr);
	ip.IP(htonl(dwAddr));
	ip.Port(0xffff);
	m_sPBXIP = ip.AsString();
	ipCtrl.Detach();

	CButton RadioAlaw;
	RadioAlaw.Attach(*GetDlgItem(IDC_RADIO_ALAW));

	if (RadioAlaw.GetCheck() == BST_CHECKED)
		m_iOutDataFormat = 6;
	else 
		m_iOutDataFormat = 7;

	RadioAlaw.Detach();
	

	CButton RadioModeDef;
	RadioModeDef.Attach(*GetDlgItem(IDC_RADIO_MODE_DEFAULT));
	
	m_sMode = "nsip";
	if (RadioModeDef.GetCheck() == BST_CHECKED) {
		m_sMode = "default";
	}

	RadioModeDef.Detach();
	SaveConfig();

	OnOK();
}
