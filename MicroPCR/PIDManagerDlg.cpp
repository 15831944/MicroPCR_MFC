// PIDManagerDlg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "MicroPCR.h"
#include "PIDManagerDlg.h"


// CPIDManagerDlg ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CPIDManagerDlg, CDialog)

CPIDManagerDlg::CPIDManagerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPIDManagerDlg::IDD, pParent)
{

}

CPIDManagerDlg::~CPIDManagerDlg()
{
}

void CPIDManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_PID, m_cPidList);
	DDX_Control(pDX, IDC_CUSTOM_PID_TABLE2, m_cPidGridList);
}


BEGIN_MESSAGE_MAP(CPIDManagerDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_PID_NEW, &CPIDManagerDlg::OnBnClickedButtonPidNew)
	ON_BN_CLICKED(IDC_BUTTON_SELECT, &CPIDManagerDlg::OnBnClickedButtonSelect)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, &CPIDManagerDlg::OnBnClickedButtonDelete)
END_MESSAGE_MAP()


// CPIDManagerDlg �޽��� ó�����Դϴ�.

// Enter, Esc ��ư ����
BOOL CPIDManagerDlg::PreTranslateMessage(MSG* pMsg)
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


#include "PIDCreateDlg.h"
// ���ο� PID �� ������ �� �ִ� Dialog �� �����Ѵ�.
void CPIDManagerDlg::OnBnClickedButtonPidNew()
{
	static CPIDCreateDlg dlg;

	if( dlg.DoModal() == IDOK )
	{
		
	}
}

void CPIDManagerDlg::OnBnClickedButtonSelect()
{
// 	OnOK();
}

void CPIDManagerDlg::OnBnClickedButtonDelete()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
}