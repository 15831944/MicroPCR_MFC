#pragma once
#include "afxwin.h"
#include ".\gridctrl_src\gridctrl.h"
#include "UserDefs.h"

// CPIDCreateDlg ��ȭ �����Դϴ�.

class CPIDCreateDlg : public CDialog
{
private:
	BYTE m_cStartTemp;
	BYTE m_cTargetTemp;
	float m_cKp;
	float m_cKi;
	float m_cKd;
	CComboBox m_cRemovalList;
	CGridCtrl m_cPidTable;

	vector< PID > pids;

	void initPidTable();


	DECLARE_DYNAMIC(CPIDCreateDlg)

public:
	CPIDCreateDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CPIDCreateDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_DIALOG_PID_CREATE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonPidCreateAdd();
	afx_msg void OnBnClickedButtonPidCreateComplete();
	afx_msg void OnBnClickedButtonPidCreateRemoval();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
};
