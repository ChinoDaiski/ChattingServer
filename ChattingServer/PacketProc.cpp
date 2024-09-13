#include "pch.h"
#include "PacketProc.h"
#include "Packet.h"
#include "MakePacket.h"

#include "Content.h"

#include "User.h"

//
//#include "SessionManager.h"
//#include "ObjectManager.h"
//

bool PacketProc(CSession* pSession, PACKET_TYPE packetType, CPacket* pPacket)
{
    CObject* pObj = pSession->pObj;

    switch (packetType)
    {
    case PACKET_TYPE::PT_REQ_LOGIN:
    {
        WCHAR szNickName[dfNICK_MAX_LEN + 1];
        pPacket->GetData((char*)szNickName, dfNICK_MAX_LEN * sizeof(WCHAR));
        szNickName[dfNICK_MAX_LEN] = '\0';

        return REQ_LOGIN(pSession, szNickName);
    }
    break;
    case PACKET_TYPE::PT_REQ_ROOM_LIST:
    {
        return REQ_ROOM_LIST(pSession);
    }
    break;
    case PACKET_TYPE::PT_REQ_ROOM_CREATE:
    {
        UINT16 strSize;
        *pPacket >> strSize;

        WCHAR szNickName[100];
        pPacket->GetData((char*)szNickName, strSize);
        szNickName[(strSize >> 1)] = L'\0';

        return REQ_ROOM_CREATE(pSession, strSize, szNickName);
    }
    break;
    case PACKET_TYPE::PT_REQ_ROOM_ENTER:
    {
        UINT32 roomNo;
        *pPacket >> roomNo;

        return REQ_ROOM_ENTER(pSession, roomNo);
    }
    break;
    case PACKET_TYPE::PT_REQ_CHAT:
    {
        UINT16 strSize;
        *pPacket >> strSize;

        WCHAR szNickName[400];
        pPacket->GetData((char*)szNickName, strSize);
        szNickName[strSize >> 1] = '\0';

        return REQ_CHAT(pSession, strSize, szNickName);
    }
    break;
    case PACKET_TYPE::PT_REQ_ROOM_LEAVE:
    {
        return REQ_ROOM_LEAVE(pSession);
    }
    break;
    default:
        return false;
        break;
    }

    return false;
}
bool REQ_LOGIN(CSession* pSession, WCHAR* szNickName)
{
    CPacket Packet;

    UINT8 result = df_RESULT_LOGIN_ETC;
    CUser* pUser;

    // �г����� ������ �ִ� ������ ��ġ�� �˻�
    std::wstring strNickName{ szNickName };

    // ��ġ�� �г����� ���ٸ�
    if (RegisterUserName(strNickName))
    {
        result = df_RESULT_LOGIN_OK;

        // ���� ����
        pUser = new CUser(strNickName);

        // ������ ���� ������ ������ �ְ�
        // ���ǵ� ���� ������ void* �������� ������ �ִ�.
        pUser->m_pSession = pSession;
        pSession->pObj = pUser;

        // ���ο� ������ ���������� �����ο��� ����ĳ��Ʈ�� �˸�
        RES_LOGIN_FOR_SINGLE(pSession, result, pUser->GetUID());

        // ���� ������ ������ ������ �ٸ� ��� �����鿡�� ���ο� ������ ���������� �˸�
        RES_USER_ENTER_FOR_All(pSession, szNickName, pUser->m_uID);
    }
    else
    {
        result = df_RESULT_LOGIN_DNICK;

        // �ߺ� �г������� ���ӿ� ���������� ����ĳ��Ʈ�� �˸�
        RES_LOGIN_FOR_SINGLE(pSession, result, 0);
    }

    return true;
}
bool REQ_ROOM_LIST(CSession* pSession)
{
    // �ش� �������� ��� �濡 ���� ������ ����ĳ��Ʈ�� �˸�
    std::list<CRoom*> roomList;
    for (auto& iter : g_roomList)
    {
        roomList.push_back(iter.second);
    }
    RES_ROOM_LIST_FOR_SINGLE(pSession, roomList);

    return true;
}
bool REQ_ROOM_CREATE(CSession* pSession, UINT16 strSize, WCHAR* roomName)
{
    // �г����� �޾� ���� ����
    // �г��� �ִ� ũ��� �뷫 255��� ����. �� �̻� �̸��� ������� ������ ����� ���� ���� ������ ũ�� �ø� ��
    WCHAR szName[255];
    std::wstring strRoomName{ roomName };

    UINT8 result;
    CRoom* pRoom = nullptr;

    // �� �̸��� �ߺ��ȴٸ�
    if (!RegisterRoomName(strRoomName))
        result = df_RESULT_ROOM_CREATE_DNICK;

    // �� �̸��� �ߺ����� �ʴ´ٸ�
    else
    {
        result = df_RESULT_ROOM_CREATE_OK;

        // �� ����
        pRoom = new CRoom(strRoomName);
        g_roomList[pRoom->m_roomID] = pRoom;
    }

    // ���ο� ���� �����Ǿ����� ��ε�ĳ��Ʈ�� �˸�
    RES_ROOM_CREATE_FOR_All(pSession, result, pRoom);

    return true;
}
bool REQ_ROOM_ENTER(CSession* pSession, UINT16 roomNo)
{
    // �ش� ���ǰ� ����� ���� ��ü�� Ư�� ������ ����
    CUser* pUser = static_cast<CUser*>(pSession->pObj);
    UINT8 result;
    CRoom* pRoom = nullptr;

    // ���� ������ �̹� �濡 ���ٸ�
    if (pUser->m_enterRoomNo != ROBBY_ROOM_NUMBER)
    {
        result = df_RESULT_ROOM_ENTER_NOT;
    }
    else
    {
        // ���� ã��
        auto iter = g_roomList.find(roomNo);
        if (iter != g_roomList.end())
        {
            // ���� �����Ѵٸ�
            result = df_RESULT_ROOM_ENTER_OK;
            pRoom = iter->second;
        }
    }

    // ����� ���ǿ��� �濡 �����ߴٰ� �޽����� ����
    RES_ROOM_ENTER_FOR_SINGLE(pSession, result, pRoom);

    if (result == df_RESULT_ROOM_ENTER_OK)
    {
        pRoom->Participate(pUser);

        // �̹� �濡 �� �ִ� ���ǵ鿡�� ���ο� ������ �����ٰ� �˸�
        CUser* pExistingUser;
        for (auto& participant : pRoom->m_participants)
        {
            pExistingUser = participant.second;
            WCHAR szName[dfNICK_MAX_LEN + 1];
            memcpy(szName, pUser->m_strName.c_str(), dfNICK_MAX_LEN);
            szName[dfNICK_MAX_LEN] = '\0';

            RES_USER_ENTER_FOR_SINGLE(pExistingUser->m_pSession, szName, pUser->m_uID);
        }
    }

    return true;
}
bool REQ_CHAT(CSession* pSession, UINT16 strSize, WCHAR* szChat)
{
    // �ش� ���ǰ� ����� ������ �����ϴ� �� ������ ������
    CUser* pUser = static_cast<CUser*>(pSession->pObj);
    auto iter = g_roomList.find(pUser->m_enterRoomNo);

    // ���� �����Ѵٸ�
    if (iter != g_roomList.end())
    {
        // �濡 �����ϴ� �������� ���ƴٴϸ� ä�� ���� ����
        for (auto& participant : iter->second->m_participants)
        {
            // ���� �ڱ� �ڽ��̸� �Ѿ��
            if (participant.second == pUser)
                continue;

            RES_CHAT_FOR_SINGLE(participant.second->m_pSession, pUser->m_uID, strSize, szChat);
        }
    }

    return true;
}
bool REQ_ROOM_LEAVE(CSession* pSession)
{
    // �ش� ���ǰ� ����� ������ �濡�� ���� �κ�� ��
    CUser* pUser = static_cast<CUser*>(pSession->pObj);
    
    auto iter = g_roomList.find(pUser->m_enterRoomNo);
    if (iter != g_roomList.end())
    {
        // �濡 ���� �ִ� �����鿡�� �ش� ���ǰ� ����� ������ �������� �˸�
        for (const auto& participant : iter->second->m_participants)
        {
            RES_ROOM_LEAVE_FOR_SINGLE(participant.second->m_pSession, pUser->m_uID);
        }

        // �ش� ���ǰ� ����� ������ �濡�� ����
        iter->second->Withdraw(pUser);

        // ���� �濡 �ƹ��� ���� ���� �ʴٸ� ���� �ı��ϰ� ��ε�ĳ��Ʈ�� �˸�
        if (iter->second->m_participants.empty())
        {
            // �� �̸� ����
            RemoveRoomName(iter->second->m_roomName);

            // �� �ı� �������� ��ε�ĳ��Ʈ
            RES_ROOM_DELETE_FOR_All(nullptr, iter->first);

            // �� ���� ����
            g_roomList.erase(iter->first);

        }
    }

    return true;
}

