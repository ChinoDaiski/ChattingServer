#include "pch.h"
#include "MakePacket.h"
#include "SessionManager.h"
#include "Packet.h"
#include "User.h"

void RES_LOGIN_FOR_All(CSession* pSession, UINT8 result, UINT32 userNO)
{
    PACKET_HEADER header;
    CPacket Packet;

    //================================================================================

    Packet << result;
    Packet << userNO;

    //================================================================================

    header.wMsgType = df_RES_LOGIN;
    header.byCode = dfPACKET_CODE;
    header.wPayloadSize = Packet.GetDataSize();

    header.byCheckSum = static_cast<UINT8>(header.wMsgType);
    char* pBuffer = Packet.GetBufferPtr();
    for (UINT16 i = 0; i < header.wPayloadSize; ++i)
    {
        header.byCheckSum += pBuffer[i];
    }
    header.byCheckSum %= 256;

	BroadcastPacket(pSession, &header, &Packet);
}

void RES_LOGIN_FOR_SINGLE(CSession* pSession, UINT8 result, UINT32 userNO)
{
    PACKET_HEADER header;
    CPacket Packet;

    //================================================================================

    Packet << result;
    Packet << userNO;

    //================================================================================

    header.wMsgType = df_RES_LOGIN;
    header.byCode = dfPACKET_CODE;
    header.wPayloadSize = Packet.GetDataSize();

    header.byCheckSum = static_cast<UINT8>(header.wMsgType);
    char* pBuffer = Packet.GetBufferPtr();
    for (UINT16 i = 0; i < header.wPayloadSize; ++i)
    {
        header.byCheckSum += pBuffer[i];
    }
    header.byCheckSum %= 256;

    UnicastPacket(pSession, &header, &Packet);
}

void RES_ROOM_LIST_FOR_All(CSession* pSession, const std::list<CRoom*>& roomList)
{
    PACKET_HEADER header;
    CPacket Packet;

    //================================================================================

    Packet << roomList.size();
    for (auto& iter : roomList)
    {
        CRoom room = *iter;

        Packet << room.m_roomID;

        Packet << room.m_roomName.size();
        Packet << room.m_roomName.c_str();

        Packet << room.m_participants.size();

        CUser* pUser;
        for (auto& iter : room.m_participants)
        {
            pUser = iter.second;
            Packet.PutData((char*)pUser->GetName().c_str(), dfNICK_MAX_LEN * sizeof(WCHAR));
        }
    }

    //================================================================================

    header.wMsgType = df_RES_ROOM_LIST;
    header.byCode = dfPACKET_CODE;
    header.wPayloadSize = Packet.GetDataSize();

    header.byCheckSum = static_cast<UINT8>(header.wMsgType);
    char* pBuffer = Packet.GetBufferPtr();
    for (UINT16 i = 0; i < header.wPayloadSize; ++i)
    {
        header.byCheckSum += pBuffer[i];
    }
    header.byCheckSum %= 256;

    BroadcastPacket(pSession, &header, &Packet);
}

void RES_ROOM_LIST_FOR_SINGLE(CSession* pSession, const std::list<CRoom*>& roomList)
{
    PACKET_HEADER header;
    CPacket Packet;

    //================================================================================

    Packet << (UINT16)roomList.size();

    for (auto& iter : roomList)
    {
        CRoom room = *iter;

        Packet << room.m_roomID;

        UINT16 size = (UINT16)room.m_roomName.size() * sizeof(WCHAR);
        Packet << size;
        Packet.PutData((char*)room.m_roomName.c_str(), size);

        Packet << (UINT8)room.m_participants.size();

        CUser* pUser;
        for (auto& iter : room.m_participants)
        {
            pUser = iter.second;
            Packet.PutData((char*)pUser->GetName().c_str(), dfNICK_MAX_LEN * sizeof(WCHAR));
        }
    }

    //================================================================================

    header.wMsgType = df_RES_ROOM_LIST;
    header.byCode = dfPACKET_CODE;
    header.wPayloadSize = Packet.GetDataSize();

    header.byCheckSum = static_cast<UINT8>(header.wMsgType);
    char* pBuffer = Packet.GetBufferPtr();
    for (UINT16 i = 0; i < header.wPayloadSize; ++i)
    {
        header.byCheckSum += pBuffer[i];
    }
    header.byCheckSum %= 256;

    UnicastPacket(pSession, &header, &Packet);
}

