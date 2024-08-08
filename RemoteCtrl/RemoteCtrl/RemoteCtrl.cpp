﻿// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "ServerScoket.h"
#include <direct.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 唯一的应用程序对象
// master branch

CWinApp theApp;

using namespace std; 

void Dump(BYTE* pData, size_t nSize) {
    string strOut;
    for (size_t i = 0; i < nSize; i++) {
        char buf[8] = "";
        if (i > 0 && i % 16 == 0) {
            strOut += "\n";
        }
        snprintf(buf, sizeof(buf), "%02X ", pData[i] & 0xFF);
        strOut += buf;
    }
    strOut += "\n";
    OutputDebugStringA(strOut.c_str());
}


int MakeDriverInfo() {//1==>A 2==>B 3==>C ... 26==>Z
    std::string result;
    for (int i = 1; i <= 26; i++) {
        if (_chdrive(i) == 0) {
            if (result.size() > 0)
                result += ',';
            result += 'A' + i - 1;
        }
    }
    CPacket pack(1, (BYTE*)result.c_str(), result.size());//打包
    Dump((BYTE*)pack.Data(), pack.Size());
    //CServerScoket::getInstance()->Send(pack);
    return 0;
}

int main()
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // 初始化 MFC 并在失败时显示错误
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: 在此处为应用程序的行为编写代码。
            wprintf(L"错误: MFC 初始化失败\n");
            nRetCode = 1;
        }
        else
        {
            // TODO: 在此处为应用程序的行为编写代码。
            // TODO: 服务器  socket bind listen  accept read write close            
            //CServerScoket* pserver = CServerScoket::getInstance();
            //int count = 0;
            //if (pserver->InitSocket() == false) {
            //    MessageBox(NULL, _T(""), _T("网络初始化失败"), MB_OK | MB_ICONERROR);
            //    exit(0);
            //}
            //while (CServerScoket::getInstance() != NULL) {               
            //    if (pserver->AcceptClient() == false) {
            //        if (count >= 3) {
            //            MessageBox(NULL, _T(""), _T("多次接入用户失败，退出！"), MB_OK | MB_ICONERROR);
            //            exit(0);
            //        }
            //        MessageBox(NULL, _T(""), _T("接入用户失败"), MB_OK | MB_ICONERROR);
            //        count++;
            //    }
            //    int ret = pserver->DealCommand();
            //    //TODO 
            //}

            int nCmd = 1;
            switch (nCmd)
            {
            case 1://查看磁盘分区
                MakeDriverInfo();
                break;
            default:
                break;
            }





        }
    }
    else
    {
        // TODO: 更改错误代码以符合需要
        wprintf(L"错误: GetModuleHandle 失败\n");
        nRetCode = 1;
    }

    return nRetCode;
}
