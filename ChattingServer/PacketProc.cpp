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
    UINT8 result = df_RESULT_LOGIN_ETC;
    CUser* pConnectedUser;

    // 닉네임이 기존에 있는 유저와 겹치는 검사
    std::wstring strNickName{ szNickName };

    // 겹치는 닉네임이 없다면
    if (RegisterUserName(strNickName))
    {
        result = df_RESULT_LOGIN_OK;

        // 유저 생성
        pConnectedUser = new CUser(strNickName);

        // 유저가 세션 정보를 가지고 있고
        // 세션도 유저 정보를 void* 형식으로 가지고 있다.
        pConnectedUser->m_pSession = pSession;
        pSession->pObj = pConnectedUser;

        // 스스로가 접속 했음을 알림
        RES_LOGIN_FOR_SINGLE(pSession, result, pConnectedUser->m_uID);

        // 유저 정보 추가
        g_userList.push_back(pConnectedUser);
    }
    else
    {
        result = df_RESULT_LOGIN_DNICK;

        // 중복 닉네임으로 접속에 실패했음을 유니캐스트로 알림
        RES_LOGIN_FOR_SINGLE(pSession, result, 0);
    }

    return true;
}
bool REQ_ROOM_LIST(CSession* pSession)
{
    // 해당 유저에게 모든 방에 대한 정보를 유니캐스트로 알림
    std::list<CRoom*> roomList;
    for (auto& iter : g_roomUMapList)
    {
        roomList.push_back(iter.second);
    }

    RES_ROOM_LIST_FOR_SINGLE(pSession, roomList);

    return true;
}
bool REQ_ROOM_CREATE(CSession* pSession, UINT16 strSize, WCHAR* roomName)
{
    // 닉네임을 받아 방을 생성
    std::wstring strRoomName{ roomName };

    UINT8 result;
    CRoom* pRoom = nullptr;

    // 방 이름이 중복된다면
    if (!RegisterRoomName(strRoomName))
        result = df_RESULT_ROOM_CREATE_DNICK;

    // 방 이름이 중복되지 않는다면
    else
    {
        result = df_RESULT_ROOM_CREATE_OK;

        // 방 생성
        pRoom = new CRoom(strRoomName);
        g_roomUMapList[pRoom->m_roomID] = pRoom;
    }

    // 새로운 방이 생성되었음을 접속한 유저들에게 알림
    for (const auto& user : g_userList)
    {
        RES_ROOM_CREATE_FOR_SINGLE(user->m_pSession, result, pRoom);
    }

    return true;
}
bool REQ_ROOM_ENTER(CSession* pSession, UINT16 roomNo)
{
    // 해당 세션과 연결된 유저 객체를 특정 방으로 입장
    CUser* pUser = static_cast<CUser*>(pSession->pObj);
    UINT8 result = df_RESULT_ROOM_ENTER_ETC;
    CRoom* pRoom = nullptr;

    // 만약 유저가 이미 방에 들어갔다면
    if (pUser->m_enterRoomNo != ROBBY_ROOM_NUMBER)
    {
        result = df_RESULT_ROOM_ENTER_NOT;
    }
    else
    {
        // 방을 찾음
        auto iter = g_roomUMapList.find(roomNo);
        if (iter != g_roomUMapList.end())
        {
            // 방이 존재한다면
            result = df_RESULT_ROOM_ENTER_OK;
            pRoom = iter->second;
        }
        // 방을 못찾았다면
        else
        {
            // 방 정보가 없는 방에 접근했다는 의미이므로 심각한 오류.
            result = df_RESULT_ROOM_ENTER_ETC;
        }
    }

    // 연결된 세션에게 방에 접속했다고 메시지를 보냄
    RES_ROOM_ENTER_FOR_SINGLE(pSession, result, pRoom);

    if (result == df_RESULT_ROOM_ENTER_OK)
    {
        pRoom->Participate(pUser);

        // 이미 방에 들어가 있는 세션들에게 새로운 세션이 연결됬다고 알림
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
    // 해당 세션과 연결된 유저가 존재하는 방 정보를 가져옴
    CUser* pUser = static_cast<CUser*>(pSession->pObj);
    auto iter = g_roomUMapList.find(pUser->m_enterRoomNo);

    // 방이 존재한다면
    if (iter != g_roomUMapList.end())
    {
        // 방에 존재하는 유저들을 돌아다니며 채팅 정보 전송
        for (auto& participant : iter->second->m_participants)
        {
            // 만약 자기 자신이면 넘어가기
            if (participant.second == pUser)
                continue;

            RES_CHAT_FOR_SINGLE(participant.second->m_pSession, pUser->m_uID, strSize, szChat);
        }
    }

    return true;
}
bool REQ_ROOM_LEAVE(CSession* pSession)
{
    // 해당 세션과 연결된 유저가 방에서 나와 로비로 들어감
    CUser* pUser = static_cast<CUser*>(pSession->pObj);

    // 방을 검색해서 해당 방에 있는 유저들에게 나간다고 알림
    auto iter = g_roomUMapList.find(pUser->m_enterRoomNo);
    if (iter != g_roomUMapList.end())
    {
        // 유저가 있던 방 정보 획득
        CRoom* pRoom = iter->second;

        // 방에 남아 있는 유저들에게 해당 세션과 연결된 유저가 나갔음을 알림
        for (const auto& participant : pRoom->m_participants)
        {
            RES_ROOM_LEAVE_FOR_SINGLE(participant.second->m_pSession, pUser->m_uID);
        }

        // 해당 세션과 연결된 유저가 방에서 나옴
        pRoom->Withdraw(pUser);

        // 만약 방에 아무도 남아 있지 않다면 방을 파괴하고 브로드캐스트로 알림
        if (pRoom->m_participants.empty())
        {
            // 룸 이름 삭제
            RemoveRoomName(pRoom->m_roomName);

            // 방 파괴 프로토콜을 접속한 유저들에게 알림
            for (const auto& user : g_userList)
            {
                RES_ROOM_DELETE_FOR_SINGLE(user->m_pSession, pRoom->m_roomID);
            }

            // 방 정보 삭제
            g_roomUMapList.erase(pRoom->m_roomID);
        }
    }

    return true;
}

void DisconnectSessionProc(CSession* pSession)
{
    // 세션이 가지고 있는 오브젝트가 존재하지 않는다면, 애초에 연결을 위한 객체가 생성 되지 않은 것이므로 넘김
    if (pSession->pObj == nullptr)
        return;

    CUser* pUser = static_cast<CUser*>(pSession->pObj);

    // 만약 유저가 로비로 나가지 않았다면
    if (pUser->m_enterRoomNo != ROBBY_ROOM_NUMBER)
    {
        // 방 나가는 프로토콜 실행
        REQ_ROOM_LEAVE(pSession);
    }

    for (auto iter = g_userList.begin(); iter != g_userList.end();)
    {
        if ((*iter) == pUser)
        {
            iter = g_userList.erase(iter);
        }
        else
            ++iter;
    }

    // 접속 종료되니 이름 삭제
    RemoveUserName(pUser->m_strName);
}