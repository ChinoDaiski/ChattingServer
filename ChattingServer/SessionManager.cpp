#include "pch.h"
#include "WinSockManager.h"
#include "SessionManager.h"
#include "Packet.h"

std::list<CSession*> g_clientList;   // 서버에 접속한 세션들에 대한 정보

DisconnectCallback CSessionManager::m_callbackDisconnect;

CSessionManager::CSessionManager() noexcept
{
}

CSessionManager::~CSessionManager() noexcept
{
}

void CSessionManager::Update(void)
{
    // 비활성화된 클라이언트를 리스트에서 제거
    // 여기서 제거하는 이유는 이전 프레임에 로직 상에서 제거될 세션들의 sendQ가 비워지고 나서 제거되길 원해서 이렇게 작성.
    auto it = g_clientList.begin();
    while (it != g_clientList.end())
    {
        // 비활성화 되었다면
        if (!(*it)->isAlive)
        {
            // 컨텐츠에서 넣어준 함수 호출
            m_callbackDisconnect((*it));

            // 제거
            closesocket((*it)->sock);
            delete (*it);   // 세션 삭제

            it = g_clientList.erase(it);
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
            // 이런 일은 있어선 안되지만 혹시 모르니 검사
            // enqueue에서 문제가 난 것은 링버퍼의 크기가 가득찼다는 의미이므로, 여기에 들어왔다는 것은 resize로 해결 가능.
            // 다만, 링버퍼의 크기가 가득 찰 수 있는 상황 중 tcp 윈도우 사이즈가 0인 경우엔 resize로 해결이 안될 확률이 높으니 끊는게 맞음. 
            // 할지 말지는 오류 생길시 가서 테스트하면서 진행

            NotifyClientDisconnected(client);

            int error = WSAGetLastError();
            std::cout << "Error : BroadcastData(), sendQ가 가득찼음" << error << "\n";
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
            // 이런 일은 있어선 안되지만 혹시 모르니 검사
            // enqueue에서 문제가 난 것은 링버퍼의 크기가 가득찼다는 의미이므로, 여기에 들어왔다는 것은 resize로 해결 가능.
            // 다만, 링버퍼의 크기가 가득 찰 수 있는 상황 중 tcp 윈도우 사이즈가 0인 경우엔 resize로 해결이 안될 확률이 높으니 끊는게 맞음. 
            // 할지 말지는 오류 생길시 가서 테스트하면서 진행

            NotifyClientDisconnected(client);

            int error = WSAGetLastError();
            std::cout << "Error : BroadcastData(), sendQ가 가득찼음" << error << "\n";
            DebugBreak();
        }
    }
}

void BroadcastPacket(CSession* excludeCSession, PACKET_HEADER* pHeader, CPacket* pPacket)
{
    BroadcastData(excludeCSession, pHeader, sizeof(PACKET_HEADER));
    BroadcastData(excludeCSession, pPacket, pHeader->wPayloadSize);
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
        // 이런 일은 있어선 안되지만 혹시 모르니 검사
        // enqueue에서 문제가 난 것은 링버퍼의 크기가 가득찼다는 의미이므로, 여기에 들어왔다는 것은 resize로 해결 가능.
        // 다만, 링버퍼의 크기가 가득 찰 수 있는 상황 중 tcp 윈도우 사이즈가 0인 경우엔 resize로 해결이 안될 확률이 높으니 끊는게 맞음. 
        // 할지 말지는 오류 생길시 가서 테스트하면서 진행

        NotifyClientDisconnected(includeCSession);

        int error = WSAGetLastError();
        std::cout << "Error : UnicastData(), sendQ가 가득찼음" << error << "\n";
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
        // 이런 일은 있어선 안되지만 혹시 모르니 검사
        // enqueue에서 문제가 난 것은 링버퍼의 크기가 가득찼다는 의미이므로, 여기에 들어왔다는 것은 resize로 해결 가능.
        // 다만, 링버퍼의 크기가 가득 찰 수 있는 상황 중 tcp 윈도우 사이즈가 0인 경우엔 resize로 해결이 안될 확률이 높으니 끊는게 맞음. 
        // 할지 말지는 오류 생길시 가서 테스트하면서 진행

        NotifyClientDisconnected(includeCSession);

        int error = WSAGetLastError();
        std::cout << "Error : UnicastData(), sendQ가 가득찼음" << error << "\n";
        DebugBreak();
    }
}

void UnicastPacket(CSession* includeCSession, PACKET_HEADER* pHeader, CPacket* pPacket)
{
    UnicastData(includeCSession, pHeader, sizeof(PACKET_HEADER));
    UnicastData(includeCSession, pPacket, pHeader->wPayloadSize);
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