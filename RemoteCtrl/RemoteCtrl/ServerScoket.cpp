#include "pch.h"
#include "ServerScoket.h"

//CServerScoket server;

CServerScoket* CServerScoket::m_instance = NULL;
CServerScoket::CHelper CServerScoket::m_helper;

CServerScoket* pserver = CServerScoket::getInstance();

