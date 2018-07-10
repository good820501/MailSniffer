// MailFinderDlg.h : header file
//

#if !defined(AFX_MAILFINDERDLG_H__2BB02CD9_4038_4B0F_A176_5A81E1320017__INCLUDED_)
#define AFX_MAILFINDERDLG_H__2BB02CD9_4038_4B0F_A176_5A81E1320017__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "winsock.h"

#define EVENT_CHECKMAIL 0x00ABCDEF
#define EVENT_SHOWHINT  0x0000AFFE
#define EVENT_NEWMSG	0x00BACDFE
#define WM_TRAYNOTIFY	0x0000A44C
#define ONE_SECOND		0x000003E8
#define POP3_PORT		0x006E
#define MAX_RECV		0x255
DWORD WINAPI ThreadProc(LPVOID lpParameter);
void OnSocketMsg();	
void HandleDataRecv(unsigned char* data, int datalen);
void ShowHintWindow(CString Caption);
/////////////////////////////////////////////////////////////////////////////
// CMailFinderDlg dialog
#include "XpButton.h"
#include "HoverEdit.h"
class CMailFinderDlg : public CDialog
{
// Construction
public:
	
	BOOL CreateTCPSocket(SOCKET* sock);
	BOOL CheckMail(SOCKET* sock);
	void OnTimer(unsigned int EventID);
	void CenterWindow();
	BOOL InitCheckMail();
	int GetSettingsFile(char* buffer, int bufflen);
	void CheckSettings();
	void OnTrayClick();
	CMailFinderDlg(CWnd* pParent = NULL);	// standard constructor
	CBitmap m_bmpBackground;
// Dialog Data
	//{{AFX_DATA(CMailFinderDlg)
	enum { IDD = IDD_MAILFINDER_DIALOG };
	CHoverEdit	m_Start;
	CHoverEdit	m_Minutes;
	CHoverEdit	m_Voice;
	CHoverEdit	m_Pass;
	CHoverEdit	m_Account;
	CHoverEdit	m_ServerName;
	CXPButton	m_SaveBtn;
	CXPButton	m_HideBtn;
	CXPButton	m_FindBtn;
	CXPButton	m_CanBtn;
	CXPButton	m_OKBtn;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMailFinderDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	
	// Generated message map functions
	//{{AFX_MSG(CMailFinderDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnButton2();
	virtual void OnOK();
	afx_msg void OnButton3();
	afx_msg void OnButton1();
	afx_msg void OnButton4();
	afx_msg void OnButton5();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAILFINDERDLG_H__2BB02CD9_4038_4B0F_A176_5A81E1320017__INCLUDED_)
