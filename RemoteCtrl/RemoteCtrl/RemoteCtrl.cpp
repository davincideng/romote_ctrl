// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "ServerScoket.h"
#include <direct.h>
#include <atlimage.h>

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
    CServerScoket::getInstance()->Send(pack);
    return 0;
}
#include<stdio.h>
#include<io.h>
#include <list>

int MakeDirectoryInfo() {
    std::string strPath;
    //std::list<FILEINFO> lstFileInfos;
    if (CServerScoket::getInstance()->GetFilePath(strPath) == false) {
        OutputDebugString(_T("当前的命令不是获取文件列表，命令解析错误"));
        return -1;
    }
    if (_chdir(strPath.c_str()) != 0) {
        FILEINFO fInfo;   
        fInfo.HasNext = FALSE;    
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

int RunFile() {
    std::string strPath;
    CServerScoket::getInstance()->GetFilePath(strPath);
    ShellExecuteA(NULL, NULL, strPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
    CPacket pack(3, NULL, 0);
    CServerScoket::getInstance()->Send(pack);
    return 0;
}

int DownloadFile() {
    std::string strPath;
    CServerScoket::getInstance()->GetFilePath(strPath);
    long long data = 0;
    FILE* pFile = NULL;
    errno_t err = fopen_s(&pFile, strPath.c_str(), "rb");
    if (err !=  0) {
        CPacket pack(4, (BYTE*)&data, 8);
        CServerScoket::getInstance()->Send(pack);
        return -1;
    }
    if (pFile != NULL) {
        fseek(pFile, 0, SEEK_END);
        data = _ftelli64(pFile);
        CPacket head(4, (BYTE*)&data, 8);
        fseek(pFile, 0, SEEK_SET);
        char buffer[1024] = "";
        size_t rlen = 0;
        do {
            rlen = fread(buffer, 1, 1024, pFile);
            CPacket pack(4, (BYTE*)buffer, rlen);
            CServerScoket::getInstance()->Send(pack);
        } while (rlen >= 1024);        
        fclose(pFile);
    }
    CPacket pack(4, NULL, 0);
    CServerScoket::getInstance()->Send(pack);
    return 0;
    
}

int MouseEvent() {
    MOUSEEV mouse;
    if (CServerScoket::getInstance()->GetMouseEvent(mouse)) {       
        DWORD nFlags = 0;      
        switch (mouse.nButton)
        {
        case 0://左键
            nFlags = 1;
            break;
        case 1://右键
            nFlags = 2;
            break;
        case 2://中键
            nFlags = 4;
            break;
        case 4://没有按键
            nFlags = 8;
            break;
        default:
            break;
        }
        if (nFlags != 8) SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);
        switch (mouse.nAction)
        {
        case 0://单击
            nFlags |= 0x10;
            break;
        case 1://双击
            nFlags |= 0x20;
            break;
        case 2://按下
            nFlags |= 0x40;
            break;
        case 3://松开
            nFlags |= 0x80;
            break;       
        default:
            break;
        }
        switch (nFlags)
        {
        case 0x21://左键双击
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
        case 0x11://左键单击
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x41://左键按下
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x81://左键松开
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x22://右键双击
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
        case 0x12://右键单击
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
            break;      
        case 0x42://右键按下
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x82://右键松开
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x24://中键双击
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
        case 0x14://中键单击
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x44://中键按下
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x84://中键松开
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x80://单纯鼠标移动
            mouse_event(MOUSEEVENTF_MOVE, mouse.ptXY.x, mouse.ptXY.y, 0, GetMessageExtraInfo());
            break;
        default:
            break;
        }
        CPacket pack(4, NULL, 0);
        CServerScoket::getInstance()->Send(pack);
    }
    else {
        OutputDebugString(_T("获取鼠标操作参数失败"));
        return -1;
    }    
    return 0;
}

int SendScreen() {
    CImage screen;//GDI  全局设备接口
    HDC hScreen = ::GetDC(NULL); //拿到屏幕句柄
    int nBitPerPixel = GetDeviceCaps(hScreen, BITSPIXEL);//24  RGB888  24bit  返回使用多少个bit表示颜色
    int nWidth = GetDeviceCaps(hScreen, HORZRES);
    int nHeight = GetDeviceCaps(hScreen, VERTRES);
    screen.Create(nWidth, nHeight, nBitPerPixel);
    BitBlt(screen.GetDC(), 0, 0, 1920, 1020, hScreen, 0, 0, SRCCOPY);
    ReleaseDC(NULL, hScreen);
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);//在堆区开辟一片内存
    if (hMem == NULL) return -1;
    IStream* pStream = NULL; 
    HRESULT ret = CreateStreamOnHGlobal(hMem, TRUE, &pStream);
    if (ret == S_OK) {
        screen.Save(pStream, Gdiplus::ImageFormatPNG);//会默认将指针设到尾部
        LARGE_INTEGER bg = {0};
        pStream->Seek(bg, STREAM_SEEK_SET, NULL);
        PBYTE pData = (PBYTE)GlobalLock(hMem);
        SIZE_T nSize = GlobalSize(hMem);
        CPacket pack(6, pData, nSize);
        CServerScoket::getInstance()->Send(pack);
        GlobalUnlock(hMem);
    }    
    //screen.Save(_T("test.png"), Gdiplus::ImageFormatPNG);
    /*
    TRACE("png %d\r\n", GetTickCount64() - tick);
    for (int i = 0; i < 10; i++) {
        DWORD tick = GetTickCount64();
        screen.Save(_T("test2020.png"), Gdiplus::ImageFormatPNG);
        TRACE("png %d\r\n", GetTickCount64() - tick);
        tick = GetTickCount64();
        screen.Save(_T("test2020.jpg"), Gdiplus::ImageFormatJPEG);
        TRACE("jpg %d\r\n", GetTickCount64() - tick) ;
    }*/
    pStream->Release();
    GlobalFree(hMem);
    screen.ReleaseDC();   
    return 0;
}
#include "LcokDialog.h"
CLcokDialog dlg;
unsigned threadid = 0;

unsigned __stdcall threadLockDlg(void* arg)
{
    TRACE("%s(%d):%d\r\n", __FUNCTION__, __LINE__, GetCurrentThreadId());
    dlg.Create(IDD_DIALOG_INFO, NULL);
    dlg.ShowWindow(SW_SHOW);
    //遮蔽后台窗口
    CRect rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = GetSystemMetrics(SM_CXFULLSCREEN);//w1
    rect.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
    rect.bottom = LONG(rect.bottom * 1.10);
    TRACE("right = %d bottom = %d\r\n", rect.right, rect.bottom);
    dlg.MoveWindow(rect);
    CWnd* pText = dlg.GetDlgItem(IDC_STATIC);
    if (pText) {
        CRect rtText;
        pText->GetWindowRect(rtText);
        int nWidth = rtText.Width();//w0
        int x = (rect.right - nWidth) / 2;
        int nHeight = rtText.Height();
        int y = (rect.bottom - nHeight) / 2;
        pText->MoveWindow(x, y, rtText.Width(), rtText.Height());
    }

    //窗口置顶
    dlg.SetWindowPos(&dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
    //限制鼠标功能
    ShowCursor(false);
    //隐藏任务栏
    ::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_HIDE);
    //限制鼠标活动范围
    dlg.GetWindowRect(rect);
    rect.left = 0;
    rect.top = 0;
    rect.right = 1;
    rect.bottom = 1;
    ClipCursor(rect);
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (msg.message == WM_KEYDOWN) {
            TRACE("msg:%08X wparam:%08x lparam:%08X\r\n", msg.message, msg.wParam, msg.lParam);
            if (msg.wParam == 0x41) {//按下a键 退出  ESC（1B)
                break;
            }
        }
    }
    ClipCursor(NULL);
    //恢复鼠标
    ShowCursor(true);
    //恢复任务栏
    ::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_SHOW);
    dlg.DestroyWindow();
    _endthreadex(0);
    return 0;
}