void RES_ROOM_CREATE_FOR_All(CSession* pSession, UINT8 result, const CRoom* room)
{
    PACKET_HEADER header;
    CPacket Packet;

    //================================================================================

    Packet << result;

    if (nullptr != room)
    {
        Packet << room->m_roomID;
        UINT16 size = static_cast<UINT16>(room->m_roomName.size() * sizeof(WCHAR));
        Packet << size;
        Packet.PutData((char*)room->m_roomName.c_str(), size);
    }

    //================================================================================

    header.wMsgType = df_RES_ROOM_CREATE;
    header.byCode = dfPACKET_CODE;
    header.wPayloadSize = Packet.GetDataSize();

    header.byCheckSum = static_cast<UINT8>(header.wMsgType);
    char* pBuffer = Packet.GetBufferPtr();
    for (UINT16 i = 0; i < header.wPayloadSize; ++i)
    {
        header.byCheckSum += pBuffer[i];
    }
    header.byCheckSum %= 256;

    BroadcastPacket(pSession, &header, &Packet);
}

void RES_ROOM_CREATE_FOR_SINGLE(CSession* pSession, UINT8 result, const CRoom* room)
{
    PACKET_HEADER header;
    CPacket Packet;

    //================================================================================

    Packet << result;

    if (nullptr != room)
    {
        Packet << room->m_roomID;
        UINT16 size = static_cast<UINT16>(room->m_roomName.size() * sizeof(WCHAR));
        Packet << size;
        Packet.PutData((char*)room->m_roomName.c_str(), size);
    }

    //================================================================================

    header.wMsgType = df_RES_ROOM_CREATE;
    header.byCode = dfPACKET_CODE;
    header.wPayloadSize = Packet.GetDataSize();

    header.byCheckSum = static_cast<UINT8>(header.wMsgType);
    char* pBuffer = Packet.GetBufferPtr();
    for (UINT16 i = 0; i < header.wPayloadSize; ++i)
    {
        header.byCheckSum += pBuffer[i];
    }
    header.byCheckSum %= 256;

    UnicastPacket(pSession, &header, &Packet);
}

void RES_ROOM_ENTER_FOR_All(CSession* pSession, UINT8 result, const CRoom* room)
{
    PACKET_HEADER header;
    CPacket Packet;

    //================================================================================

    Packet << result;

    // 방에 들어갈 수 있을 경우
    if (result == df_RESULT_ROOM_ENTER_OK)
    {
        Packet << room->m_roomID;
        UINT16 size = static_cast<UINT16>(room->m_roomName.size() * sizeof(WCHAR));
        Packet << size;
        Packet.PutData((char*)room->m_roomName.c_str(), size);

        UINT8 participantSize = static_cast<UINT8>(room->m_participants.size());
        Packet << participantSize;

        CUser* pUser;
        for (auto& iter : room->m_participants)
        {
            pUser = iter.second;
            Packet.PutData((char*)pUser->GetName().c_str(), dfNICK_MAX_LEN * sizeof(WCHAR));
            Packet << pUser->GetUID();
        }
    }

    //================================================================================

    header.wMsgType = df_RES_ROOM_ENTER;
    header.byCode = dfPACKET_CODE;
    header.wPayloadSize = Packet.GetDataSize();

    header.byCheckSum = static_cast<UINT8>(header.wMsgType);
    char* pBuffer = Packet.GetBufferPtr();
    for (UINT16 i = 0; i < header.wPayloadSize; ++i)
    {
        header.byCheckSum += pBuffer[i];
    }
    header.byCheckSum %= 256;

    BroadcastPacket(pSession, &header, &Packet);
}

