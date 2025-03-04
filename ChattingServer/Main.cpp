

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

#include "User.h"


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

    winSockManager.StartServer(PROTOCOL_TYPE::TCP_IP, SERVERPORT, options, INADDR_ANY, SOMAXCONN_HINT(65535));

    //=====================================================================================================================================
    // 서버 시간 설정
    //=====================================================================================================================================
    timeBeginPeriod(1);                     // 타이머 정밀도(해상도) 1ms 설

    g_targetFPS = 50;                       // 목표 초당 프레임
    g_targetFrame = 1000 / g_targetFPS;     // 1 프레임에 주어지는 시간
    g_currentServerTime = timeGetTime();    // 전역 서버 시간 설정

    //=====================================================================================================================================
    // recv 한 프로토콜을 처리하는 함수 등록
    //=====================================================================================================================================
    CNetIOManager::RegisterPacketProcCallback(PacketProc);
    
    //=====================================================================================================================================
    // 세션이 종료될 때 호출될 함수 등록
    //=====================================================================================================================================
    CSessionManager::RegisterDisconnectCallback(DisconnectSessionProc);

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

    CSessionManager& sessionManager = CSessionManager::getInstance();
    sessionManager.Update();
}