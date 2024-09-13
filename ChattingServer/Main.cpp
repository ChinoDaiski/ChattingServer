

//===================================================
// pch
//===================================================
#include "pch.h"

//===================================================
// 매니저 class
//===================================================
#include "WinSockManager.h" // 윈도우 소켓
#include "SessionManager.h"
#include "TimerManager.h"
#include "NetIOManager.h"
//===================================================

//===================================================
// 직렬화 버퍼 관련
//===================================================
#include "Packet.h"
#include "PacketProc.h"


//#define SERVERIP "192.168.30.16"
#define SERVERIP "127.0.0.1"
#define SERVERPORT 6000

bool g_bShutdown = false;

void Update(void);

DWORD g_targetFPS;			    // 1초당 목표 프레임
DWORD g_targetFrame;		    // 1초당 주어지는 시간 -> 1000 / targetFPS
DWORD g_currentServerTime;		// 서버 로직이 시작될 때 초기화되고, 이후에 프레임이 지날 때 마다 targetFrameTime 만큼 더함.

int main()
{
    //=====================================================================================================================================
    // listen 소켓 준비
    //=====================================================================================================================================
    CWinSockManager& winSockManager = CWinSockManager::getInstance();

    UINT8 options = 0;
    options |= OPTION_NONBLOCKING;

    winSockManager.StartServer(PROTOCOL_TYPE::TCP_IP, SERVERPORT, options);

    //=====================================================================================================================================
    // 서버 시간 설정
    //=====================================================================================================================================
    timeBeginPeriod(1);                     // 타이머 정밀도(해상도) 1ms 설

    g_targetFPS = 50;                       // 목표 초당 프레임
    g_targetFrame = 1000 / g_targetFPS;     // 1 프레임에 주어지는 시간
    g_currentServerTime = timeGetTime();    // 전역 서버 시간 설정

    //=====================================================================================================================================
    // I/O 처리를 담당하는 매니저 호출
    //=====================================================================================================================================
    CNetIOManager::RegisterCallbackFunction(PacketProc);

    //=====================================================================================================================================
    // 전역 유저 id 초기 값 설정
    //=====================================================================================================================================
    //g_id = 0;

    //=====================================================================================================================================
    // select에서 사용할 timeVal 설정
    //=====================================================================================================================================
    TIMEVAL timeVal;
    timeVal.tv_sec = 0;
    timeVal.tv_usec = 0;

    //=====================================================================================================================================
    // 메인 로직
    //=====================================================================================================================================
    while (!g_bShutdown)
    {
        try
        {
            // 네트워크 I/O 처리
            CNetIOManager::netIOProcess();

            // 게임 로직 업데이트
            Update();
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << "\n";

            g_bShutdown = true;
        }
    }

    // 현재 서버에 있는 정보 안전하게 DB등에 저장
}

// 메인 로직
void Update(void)
{
    // 프레임 맞추기
    if (timeGetTime() < (g_currentServerTime + g_targetFrame))
        return;
    else
        g_currentServerTime += g_targetFrame;

    /*
    // 비활성화된 클라이언트를 리스트에서 제거
    // 여기서 제거하는 이유는 이전 프레임에 로직 상에서 제거될 세션들의 sendQ가 비워지고 나서 제거되길 원해서 이렇게 작성.
    auto it = g_clientList.begin();
    while (it != g_clientList.end())
    {
        // 비활성화 되었다면
        if (!(*it)->isAlive)
        {


            // 제거
            closesocket((*it)->sock);

            delete (*it)->pPlayer;  // 플레이어 삭제
            delete (*it);           // 세션 삭제

            it = g_clientList.erase(it);
        }
        // 활성 중이라면
        else
        {
            ++it;
        }
    }
    */

}


