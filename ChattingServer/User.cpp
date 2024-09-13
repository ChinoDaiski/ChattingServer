#include "pch.h"
#include "User.h"
#include "Object.h"

UINT32 CUser::g_ID = 0;

CUser::CUser(const std::wstring NickName)
	: m_strName{ NickName }, m_enterRoomNo{ ROBBY_ROOM_NUMBER }
{
	m_uID = g_ID;
	++g_ID;
}

CUser::~CUser(void)
{
}
