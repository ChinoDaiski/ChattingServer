#include "pch.h"
#include "WinSockManager.h"
#include "SessionManager.h"
#include "Packet.h"

std::map<UINT16, CSession*> CSessionManager::m_UserCSessionMap;

std::list<CSession*> g_clientList;   // ������ ������ ���ǵ鿡 ���� ����

UINT16 CSessionManager::m_gID = 0;  // ���� ���� id �ʱ� �� ����

CSessionManager::CSessionManager() noexcept
{
}

CSessionManager::~CSessionManager() noexcept
{
}

void CSessionManager::Update(void)
{
    auto it = m_UserCSessionMap.begin();
    while (it != m_UserCSessionMap.end())
    {
        CSession* pCSession = it->second;

        // ��Ȱ��ȭ �Ǿ��ٸ�
        if (!pCSession->isAlive)
        {
            it = m_UserCSessionMap.erase(it);
        }
        // Ȱ�� ���̶��
        else
        {
            ++it;
        }
    }
}

void BroadcastData(CSession* excludeCSession, PACKET_HEADER* pPacket, UINT8 dataSize)
{
    for (auto& client : g_clientList)
    {
        // �ش� ������ alive�� �ƴϰų� ������ �����̶�� �Ѿ��
        if (!client->isAlive || excludeCSession == client)
            continue;

        // �޽��� ����, ������ sendQ�� �����͸� ����
        int retVal = client->sendQ.Enqueue((const char*)pPacket, dataSize);

        if (retVal != dataSize)
        {
            NotifyClientDisconnected(client);

            // �̷� ���� �־ �ȵ����� Ȥ�� �𸣴� �˻�, enqueue���� ������ �� ���� �������� ũ�Ⱑ ����á�ٴ� �ǹ��̹Ƿ�, resize���� ������ ���� ����� ���� �׽�Ʈ�ϸ鼭 ����
            int error = WSAGetLastError();
            std::cout << "Error : BroadcastData(), It might be full of sendQ" << error << "\n";
            DebugBreak();
        }
    }
}

// Ŭ���̾�Ʈ���� �����͸� ��ε�ĳ��Ʈ�ϴ� �Լ�
void BroadcastData(CSession* excludeCSession, CPacket* pPacket, UINT8 dataSize)
{
    for (auto& client : g_clientList)
    {
        // �ش� ������ alive�� �ƴϰų� ������ �����̶�� �Ѿ��
        if (!client->isAlive || excludeCSession == client)
            continue;

        // �޽��� ����, ������ sendQ�� �����͸� ����
        int retVal = client->sendQ.Enqueue((const char*)pPacket->GetBufferPtr(), dataSize);

        if (retVal != dataSize)
        {
            NotifyClientDisconnected(client);

            // �̷� ���� �־ �ȵ����� Ȥ�� �𸣴� �˻�, enqueue���� ������ �� ���� �������� ũ�Ⱑ ����á�ٴ� �ǹ��̹Ƿ�, resize���� ������ ���� ����� ���� �׽�Ʈ�ϸ鼭 ����
            int error = WSAGetLastError();
            std::cout << "Error : BroadcastData(), It might be full of sendQ" << error << "\n";
            DebugBreak();
        }
    }
}

void BroadcastPacket(CSession* excludeCSession, PACKET_HEADER* pHeader, CPacket* pPacket)
{
    BroadcastData(excludeCSession, pHeader, sizeof(PACKET_HEADER));
    BroadcastData(excludeCSession, pPacket, pHeader->wPayloadSize);

    pPacket->MoveReadPos(pHeader->wPayloadSize);
    pPacket->Clear();
}

// Ŭ���̾�Ʈ ������ ������ ��쿡 ȣ��Ǵ� �Լ�
void NotifyClientDisconnected(CSession* disconnectedCSession)
{
    // ���� �̹� �׾��ٸ� NotifyClientDisconnected�� ȣ��Ǿ��� �����̹Ƿ� �ߺ��� ����. üũ�� ��.
    if (disconnectedCSession->isAlive == false)
    {
        DebugBreak();
    }

    disconnectedCSession->isAlive = false;
}

void UnicastData(CSession* includeCSession, PACKET_HEADER* pPacket, UINT8 dataSize)
{
    if (!includeCSession->isAlive)
        return;

    // ������ sendQ�� �����͸� ����
    int retVal = includeCSession->sendQ.Enqueue((const char*)pPacket, dataSize);

    if (retVal != dataSize)
    {
        NotifyClientDisconnected(includeCSession);

        // �̷� ���� �־ �ȵ����� Ȥ�� �𸣴� �˻�, enqueue���� ������ �� ���� �������� ũ�Ⱑ ����á�ٴ� �ǹ��̹Ƿ�, resize���� ������ ���� ����� ���� �׽�Ʈ�ϸ鼭 ����
        int error = WSAGetLastError();
        std::cout << "Error : UnicastData(), It might be full of sendQ" << error << "\n";
        DebugBreak();
    }
}

// ���ڷ� ���� ���ǵ鿡�� ������ ������ �õ��ϴ� �Լ�
void UnicastData(CSession* includeCSession, CPacket* pPacket, UINT8 dataSize)
{
    if (!includeCSession->isAlive)
        return;

    // ������ sendQ�� �����͸� ����
    int retVal = includeCSession->sendQ.Enqueue((const char*)pPacket->GetBufferPtr(), dataSize);

    if (retVal != dataSize)
    {
        NotifyClientDisconnected(includeCSession);

        // �̷� ���� �־ �ȵ����� Ȥ�� �𸣴� �˻�, enqueue���� ������ �� ���� �������� ũ�Ⱑ ����á�ٴ� �ǹ��̹Ƿ�, resize���� ������ ���� ����� ���� �׽�Ʈ�ϸ鼭 ����
        int error = WSAGetLastError();
        std::cout << "Error : UnicastData(), It might be full of sendQ" << error << "\n";
        DebugBreak();
    }
}

void UnicastPacket(CSession* includeCSession, PACKET_HEADER* pHeader, CPacket* pPacket)
{
    UnicastData(includeCSession, pHeader, sizeof(PACKET_HEADER));
    UnicastData(includeCSession, pPacket, pHeader->wPayloadSize);

    pPacket->MoveReadPos(pHeader->wPayloadSize);
    pPacket->Clear();
}

CSession* createSession(SOCKET ClientSocket, SOCKADDR_IN ClientAddr)
{
    CWinSockManager& winSockManager = CWinSockManager::getInstance();

    // accept�� �Ϸ�Ǿ��ٸ� ���ǿ� ��� ��, �ش� ���ǿ� ��Ŷ ����
    CSession* Session = new CSession;
    Session->isAlive = true;

    // ���� ���� �߰�
    Session->sock = ClientSocket;

    // IP / PORT ���� �߰�
    memcpy(Session->IP, winSockManager.GetIP(ClientAddr).c_str(), sizeof(Session->IP));
    Session->port = winSockManager.GetPort(ClientAddr);

    // ������ ������ ���� ��ü ������ ���� �ʱ�ȭ
    Session->pObj = nullptr;

    return Session;
}