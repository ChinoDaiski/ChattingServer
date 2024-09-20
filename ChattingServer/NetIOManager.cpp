#include "pch.h"
#include "NetIOManager.h"

#include "WinSockManager.h"

#include "Session.h"
#include "SessionManager.h"

#include "ObjectManager.h"
#include "Packet.h"

PacketProcCallback CNetIOManager::m_callbackPacketProc;

CNetIOManager::CNetIOManager() noexcept
{
}

CNetIOManager::~CNetIOManager() noexcept
{
}

void CNetIOManager::netIOProcess(void)
{
    FD_SET ReadSet;
    FD_SET WriteSet;
    int iResult;

    TIMEVAL timeVal;
    timeVal.tv_sec = 0;
    timeVal.tv_usec = 0;

    // select는 한번에 최대 64개까지 처리 가능
    // 접속중인 모든 클라이언트에 대해 SOCKET 체크
    UINT32 iCSessionSize = g_clientList.size();

    auto iter = g_clientList.begin();


    static UINT32 netProc_RecvCnt = 0;
    static UINT32 netProc_SendCnt = 0;

    // 연결된 세션의 갯수가 64개 이상이라면
    while (iCSessionSize >= 64)
    {
        // 셋 초기화
        FD_ZERO(&ReadSet);
        FD_ZERO(&WriteSet);

        // begin부터 64개를 돌며 실행
        for (UINT8 i = 0; i < 64; ++i)
        {
            FD_SET((*iter)->sock, &ReadSet);

            // 만약 보낼 데이터가 있다면 WriteSet에 등록
            if ((*iter)->sendQ.GetUseSize() > 0)
                FD_SET((*iter)->sock, &WriteSet);

            ++iter;
        }

        // 전체 사이즈 64 빼기
        iCSessionSize -= 64;

        // select 호출
        iResult = select(0, &ReadSet, &WriteSet, NULL, &timeVal);

        // iResult 값이 0 이상이라면 읽을 데이터 / 쓸 데이터가 있다는 뜻
        if (iResult > 0)
        {
            auto iter2 = iter;
            for (UINT8 i = 0; i < 64; ++i)
            {
                iter2--;

                if (FD_ISSET((*iter2)->sock, &ReadSet))
                {
                    --iResult;

                    // recv 이벤트 처리. 메시지 수신 및 메시지 분기 로직 처리
                    if ((*iter2)->isAlive)
                    {
                        netProc_Recv((*iter2));
                        ++netProc_RecvCnt;
                    }
                }

                if (FD_ISSET((*iter2)->sock, &WriteSet))
                {
                    --iResult;

                    // send 이벤트 처리. 메시지 송신
                    if ((*iter2)->isAlive)
                    {
                        netProc_Send((*iter2));
                        ++netProc_SendCnt;
                    }
                }
            }
        }
        else if (iResult == SOCKET_ERROR)
        {
            std::cout << "Error : select(), " << WSAGetLastError() << "\n";
            DebugBreak();
        }
    }

    // 연결된 세션의 갯수가 64개 미만이라면

    // 셋 초기화
    FD_ZERO(&ReadSet);
    FD_ZERO(&WriteSet);

    // 남은 사이즈만큼 for문 돌면서 FD_SET 실행
    for (UINT8 i = 0; i < iCSessionSize; ++i)
    {
        FD_SET((*iter)->sock, &ReadSet);

        // 만약 보낼 데이터가 있다면 WriteSet에 등록
        if ((*iter)->sendQ.GetUseSize() > 0)
            FD_SET((*iter)->sock, &WriteSet);

        ++iter;
    }


    //==============================================================================================
    // listen 소켓 처리
    //==============================================================================================
    CWinSockManager& winSockManager = CWinSockManager::getInstance();
    SOCKET listenSocket = winSockManager.GetListenSocket();
    FD_SET(listenSocket, &ReadSet);

    // 이렇게 listen 소켓을 처리하는 이유는 listen 소켓이 blocking이기 때문


    // select 호출
    iResult = select(0, &ReadSet, &WriteSet, NULL, &timeVal);

    // iResult 값이 0 이상이라면 읽을 데이터 / 쓸 데이터가 있다는 뜻
    if (iResult > 0)
    {
        // 백로그 큐에 소켓이 있다면, accept 진행
        if (FD_ISSET(listenSocket, &ReadSet))
        {
            --iResult;

            // accept 이벤트 처리. 접속 및 CSession 생성
            netProc_Accept();
        }

        // listen 소켓을 제외한 세션 처리
        auto iter2 = iter;
        for (UINT8 i = 0; i < iCSessionSize; ++i)
        {
            iter2--;

            if (FD_ISSET((*iter2)->sock, &ReadSet))
            {
                --iResult;

                // recv 이벤트 처리. 메시지 수신 및 메시지 분기 로직 처리
                if ((*iter2)->isAlive)
                {
                    netProc_Recv((*iter2));
                    ++netProc_RecvCnt;
                }
            }

            if (FD_ISSET((*iter2)->sock, &WriteSet))
            {
                --iResult;

                // send 이벤트 처리. 메시지 송신
                if ((*iter2)->isAlive)
                {
                    netProc_Send((*iter2));
                    ++netProc_SendCnt;
                }
            }
        }
    }
    else if (iResult == SOCKET_ERROR)
    {
        std::cout << "Error : select(), " << WSAGetLastError() << "\n";
        DebugBreak();
    }
}