//void netProc_Accept()
//{
//    // 클라이언트가 접속했을 때 진행되는 과정
//    // 백로그 큐에 접속이 되었음을 감지하고 Accept 시도
//    SOCKET ClientSocket;
//    SOCKADDR_IN ClientAddr;
//
//    CWinSockManager<CSession>& winSockManager = CWinSockManager<CSession>::getInstance();
//    SOCKET listenSocket = winSockManager.GetListenSocket();
//
//    // accept 시도
//    ClientSocket = winSockManager.Accept(ClientAddr);
//
//    // accept가 완료되었다면 세션에 등록 후, 해당 세션에 패킷 전송
//    CSession* CSession = createSession(ClientSocket, ClientAddr,
//        g_id,
//        rand() % (dfRANGE_MOVE_RIGHT - dfRANGE_MOVE_LEFT) + dfRANGE_MOVE_LEFT,
//        rand() % (dfRANGE_MOVE_BOTTOM - dfRANGE_MOVE_TOP) + dfRANGE_MOVE_TOP,
//        100,
//        dfPACKET_MOVE_DIR_LL
//    );
//
//    // 1. 연결된 세션에 PACKET_SC_CREATE_MY_CHARACTER 를 전송
//    // 2. PACKET_SC_CREATE_OTHER_CHARACTER 에 연결된 세션의 정보를 담아 브로드캐스트
//    // 3. PACKET_SC_CREATE_OTHER_CHARACTER 에 g_clientList에 있는 모든 캐릭터 정보를 담아 연결된 세션에게 전송
//
//    //=====================================================================================================================================
//    // 1. 연결된 세션에 PACKET_SC_CREATE_MY_CHARACTER 를 전송
//    //=====================================================================================================================================
//    UINT16 posX, posY;
//    CSession->pPlayer->getPosition(posX, posY);
//    serverProxy.SC_CREATE_MY_CHARACTER_FOR_SINGLE(CSession, CSession->uid, CSession->pPlayer->GetDirection(), posX, posY, CSession->pPlayer->GetHp());
//
//    //=====================================================================================================================================
//    // 2. PACKET_SC_CREATE_OTHER_CHARACTER 에 연결된 세션의 정보를 담아 브로드캐스트
//    //=====================================================================================================================================
//    serverProxy.SC_CREATE_OTHER_CHARACTER_FOR_All(CSession, CSession->uid, CSession->pPlayer->GetDirection(), posX, posY, CSession->pPlayer->GetHp());
//
//    //=====================================================================================================================================
//    // 3. PACKET_SC_CREATE_OTHER_CHARACTER 에 g_clientList에 있는 모든 캐릭터 정보를 담아 연결된 세션에게 전송
//    //=====================================================================================================================================
//
//    // 새로운 연결을 시도하는 클라이언트에 기존 클라이언트 정보들을 전달
//    for (const auto& client : g_clientList)
//    {
//        client->pPlayer->getPosition(posX, posY);
//        serverProxy.SC_CREATE_OTHER_CHARACTER_FOR_SINGLE(CSession, client->uid, client->pPlayer->GetDirection(), posX, posY, client->pPlayer->GetHp());
//
//        // 움직이고 있는 상황이라면
//        if (client->pPlayer->isBitSet(FLAG_MOVING))
//        {
//            serverProxy.SC_MOVE_START_FOR_SINGLE(CSession, client->uid, client->pPlayer->GetDirection(), posX, posY);
//        }
//    }
//
//    // 데이터를 보내는 중에 삭제될 수 도 있으니 살아있는 여부 검사. 원래 있어서는 안되지만 sendQ가 가득찼을 경우 에러가 발생할 수 있음. 혹시 모르니 검사
//    if (CSession->isAlive)
//    {
//        // 모든 과정 이후 g_clientList에 세션 등록
//        g_clientList.push_back(CSession);
//
//        // 전역 id값 증가
//        ++g_id;
//    }
//    else
//    {
//        int error = WSAGetLastError();
//        DebugBreak();
//
//        delete CSession;
//    }
//}