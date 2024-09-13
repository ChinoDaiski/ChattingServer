#include "pch.h"
#include "WinSockManager.h"
#include "SessionManager.h"
#include "Packet.h"

std::map<UINT16, CSession*> CSessionManager::m_UserCSessionMap;

std::list<CSession*> g_clientList;   // 서버에 접속한 세션들에 대한 정보

UINT16 CSessionManager::m_gID = 0;  // 전역 유저 id 초기 값 설정

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

        // 비활성화 되었다면
        if (!pCSession->isAlive)
        {
            it = m_UserCSessionMap.erase(it);
        }
        // 활성 중이라면
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
        // 해당 세션이 alive가 아니거나 제외할 세션이라면 넘어가기
        if (!client->isAlive || excludeCSession == client)
            continue;

        // 메시지 전파, 세션의 sendQ에 데이터를 삽입
        int retVal = client->sendQ.Enqueue((const char*)pPacket, dataSize);

        if (retVal != dataSize)
        {
            NotifyClientDisconnected(client);

            // 이런 일은 있어선 안되지만 혹시 모르니 검사, enqueue에서 문제가 난 것은 링버퍼의 크기가 가득찼다는 의미이므로, resize할지 말지는 오류 생길시 가서 테스트하면서 진행
            int error = WSAGetLastError();
            std::cout << "Error : BroadcastData(), It might be full of sendQ" << error << "\n";
            DebugBreak();
        }
    }
}

// 클라이언트에게 데이터를 브로드캐스트하는 함수
void BroadcastData(CSession* excludeCSession, CPacket* pPacket, UINT8 dataSize)
{
    for (auto& client : g_clientList)
    {
        // 해당 세션이 alive가 아니거나 제외할 세션이라면 넘어가기
        if (!client->isAlive || excludeCSession == client)
            continue;

        // 메시지 전파, 세션의 sendQ에 데이터를 삽입
        int retVal = client->sendQ.Enqueue((const char*)pPacket->GetBufferPtr(), dataSize);

        if (retVal != dataSize)
        {
            NotifyClientDisconnected(client);

            // 이런 일은 있어선 안되지만 혹시 모르니 검사, enqueue에서 문제가 난 것은 링버퍼의 크기가 가득찼다는 의미이므로, resize할지 말지는 오류 생길시 가서 테스트하면서 진행
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

// 클라이언트 연결이 끊어진 경우에 호출되는 함수
void NotifyClientDisconnected(CSession* disconnectedCSession)
{
    // 만약 이미 죽었다면 NotifyClientDisconnected가 호출되었던 상태이므로 중복인 상태. 체크할 것.
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

    // 세션의 sendQ에 데이터를 삽입
    int retVal = includeCSession->sendQ.Enqueue((const char*)pPacket, dataSize);

    if (retVal != dataSize)
    {
        NotifyClientDisconnected(includeCSession);

        // 이런 일은 있어선 안되지만 혹시 모르니 검사, enqueue에서 문제가 난 것은 링버퍼의 크기가 가득찼다는 의미이므로, resize할지 말지는 오류 생길시 가서 테스트하면서 진행
        int error = WSAGetLastError();
        std::cout << "Error : UnicastData(), It might be full of sendQ" << error << "\n";
        DebugBreak();
    }
}

// 인자로 받은 세션들에게 데이터 전송을 시도하는 함수
void UnicastData(CSession* includeCSession, CPacket* pPacket, UINT8 dataSize)
{
    if (!includeCSession->isAlive)
        return;

    // 세션의 sendQ에 데이터를 삽입
    int retVal = includeCSession->sendQ.Enqueue((const char*)pPacket->GetBufferPtr(), dataSize);

    if (retVal != dataSize)
    {
        NotifyClientDisconnected(includeCSession);

        // 이런 일은 있어선 안되지만 혹시 모르니 검사, enqueue에서 문제가 난 것은 링버퍼의 크기가 가득찼다는 의미이므로, resize할지 말지는 오류 생길시 가서 테스트하면서 진행
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

    // accept가 완료되었다면 세션에 등록 후, 해당 세션에 패킷 전송
    CSession* Session = new CSession;
    Session->isAlive = true;

    // 소켓 정보 추가
    Session->sock = ClientSocket;

    // IP / PORT 정보 추가
    memcpy(Session->IP, winSockManager.GetIP(ClientAddr).c_str(), sizeof(Session->IP));
    Session->port = winSockManager.GetPort(ClientAddr);

    // 세션이 가지고 있을 객체 포인터 정보 초기화
    Session->pObj = nullptr;

    return Session;
}