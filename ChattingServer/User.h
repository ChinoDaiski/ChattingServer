#pragma once

#include "Object.h"

#define ROBBY_ROOM_NUMBER UINT32_MAX

class CUser : public CObject
{
public:
	explicit CUser(const std::wstring NickName);
	virtual ~CUser(void);

public:
	UINT32 GetUID(void) const { return m_uID; }
	const std::wstring GetName(void) const { return m_strName; }

public:
	std::wstring m_strName;
	UINT32 m_uID;
	UINT32 m_enterRoomNo;

private:
	static UINT32 g_ID;
};