void CNetIOManager::netProc_Accept(void)
{
    // 클라이언트가 접속했을 때 진행되는 과정
    // 백로그 큐에 접속이 되었음을 감지하고 Accept 시도
    SOCKET ClientSocket;
    SOCKADDR_IN ClientAddr;

    CWinSockManager& winSockManager = CWinSockManager::getInstance();
    SOCKET listenSocket = winSockManager.GetListenSocket();

    // accept 시도
    ClientSocket = winSockManager.Accept(ClientAddr);

    // 세션을 만들어서 m_AcceptSessionList에 넣음
    CSessionManager& sessionManger = CSessionManager::getInstance();
    CSession* pSession = createSession(ClientSocket, ClientAddr);

    g_clientList.push_back(pSession);
}


void CNetIOManager::netProc_Send(CSession* pSession)
{
    // 링버퍼에 있는 데이터 한번에 send. 
    int directDeqSize = pSession->sendQ.DirectDequeueSize();
    int useSize = pSession->sendQ.GetUseSize();

    // 사용하는 용량(useSize)이 directDeqSize보다 작거나 같을 경우 useSize만큼만 보내고, 클 경우 directDeqSize만큼 보낸다.

    int retval = send(pSession->sock, pSession->sendQ.GetFrontBufferPtr(), std::min(useSize, directDeqSize), 0);

    // 여기서 소켓 에러 처리
    if (retval == SOCKET_ERROR)
    {
        int error = WSAGetLastError();

        // 중간에 강제로 연결 끊김.
        if (error == WSAECONNRESET)
        {
            NotifyClientDisconnected(pSession);
            return;
        }

        // 여기서 예외 처리 작업
        DebugBreak();
    }

    // 만약 send가 실패한다면
    if (retval != std::min(useSize, directDeqSize))
    {
        int error = WSAGetLastError();

        // 여기서 예외 처리 작업
        DebugBreak();
    }

    pSession->sendQ.MoveFront(retval);
}

