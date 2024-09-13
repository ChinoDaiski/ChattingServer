

//===================================================
// pch
//===================================================
#include "pch.h"

//===================================================
// �Ŵ��� class
//===================================================
#include "WinSockManager.h" // ������ ����
#include "SessionManager.h"
#include "TimerManager.h"
#include "NetIOManager.h"
//===================================================

//===================================================
// ����ȭ ���� ����
//===================================================
#include "Packet.h"
#include "PacketProc.h"


//#define SERVERIP "192.168.30.16"
#define SERVERIP "127.0.0.1"
#define SERVERPORT 6000

bool g_bShutdown = false;

void Update(void);

DWORD g_targetFPS;			    // 1�ʴ� ��ǥ ������
DWORD g_targetFrame;		    // 1�ʴ� �־����� �ð� -> 1000 / targetFPS
DWORD g_currentServerTime;		// ���� ������ ���۵� �� �ʱ�ȭ�ǰ�, ���Ŀ� �������� ���� �� ���� targetFrameTime ��ŭ ����.

int main()
{
    //=====================================================================================================================================
    // listen ���� �غ�
    //=====================================================================================================================================
    CWinSockManager& winSockManager = CWinSockManager::getInstance();

    UINT8 options = 0;
    options |= OPTION_NONBLOCKING;

    winSockManager.StartServer(PROTOCOL_TYPE::TCP_IP, SERVERPORT, options);

    //=====================================================================================================================================
    // ���� �ð� ����
    //=====================================================================================================================================
    timeBeginPeriod(1);                     // Ÿ�̸� ���е�(�ػ�) 1ms ��

    g_targetFPS = 50;                       // ��ǥ �ʴ� ������
    g_targetFrame = 1000 / g_targetFPS;     // 1 �����ӿ� �־����� �ð�
    g_currentServerTime = timeGetTime();    // ���� ���� �ð� ����

    //=====================================================================================================================================
    // I/O ó���� ����ϴ� �Ŵ��� ȣ��
    //=====================================================================================================================================
    CNetIOManager::RegisterCallbackFunction(PacketProc);

    //=====================================================================================================================================
    // ���� ���� id �ʱ� �� ����
    //=====================================================================================================================================
    //g_id = 0;

    //=====================================================================================================================================
    // select���� ����� timeVal ����
    //=====================================================================================================================================
    TIMEVAL timeVal;
    timeVal.tv_sec = 0;
    timeVal.tv_usec = 0;

    //=====================================================================================================================================
    // ���� ����
    //=====================================================================================================================================
    while (!g_bShutdown)
    {
        try
        {
            // ��Ʈ��ũ I/O ó��
            CNetIOManager::netIOProcess();

            // ���� ���� ������Ʈ
            Update();
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << "\n";

            g_bShutdown = true;
        }
    }

    // ���� ������ �ִ� ���� �����ϰ� DB� ����
}

// ���� ����
void Update(void)
{
    // ������ ���߱�
    if (timeGetTime() < (g_currentServerTime + g_targetFrame))
        return;
    else
        g_currentServerTime += g_targetFrame;

    /*
    // ��Ȱ��ȭ�� Ŭ���̾�Ʈ�� ����Ʈ���� ����
    // ���⼭ �����ϴ� ������ ���� �����ӿ� ���� �󿡼� ���ŵ� ���ǵ��� sendQ�� ������� ���� ���ŵǱ� ���ؼ� �̷��� �ۼ�.
    auto it = g_clientList.begin();
    while (it != g_clientList.end())
    {
        // ��Ȱ��ȭ �Ǿ��ٸ�
        if (!(*it)->isAlive)
        {


            // ����
            closesocket((*it)->sock);

            delete (*it)->pPlayer;  // �÷��̾� ����
            delete (*it);           // ���� ����

            it = g_clientList.erase(it);
        }
        // Ȱ�� ���̶��
        else
        {
            ++it;
        }
    }
    */

}


