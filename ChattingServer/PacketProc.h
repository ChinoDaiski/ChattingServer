#pragma once

#include "Session.h"

class CObject;
class CSession;

bool PacketProc(CSession* pSession, PACKET_TYPE packetType, CPacket* pPacket);

bool REQ_LOGIN(CSession* pSession, WCHAR* szNickName);
bool REQ_ROOM_LIST(CSession* pSession);
bool REQ_ROOM_CREATE(CSession* pSession, UINT16 strSize, WCHAR* roomName);
bool REQ_ROOM_ENTER(CSession* pSession, UINT16 roomNo);
bool REQ_CHAT(CSession* pSession, UINT16 strSize, WCHAR* szChat);
bool REQ_ROOM_LEAVE(CSession* pSession);

