
// MicroPCRDlg.h : ��� ����
//

#pragma once
#include "DeviceConnect.h"
#include "afxcmn.h"
#include ".\gridctrl_src\gridctrl.h"
#include "Chart.h"
#include "UserDefs.h"

#ifdef USE_MMTIMER
#include "mmTimers.h"
#endif

// CMicroPCRDlg ��ȭ ����
class CMicroPCRDlg : public CDialog
{
private:
	CDeviceConnect *device;
	bool isConnected;
	
	CBitmap bmpProgress, bmpRecNotWork, bmpRecWork;
	bool openConstants, openGraphView;

	double m_cTemperature;

	CProgressCtrl m_cProgressBar;
	CGridCtrl m_cPidTable;
	CListCtrl m_cProtocolList;

#ifdef USE_MMTIMER
	CMMTimers* m_Timer;
#endif

	CXYChart m_Chart;
	vector< double > sensorValues;

	int m_cGraphYMin;
	int m_cGraphYMax;

	bool isFirstDraw;

	void addSensorValue(double val);
	void clearSensorValue();

private:
	vector< PID > pids;

	int m_cMaxActions;
	int m_cTimeOut;
	float m_cArrivalDelta;

	void initPidTable();
	void loadPidTable();
	void savePidTable();
	void enableWindows();

	CString m_sProtocolName;
	int m_totalActionNumber;
	int m_currentActionNumber;
	
	Action *actions;
	
	void saveRecentProtocol(CString path);
	void loadRecentProtocol();
	void readProtocol(CString path);
	void displayList();
	CString getProtocolName(CString path);

	// for temporary
	void blinkTask();
	void timeTask();
	void PCREndTask();
	
	int m_blinkCounter, m_timerCounter;

	bool isRecording, recordFlag;
	bool blinkFlag;
	bool isStarted;
	bool isCompletePCR;
	bool isTargetArrival;

	double m_startTime;
	DWORD m_recStartTime;
	double m_prevTargetTemp, m_currentTargetTemp;

	int m_nLeftTotalSecBackup;
	int m_nLeftSec, m_nLeftTotalSec;
	int m_timeOut;

	int m_leftGotoCount;
	int ledControl;

	double temp_buffer[5], temp_buffer2[5];

	CStdioFile m_recFile, m_recPDFile;
	int m_recordingCount, m_cycleCount;

	// ��ư�� ���� ���� ���� command ���� �����Ѵ�.
	BYTE currentCmd;


// �����Դϴ�.
public:
	CMicroPCRDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.
	virtual ~CMicroPCRDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_MICROPCR_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �����Դϴ�.


// �����Դϴ�.
protected:
	HICON m_hIcon;

	// ������ �޽��� �� �Լ�
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT SetSerial(WPARAM wParam, LPARAM lParam);
#ifdef USE_MMTIMER
	virtual LRESULT OnmmTimer(WPARAM wParam, LPARAM lParam);
#else
	afx_msg void OnTimer(UINT_PTR nIDEvent);
#endif
	DECLARE_MESSAGE_MAP()
public:
	BOOL OnDeviceChange(UINT nEventType, DWORD dwData);
	afx_msg void OnBnClickedButtonConstants();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void Serialize(CArchive& ar);
	afx_msg void OnBnClickedButtonConstantsApply();
	afx_msg void OnBnClickedButtonPcrStart();
	afx_msg void OnBnClickedButtonPcrOpen();
	afx_msg void OnBnClickedButtonPcrRecord();
	afx_msg void OnBnClickedButtonGraphview();
	afx_msg void OnBnClickedButtonBootloader();
};