//void netProc_Accept()
//{
//    // Ŭ���̾�Ʈ�� �������� �� ����Ǵ� ����
//    // ��α� ť�� ������ �Ǿ����� �����ϰ� Accept �õ�
//    SOCKET ClientSocket;
//    SOCKADDR_IN ClientAddr;
//
//    CWinSockManager<CSession>& winSockManager = CWinSockManager<CSession>::getInstance();
//    SOCKET listenSocket = winSockManager.GetListenSocket();
//
//    // accept �õ�
//    ClientSocket = winSockManager.Accept(ClientAddr);
//
//    // accept�� �Ϸ�Ǿ��ٸ� ���ǿ� ��� ��, �ش� ���ǿ� ��Ŷ ����
//    CSession* CSession = createSession(ClientSocket, ClientAddr,
//        g_id,
//        rand() % (dfRANGE_MOVE_RIGHT - dfRANGE_MOVE_LEFT) + dfRANGE_MOVE_LEFT,
//        rand() % (dfRANGE_MOVE_BOTTOM - dfRANGE_MOVE_TOP) + dfRANGE_MOVE_TOP,
//        100,
//        dfPACKET_MOVE_DIR_LL
//    );
//
//    // 1. ����� ���ǿ� PACKET_SC_CREATE_MY_CHARACTER �� ����
//    // 2. PACKET_SC_CREATE_OTHER_CHARACTER �� ����� ������ ������ ��� ��ε�ĳ��Ʈ
//    // 3. PACKET_SC_CREATE_OTHER_CHARACTER �� g_clientList�� �ִ� ��� ĳ���� ������ ��� ����� ���ǿ��� ����
//
//    //=====================================================================================================================================
//    // 1. ����� ���ǿ� PACKET_SC_CREATE_MY_CHARACTER �� ����
//    //=====================================================================================================================================
//    UINT16 posX, posY;
//    CSession->pPlayer->getPosition(posX, posY);
//    serverProxy.SC_CREATE_MY_CHARACTER_FOR_SINGLE(CSession, CSession->uid, CSession->pPlayer->GetDirection(), posX, posY, CSession->pPlayer->GetHp());
//
//    //=====================================================================================================================================
//    // 2. PACKET_SC_CREATE_OTHER_CHARACTER �� ����� ������ ������ ��� ��ε�ĳ��Ʈ
//    //=====================================================================================================================================
//    serverProxy.SC_CREATE_OTHER_CHARACTER_FOR_All(CSession, CSession->uid, CSession->pPlayer->GetDirection(), posX, posY, CSession->pPlayer->GetHp());
//
//    //=====================================================================================================================================
//    // 3. PACKET_SC_CREATE_OTHER_CHARACTER �� g_clientList�� �ִ� ��� ĳ���� ������ ��� ����� ���ǿ��� ����
//    //=====================================================================================================================================
//
//    // ���ο� ������ �õ��ϴ� Ŭ���̾�Ʈ�� ���� Ŭ���̾�Ʈ �������� ����
//    for (const auto& client : g_clientList)
//    {
//        client->pPlayer->getPosition(posX, posY);
//        serverProxy.SC_CREATE_OTHER_CHARACTER_FOR_SINGLE(CSession, client->uid, client->pPlayer->GetDirection(), posX, posY, client->pPlayer->GetHp());
//
//        // �����̰� �ִ� ��Ȳ�̶��
//        if (client->pPlayer->isBitSet(FLAG_MOVING))
//        {
//            serverProxy.SC_MOVE_START_FOR_SINGLE(CSession, client->uid, client->pPlayer->GetDirection(), posX, posY);
//        }
//    }
//
//    // �����͸� ������ �߿� ������ �� �� ������ ����ִ� ���� �˻�. ���� �־�� �ȵ����� sendQ�� ����á�� ��� ������ �߻��� �� ����. Ȥ�� �𸣴� �˻�
//    if (CSession->isAlive)
//    {
//        // ��� ���� ���� g_clientList�� ���� ���
//        g_clientList.push_back(CSession);
//
//        // ���� id�� ����
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