﻿#pragma once
#include "afxdialogex.h"


// CLcokDialog 对话框

class CLcokDialog : public CDialog
{
	DECLARE_DYNAMIC(CLcokDialog)

public:
	CLcokDialog(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CLcokDialog();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_INFO };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};
