// LcokDialog.cpp: 实现文件
//

#include "pch.h"
#include "RemoteCtrl.h"
#include "afxdialogex.h"
#include "LcokDialog.h"


// CLcokDialog 对话框

IMPLEMENT_DYNAMIC(CLcokDialog, CDialog)

CLcokDialog::CLcokDialog(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DIALOG_INFO, pParent)
{

}

CLcokDialog::~CLcokDialog()
{
}

void CLcokDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CLcokDialog, CDialog)
END_MESSAGE_MAP()


// CLcokDialog 消息处理程序
