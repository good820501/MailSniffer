// MailFinderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MailFinder.h"
#include "MailFinderDlg.h"
#include <mmsystem.h>	// for wav playing
#include "msgwnd.h"
#define SETTING_FILE	"POP3CHECKER.SET"
HANDLE t_handle;
HICON main_icon;
HICON blank_icon;
BOOL  doblink;
int current_ticks;
HWND hWnd;
SOCKET sock;
fd_set read_list;
bool Stages[3];
NOTIFYICONDATA nid;
int timetowait;
char* server ;
char* password;
char* account;
char* wavfile;
char* startfile;
int   OldNums;
static char szFileBuff[266];
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
unsigned int gfs(FILE* f);
void HandleErrorMessage(unsigned char* data, int datalen);
void HandleStage(int stage);
void HandleStatsRecv(unsigned char* data, int datalen);
void CloseDownSocket(SOCKET* sock);
void PlayWav();
char* GetFile(char* title, char* ext);
/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMailFinderDlg dialog

CMailFinderDlg::CMailFinderDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMailFinderDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMailFinderDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	timetowait = 0;
	server = NULL;
	password = NULL;
	account = NULL;
	wavfile = NULL;
	startfile=NULL;
	current_ticks=0;
	OldNums=0;
	doblink=false;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMailFinderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMailFinderDlg)
	DDX_Control(pDX, IDC_EDIT6, m_Start);
	DDX_Control(pDX, IDC_EDIT5, m_Minutes);
	DDX_Control(pDX, IDC_EDIT4, m_Voice);
	DDX_Control(pDX, IDC_EDIT3, m_Pass);
	DDX_Control(pDX, IDC_EDIT2, m_Account);
	DDX_Control(pDX, IDC_EDIT1, m_ServerName);
	DDX_Control(pDX, IDC_BUTTON3, m_SaveBtn);
	DDX_Control(pDX, IDC_BUTTON2, m_HideBtn);
	DDX_Control(pDX, IDC_BUTTON1, m_FindBtn);
	DDX_Control(pDX, IDCANCEL, m_CanBtn);
	DDX_Control(pDX, IDOK, m_OKBtn);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMailFinderDlg, CDialog)
	//{{AFX_MSG_MAP(CMailFinderDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_LBUTTONDOWN()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON2, OnButton2)
	ON_BN_CLICKED(IDC_BUTTON3, OnButton3)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_BN_CLICKED(IDC_BUTTON4, OnButton4)
	ON_BN_CLICKED(IDC_BUTTON5, OnButton5)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMailFinderDlg message handlers

BOOL CMailFinderDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
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

	CheckSettings();

	WSAData wsad;
	WSAStartup(0x0101, &wsad);

	main_icon = AfxGetApp()->LoadIcon(MAKEINTRESOURCE(IDI_ICONMAIN));
	blank_icon = AfxGetApp()->LoadIcon(MAKEINTRESOURCE(IDR_MAINFRAME));
	if(main_icon)
		::SendMessage(this->GetSafeHwnd(), WM_SETICON, ICON_SMALL, (LPARAM)(HICON)main_icon);
	m_bmpBackground.LoadBitmap(IDB_BITMAP1);	/// 加载背景

	nid.cbSize = sizeof(nid);
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	nid.uCallbackMessage = WM_TRAYNOTIFY;
	nid.hIcon = main_icon;
	nid.hWnd = this->GetSafeHwnd();
	nid.uID = (int)this->GetSafeHwnd();
	strcpy(nid.szTip, "POP3 邮件自动检查...");
	Shell_NotifyIcon(NIM_ADD, &nid);
	// TODO: Add extra initialization here
	hWnd=this->GetSafeHwnd();
	if(InitCheckMail())
	{
		if(::IsDlgButtonChecked(this->GetSafeHwnd(), IDC_CHECK1))
			::SetTimer(this->GetSafeHwnd(), EVENT_CHECKMAIL, ONE_SECOND, NULL);
	}else
	{
		::SetWindowText(GetDlgItem(IDC_STATUS)->GetSafeHwnd(), "请设定一个账户!");
	}
	::SetTimer(this->GetSafeHwnd(), EVENT_SHOWHINT, ONE_SECOND, NULL);
	CenterWindow();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMailFinderDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CMailFinderDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

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
		CPaintDC dc(this);
		CRect rect;
		GetClientRect(&rect);
		CDC dcMem; 
		dcMem.CreateCompatibleDC(&dc); 
		BITMAP bitMap;
		m_bmpBackground.GetBitmap(&bitMap);
		CBitmap *pbmpOld=dcMem.SelectObject(&m_bmpBackground);
		dc.StretchBlt(0,0,rect.Width(),rect.Height(),&dcMem,0,0,bitMap.bmWidth,bitMap.bmHeight,SRCCOPY);
		
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMailFinderDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CMailFinderDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
PostMessage(WM_NCLBUTTONDOWN,HTCAPTION,MAKELPARAM(point.x, point.y));	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnLButtonDown(nFlags, point);
}

void CMailFinderDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	::KillTimer(hWnd, EVENT_CHECKMAIL);	
	::KillTimer(hWnd, EVENT_SHOWHINT);	
	CloseDownSocket(&sock);
	
	WSACleanup();
	Shell_NotifyIcon(NIM_DELETE, &nid);
	if(main_icon)
		FreeResource(main_icon);
	if(server)
		free(server);
	if(password)
		free(password);
	if(account)
		free(account);
	if(wavfile)
		free(wavfile);
	if(startfile)
		free(startfile);
	// TODO: Add your message handler code here
	
}

LRESULT CMailFinderDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	switch(message)
	{
		case WM_TRAYNOTIFY:
			switch(LOWORD(lParam))
			{
			case WM_LBUTTONDBLCLK:
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_RBUTTONDBLCLK:
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
				OnTrayClick();
				break;
			}
			break;
		case WM_TIMER:
				OnTimer(wParam);
				break;
			default:
				break;
	}
	return CDialog::DefWindowProc(message, wParam, lParam);
}

void CMailFinderDlg::OnTrayClick()
{
	//ShowWindow(this->GetSafeHwnd(), SW_SHOW);
	this->ShowWindow(SW_SHOW);
	::SetForegroundWindow(this->GetSafeHwnd());
	::SetActiveWindow(this->GetSafeHwnd());
	
	nid.hIcon = main_icon;
	Shell_NotifyIcon(NIM_MODIFY, &nid);
	doblink = false;
}

void CMailFinderDlg::OnButton2() 
{
	this->ShowWindow(SW_HIDE);
	
}

void CMailFinderDlg::OnOK() 
{
	// TODO: Add extra validation here
	if (CheckMail(&sock))
	{
	}
	else
	{
		AfxMessageBox("网络连接失败，请检查配置。");
	}
}

void CMailFinderDlg::CheckSettings()
{
	char sfile[MAX_PATH];
	GetSettingsFile(sfile, MAX_PATH);
	
	FILE* s = fopen(sfile, "rb");
	if(!s)
		return;
	int fsize = gfs(s);
	if(!fsize)
		return;
	char* pguts = (char*)malloc(fsize);
	char* guts = pguts;
	if(!guts){
		fclose(s);
		return;
	}
	if(fread((void*)guts, fsize, 1, s) != 1){
		fclose(s);
		free(guts);
		return;
	}
	fclose(s);
	for(int i = 0; i < fsize; i++){
		if((guts[i] == 0x0D) || (guts[i] == 0x0A))
			guts[i] = 0x0;
	}
	for(i = 0; i < 7; i++){
		if(strlen(guts))
		{
			if(i == 0)
			{
				m_ServerName.SetWindowText(guts);
			}
			else if(i == 1)
			{
				m_Account.SetWindowText(guts);
			}
			else if(i == 2)
			{
				m_Pass.SetWindowText(guts);
			}
			else if(i == 3)
			{
				::CheckDlgButton(this->GetSafeHwnd(), IDC_CHECK1, atoi(guts));
			}
			else if(i == 4)
			{
				m_Minutes.SetWindowText(guts);
			}
			else if(i == 5)
			{
				m_Voice.SetWindowText(guts);
			}
			else if(i == 6)
			{
				m_Start.SetWindowText(guts);
			}
		}
		guts += strlen(guts) + 2;
	}
	free(pguts);
}

