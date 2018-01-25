
// SettingsDlg.h : header file
//

#pragma once
#include <vector>
#include <string>

// CSettingsDlg dialog
class CSettingsDlg : public CDialog
{
// Construction
public:
	CSettingsDlg(CWnd* pParent = NULL);	// standard constructor

	void LoadInterfaceList();
	void LoadSettings();
	std::string GetConfigPath();
	void SaveConfig();



// Dialog Data
	enum { IDD = IDD_SETTINGS_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	std::vector<std::string> m_xInterfaces;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

	std::string		m_sInterfaceName;
	std::string		m_sPBXIP;
	unsigned short	m_uSIPPort;
	int				m_iOutDataFormat;
	std::string		m_sMode;



	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnBnClickedOk();
};
