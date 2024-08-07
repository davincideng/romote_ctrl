#pragma once
#include "pch.h"
#include "framework.h"

#define SERV_PORT 9527

class CServerScoket
{
public:
	//��̬����  �ⲿ��ͨ���������е���
	static CServerScoket* getInstance() {
		if (m_instance == NULL) { //��̬����û��thisָ��  �޷�ֱ�ӷ��ʳ�Ա����
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
			//TODO ��������

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
			MessageBox(NULL, _T("�޷���ʼ���׽��ֻ���,������������"), _T("��ʼ������!"), MB_OK | MB_ICONERROR);
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
	//ȫ�־�̬����  main��������֮ǰ���ʼ��  main����������Ż�����
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
//extern ����һ���ⲿ����  ����������RemoteCtrl��ֱ�ӵ�����̬����ʹ��
//extern CServerScoket server;

