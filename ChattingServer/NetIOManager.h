#pragma once

#include "Singleton.h"

class CSession;
class CPacket;

typedef bool(*PacketCallback)(CSession* pSession, PACKET_TYPE packetType, CPacket* pPacket);

class CNetIOManager : public SingletonBase<CNetIOManager> {
private:
    friend class SingletonBase<CNetIOManager>;

public:
    explicit CNetIOManager() noexcept;
    ~CNetIOManager() noexcept;

    // 복사 생성자와 대입 연산자를 삭제하여 복사 방지
    CNetIOManager(const CNetIOManager&) = delete;
    CNetIOManager& operator=(const CNetIOManager&) = delete;

public:
    void static netIOProcess(void);
    void static netProc_Accept(void);
    void static netProc_Send(CSession* pSession);
    void static netProc_Recv(CSession* pSession);

public:
    //std::queue<CSession*>& GetAcceptSessionQueue(void) { return m_AcceptSessionQueue; }

public:
    static void RegisterCallbackFunction(PacketCallback callback);

private:
    //std::queue<CSession*> m_AcceptSessionQueue;
    static PacketCallback m_callback;
    static UINT16 g_ID;
};
