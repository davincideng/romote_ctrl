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
#include<stdio.h>
#include<io.h>
#include <list>
typedef struct file_info{
    file_info() {//结构体构造函数  不需要析构
        IsInvalid = FALSE;
        IsDirectory = -1;
        HasNext = TRUE;
        memset(szFileName, 0, sizeof(szFileName));
    }
    BOOL IsInvalid;//是否有效
    BOOL IsDirectory;//是否为目录  0 否   1是
    BOOL HasNext;//是否还有后续 0 没有  1有
    char szFileName[256];//文件名
    
}FILEINFO, PFILEINFO;
int MakeDirectoryInfo() {
    std::string strPath;
    //std::list<FILEINFO> lstFileInfos;
    if (CServerScoket::getInstance()->GetFilePath(strPath) == false) {
        OutputDebugString(_T("当前的命令不是获取文件列表，命令解析错误"));
        return -1;
    }
    if (_chdir(strPath.c_str()) != 0) {
        FILEINFO fInfo;
        fInfo.IsInvalid = TRUE;
        fInfo.IsDirectory = TRUE;
        fInfo.HasNext = FALSE;
        memcpy(fInfo.szFileName, strPath.c_str(), strPath.size());       
        //lstFileInfos.push_back(fInfo);
        CPacket pack(2, (BYTE*)&fInfo, sizeof(fInfo));
        CServerScoket::getInstance()->Send(pack);
        OutputDebugString(_T("没有权限访问目录"));
        return -2;
    }
    _finddata_t fdata;
    int hfind = 0;
    if ((hfind =_findfirst("*", &fdata)) == -1) {
        OutputDebugString(_T("没有找到任何文件"));
        return -3;
    }
    do {
        FILEINFO fInfo;
        fInfo.IsDirectory = (fdata.attrib & _A_SUBDIR) != 0;
        memcpy(fInfo.szFileName, fdata.name, strlen(fdata.name));
        //lstFileInfos.push_back(fInfo);
        CPacket pack(2, (BYTE*)&fInfo, sizeof(fInfo));
        CServerScoket::getInstance()->Send(pack);
    } while (!_findnext(hfind, &fdata));
    //发送信息到控制端
    FILEINFO fInfo;
    fInfo.HasNext = FALSE;
    CPacket pack(2, (BYTE*)&fInfo, sizeof(fInfo));
    CServerScoket::getInstance()->Send(pack);
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
            case 2://查看指定目录下的文件
                MakeDirectoryInfo();
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
