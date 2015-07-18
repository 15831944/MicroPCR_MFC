
// MicroPCRDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "MicroPCR.h"
#include "MicroPCRDlg.h"
#include "ConvertTool.h"

#include <locale.h>
#include <mmsystem.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMicroPCRDlg ��ȭ ����

static const CString PID_TABLE_COLUMNS[5] = { L"Start Temp", L"Target Temp", L"Kp", L"Kd", L"Ki" };


CMicroPCRDlg::CMicroPCRDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMicroPCRDlg::IDD, pParent)
	, openConstants(true)	// true �� �� ��, �Լ��� ȣ���Ͽ� �ݵ��� �Ѵ�. 
	, openGraphView(false)
	, isConnected(false)
	, m_cMaxActions(1000)
	, m_cTimeOut(1000)
	, m_cArrivalDelta(0.5)
	, m_cTemperature(0)
	, m_sProtocolName(L"")
	, m_totalActionNumber(0)
	, m_currentActionNumber(-1)
	, actions(NULL)
	#ifdef USE_MMTIMER
	, m_Timer(NULL)
	#endif
	, m_nLeftSec(0)
	, m_blinkCounter(0)
	, m_timerCounter(0)
	, recordFlag(false)
	, blinkFlag(false)
	, isRecording(false)
	, isStarted(false)
	, isCompletePCR(false)
	, isTargetArrival(false)
	, isFirstDraw(false)
	, m_startTime(0)
	, m_nLeftTotalSec(0)
	, m_prevTargetTemp(0)
	, m_currentTargetTemp(0)
	, m_timeOut(0)
	, m_leftGotoCount(-1)
	, m_recordingCount(0)
	, m_recStartTime(0)
	, m_cGraphYMin(0)
	, m_cGraphYMax(4096)
	, ledControl(1)
	, currentCmd(CMD_READY)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CMicroPCRDlg::~CMicroPCRDlg()
{
	// ������ ��ü�� �Ҹ��ڿ��� �������ش�.
	if( device != NULL )
		delete device;
	if( actions != NULL )
		delete []actions;
#ifdef USE_MMTIMER
	if( m_Timer != NULL )
		delete m_Timer;
#endif
}

void CMicroPCRDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS_CHAMBER_TEMP, m_cProgressBar);
	DDX_Control(pDX, IDC_CUSTOM_PID_TABLE, m_cPidTable);
	DDX_Text(pDX, IDC_EDIT_MAX_ACTIONS, m_cMaxActions);
	DDV_MinMaxInt(pDX, m_cMaxActions, 1, 10000);
	DDX_Text(pDX, IDC_EDIT_TIME_OUT, m_cTimeOut);
	DDV_MinMaxInt(pDX, m_cTimeOut, 1, 10000);
	DDX_Text(pDX, IDC_EDIT_ARRIVAL_DELTA, m_cArrivalDelta);
	DDV_MinMaxFloat(pDX, m_cArrivalDelta, 0, 10.0);
	DDX_Text(pDX, IDC_EDIT_CHAMBER_TEMP, m_cTemperature);
	DDX_Control(pDX, IDC_LIST_PCR_PROTOCOL, m_cProtocolList);
	DDX_Text(pDX, IDC_EDIT_GRAPH_Y_MIN, m_cGraphYMin);
	DDV_MinMaxInt(pDX, m_cGraphYMin, 0, 4096);
	DDX_Text(pDX, IDC_EDIT_Y_MAX, m_cGraphYMax);
	DDV_MinMaxInt(pDX, m_cGraphYMax, 0, 4096);
}

BEGIN_MESSAGE_MAP(CMicroPCRDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DEVICECHANGE()
	ON_MESSAGE(WM_SET_SERIAL, SetSerial)
#ifdef USE_MMTIMER
	ON_MESSAGE(WM_MMTIMER, OnmmTimer)
#else
	ON_WM_TIMER()
#endif
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_CONSTANTS, &CMicroPCRDlg::OnBnClickedButtonConstants)
	ON_BN_CLICKED(IDC_BUTTON_CONSTANTS_APPLY, &CMicroPCRDlg::OnBnClickedButtonConstantsApply)
	ON_BN_CLICKED(IDC_BUTTON_PCR_START, &CMicroPCRDlg::OnBnClickedButtonPcrStart)
	ON_BN_CLICKED(IDC_BUTTON_PCR_OPEN, &CMicroPCRDlg::OnBnClickedButtonPcrOpen)
	ON_BN_CLICKED(IDC_BUTTON_PCR_RECORD, &CMicroPCRDlg::OnBnClickedButtonPcrRecord)
	ON_BN_CLICKED(IDC_BUTTON_GRAPHVIEW, &CMicroPCRDlg::OnBnClickedButtonGraphview)
	ON_BN_CLICKED(IDC_BUTTON_BOOTLOADER, &CMicroPCRDlg::OnBnClickedButtonBootloader)
END_MESSAGE_MAP()


// CMicroPCRDlg �޽��� ó����

BOOL CMicroPCRDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// �� ��ȭ ������ �������� �����մϴ�. ���� ���α׷��� �� â�� ��ȭ ���ڰ� �ƴ� ��쿡��
	//  �����ӿ�ũ�� �� �۾��� �ڵ����� �����մϴ�.
	SetIcon(m_hIcon, TRUE);			// ū �������� �����մϴ�.
	SetIcon(m_hIcon, FALSE);		// ���� �������� �����մϴ�.

	/* UI Settings */
	bmpProgress.LoadBitmapW(IDB_BITMAP_SCALE);
	bmpRecNotWork.LoadBitmapW(IDB_BITMAP_REC_NOT_WORKING); 
	bmpRecWork.LoadBitmapW(IDB_BITMAP_REC_WORKING);

	((CButton*)GetDlgItem(IDC_BUTTON_PCR_RECORD))->SetBitmap((HBITMAP)bmpRecNotWork);

	m_cProgressBar.SetRange32(-10, 110);
	m_cProgressBar.SendMessage(PBM_SETBARCOLOR,0,RGB(200, 0, 50));

	SetDlgItemText(IDC_EDIT_ELAPSED_TIME, L"0m 0s");

	CFont font;
	CRect rect;
	CString labels[3] = { L"No", L"Temp.", L"Time" };

	font.CreatePointFont(100, L"Arial", NULL);

	m_cProtocolList.SetFont(&font);
	m_cProtocolList.GetClientRect(&rect);

	for(int i=0; i<3; ++i)
		m_cProtocolList.InsertColumn(i, labels[i], LVCFMT_CENTER, (rect.Width()/3));

	OnBnClickedButtonConstants();