void RES_ROOM_ENTER_FOR_SINGLE(CSession* pSession, UINT8 result, const CRoom* room)
{
    PACKET_HEADER header;
    CPacket Packet;

    //================================================================================

    Packet << result;

    // 방에 들어갈 수 있을 경우
    if (result == df_RESULT_ROOM_ENTER_OK)
    {
        Packet << room->m_roomID;
        UINT16 size = static_cast<UINT16>(room->m_roomName.size() * sizeof(WCHAR));
        Packet << size;
        Packet.PutData((char*)room->m_roomName.c_str(), size);

        UINT8 participantSize = static_cast<UINT8>(room->m_participants.size());
        Packet << participantSize;

        CUser* pUser;
        for (auto& iter : room->m_participants)
        {
            pUser = iter.second;
            Packet.PutData((char*)pUser->GetName().c_str(), dfNICK_MAX_LEN * sizeof(WCHAR));
            Packet << pUser->GetUID();
        }
    }

    //================================================================================

    header.wMsgType = df_RES_ROOM_ENTER;
    header.byCode = dfPACKET_CODE;
    header.wPayloadSize = Packet.GetDataSize();

    header.byCheckSum = static_cast<UINT8>(header.wMsgType);
    char* pBuffer = Packet.GetBufferPtr();
    for (UINT16 i = 0; i < header.wPayloadSize; ++i)
    {
        header.byCheckSum += pBuffer[i];
    }
    header.byCheckSum %= 256;

    UnicastPacket(pSession, &header, &Packet);
}

void RES_CHAT_FOR_All(CSession* pSession, UINT32 senderNo, UINT16 strlen, WCHAR* str)
{
    PACKET_HEADER header;
    CPacket Packet;

    //================================================================================

    Packet << senderNo;
    Packet << strlen;
    Packet.PutData((char*)str, strlen * sizeof(WCHAR));

    //================================================================================

    header.wMsgType = df_RES_CHAT;
    header.byCode = dfPACKET_CODE;
    header.wPayloadSize = Packet.GetDataSize();

    header.byCheckSum = static_cast<UINT8>(header.wMsgType);
    char* pBuffer = Packet.GetBufferPtr();
    for (UINT16 i = 0; i < header.wPayloadSize; ++i)
    {
        header.byCheckSum += pBuffer[i];
    }
    header.byCheckSum %= 256;

    BroadcastPacket(pSession, &header, &Packet);
}

void RES_CHAT_FOR_SINGLE(CSession* pSession, UINT32 senderNo, UINT16 strlen, WCHAR* str)
{
    PACKET_HEADER header;
    CPacket Packet;

    //================================================================================

    Packet << senderNo;
    Packet << strlen;
    Packet.PutData((char*)str, strlen * sizeof(WCHAR));

    //================================================================================

    header.wMsgType = df_RES_CHAT;
    header.byCode = dfPACKET_CODE;
    header.wPayloadSize = Packet.GetDataSize();

    header.byCheckSum = static_cast<UINT8>(header.wMsgType);
    char* pBuffer = Packet.GetBufferPtr();
    for (UINT16 i = 0; i < header.wPayloadSize; ++i)
    {
        header.byCheckSum += pBuffer[i];
    }
    header.byCheckSum %= 256;

    UnicastPacket(pSession, &header, &Packet);
}

void RES_ROOM_LEAVE_FOR_All(CSession* pSession, UINT32 userNO)
{
    PACKET_HEADER header;
    CPacket Packet;

    //================================================================================

    Packet << userNO;

    //================================================================================

    header.wMsgType = df_RES_ROOM_LEAVE;
    header.byCode = dfPACKET_CODE;
    header.wPayloadSize = Packet.GetDataSize();

    header.byCheckSum = static_cast<UINT8>(header.wMsgType);
    char* pBuffer = Packet.GetBufferPtr();
    for (UINT16 i = 0; i < header.wPayloadSize; ++i)
    {
        header.byCheckSum += pBuffer[i];
    }
    header.byCheckSum %= 256;

    BroadcastPacket(pSession, &header, &Packet);
}

