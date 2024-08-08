#pragma once
#include "pch.h"
#include "framework.h"

#define SERV_PORT 9527
#pragma pack(push)
#pragma pack(1)
class CPacket {
public:
	CPacket(): sHead(0), nLength(0), sCmd(0), sSum(0){}
	//打包
	CPacket(WORD nCmd, const BYTE* pData, size_t nSize) {
		sHead = 0xFEFF;
		nLength = nSize + 4;
		sCmd = nCmd;
		if (nSize > 0) {
			strData.resize(nSize);
			memcpy((void*)strData.c_str(), pData, nSize);
		}
		else {
			strData.clear();
		}
		sSum = 0;
		for (size_t j = 0; j < strData.size(); j++)
		{
			sSum += BYTE(strData[j]) & 0xFF;
		}
	}
	CPacket(const CPacket& pack) {
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}
	//解包
	CPacket(const BYTE* pData, size_t& nSize) {//nSize传入接收到的数据大小， 引用类型  返回成功读取的大小  因为有的未读取
		size_t i = 0;
		for (; i < nSize; i++) {
			if (*(WORD*)(pData + i) == 0xFEFF) {//pData + i  表示从pData指针开始的第i个字节
				sHead = *(WORD*)(pData + i);
				i += 2;//防止特殊情况  如包中只有FEFF
				break;
			}
		}
		//数据可能不全  或包头未能全部接收
		if (i+4+2+2 > nSize) {//分别表示nLength, sCmd, sSum
			nSize = 0;
			return;
		}
		nLength = *(WORD*)(pData + i); i += 4;
		if (nLength + i > nSize) {//包未完全接收到  返回  接收失败
			nSize = 0;
			return;
		}
		sCmd = *(WORD*)(pData + i); i += 2;
		if (nLength > 4) {
			strData.resize(nLength - 2 - 2);
			memcpy((void*)strData.c_str(), pData + i, nLength - 4);
			i += nLength - 4;
		}
		sSum = *(WORD*)(pData + i); i += 2;
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++) {
			sum += BYTE(strData[j]) & 0xFF;
		}
		if (sum == sSum) {
			nSize = i;//head  length
		}
	}
	~CPacket() {}
	CPacket& operator=(const CPacket& pack) {
		if (this != &pack) {
			sHead = pack.sHead;
			nLength = pack.nLength;
			sCmd = pack.sCmd;
			strData = pack.strData;
			sSum = pack.sSum;
		}
		return *this;
	}
	int Size() {//包数据的大小
		return nLength + 6;
	}
	const char* Data() {
		strOut.resize(nLength + 6);
		BYTE* pData = (BYTE*)strOut.c_str();
		*(WORD*)pData = sHead; pData += 2;
		*(WORD*)pData = nLength; pData += 4;
		*(WORD*)pData = sCmd; pData += 2;
		memcpy(pData, strData.c_str(), strData.size()); pData += strData.size();
		*(WORD*)pData = sSum;
		return strOut.c_str();
	}
public:
	//在 C++ 中，WORD-->unsigned int 16       BYTE-->unsigned int 8	
	WORD sHead;//包头 固定位   用FE FF 
	DWORD nLength;//包长度  从控制命令开始到和校验结束  包括控制命令和校验
	WORD sCmd;//控制命令
	std::string strData;//包数据
	WORD sSum;//和校验
	std::string strOut;//整个包的数据
};

#pragma pack(pop)

class CServerScoket
{
public:
	//静态函数  外部可通过类名进行调用
	static CServerScoket* getInstance() {
		if (m_instance == NULL) { //静态函数没有this指针  无法直接访问成员变量
			m_instance = new CServerScoket;
		}
		return m_instance;		
	}
	bool InitSocket() {		
		if (m_sock == -1) return false;
		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_addr.s_addr = INADDR_ANY;
		serv_adr.sin_port = htons(SERV_PORT);
		//bind	
		if (bind(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) {
			return false;
		}
		if (listen(m_sock, 1) == -1) {
			return false;
		}
		return true;		
	}
	bool AcceptClient() {
		sockaddr_in client_adr;
		int cli_sz = sizeof(client_adr);
		m_client = accept(m_sock, (sockaddr*)&client_adr, &cli_sz);
		if (m_client == -1) return false;
		//recv(client, buffer, sizeof(buffer), 0);
		//send(client, buffer, sizeof(buffer), 0);	
		return true;
	}
#define BUFFER_SIZE 4096
	int DealCommand() {
		if (m_client == -1) return -1;
		//char buffer[1024] = "";
		char* buffer = new char[4096];
		memset(buffer, 0, sizeof(buffer));
		size_t index = 0;
		while (1) {
			size_t len = recv(m_client, buffer+index, BUFFER_SIZE-index, 0);
			if (len <= 0) {
				return -1;
			}
			index += len;
			len = index;
			m_packet = CPacket ((BYTE*)buffer, len);
			if (len > 0) {
				memmove(buffer, buffer + len, BUFFER_SIZE - len);
				index -= len;
				return m_packet.sCmd;
			}
		}
		return -1;
	}

	bool Send(const char* pData, int nSize) {
		if (m_client == -1) return false;
		return send(m_client, pData, nSize, 0) > 0;
	}
	bool Send(CPacket& pack) {
		if (m_client == -1) return false;
		return send(m_client, pack.Data(), pack.Size(), 0) > 0;
	}



private:
	SOCKET m_sock, m_client;
	CPacket m_packet;
	CServerScoket& operator=(const CServerScoket& ss) {}
	CServerScoket(const CServerScoket& ss) {
		m_sock = ss.m_sock;
		m_client = ss.m_client;
	}
	CServerScoket() {
		m_client = INVALID_SOCKET;
		if (InitSocketEnv() == FALSE) {
			MessageBox(NULL, _T("无法初始化套接字环境,请检查网络设置"), _T("初始化错误!"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		m_sock = socket(PF_INET, SOCK_STREAM, 0);
	}
	~CServerScoket() {
		closesocket(m_sock);
		WSACleanup();	
	}
	BOOL InitSocketEnv() {
		WSADATA data;
		if (WSAStartup(MAKEWORD(1, 1), &data) != 0) {
			return FALSE;
		}
		return TRUE;
	}
	
	static void releaseInstance() {
		if (m_instance != NULL) {
			CServerScoket* tmp = m_instance;
			m_instance = NULL;
			delete tmp;
		}
	}
	//全局静态变量  main函数调用之前会初始化  main函数结束后才会析构
	static CServerScoket* m_instance;


	class CHelper {
	public:
		CHelper() {
			CServerScoket::getInstance();
		}
		~CHelper() {
			CServerScoket::releaseInstance();
		}
	};
	static CHelper m_helper;

};
//extern 声明一个外部变量  这样可以在RemoteCtrl中直接当做静态变量使用
//extern CServerScoket server;