int CMailFinderDlg::GetSettingsFile(char *buffer, int bufflen)
{
	GetWindowsDirectory(buffer, bufflen);
	if(buffer[strlen(buffer) - 1] != '\\')
		strcat(buffer, "\\");
	strcat(buffer, SETTING_FILE);
	return strlen(buffer);
}
unsigned int gfs(FILE* f)
{
	if(!f)
		return 0;
	
	fseek(f, 0, SEEK_END);
	unsigned int fLen = ftell(f);
	rewind(f);
	
	return fLen;
}

BOOL CMailFinderDlg::InitCheckMail()
{
	for(int i = 0; i < 3; i++)
	{Stages[i] = false;}
	
	if(server)
		free(server);
	if(password)
		free(password);
	if(account)
		free(account);
	if(wavfile)
		free(wavfile);
	if(startfile)
		free(startfile);
	int len;
	len = ::GetWindowTextLength(::GetDlgItem(this->GetSafeHwnd(), IDC_EDIT1));
	if(!len)
		return false;
	server = (char*)malloc(len + 1);
	if(!server)
		return false;
	::GetWindowText(::GetDlgItem(this->GetSafeHwnd(), IDC_EDIT1), server, len + 1);
	
	len = ::GetWindowTextLength(::GetDlgItem(this->GetSafeHwnd(), IDC_EDIT3));
	if(!len)
		return false;
	password = (char*)malloc(len + 1);
	if(!password)
		return false;
	::GetWindowText(::GetDlgItem(this->GetSafeHwnd(), IDC_EDIT3), password, len + 1);
	
	char temp[10];
	::GetWindowText(::GetDlgItem(this->GetSafeHwnd(), IDC_EDIT5), temp, 10);
	timetowait = atoi(temp);
	
	len = ::GetWindowTextLength(::GetDlgItem(this->GetSafeHwnd(), IDC_EDIT2));
	if(!len)
		return false;
	account = (char*)malloc(len + 1);
	if(!account)
		return false;
	::GetWindowText(::GetDlgItem(this->GetSafeHwnd(), IDC_EDIT2), account, len + 1);
	
	len = ::GetWindowTextLength(::GetDlgItem(this->GetSafeHwnd(), IDC_EDIT4));
	if(!len)
	{
		wavfile = NULL;
		return true;
	}
	wavfile = (char*)malloc(len + 1);
	if(!wavfile)
		return true;
	::GetWindowText(::GetDlgItem(this->GetSafeHwnd(), IDC_EDIT4), wavfile, len + 1);
	
	len = ::GetWindowTextLength(::GetDlgItem(this->GetSafeHwnd(), IDC_EDIT6));
	if(!len)
	{
		startfile = NULL;
		return true;
	}
	startfile = (char*)malloc(len + 1);
	if(!startfile)
		return true;
	::GetWindowText(::GetDlgItem(this->GetSafeHwnd(), IDC_EDIT4), startfile, len + 1);	


	return true;
}

