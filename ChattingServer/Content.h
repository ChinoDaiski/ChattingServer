#pragma once

class CUser;
class CRoom;

extern std::unordered_map<UINT32, CRoom*> g_roomUMapList;
extern std::list<CUser*> g_userList;

bool RegisterUserName(const std::wstring& userName);
bool RemoveUserName(const std::wstring& userName);

bool RegisterRoomName(const std::wstring& roomName);
bool RemoveRoomName(const std::wstring& roomName);

class CRoom
{
public:
	explicit CRoom(const std::wstring& roomName);
	~CRoom();

public:
	void Participate(CUser* pUser);
	void Withdraw(CUser* pUser);

public:
	UINT32 m_roomID;			// �� ID
	std::wstring m_roomName;	// �� �̸�
	std::unordered_map<UINT32, CUser*> m_participants;	// �����ο�

private:
	static UINT32 g_roomID;
};