void RES_ROOM_LEAVE_FOR_SINGLE(CSession* pSession, UINT32 userNO)
{
    PACKET_HEADER header;
    CPacket Packet;

    //================================================================================

    Packet << userNO;

    //================================================================================

    header.wMsgType = df_RES_ROOM_LEAVE;
    header.byCode = dfPACKET_CODE;
    header.wPayloadSize = Packet.GetDataSize();

    header.byCheckSum = static_cast<UINT8>(header.wMsgType);
    char* pBuffer = Packet.GetBufferPtr();
    for (UINT16 i = 0; i < header.wPayloadSize; ++i)
    {
        header.byCheckSum += pBuffer[i];
    }
    header.byCheckSum %= 256;

    UnicastPacket(pSession, &header, &Packet);
}

void RES_ROOM_DELETE_FOR_All(CSession* pSession, UINT32 userNO)
{
    PACKET_HEADER header;
    CPacket Packet;

    //================================================================================

    Packet << userNO;

    //================================================================================

    header.wMsgType = df_RES_ROOM_DELETE;
    header.byCode = dfPACKET_CODE;
    header.wPayloadSize = Packet.GetDataSize();

    header.byCheckSum = static_cast<UINT8>(header.wMsgType);
    char* pBuffer = Packet.GetBufferPtr();
    for (UINT16 i = 0; i < header.wPayloadSize; ++i)
    {
        header.byCheckSum += pBuffer[i];
    }
    header.byCheckSum %= 256;

    BroadcastPacket(pSession, &header, &Packet);
}

void RES_ROOM_DELETE_FOR_SINGLE(CSession* pSession, UINT32 userNO)
{
    PACKET_HEADER header;
    CPacket Packet;

    //================================================================================

    Packet << userNO;

    //================================================================================

    header.wMsgType = df_RES_ROOM_DELETE;
    header.byCode = dfPACKET_CODE;
    header.wPayloadSize = Packet.GetDataSize();

    header.byCheckSum = static_cast<UINT8>(header.wMsgType);
    char* pBuffer = Packet.GetBufferPtr();
    for (UINT16 i = 0; i < header.wPayloadSize; ++i)
    {
        header.byCheckSum += pBuffer[i];
    }
    header.byCheckSum %= 256;

    UnicastPacket(pSession, &header, &Packet);
}

void RES_USER_ENTER_FOR_All(CSession* pSession, WCHAR* str, UINT32 userNO)
{
    PACKET_HEADER header;
    CPacket Packet;

    //================================================================================

    Packet.PutData((char*)str, sizeof(WCHAR) * dfNICK_MAX_LEN);
    Packet << userNO;

    //================================================================================

    header.wMsgType = df_RES_USER_ENTER;
    header.byCode = dfPACKET_CODE;
    header.wPayloadSize = Packet.GetDataSize();

    header.byCheckSum = static_cast<UINT8>(header.wMsgType);
    char* pBuffer = Packet.GetBufferPtr();
    for (UINT16 i = 0; i < header.wPayloadSize; ++i)
    {
        header.byCheckSum += pBuffer[i];
    }
    header.byCheckSum %= 256;

    BroadcastPacket(pSession, &header, &Packet);
}

void RES_USER_ENTER_FOR_SINGLE(CSession* pSession, WCHAR* str, UINT32 userNO)
{
    PACKET_HEADER header;
    CPacket Packet;

    //================================================================================

    Packet.PutData((char*)str, sizeof(WCHAR) * dfNICK_MAX_LEN);
    Packet << userNO;

    //================================================================================

    header.wMsgType = df_RES_USER_ENTER;
    header.byCode = dfPACKET_CODE;
    header.wPayloadSize = Packet.GetDataSize();

    header.byCheckSum = static_cast<UINT8>(header.wMsgType);
    char* pBuffer = Packet.GetBufferPtr();
    for (UINT16 i = 0; i < header.wPayloadSize; ++i)
    {
        header.byCheckSum += pBuffer[i];
    }
    header.byCheckSum %= 256;

    UnicastPacket(pSession, &header, &Packet);
}