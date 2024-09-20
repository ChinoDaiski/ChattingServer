#pragma once

class CPacket;
class CObject;

class CSession
{
public:
    explicit CSession();
	~CSession();

public:
    void RegisterObject(CObject** ppObj);

public:
    // 삭제 여부를 판별하는 변수
    bool isAlive;

    // 세션 info - 소켓, ip, port
    USHORT port;
    char IP[16];
    SOCKET sock;
    CRingBuffer recvQ;  // 수신용 링버퍼
    CRingBuffer sendQ;  // 송신용 링버퍼

    CObject* pObj;

private:
    static UINT16 g_uID;
};