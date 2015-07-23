// PIDCreateDlg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "MicroPCR.h"
#include "PIDCreateDlg.h"


// CPIDCreateDlg ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CPIDCreateDlg, CDialog)

CPIDCreateDlg::CPIDCreateDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPIDCreateDlg::IDD, pParent)
	, m_cStartTemp(25)
	, m_cTargetTemp(25)
	, m_cKp(0)
	, m_cKi(0)
	, m_cKd(0)
{
	
}

CPIDCreateDlg::~CPIDCreateDlg()
{
}

void CPIDCreateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_PID_START_TEMP, m_cStartTemp);
	DDV_MinMaxByte(pDX, m_cStartTemp, 4, 104);
	DDX_Text(pDX, IDC_EDIT_PID_TARGET_TEMP2, m_cTargetTemp);
	DDV_MinMaxByte(pDX, m_cTargetTemp, 4, 104);
	DDX_Text(pDX, IDC_EDIT_PID_TARGET_TEMP3, m_cKp);
	DDV_MinMaxFloat(pDX, m_cKp, 0, 200);
	DDX_Text(pDX, IDC_EDIT_PID_TARGET_TEMP4, m_cKi);
	DDV_MinMaxFloat(pDX, m_cKi, 0, 1);
	DDX_Text(pDX, IDC_EDIT_PID_TARGET_TEMP5, m_cKd);
	DDV_MinMaxFloat(pDX, m_cKd, 0, 100);
	DDX_Control(pDX, IDC_COMBO_PID_REMOVAL, m_cRemovalList);
	DDX_Control(pDX, IDC_CUSTOM_PID_TABLE3, m_cPidTable);
}


BEGIN_MESSAGE_MAP(CPIDCreateDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_PID_CREATE_ADD, &CPIDCreateDlg::OnBnClickedButtonPidCreateAdd)
	ON_BN_CLICKED(IDC_BUTTON_PID_CREATE_COMPLETE, &CPIDCreateDlg::OnBnClickedButtonPidCreateComplete)
	ON_BN_CLICKED(IDC_BUTTON_PID_CREATE_REMOVAL, &CPIDCreateDlg::OnBnClickedButtonPidCreateRemoval)
END_MESSAGE_MAP()


BOOL CPIDCreateDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	initPidTable();

	return TRUE;  // return TRUE unless you set the focus to a control
	// ����: OCX �Ӽ� �������� FALSE�� ��ȯ�ؾ� �մϴ�.
}


// Enter, Esc ��ư ����
BOOL CPIDCreateDlg::PreTranslateMessage(MSG* pMsg)
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

void CPIDCreateDlg::OnBnClickedButtonPidCreateAdd()
{
	if( !UpdateData() )
		return;

	if( m_cStartTemp == m_cTargetTemp )
	{
		AfxMessageBox(L"���ۿµ��� Ÿ�ٿµ��� ���� ���� �� �� �����ϴ�.");
		return;
	}
}

void CPIDCreateDlg::OnBnClickedButtonPidCreateComplete()
{
	// removal list �� ������� ���� ����� ��ü pid ������ �� �� ����
	int size = m_cRemovalList.GetCount();

	if( size == 0 ){
		int res = MessageBox(L"������ PID �� ���� ������� �ʽ��ϴ�.\n�����ϰڽ��ϱ�?", L"�˸�", MB_OKCANCEL);

		if( res == IDOK )
			OnCancel();
		else
			return;
	}

	
}

void CPIDCreateDlg::OnBnClickedButtonPidCreateRemoval()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
}

static const int PID_TABLE_COLUMN_WIDTHS[6] = { 50, 100, 100, 50, 50, 50 };

void CPIDCreateDlg::initPidTable()
{
	// grid control �� ���� ������ �����Ѵ�.
	m_cPidTable.SetListMode(true);

	m_cPidTable.DeleteAllItems();

	m_cPidTable.SetSingleRowSelection();
	m_cPidTable.SetSingleColSelection();
	m_cPidTable.SetRowCount(1);
	m_cPidTable.SetColumnCount(PID_CONSTANTS_MAX+1);
    m_cPidTable.SetFixedRowCount(1);
    m_cPidTable.SetFixedColumnCount(1);
	m_cPidTable.SetEditable(true);

	// �ʱ� gridControl �� table ������ �������ش�.
	DWORD dwTextStyle = DT_RIGHT|DT_VCENTER|DT_SINGLELINE;

    for (int col = 0; col < m_cPidTable.GetColumnCount(); col++) { 
		GV_ITEM Item;
        Item.mask = GVIF_TEXT|GVIF_FORMAT;
        Item.row = 0;
        Item.col = col;

        if (col > 0) {
                Item.nFormat = DT_LEFT|DT_WORDBREAK;
                Item.strText = PID_TABLE_COLUMNS[col-1];
        }

        m_cPidTable.SetItem(&Item);
		m_cPidTable.SetColumnWidth(col, PID_TABLE_COLUMN_WIDTHS[col]);
    }
}