void CNetIOManager::netProc_Recv(CSession* pSession)
{
    // 임시 수신 버퍼
    char tempRecvBuffer[RINGBUFFER_SIZE];
    char tempPacketBuffer[MAX_PACKET_SIZE + 1];

    // recv 임시 수신 버퍼로 호출
    int recvRetVal = recv(pSession->sock, tempRecvBuffer, RINGBUFFER_SIZE, 0);

    // recv 반환값이 0이라는 것은 상대방쪽에서 rst를 보냈다는 의미이므로 disconnect 처리를 해줘야한다.
    if (recvRetVal == 0)
    {
        // 더 이상 받을 값이 없으므로 break!
        NotifyClientDisconnected(pSession);
        //std::cout << "recv() return 0," << WSAGetLastError() << "\n";
        return;
    }
    // recv 에러일 경우
    else if (recvRetVal == SOCKET_ERROR)
    {
        int error = WSAGetLastError();

        // 수신 버퍼에 데이터가 없으므로 나중에 다시 recv 시도
        if (error == WSAEWOULDBLOCK)
            return;

        // 수신 버퍼와의 연결이 끊어진다는, rst가 왔다는 의미.
        if (error == WSAECONNRESET)
        {
            // 다른 클라이언트들에 연결이 끊어졌다는 것을 알림.
            NotifyClientDisconnected(pSession);
            return;
        }

        // 상대방 쪽에서 closesocket을 하면 recv 실패
        if (error == WSAECONNABORTED)
        {
            NotifyClientDisconnected(pSession);
            return;
        }

        // 만약 WSAEWOULDBLOCK이 아닌 다른 에러라면 진짜 문제
        std::cout << "Error : recv(), " << WSAGetLastError() << "\n";
        DebugBreak();
    }

    // 우선 데이터를 모두 넣음.
    int recvQEnqRetVal = pSession->recvQ.Enqueue(tempRecvBuffer, recvRetVal);
    if (recvQEnqRetVal != recvRetVal)
    {
        std::cout << "Error : recvQ.Enqueue(), " << WSAGetLastError() << "\n";
        DebugBreak();
    }

    // 완료패킷 처리 부
    // RecvQ에 있는 모든 완성패킷을 해석/처리
    while (true)
    {
        // 1. RecvQ에 최소한의 사이즈가 있는지 확인. 조건은 [ 헤더 사이즈 이상의 데이터가 있는지 확인 ]하는 것.
        if (pSession->recvQ.GetUseSize() < sizeof(PACKET_HEADER))
            break;

        // 2. RecvQ에서 PACKET_HEADER 정보 Peek
        PACKET_HEADER header;
        int headerSize = sizeof(header);
        int retVal = pSession->recvQ.Peek(reinterpret_cast<char*>(&header), headerSize);

        // 3. header의 Code 부분 확인. CRC 확인으로 이상한 값이 있으면 disconnect 처리
        if (header.byCode != dfPACKET_CODE)
        {
            NotifyClientDisconnected(pSession);
            break;
        }

        // 추가로 수신된 데이터의 크기가 최대 패킷 크기를 넘어서면 검사
        if (header.wPayloadSize > MAX_PACKET_SIZE)
        {
            DebugBreak();
        }

        // 4. 헤더의 len값과 RecvQ의 데이터 사이즈 비교
        if ((header.wPayloadSize + sizeof(PACKET_HEADER)) > pSession->recvQ.GetUseSize())
            break;

        // 5. Peek 했던 헤더를 RecvQ에서 지운다.
        pSession->recvQ.MoveFront(sizeof(PACKET_HEADER));

        // 6. RecvQ에서 header의 len 크기만큼 임시 패킷 버퍼를 뽑는다.
        int recvQDeqRetVal = pSession->recvQ.Dequeue(tempPacketBuffer, header.wPayloadSize);
        tempPacketBuffer[recvQDeqRetVal] = '\0';

        // 7. CheckSum 값으로 패킷의 데이터가 올바르게 왔는지 체크
        // checkSum - 각 MsgType, Payload 의 각 바이트 더하기 % 256
        UINT16 sum = header.wMsgType;
        for (UINT16 i = 0; i < header.wPayloadSize; ++i)
        {
            sum += tempPacketBuffer[i];
        }
        sum %= 256;

        if (sum != header.byCheckSum)
        {
            DebugBreak();
        }

        // 8. 헤더의 타입에 따른 분기를 위해 패킷 프로시저 호출
        CPacket Packet;
        Packet.PutData(tempPacketBuffer, recvQDeqRetVal);
        if (!m_callbackPacketProc(pSession, static_cast<PACKET_TYPE>(header.wMsgType), &Packet))
        {
            NotifyClientDisconnected(pSession);
            break;
        }
    }
}