int LockMachine()
{
    if ((dlg.m_hWnd == NULL) || (dlg.m_hWnd == INVALID_HANDLE_VALUE)) {
        //_beginthread(threadLockDlg, 0, NULL);
        _beginthreadex(NULL, 0, threadLockDlg, NULL, 0, &threadid);
        TRACE("threadid=%d\r\n", threadid);
    }
    CPacket pack(7, NULL, 0);
    CServerScoket::getInstance()->Send(pack);
    return 0;
}

int UnlockMachine()
{
    //dlg.SendMessage(WM_KEYDOWN, 0x41, 0x01E0001);
    //::SendMessage(dlg.m_hWnd, WM_KEYDOWN, 0x41, 0x01E0001);
    PostThreadMessage(threadid, WM_KEYDOWN, 0x41, 0);
    CPacket pack(8, NULL, 0);
    CServerScoket::getInstance()->Send(pack);
    return 0;
}

int TestConnect() {
    CPacket pack(1981, NULL, 0);
    CServerScoket::getInstance()->Send(pack);
    return 0;
}

int ExcuteCmmond(int nCmd) {
    int ret = 0;
    switch (nCmd)
    {
    case 1://查看磁盘分区
        ret = MakeDriverInfo();
        break;
    case 2://查看指定目录下的文件
        ret = MakeDirectoryInfo();
        break;
    case 3://打开文件
        ret = RunFile();
        break;
    case 4://下载文件
        ret = DownloadFile();
        break;
    case 5://鼠标移动
        ret = MouseEvent();
    case 6://发送屏幕内容-->本质 发送屏幕截图
        ret = SendScreen();
    case 7://锁机
        ret = LockMachine();
        break;
    case 8://解锁
        ret = UnlockMachine();
        break;
    case 1981://测试
        ret = TestConnect();
    default:
        break;
    }
    return ret;
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
             //TODO: 在此处为应用程序的行为编写代码。
             //TODO: 服务器  socket bind listen  accept read write close            
            CServerScoket* pserver = CServerScoket::getInstance();
            int count = 0;
            if (pserver->InitSocket() == false) {
                MessageBox(NULL, _T(""), _T("网络初始化失败"), MB_OK | MB_ICONERROR);
                exit(0);
            }
            while (CServerScoket::getInstance() != NULL) {               
                if (pserver->AcceptClient() == false) {
                    if (count >= 3) {
                        MessageBox(NULL, _T(""), _T("多次接入用户失败，退出！"), MB_OK | MB_ICONERROR);
                        exit(0);
                    }
                    MessageBox(NULL, _T(""), _T("接入用户失败"), MB_OK | MB_ICONERROR);
                    count++;
                }
                int ret = pserver->DealCommand();       
                if (ret > 0) {
                    ret = ExcuteCmmond(pserver->GetPacket().sCmd);
                    if (ret != 0) {
                        TRACE("执行命令失败：%d ret = %d\r\n", pserver->GetPacket().sCmd, ret);
                    }
                    pserver->CloseClient();
                }               
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