#ifndef DEBUG_MODE
	GetDlgItem(IDC_STATIC_PID_DEBUG)->ShowWindow(SW_HIDE);
#endif

	initPidTable();
	loadPidTable();

	// Chart Settings
	CAxis *axis;
	axis = m_Chart.AddAxis( kLocationBottom );
	axis->SetTitle(L"PCR Cycles");
	axis->SetRange(0, 40);

	axis = m_Chart.AddAxis( kLocationLeft );
	axis->SetTitle(L"Sensor Value");
	axis->SetRange(m_cGraphYMin, m_cGraphYMax);

	sensorValues.push_back( 1.0 );

	device = new CDeviceConnect( GetSafeHwnd() );
#ifdef USE_MMTIMER
	m_Timer = new CMMTimers(1, GetSafeHwnd());
#endif

	// ���� �õ�
	BOOL status = device->CheckDevice();

	if( status )
	{
		SetDlgItemText(IDC_EDIT_DEVICE_STATUS, L"Connected");
		isConnected = true;

		CFile file;
		BOOL status = file.Open(RECENT_PATH, CFile::modeRead);
		// �ֱ� �������� ��ΰ� ����� ������ �ִٸ�
		if( status )
		{
			file.Close();
			// �� ��η� ������ �ε��Ͽ� ����Ʈ�� �����.
			loadRecentProtocol();
 
#ifdef USE_MMTIMER
			m_Timer->startTimer(TIMER_DURATION, FALSE);
#else
			SetTimer(1, TIMER_DURATION, NULL);
#endif
		}
		else
		{
			// ������ ���ٸ� ���� �����ΰ� �����޽����� ȣ���Ѵ�.
			file.Open(RECENT_PATH, CFile::modeCreate);
			file.Close();
			AfxMessageBox(L"No Recent Protocol File! Please Read Protocol!");
		}
	}
	else
	{
		SetDlgItemText(IDC_EDIT_DEVICE_STATUS, L"Disconnected");
		isConnected = false;
	}

	enableWindows();


	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
}

// ��ȭ ���ڿ� �ּ�ȭ ���߸� �߰��� ��� �������� �׸�����
//  �Ʒ� �ڵ尡 �ʿ��մϴ�. ����/�� ���� ����ϴ� MFC ���� ���α׷��� ��쿡��
//  �����ӿ�ũ���� �� �۾��� �ڵ����� �����մϴ�.

void CMicroPCRDlg::OnPaint()
{
	CPaintDC dc(this); // �׸��⸦ ���� ����̽� ���ؽ�Ʈ

	if (IsIconic())
	{
		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Ŭ���̾�Ʈ �簢������ �������� ����� ����ϴ�.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �������� �׸��ϴ�.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CRect graphRect;

		int oldMode = dc.SetMapMode( MM_LOMETRIC );

		graphRect.SetRect( 15, 350, 570, 760 );
		
		dc.DPtoLP( (LPPOINT)&graphRect, 2 );

		CDC *dc2 = CDC::FromHandle(dc.m_hDC);
		m_Chart.OnDraw( dc2, graphRect, graphRect );

		dc.SetMapMode( oldMode );

		CDialog::OnPaint();
	}
}

// ����ڰ� �ּ�ȭ�� â�� ���� ���ȿ� Ŀ���� ǥ�õǵ��� �ý��ۿ���
//  �� �Լ��� ȣ���մϴ�.
HCURSOR CMicroPCRDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CMicroPCRDlg::SetSerial(WPARAM wParam, LPARAM lParam)
{
	char *Serial = (char *)lParam;
	return TRUE;
}

BOOL CMicroPCRDlg::OnDeviceChange(UINT nEventType, DWORD dwData)
{
	// ���� �õ�
	BOOL status = device->CheckDevice();

	if( status )
	{
		SetDlgItemText(IDC_EDIT_DEVICE_STATUS, L"Connected");
		isConnected = true;

#ifdef USE_MMTIMER
		m_Timer->startTimer(TIMER_DURATION, FALSE);
#else
		SetTimer(1, TIMER_DURATION, NULL);
#endif
	}
	else
	{
		SetDlgItemText(IDC_EDIT_DEVICE_STATUS, L"Disconnected");
		isConnected = false;

#ifdef USE_MMTIMER
		m_Timer->stopTimer();
#else
		KillTimer(1);
#endif
	}

	enableWindows();

	return TRUE;
}

