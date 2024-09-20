#pragma once

#include "Singleton.h"
#include <list>
class CSession;

//==========================================================================================================================================
// Broadcast
//==========================================================================================================================================
void BroadcastData(CSession* excludeCSession, PACKET_HEADER* pPacket, UINT32 dataSize);
void BroadcastData(CSession* excludeCSession, CPacket* pPacket, UINT32 dataSize);
void BroadcastPacket(CSession* excludeCSession, PACKET_HEADER* pHeader, CPacket* pPacket);

//==========================================================================================================================================
// Unicast
//==========================================================================================================================================
void UnicastData(CSession* includeCSession, PACKET_HEADER* pPacket, UINT32 dataSize);
void UnicastData(CSession* includeCSession, CPacket* pPacket, UINT32 dataSize);
void UnicastPacket(CSession* includeCSession, PACKET_HEADER* pHeader, CPacket* pPacket);

//==========================================================================================================================================
// 생성 / 소멸
//==========================================================================================================================================
void NotifyClientDisconnected(CSession* disconnectedCSession);
CSession* createSession(SOCKET ClientSocket, SOCKADDR_IN ClientAddr);

extern std::list<CSession*> g_clientList;   // 서버에 접속한 세션들에 대한 정보


typedef void(*DisconnectCallback)(CSession* pSession);

class CSessionManager : public SingletonBase<CSessionManager> {
private:
    friend class SingletonBase<CSessionManager>;

public:
    explicit CSessionManager() noexcept;
    ~CSessionManager() noexcept;

    // 복사 생성자와 대입 연산자를 삭제하여 복사 방지
    CSessionManager(const CSessionManager&) = delete;
    CSessionManager& operator=(const CSessionManager&) = delete;

public:
    void Update(void);

public:
    friend void BroadcastData(CSession* excludeCSession, PACKET_HEADER* pPacket, UINT8 dataSize);
    friend void BroadcastData(CSession* excludeCSession, CPacket* pPacket, UINT8 dataSize);
    friend void BroadcastPacket(CSession* excludeCSession, PACKET_HEADER* pHeader, CPacket* pPacket);

    friend void UnicastData(CSession* includeCSession, PACKET_HEADER* pPacket, UINT8 dataSize);
    friend void UnicastData(CSession* includeCSession, CPacket* pPacket, UINT8 dataSize);
    friend void UnicastPacket(CSession* includeCSession, PACKET_HEADER* pHeader, CPacket* pPacket);

public:
    static void RegisterDisconnectCallback(DisconnectCallback callback) { m_callbackDisconnect = callback; }

private:
    static DisconnectCallback m_callbackDisconnect;
};