void CMailFinderDlg::CenterWindow()
{
	/*RECT rect, rect2;
	::GetWindowRect(GetDesktopWindow(), &rect);
	::GetClientRect(this->GetSafeHwnd(), &rect2);
	int left, right, top, bottom;
	left = rect.right / 2 - rect2.right / 2;
	top  = rect.bottom / 2 - rect2.bottom / 2;
	right = rect2.right + (GetSystemMetrics(SM_CXFRAME) * 2);
	bottom = rect2.bottom + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CXFRAME);
	SetWindowPos(this->GetSafeHwnd(), NULL, left, top, right, bottom, SWP_NOZORDER | SWP_NOACTIVATE);
	*/
}

void CMailFinderDlg::OnTimer(unsigned int EventID)
{
	// check mail
	if(EventID != EVENT_CHECKMAIL)
		return;
	
	if(doblink)
	{
		if(nid.hIcon == main_icon)
			nid.hIcon = blank_icon;
		else
			nid.hIcon = main_icon;
		ShowHintWindow(nid.szTip);
		doblink=false;
	}else
	{
		nid.hIcon = main_icon;
	}
	
	Shell_NotifyIcon(NIM_MODIFY, &nid);
	
	current_ticks++;
	
	char buff[25];
	wsprintf(buff, "(Time: %d seconds)", (timetowait * 60) - current_ticks);
	//SetWindowText(GetDlgItem(this->GetSafeHwnd(), IDC_TIME), buff);
	
	if(current_ticks / 60 < timetowait)
		return;
	
	current_ticks = 0;
	
	CheckMail(&sock);
}

BOOL CMailFinderDlg::CheckMail(SOCKET *sock)
{
	if(!sock)
		return false;
	
	if(!CreateTCPSocket(sock))
	{
		AfxMessageBox("Could not create a TCP socket!");
		return false;
	}
	
	if(!InitCheckMail())
	{
		AfxMessageBox("Missing parameters!  Make sure you have filled in all parameters!");
		return false;
	}
	
	hostent* h_entity;
	sockaddr_in addr;
	
	unsigned long ip = inet_addr(server);
	if(ip == 0xFFFFFFFF)
	{
		h_entity = gethostbyname(server);
		if(!h_entity)
			return false;
		memcpy(&ip, h_entity->h_addr_list[0], sizeof(int));
	}
	
	memcpy(&addr.sin_addr, &ip, sizeof(int));
	addr.sin_port = htons(POP3_PORT);
	addr.sin_family = AF_INET;
	memset(&addr.sin_zero, 0x0, 0x08);
	
	DWORD id;
	
	if(t_handle)
	{
		DWORD ec;
		GetExitCodeThread(t_handle, &ec);
		if(ec == STILL_ACTIVE)
			TerminateThread(t_handle, 0x03);
		CloseHandle(t_handle);
		t_handle = NULL;
	}
	
	t_handle = CreateThread(NULL, 0, ThreadProc, 0, 0, &id); 
	if(!t_handle)
		return false;
	
	//SetWindowText(GetDlgItem(hWnd, IDC_STATUS), "Checking Mail...");
	::EnableWindow(::GetDlgItem(this->GetSafeHwnd(), IDOK), FALSE);	
	return (connect(*sock, (sockaddr*)&addr, sizeof(addr)) != -1);
}

BOOL CMailFinderDlg::CreateTCPSocket(SOCKET *sock)
{
	*sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	
	if(*sock == SOCKET_ERROR)
		return false;
	
	FD_ZERO(&read_list);
	FD_SET(*sock, &read_list);
	
	return true;
}

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
	while(true)
	{
		int d = select(sock + 1, &read_list, NULL, NULL, NULL);
		if(d <= 0)
			break;
		if(FD_ISSET(sock, &read_list))
		{
			OnSocketMsg();
		}
	}
	return 0;
}