BOOL CMicroPCRDlg::PreTranslateMessage(MSG* pMsg)
{
	if( pMsg -> message == WM_KEYDOWN )
	{
		if( pMsg -> wParam == VK_RETURN )
			return TRUE;

		if( pMsg -> wParam == VK_ESCAPE )
			return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CMicroPCRDlg::Serialize(CArchive& ar)
{
	// Constants ���� ������ �� �����.
	if (ar.IsStoring())
	{
		ar << m_cMaxActions << m_cTimeOut << m_cArrivalDelta << m_cGraphYMin << m_cGraphYMax;

		for(int i=0; i<pids.size(); ++i){
			ar << pids[i].startTemp;
			ar << pids[i].targetTemp;
			ar << pids[i].kp;
			ar << pids[i].kd;
			ar << pids[i].ki;
		}
	}
	else	// Constants ���� ���Ϸκ��� �ҷ��� �� ����Ѵ�.
	{
		pids.clear();

		ar >> m_cMaxActions >> m_cTimeOut >> m_cArrivalDelta >> m_cGraphYMin >> m_cGraphYMax;

		for(int i=0; i<PID_CONSTANTS_MAX; ++i){
			float startTemp, targetTemp, kp, kd, ki;
			ar >> startTemp;
			ar >> targetTemp;
			ar >> kp;
			ar >> kd;
			ar >> ki;

			// �ҷ��ͼ� pids vector �� �����Ѵ�. 
			pids.push_back( PID( startTemp, targetTemp, kp, kd, ki ) );
		}

		UpdateData(FALSE);
	}
}

// pid table �� grid control �� �׸��� ���� ����.
void CMicroPCRDlg::initPidTable()
{
	// grid control �� ���� ������ �����Ѵ�.
	m_cPidTable.SetListMode(true);

	m_cPidTable.DeleteAllItems();

	m_cPidTable.SetSingleRowSelection();
	m_cPidTable.SetSingleColSelection();
	m_cPidTable.SetRowCount(PID_CONSTANTS_MAX+1);
	m_cPidTable.SetColumnCount(PID_CONSTANTS_MAX+1);
    m_cPidTable.SetFixedRowCount(1);
    m_cPidTable.SetFixedColumnCount(1);
	m_cPidTable.SetEditable(true);

	// �ʱ� excel �� table ������ �������ش�.
	DWORD dwTextStyle = DT_RIGHT|DT_VCENTER|DT_SINGLELINE;
    for (int row = 0; row < m_cPidTable.GetRowCount(); row++) {
            for (int col = 0; col < m_cPidTable.GetColumnCount(); col++) { 
                    GV_ITEM Item;
                    Item.mask = GVIF_TEXT|GVIF_FORMAT;
                    Item.row = row;
                    Item.col = col;

                    if (row < 1 && col > 0) {
                            Item.nFormat = DT_LEFT|DT_WORDBREAK;
                            Item.strText = PID_TABLE_COLUMNS[col-1];
                    } else if (col < 1 && row > 0) {
                            Item.nFormat = dwTextStyle;
                            Item.strText.Format(_T("PID Setting %d"),row);
                    }

                    m_cPidTable.SetItem(&Item);  
            }
    }
}

// ���Ϸκ��� �ҷ��� pid ���� table �� �׷��ش�.
void CMicroPCRDlg::loadPidTable()
{
	CFile file;

	if( file.Open(CONSTANTS_PATH, CFile::modeRead) ){
		CArchive ar(&file, CArchive::load);
		Serialize(ar);
		ar.Close();
		file.Close();
	}
	else{
		for(int i=0; i<PID_CONSTANTS_MAX; ++i)
			pids.push_back( PID() );
		AfxMessageBox(L"Constants ������ ���� ���� �ʱ�ȭ�Ǿ����ϴ�.\n�ٽ� ���� �������ּ���.");
		OnBnClickedButtonConstants();
	}

	// �ҷ��� pid ������ grid control �� ǥ�����ش�.
	for(int i=0; i<pids.size(); ++i){
		float *temp[5] = { &(pids[i].startTemp), &(pids[i].targetTemp), 
			&(pids[i].kp), &(pids[i].kd), &(pids[i].ki)};

		for(int j=0; j<5; ++j){
			GV_ITEM item;
			item.mask = GVIF_TEXT|GVIF_FORMAT;
			item.row = i+1;
			item.col = j+1;
			item.nFormat = DT_LEFT|DT_WORDBREAK;
			if( j == 4 )
				item.strText.Format(L"%.4f", *temp[j]);
			else
				item.strText.Format(L"%.1f", *temp[j]);

			m_cPidTable.SetItem(&item); 
		}
	}

	// ������ actions ���� ����
	// ���ڰ� �ٲ� �� �ֱ� ������ ����� ���� �����ϵ��� �Ѵ�.
	if( actions != NULL )
		delete []actions;

	// ���ο� actions ���� �����Ѵ�. 
	actions = new Action[m_cMaxActions];
	m_totalActionNumber = 0;
}

void CMicroPCRDlg::savePidTable()
{
	CFile file;

	file.Open(CONSTANTS_PATH, CFile::modeCreate|CFile::modeWrite);
	CArchive ar(&file, CArchive::store);
	Serialize(ar);
	ar.Close();
	file.Close();
}

void CMicroPCRDlg::enableWindows()
{
	GetDlgItem(IDC_BUTTON_PCR_OPEN)->EnableWindow(isConnected);
	GetDlgItem(IDC_BUTTON_PCR_START)->EnableWindow(isConnected);
	GetDlgItem(IDC_BUTTON_PCR_RECORD)->EnableWindow(isConnected);
}

void CMicroPCRDlg::saveRecentProtocol(CString path)
{
	CFile file;
	char str[256] = "";
	int j =0;
	file.Open(RECENT_PATH, CFile::modeCreate | CFile::modeWrite);
	// �������� ������ �����ϱ� ���� ��� ����, �ѱ��� ���� �� �ֱ� ������ �߰���.
	WideCharToMultiByte(CP_ACP, 0, path, -1, str, 256, NULL, NULL);
	strcat(str, "\r\n");
	while(str[j] != '\n')
		j++;
	file.Write(str, j+3);
	file.Close();
}

void CMicroPCRDlg::loadRecentProtocol(void)
{
	CString path;
	CStdioFile file;
	file.Open(RECENT_PATH, CFile::modeRead);
	// ��ο� �ѱ��� ���Ե� �� �ֱ� ������ �߰���.
	setlocale(LC_ALL, "korean");
	file.ReadString(path);
	file.Close();

	readProtocol(path);
}

void CMicroPCRDlg::readProtocol(CString path)
{
	CFile file;
	if( path.IsEmpty() || !file.Open(path, CFile::modeRead) )
	{
		AfxMessageBox(L"No Recent Protocol File! Please Read Protocol!");
		return;
	}

	int fileSize = (int)file.GetLength() + 1;
	char *inTemp = new char[fileSize * sizeof(char)];
	file.Read(inTemp, fileSize);

	CString inString = AfxCharToString(inTemp);
	
	m_sProtocolName = getProtocolName(path);

	if( m_sProtocolName.GetLength() > MAX_PROTOCOL_LENGTH )
	{
		AfxMessageBox(L"Protocol Name is Too Long !");
		return;
	}

	if( inTemp ) delete[] inTemp;

	int markPos = inString.Find(L"%PCR%");
	if( markPos < 0 )
	{
		AfxMessageBox(L"This is not PCR File !!");
		return;
	}
	markPos = inString.Find(L"%END");
	if( markPos < 0 )
	{
		AfxMessageBox(L"This is not PCR File !!");
		return;
	}

	int line = 0;
	for(int i=0; i<markPos; i++)
	{
		if( inString.GetAt(i) == '\n' )
			line++;
	}

	m_totalActionNumber = 0;
	for(int i=0; i<line; i++)
	{
		int linePos = inString.FindOneOf(L"\n\r");
		CString oneLine = (CString)inString.Left(linePos);
		inString.Delete(0, linePos + 2);
		if( i > 0 )
		{
			int spPos = oneLine.FindOneOf(L" \t");
			// Label Extraction
			actions[m_totalActionNumber].Label = oneLine.Left(spPos);
			oneLine.Delete(0, spPos);
			oneLine.TrimLeft();
			// Temperature Extraction
			spPos = oneLine.FindOneOf(L" \t");
			CString tmpStr = oneLine.Left(spPos);
			oneLine.Delete(0, spPos);
			oneLine.TrimLeft();

			actions[m_totalActionNumber].Temp = (double)_wtof(tmpStr);

			bool timeflag = false;
			wchar_t tempChar = NULL;

			for(int j=0; j<oneLine.GetLength(); j++)
			{
				tempChar = oneLine.GetAt(j);
				if( tempChar == (wchar_t)'m' )
					timeflag = true;
				else if( tempChar == (wchar_t)'M' )
					timeflag = true;
				else
					timeflag = false;
			}

			// Duration Extraction
			double time = (double)_wtof(oneLine);

			if(timeflag)
				actions[m_totalActionNumber].Time = time * 60;
			else
				actions[m_totalActionNumber].Time = time;

			timeflag = false;
			
			if( actions[m_totalActionNumber].Label != "GOTO" )
			{
				m_nLeftTotalSec += (int)(actions[m_totalActionNumber].Time);
			}

			int label = 0;
			CString temp;
			if( actions[m_totalActionNumber].Label == "GOTO" )
			{
				while(true && actions[m_totalActionNumber].Temp != 0 && actions[m_totalActionNumber].Temp < 101 )
				{
					temp.Format(L"%.0f", actions[m_totalActionNumber].Temp);
					if( actions[label++].Label == temp) break;
				}

				for(int j=0; j<actions[m_totalActionNumber].Time; j++)
				{
					for(int k=label-1; k<m_totalActionNumber; k++)
						m_nLeftTotalSec += (int)(actions[k].Time);
				}
			}
			m_totalActionNumber++;
		}
	}

	int labelNo = 1;

	for(int i=0; i<m_totalActionNumber; i++)
	{
		if( actions[i].Label != "GOTO" )
		{
			if( _ttoi(actions[i].Label) != labelNo )
			{
				AfxMessageBox(L"Label numbering error");
				return;
			}
			else
				labelNo++;

			if( (actions[i].Temp > 100) || (actions[i].Temp < 0) )
			{
				AfxMessageBox(L"Target Temperature error!!");
				return;
			}

			if( (actions[i].Time > 3600) || (actions[i].Time < 0) )
			{
				AfxMessageBox(L"Target Duration error!!");
				return;
			}
		}
		else
		{
			if( (actions[i].Temp > (double)_wtof(actions[i-1].Label) ) ||
				(actions[i].Temp < 1) )
			{
				AfxMessageBox(L"GOTO Label error !!");
				return;
			}

			if( (actions[i].Time > 100) || (actions[i].Time < 1) )
			{
				AfxMessageBox(L"GOTO repeat count error !!");
				return;
			}
		}
	}

	m_nLeftTotalSecBackup = m_nLeftTotalSec;

	// ����Ʈ�� ǥ�����ش�.
	displayList();
}

void CMicroPCRDlg::displayList()
{
	int j=0;

	m_cProtocolList.DeleteAllItems();

	for(int i=0; i<m_totalActionNumber; i++)
	{
		m_cProtocolList.InsertItem(i, actions[i].Label);		// �� ���� ����Ʈ�� ǥ���Ѵ�.
		CString tempString;

		if( actions[i].Label == "GOTO" )		// GOTO �� ���
		{
			tempString.Format(L"%d", (int)actions[i].Temp);
			m_cProtocolList.SetItemText(i, 1, tempString);

			tempString.Format(L"%d",(int)actions[i].Time);
			m_cProtocolList.SetItemText(i, 2, tempString);
		}
		else					// ���� �� ���
		{
			tempString.Format(L"%d", (int)actions[i].Temp);
			m_cProtocolList.SetItemText(i, 1, tempString);			// �µ��� ����Ʈ�� ǥ���Ѵ�.

			// �ð� �ޱ�(�д���)
			int durs = (int)actions[i].Time;
			// �Ҽ��� ���� �ʷ� ȯ��
			int durm = durs/60;
			durs = durs%60;

			if( durs == 0 )
			{
				if( durm == 0 ) tempString.Format(L"��");
				else tempString.Format(L"%dm", durm);
			}
			else
			{
				if( durm == 0 ) tempString.Format(L"%ds", durs);
				else tempString.Format(L"%dm %ds", durm, durs);
			}

			m_cProtocolList.SetItemText(i, 2, tempString);		// �ð��� ����Ʈ�� ǥ���Ѵ�.
		}
	}

	// �������� �̸�
	// InvalidateRect(&CRect(10, 5, 10 * m_sProtocolName.GetLength() + MAX_PROTOCOL_LENGTH, 34));		// Protocol Name ����

	// ��ü �ð��� ��, ��, �� ������ ���
	int second = m_nLeftTotalSec % 60;
	int minute = m_nLeftTotalSec / 60;
	int hour   = minute / 60;
	minute = minute - hour * 60;

	// ��ü �ð� ǥ��
	CString leftTime;
	leftTime.Format(L"%02d:%02d:%02d", hour, minute, second);
	SetDlgItemText(IDC_EDIT_LEFT_PROTOCOL_TIME, leftTime);
}

CString CMicroPCRDlg::getProtocolName(CString path)
{
	int pos = 0;
	CString protocol = path;

	for(int i=0; i<protocol.GetLength(); i++)
	{
		if( protocol.GetAt(i) == '\\' )
			pos = i;
	}

	return protocol.Mid(pos + 1, protocol.GetLength());
}

void CMicroPCRDlg::OnBnClickedButtonConstants()
{
	int w = GetSystemMetrics(SM_CXSCREEN);
	int h = GetSystemMetrics(SM_CYSCREEN);

	if( openConstants )
	{
		SetWindowPos(NULL, w/3, h/3, 585, 375, SWP_NOZORDER);
		SetDlgItemText(IDC_BUTTON_CONSTANTS, L"PID ����");
	}
	else
	{
		if( openGraphView )
			OnBnClickedButtonGraphview();

		SetWindowPos(NULL, w/5, h/3, 1175, 375, SWP_NOZORDER);
		SetDlgItemText(IDC_BUTTON_CONSTANTS, L"PID �ݱ�");
	}

	m_cPidTable.SetEditable(!openConstants);
	openConstants = !openConstants;
}

void CMicroPCRDlg::OnBnClickedButtonGraphview()
{
	int w = GetSystemMetrics(SM_CXSCREEN);
	int h = GetSystemMetrics(SM_CYSCREEN);

	if( openGraphView )
	{
		SetWindowPos(NULL, w/3, h/3, 585, 375, SWP_NOZORDER);
		SetDlgItemText(IDC_BUTTON_GRAPHVIEW, L"�׷��� ����");
	}
	else
	{
		if( openConstants )
			OnBnClickedButtonConstants();
		SetWindowPos(NULL, w/3, h/7, 585, 800, SWP_NOZORDER);
		SetDlgItemText(IDC_BUTTON_GRAPHVIEW, L"�׷��� �ݱ�");
	}

	openGraphView = !openGraphView;
}

void CMicroPCRDlg::OnBnClickedButtonConstantsApply()
{
	if( !UpdateData() )
		return;

	pids.clear();

	for (int row = 1; row < m_cPidTable.GetRowCount(); row++) {
		CString startTemp = m_cPidTable.GetItemText(row, 1);
		CString targetTemp = m_cPidTable.GetItemText(row, 2);
		CString kp = m_cPidTable.GetItemText(row, 3);
		CString kd = m_cPidTable.GetItemText(row, 4);
		CString ki = m_cPidTable.GetItemText(row, 5);

		pids.push_back( PID( _wtof(startTemp), _wtof(targetTemp), _wtof(kp), _wtof(kd), _wtof(ki) ) );
	}

	savePidTable();

	CAxis *axis = m_Chart.GetAxisByLocation( kLocationLeft );
	axis->SetRange(m_cGraphYMin, m_cGraphYMax);
}

void CMicroPCRDlg::OnBnClickedButtonPcrStart()
{
	if( m_totalActionNumber < 1 )
	{
		AfxMessageBox(L"There is no action list. Please load task file!!");
		return;
	}

	if( !isStarted )
	{
		isStarted = true;

		KillTimer(1);

		GetDlgItem(IDC_BUTTON_PCR_OPEN)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_PCR_START)->EnableWindow(FALSE);
		SetDlgItemText(IDC_BUTTON_PCR_START, L"PCR Stop");

		// Protocol Progress bar by modal dialog
		CDialog *progressDlg = new CModalDialog;
		progressDlg->Create(IDD_PROGRESS_DIALOG, this);

		CRect rect, parent_rect;
		progressDlg->GetWindowRect(&rect);
		GetWindowRect(&parent_rect);

		progressDlg->SetWindowPos(this, parent_rect.left + (parent_rect.Width() - rect.Width()) / 2,
			parent_rect.top + (parent_rect.Height() - rect.Height()) / 2, parent_rect.Width(), parent_rect.Height(), SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

		CProgressCtrl *progress = (CProgressCtrl*)progressDlg->GetDlgItem(IDC_PROGRESS_PROTOCOL);
		progress->SetRange(0, pids.size() + 4);
		progress->SetDlgItemTextW(IDC_STATIC_PROGRESS, L"PCR Protocol Transmitting...");

		RxBuffer rx;
		TxBuffer tx;

		for(int i=0; i<pids.size(); ++i)
		{
			PID pid = pids[i];
			progress->SetPos(i+1);

			memset(&rx, 0, sizeof(RxBuffer));
			memset(&tx, 0, sizeof(TxBuffer));

			tx.cmd = CMD_PID_WRITE;
			tx.startTemp = (BYTE)pid.startTemp;
			tx.targetTemp = (BYTE)pid.targetTemp;
			
			// float ���� byte ������ ��ȯ�Ͽ� buffer �� ����
			BYTE *tempBuf = (BYTE*)&(pid.kp);
			memcpy(&(tx.pid_p1), tempBuf, sizeof(float));
			tempBuf = (BYTE*)&(pid.ki);
			memcpy(&(tx.pid_i1), tempBuf, sizeof(float));
			tempBuf = (BYTE*)&(pid.kd);
			memcpy(&(tx.pid_d1), tempBuf, sizeof(float));

			device->Write(&tx);

			Sleep(20);

			device->Read(&rx);

			Sleep(120);
		}

		memset(&rx, 0, sizeof(RxBuffer));
		memset(&tx, 0, sizeof(TxBuffer));

		tx.cmd = CMD_PID_END;

		device->Write(&tx);

		Sleep(40);

		device->Read(&rx);

		progress->SetPos(pids.size());

		progressDlg->DestroyWindow();

		if( progressDlg != NULL )
			delete progressDlg;

		currentCmd = CMD_PCR_RUN;

		GetDlgItem(IDC_BUTTON_PCR_START)->EnableWindow(TRUE);
		SetTimer(1, TIMER_DURATION, NULL);

		if( !isRecording )
			OnBnClickedButtonPcrRecord();

		m_startTime = clock();

		m_nLeftSec = 0;
		m_nLeftTotalSec = m_nLeftTotalSecBackup;
		m_currentActionNumber = -1;
		m_leftGotoCount = -1;

		isCompletePCR = false;

		m_prevTargetTemp = m_currentTargetTemp = 25;

		isFirstDraw = false;
		clearSensorValue();
	}
	else
	{
		GetDlgItem(IDC_BUTTON_PCR_OPEN)->EnableWindow(TRUE);
		isStarted = false;

		currentCmd = CMD_PCR_STOP;

		PCREndTask();
	}
}

void CMicroPCRDlg::OnBnClickedButtonPcrOpen()
{
	if( !isConnected )
		return;

	m_nLeftTotalSec = 0;

	CFileDialog fdlg(TRUE, NULL, NULL, NULL, L"*.txt |*.txt|");
	if( fdlg.DoModal() == IDOK )
	{
		saveRecentProtocol(fdlg.GetPathName());

		m_sProtocolName = getProtocolName(fdlg.GetPathName());

		readProtocol(fdlg.GetPathName());
	}
}

void CMicroPCRDlg::OnBnClickedButtonPcrRecord()
{
	/**
	// pd ��¥ �ð� ��.txt 
	**/
	if( !isRecording )
	{
		CreateDirectory(L"./Record/", NULL);

		CString fileName, fileName2;
		CTime time = CTime::GetCurrentTime();
		fileName = time.Format(L"./Record/%Y%m%d-%H%M-%S.txt");
		fileName2 = time.Format(L"./Record/pd%Y%m%d-%H%M-%S.txt");
		
		m_recFile.Open(fileName, CStdioFile::modeCreate|CStdioFile::modeWrite);
		m_recFile.WriteString(L"Number	Time	Temperature	Voltage\n");

		m_recPDFile.Open(fileName2, CStdioFile::modeCreate|CStdioFile::modeWrite);
		m_recPDFile.WriteString(L"Cycle	Time	Value\n");
		m_recordingCount = 0;
		m_cycleCount = 0;
		m_recStartTime = timeGetTime();
	}
	else
	{
		m_recFile.Close();
		m_recPDFile.Close();
	}

	isRecording = !isRecording;
}

void CMicroPCRDlg::blinkTask()
{
	m_blinkCounter++;
	if( m_blinkCounter > 5 )
	{
		m_blinkCounter = 0;
		if( isRecording )
		{
			recordFlag = !recordFlag;
			if( recordFlag )
				SET_BUTTON_IMAGE(IDC_BUTTON_PCR_RECORD, bmpRecWork);
			else
				SET_BUTTON_IMAGE(IDC_BUTTON_PCR_RECORD, bmpRecNotWork);
		}
		else
			SET_BUTTON_IMAGE(IDC_BUTTON_PCR_RECORD, bmpRecNotWork);

		if( isStarted )
		{
			blinkFlag = !blinkFlag;
			if( blinkFlag )
				m_cProtocolList.SetTextBkColor(RGB(240,200,250));
			else
				m_cProtocolList.SetTextBkColor(RGB(255,255,255));

			m_cProtocolList.RedrawItems(m_currentActionNumber, m_currentActionNumber);
		}
		else
		{
			m_cProtocolList.SetTextBkColor(RGB(255,255,255));
			m_cProtocolList.RedrawItems(0, m_totalActionNumber);
		}
	}
}

// YJ 
CString dslr_title;
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	char title[128];
	GetWindowTextA(hwnd,title,sizeof(title));

	if( strstr(title, "DSLR Remote Pro for Windows - Connected") != NULL )
		dslr_title = title;

	return TRUE;
}

void CMicroPCRDlg::timeTask()
{
	m_timerCounter++;

	// 1s ���� ����ǵ��� ����
	if( m_timerCounter == 10 )
	{
		m_timerCounter = 0;

		if( m_nLeftSec == 0 )
		{
			m_currentActionNumber++;

			// clear previous blink
			m_cProtocolList.SetTextBkColor(RGB(255,255,255));
			m_cProtocolList.RedrawItems(0, m_totalActionNumber);

			if( (m_currentActionNumber) >= m_totalActionNumber )
			{
				isCompletePCR = true;
				PCREndTask();
			}

			if( actions[m_currentActionNumber].Label.Compare(L"GOTO") != 0 )
			{
				m_prevTargetTemp = m_currentTargetTemp;
				m_currentTargetTemp = (int)actions[m_currentActionNumber].Temp;

				isTargetArrival = false;
				::OutputDebugString(L"test\n");
				m_nLeftSec = (int)(actions[m_currentActionNumber].Time);
				m_timeOut = m_cTimeOut*10;

				int min = m_nLeftSec/60;
				int sec = m_nLeftSec%60;

				// current left protocol time
				CString leftTime;
				if( min == 0 )
					leftTime.Format(L"%ds", sec);
				else
					leftTime.Format(L"%dm %ds", min, sec);
				m_cProtocolList.SetItemText(m_currentActionNumber, 2, leftTime);
			}
			else	// is GOTO
			{
				if( m_leftGotoCount < 0 )
					m_leftGotoCount = (int)actions[m_currentActionNumber].Time;

				if( m_leftGotoCount == 0 )
					m_leftGotoCount = -1;
				else
				{
					m_leftGotoCount--;
					CString leftGoto;
					leftGoto.Format(L"%d", m_leftGotoCount);

					m_cProtocolList.SetItemText(m_currentActionNumber, 2, leftGoto);
					
					// GOTO ���� target label ���� �־���
					CString tmpStr;
					tmpStr.Format(L"%d",(int) actions[m_currentActionNumber].Temp);

					int pos = 0;
					for (pos = 0; pos < m_totalActionNumber; ++pos)
						if( tmpStr.Compare(actions[pos].Label) == 0 )
							break;

					m_currentActionNumber = pos-1;
				}
			}
		}
		else	// action �� �������� ���
		{
			if( !isTargetArrival )
			{
				m_timeOut--;

				if( m_timeOut == 0 )
				{
					AfxMessageBox(L"The target temperature cannot be reached!!");
					PCREndTask();
				}
			}
			else
			{
				m_nLeftSec--;
				m_nLeftTotalSec--;
				::OutputDebugString(L"ok");

				int min = m_nLeftSec/60;
				int sec = m_nLeftSec%60;

				// current left protocol time
				CString leftTime;
				if( min == 0 )
					leftTime.Format(L"%ds", sec);
				else
					leftTime.Format(L"%dm %ds", min, sec);
				m_cProtocolList.SetItemText(m_currentActionNumber, 2, leftTime);
				
				// total left protocol time
				min = m_nLeftTotalSec/60;
				sec = m_nLeftTotalSec%60;

				if( min == 0 )
					leftTime.Format(L"%ds", sec);
				else
					leftTime.Format(L"%dm %ds", min, sec);
				SetDlgItemText(IDC_EDIT_LEFT_PROTOCOL_TIME, leftTime);
			}
		}

		// 150108 YJ for camera shoot
		if( m_currentActionNumber != 0 && 
			( m_nLeftSec == 10 &&( m_currentActionNumber == 3 || m_currentActionNumber == 6)) ||
			  m_nLeftSec == 1 &&(m_currentActionNumber == 3))
		{
			dslr_title = "";
			EnumWindows(EnumWindowsProc, NULL);

			if( dslr_title != "" )
			{
				HWND checker = ::FindWindow(NULL, dslr_title);

				if( checker )
					::PostMessage(checker, WM_COMMAND, MAKELONG(1003, BN_CLICKED), (LPARAM)GetSafeHwnd());
			}
		}

		if( m_nLeftSec == 11 && 
			( ((int)(actions[m_currentActionNumber].Temp) == 72) || ((int)(actions[m_currentActionNumber].Temp) == 50) ))
		{
			ledControl = 0;
		}
		else if( m_nLeftSec == 0 && 
			( ((int)(actions[m_currentActionNumber].Temp) == 72) || ((int)(actions[m_currentActionNumber].Temp) == 50) ))
		{
			ledControl = 1;
		}

		// for graph drawing
		if( ((int)(actions[m_currentActionNumber].Temp) == 72) && 
			m_nLeftSec == 1 )
		{
			/*		���� ����
			double lights = (double)(rxBuf.lightH & 0x0f)*255. + (double)(rxBuf.lightL);
			addSensorValue( lights );

			if( isRecording )
			{
				m_cycleCount++;
				CString out;
				out.Format(L"%6d	%8.0f	%3.1f\n", m_cycleCount, (double)(timeGetTime()-m_recStartTime), lights);
				m_recPDFile.WriteString(out);
			}
			*/
		}
	}

	int elapsed_time = (int)((((double)clock())-m_startTime)/1000.);
	int min = elapsed_time/60;
	int sec = elapsed_time%60;
	CString temp;
	temp.Format(L"%dm %ds", min, sec);
	SetDlgItemText(IDC_EDIT_ELAPSED_TIME, temp);
}

void CMicroPCRDlg::PCREndTask()
{
	if( isRecording )
		OnBnClickedButtonPcrRecord();

	m_currentActionNumber = 0;
	m_nLeftSec = 0;
	m_nLeftTotalSec = 0;
	m_timerCounter = 0;
	m_startTime = 0;
	recordFlag = false;
	isStarted = false;

	SetDlgItemText(IDC_BUTTON_PCR_START, L"PCR Start");

	if( isCompletePCR )
		AfxMessageBox(L"PCR ended!!");
	else
		AfxMessageBox(L"PCR incomplete!!");

	isCompletePCR = false;
}

#ifdef USE_MMTIMER
LRESULT CMicroPCRDlg::OnmmTimer(WPARAM wParam, LPARAM lParam)
{
	blinkTask();

	if( isStarted )
	{
		int test = 0;
		timeTask();
	}

	BYTE readdata[65] = { 0, };
	// BYTE senddata[65] = { 0, };

	TxBuffer tx;
	memset(&tx, 0, sizeof(TxBuffer));


	device->Write((void*)&tx);

	if( device->Read(readdata) != 0 )
	{
		rxBuf.tempH = readdata[RX_TEMPH];
		rxBuf.tempL = readdata[RX_TEMPL];
		rxBuf.state = readdata[RX_STATE];
		rxBuf.lightH = readdata[RX_LIGHTH];
		rxBuf.lightL = readdata[RX_LIGHTL];
	}
	else
	{
		rxBuf.tempH = 0;
		rxBuf.tempL = 0;
		rxBuf.state = 0;
		rxBuf.lightH = 0;
		rxBuf.lightL = 0;
		return 0;
	}

	double adc = ((double)(rxBuf.tempH & 0x0f)*255. + (double)(rxBuf.tempL))/4.0;
	double temperature = 0;

	if( adc != 0 )
	{
		double r = Rref * (1024.0 / adc - 1.0);
		double InRs = log(r);
		double tmp = A_VAL + B_VAL *InRs + C_VAL * pow(InRs,3.);

		temperature = 1./tmp-K;
	}

	// for median filtering
	temp_buffer[0] = temp_buffer[1];
	temp_buffer[1] = temp_buffer[2];
	temp_buffer[2] = temp_buffer[3];
	temp_buffer[3] = temp_buffer[4];
	temp_buffer[4] = temperature;

	memcpy(temp_buffer2, temp_buffer, 5*sizeof(double));

	temperature = AfxQuickSort(temp_buffer2, 5);

	CString tempStr;
	tempStr.Format(L"%3.1f", temperature);
	SetDlgItemText(IDC_EDIT_CHAMBER_TEMP, tempStr);
	m_cProgressBar.SetPos((int)temperature);

	double pwmValue = 0;

	if( isStarted )
	{
		// ���� �µ����� ���� �µ��� �� �� ����
		if( m_prevTargetTemp > m_currentTargetTemp )
		{
			if( temperature-m_currentTargetTemp <= FAN_STOP_TEMPDIF )
				txBuf.fan = 0;
			else
				txBuf.fan = 1;
		}
		else
			txBuf.fan = 0;

		if( fabs(temperature-m_currentTargetTemp) < m_cArrivalDelta )
			isTargetArrival = true;

		txBuf.state = M_ST_PCR;

		// duration �� ����ϱ�
		double currentErr = 0;
		double proportional = 0;
		double integral = 0;

		currentErr = m_currentTargetTemp - temperature;
		proportional = currentErr;
		integral = currentErr + m_lastIntegral;

		if( integral > INTGRALMAX )
			integral = INTGRALMAX;
		else if( integral < 0 )
			integral = 0;

		double derivative = currentErr - m_lastError;

		pwmValue = m_kp*proportional + m_ki*integral + m_kd * derivative;

		if( pwmValue > 255 )
			pwmValue = 255;
		else if( pwmValue < 0 )
			pwmValue = 0;

		pwmValue = fabs(pwmValue);

		m_lastError = currentErr;
		m_lastIntegral = integral;
	}
	else
	{
		memset(&txBuf, 0, sizeof(TxBuf));
		txBuf.ledControl = 1;
		txBuf.state = M_ST_READY;
	}

	setDuration(pwmValue);

	CString out;
	out.Format(L"%.1f\t%.1f\t%.1f\t%.1f", m_kp, m_kd, m_ki, pwmValue);
	SetDlgItemText(IDC_STATIC_PID_DEBUG, out); 

	if( isRecording )
	{
		m_recordingCount++;
		CString out;
		out.Format(L"%6d	%8.0f	%3.1f	%6.3lf\n", m_recordingCount, (double)(timeGetTime()-m_recStartTime), temperature, pwmValue);
		m_recFile.WriteString(out);
	}

	return FALSE;
}

#else	// for using SetTimer

void CMicroPCRDlg::OnTimer(UINT_PTR nIDEvent)
{
	blinkTask();

	if( isStarted )
	{
		int errorPrevent = 0;
		timeTask();
	}

	RxBuffer rx;
	TxBuffer tx;
	float currentTemp = 0.0;
	
	memset(&rx, 0, sizeof(RxBuffer));
	memset(&tx, 0, sizeof(TxBuffer));

	tx.cmd = currentCmd;
	tx.currentTargetTemp = (BYTE)m_currentTargetTemp;

	device->Write((void*)&tx);

	if( device->Read(&rx) == 0 )
		return;

	/*	���� ������ chamber adc ������ �µ��� ǥ���ϴ� �κ� 
	// ����� ����������, ���� ��⿡�� �µ��� ����ϴ� �κ��� �߰������Ƿ� 
	// �� ���� �̿��� ����
	// ���� ��⿡�� �µ� ���� �����ְ� ������ ����� �������� ����
	// ��Ⱑ ������ �׽�Ʈ�� ����.
	// �����Ͽ� �߰��� ����

	double adc = ((double)(rx.chamber_h & 0x0f)*255. + (double)(rx.chamber_l))/4.0;
	currentTemp = 0;

	if( adc != 0 )
	{
		double r = Rref * (1024.0 / adc - 1.0);
		double InRs = log(r);
		double tmp = A_VAL + B_VAL *InRs + C_VAL * pow(InRs,3.);

		currentTemp = 1./tmp-K;
	}

	// for median filtering
	temp_buffer[0] = temp_buffer[1];
	temp_buffer[1] = temp_buffer[2];
	temp_buffer[2] = temp_buffer[3];
	temp_buffer[3] = temp_buffer[4];
	temp_buffer[4] = currentTemp;

	memcpy(temp_buffer2, temp_buffer, 5*sizeof(double));

	currentTemp = AfxQuickSort(temp_buffer2, 5);
	*/

	// ���κ��� ���� �µ� ���� �޾ƿͼ� ������.
	// convert BYTE pointer to float type for reading temperature value.
	memcpy(&currentTemp, &(rx.chamber_temp_1), sizeof(float));

	if( currentTemp < 0.0 )
		return;

	if( fabs(currentTemp-m_currentTargetTemp) < m_cArrivalDelta )
		isTargetArrival = true;

	CString tempStr;
	tempStr.Format(L"%3.1f", currentTemp);
	SetDlgItemText(IDC_EDIT_CHAMBER_TEMP, tempStr);
	m_cProgressBar.SetPos((int)currentTemp);
	
	CDialog::OnTimer(nIDEvent);
}

#endif	// End of #ifdef USE_MMTIMER



void CMicroPCRDlg::addSensorValue(double val)
{
	// ������ ����� ��Ʈ�� ���� ��, 
	// ���� ������ double �� vector �� �����Ͽ�
	// �� ���� ������� �ٽ� �׸�.
	sensorValues.push_back(val);
	m_Chart.DeleteAllData();

	int size = sensorValues.size();

	double *data = new double[size*2];
	int	nDims = 2, dims[2] = { 2, size };

	for(int i=0; i<size; ++i)
	{
		data[i] = i;
		data[i+size] = sensorValues[i];
	}

	m_Chart.AddData( data, nDims, dims );

	InvalidateRect(&CRect(15, 350, 1155, 760));
}

void CMicroPCRDlg::clearSensorValue()
{
	sensorValues.clear();
	sensorValues.push_back( 1.0 );

	m_Chart.DeleteAllData();
	InvalidateRect(&CRect(15, 350, 1155, 760));
}


void CMicroPCRDlg::OnBnClickedButtonBootloader()
{
	KillTimer(1);
	
	// ���� bootloader ����� �������� ����. 
	// ��Ⱑ ����Ǽ� ���� ����� �������� ����.
	// ��Ⱑ disconnected �� ������, bootloader ���� �̵����� ����.

	RxBuffer rx;
	TxBuffer tx;
	memset(&rx, 0, sizeof(RxBuffer));
	memset(&tx, 0, sizeof(TxBuffer));
	
	tx.cmd = CMD_BOOTLOADER;

	device->Write(&tx);

	Sleep(20);

	device->Read(&rx);
}