void DisconnectSessionProc(CSession* pSession)
{
    // ������ ������ �ִ� ������Ʈ�� �������� �ʴ´ٸ�, ���ʿ� ������ ���� ��ü�� ���� ���� ���� ���̹Ƿ� �ѱ�
    if (pSession->pObj == nullptr)
        return;

    CUser* pUser = static_cast<CUser*>(pSession->pObj);

    // ���� ������ �κ�� ������ �ʾҴٸ�
    if (pUser->m_enterRoomNo != ROBBY_ROOM_NUMBER)
    {
        // ���� �˻��ؼ� �ش� �濡 �ִ� �����鿡�� �����ٰ� �˸�
        auto iter = g_roomList.find(pUser->m_enterRoomNo);
        if (iter != g_roomList.end())
        {
            // �濡 ���� �ִ� �����鿡�� �ش� ���ǰ� ����� ������ �������� �˸�
            for (const auto& participant : iter->second->m_participants)
            {
                RES_ROOM_LEAVE_FOR_SINGLE(participant.second->m_pSession, pUser->m_uID);
            }

            // �ش� ���ǰ� ����� ������ �濡�� ����
            iter->second->Withdraw(pUser);

            // ���� �濡 �ƹ��� ���� ���� �ʴٸ� ���� �ı��ϰ� ��ε�ĳ��Ʈ�� �˸�
            if (iter->second->m_participants.empty())
            {
                RES_ROOM_DELETE_FOR_All(nullptr, iter->first);
                g_roomList.erase(iter->first);
            }
        }
    }

    // ���� ����Ǵ� �̸� ����
    RemoveUserName(pUser->m_strName);
}