#pragma once
#include "pch.h"
#include "framework.h"

#define SERV_PORT 9527

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

	int DealCommand() {
		if (m_client == -1) return false;
		char buffer[1024] = "";
		while (1) {
			int len = recv(m_client, buffer, sizeof(buffer), 0);
			if (len <= 0) {
				return -1;
			}
			//TODO 处理命令

		}
	}

	bool SendCommand(const char* pData, int nSize) {
		if (m_client == -1) return false;
		return send(m_client, pData, nSize, 0) > 0;
	}



private:
	SOCKET m_sock, m_client;
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