void OnSocketMsg()
{
	unsigned char buff[MAX_RECV];
	memset(&buff[0], 0x0, MAX_RECV);
	
	int total = recv(sock, (char*)&buff[0], MAX_RECV, 0);
	if(total == -1)
		return;
	
	int pos = 0;
	// this will parce a packet by carrage return and line feed, or just line feed
	// also changes all spaces to nulls
	for(int i = 0; i < total; i++)
	{
		if(buff[i] == 0x20)
			buff[i] = 0x0;
		if((buff[i] == 0x0A) || (buff[i] == 0x0D))
		{
			buff[i] = 0x0;
			HandleDataRecv(&buff[pos], i);
			pos = i;
			if(buff[pos + 1] == 0x0A)
			{
				pos += 2;
				i++;
			}
		}
	}
}

void HandleDataRecv(unsigned char *data, int datalen)
{
	if(strcmp((char*)data, "-ERR") == 0)
	{
		HandleErrorMessage(data, datalen);
		return;
	}
	if(!Stages[0])
	{
		Stages[0] = true;
		HandleStage(0);
	}else if(!Stages[1])
	{
		Stages[1] = true;
		HandleStage(1);
	}else if(!Stages[2])
	{
		Stages[2] = true;
		HandleStage(2);
	}else
	{
		HandleStatsRecv(data, datalen);
	}
}
void HandleErrorMessage(unsigned char* data, int datalen)
{
	for(int i = 0; i < datalen; i++)
	{
		if(data[i] == 0)
			data[i] = 0x20;
	}
	
	//SetWindowText(GetDlgItem(hWnd, IDC_STATUS), (char*)data);
	EnableWindow(GetDlgItem(hWnd, IDOK), TRUE);
	
	strcpy(nid.szTip, (char*)data);
	Shell_NotifyIcon(NIM_MODIFY, &nid);
	
	CloseDownSocket(&sock);
}
void HandleStage(int stage)
{
	char sdata[255];
	sdata[0] = 0x0;
	if(stage == 0)
	{
		wsprintf(sdata, "USER %s%c%c", account, 0x0d, 0x0a);
	}else if(stage == 1)
	{
		wsprintf(sdata, "PASS %s%c%c", password, 0x0d, 0x0a);
	}else if(stage == 2)
	{
		wsprintf(sdata, "STAT%c%c", 0x0d, 0x0a);
	}
	if(strlen(sdata))
		send(sock, sdata, strlen(sdata), 0);
}
void HandleStatsRecv(unsigned char* data, int datalen)
{
	
	char* p_data = (char*)data;
	p_data += strlen(p_data) + 1;
	int msgs = atoi(p_data);
	p_data += strlen(p_data) + 1;
	int size = atoi(p_data);
	
	char buff[255];
	
	if (OldNums==0) 
	{
		OldNums=msgs;
		wsprintf(buff, "您的邮箱里有: %d 封邮件.", msgs);
	}
	else
	{
		if (msgs>OldNums)
		{
			wsprintf(buff, "您的邮箱里有: %d 封新邮件.", msgs-OldNums);
		}
		else
			wsprintf(buff, "您的邮箱里有: %d 封邮件.", msgs);
	}
	
	SetWindowText(GetDlgItem(hWnd, IDC_STATUS), buff);
	EnableWindow(GetDlgItem(hWnd, IDOK), TRUE);
	
	if(msgs)
	{
		doblink = true;		
		PlayWav();		
	}else
	{
		nid.hIcon = main_icon;
		doblink = false;
	}
	
	strcpy(nid.szTip, buff);
	Shell_NotifyIcon(NIM_MODIFY, &nid);
	
	CloseDownSocket(&sock);
}
void CloseDownSocket(SOCKET* sock)
{
	if(!sock)
		return;
	closesocket(*sock);
	*sock = 0;
	DWORD ec;
	GetExitCodeThread(t_handle, &ec);
	if(ec == STILL_ACTIVE)
		TerminateThread(t_handle, 0x03);
}
void PlayWav()
{
	sndPlaySound(wavfile, SND_ASYNC);
}

