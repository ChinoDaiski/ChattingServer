#include "pch.h"
#include "Content.h"
#include "User.h"

std::unordered_set<std::wstring> g_userNameList;
std::unordered_set<std::wstring> g_roomNameList;
std::unordered_map<UINT32, CRoom*> g_roomUMapList;
std::list<CUser*> g_userList;

bool RegisterUserName(const std::wstring& userName)
{
    if (g_userNameList.find(userName) == g_userNameList.end())
    {
        g_userNameList.insert(userName);
        return true;
    }

    return false;
}

bool RemoveUserName(const std::wstring& userName)
{
    // 삭제에 성공하면 1 반환. 왜? 중복이 없으니깐.
    if (g_userNameList.erase(userName) != 0)
    {
        return true;
    }

    return false;
}

bool RegisterRoomName(const std::wstring& roomName)
{
    if (g_roomNameList.find(roomName) == g_roomNameList.end())
    {
        g_roomNameList.insert(roomName);
        return true;
    }

    return false;
}

bool RemoveRoomName(const std::wstring& roomName)
{
    // 삭제에 성공하면 1 반환. 왜? 중복이 없으니깐.
    if (g_roomNameList.erase(roomName) != 0)
    {
        return true;
    }

    return false;
}



UINT32 CRoom::g_roomID = 0;

CRoom::CRoom(const std::wstring& roomName)
    : m_roomName(roomName)
{
    m_roomID = g_roomID;
    g_roomID++;
}

CRoom::~CRoom()
{
}

void CRoom::Participate(CUser* pUser)
{
    m_participants[pUser->GetUID()] = pUser;
    pUser->m_enterRoomNo = m_roomID;
}

void CRoom::Withdraw(CUser* pUser)
{
    m_participants.erase(pUser->m_uID);
    pUser->m_enterRoomNo = ROBBY_ROOM_NUMBER;
}