void CMailFinderDlg::OnButton3() 
{
	// TODO: Add your control notification handler code here
	if(!InitCheckMail())
	{
		AfxMessageBox("请完整填写所有内容!");
		return;
	}
	current_ticks = 0;
	
	if(::IsDlgButtonChecked(this->GetSafeHwnd(), IDC_CHECK1))
		::SetTimer(hWnd, EVENT_CHECKMAIL, ONE_SECOND, NULL);
	else
	{
		::KillTimer(hWnd, EVENT_CHECKMAIL);
		AfxMessageBox("请设置间隔时间!");
		return;
	}
	
	char sfile[MAX_PATH];
	GetSettingsFile(sfile, MAX_PATH);
	FILE* s = fopen(sfile, "wb");
	if(!s)
	{
		AfxMessageBox("读取配置文件失败!");
		return;
	}
	fprintf(s, "%s%c%c", server, 0x0d, 0x0a);
	fprintf(s, "%s%c%c", account, 0x0d, 0x0a);
	fprintf(s, "%s%c%c", password, 0x0d, 0x0a);
	fprintf(s, "%d%c%c", ::IsDlgButtonChecked(hWnd, IDC_CHECK1), 0x0d, 0x0a);
	fprintf(s, "%d%c%c", timetowait, 0x0d, 0x0a);
	
	//fprintf(s, "%s%c%c", startfile, 0x0d, 0x0a);

	if(wavfile)
		fprintf(s, "%s%c%c", wavfile, 0x0d, 0x0a);
	else
		fprintf(s, "%c%c", 0x0d, 0x0a);

	if(startfile)
		fprintf(s, "%s%c%c", startfile, 0x0d, 0x0a);
	else
		fprintf(s, "%c%c", 0x0d, 0x0a);
	fclose(s);
	
	::MessageBox(hWnd, "新的设置已保存!", "Saved", MB_OK);
}


void CMailFinderDlg::OnButton1() 
{
	char* wav = GetFile("查找wav文件...", "WAV");
	if(wavfile)
		free(wavfile);
	wavfile = (char*)malloc(strlen(wav) + 1);
	strcpy(wavfile, wav);
	m_Voice.SetWindowText(wavfile);
	//::SetWindowText(::GetDlgItem(hWnd, IDC_EDIT4), wavfile);	// TODO: Add your control notification handler code here
	
}
char* GetFile(char* title, char* ext)
{
	OPENFILENAME ofn;
	char exten[55];
	
	wsprintf(exten, "%s Files%c*.%s%cAll Files%c*.*%c", ext, 0, ext, 0, 0, 0);
	
	szFileBuff[0] = 0x0;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = szFileBuff;
	ofn.lpstrTitle = title;
	ofn.nMaxFile = sizeof(szFileBuff);
	ofn.lpstrFilter = exten;
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | 4;
	
	if(!GetOpenFileName(&ofn)){szFileBuff[0] = 0x0;}
	
	return szFileBuff;
}

void CMailFinderDlg::OnButton4() 
{
	// TODO: Add your control notification handler code here
	CString Content="您的邮箱里有3封新邮件!";
	ShowHintWindow(Content);
	
}

void ShowHintWindow(CString Caption)
{
	CMsgWnd *pMsgWindow=new CMsgWnd;
	CMsgWnd& msgWindow=*pMsgWindow;
	msgWindow.CreateMsgWindow();
	msgWindow.SetPromptCaption("邮件提示");
	msgWindow.SetPromptMessage(LPCTSTR(Caption));
	::BringWindowToTop(msgWindow.GetSafeHwnd());
	msgWindow.ShowWindow(SW_SHOW);
	msgWindow.UpdateWindow();
}

void CMailFinderDlg::OnButton5() 
{
	// TODO: Add your control notification handler code here
	char* wav = GetFile("指定启动的文件...", "exe");
	if(wavfile)
		free(wavfile);
	wavfile = (char*)malloc(strlen(wav) + 1);
	strcpy(wavfile, wav);
	m_Start.SetWindowText(wavfile);
}
