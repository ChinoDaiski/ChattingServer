#pragma once

#include "Content.h"

class CSession;

void RES_LOGIN_FOR_All(CSession* pSession, UINT8 result, UINT32 userNO);
void RES_LOGIN_FOR_SINGLE(CSession* pSession, UINT8 result, UINT32 userNO);

void RES_ROOM_LIST_FOR_All(CSession* pSession, const std::list<CRoom*>& roomList);
void RES_ROOM_LIST_FOR_SINGLE(CSession* pSession, const std::list<CRoom*>& roomList);

void RES_ROOM_CREATE_FOR_All(CSession* pSession, UINT8 result, const CRoom* room);
void RES_ROOM_CREATE_FOR_SINGLE(CSession* pSession, UINT8 result, const CRoom* room);

void RES_ROOM_ENTER_FOR_All(CSession* pSession, UINT8 result, const CRoom* room);
void RES_ROOM_ENTER_FOR_SINGLE(CSession* pSession, UINT8 result, const CRoom* room);

void RES_CHAT_FOR_All(CSession* pSession, UINT32 senderNo, UINT16 strlen, WCHAR* str);
void RES_CHAT_FOR_SINGLE(CSession* pSession, UINT32 senderNo, UINT16 strlen, WCHAR* str);

void RES_ROOM_LEAVE_FOR_All(CSession* pSession, UINT32 userNO);
void RES_ROOM_LEAVE_FOR_SINGLE(CSession* pSession, UINT32 userNO);

void RES_ROOM_DELETE_FOR_All(CSession* pSession, UINT32 userNO);
void RES_ROOM_DELETE_FOR_SINGLE(CSession* pSession, UINT32 userNO);

void RES_USER_ENTER_FOR_All(CSession* pSession, const WCHAR* str, UINT32 userNO);
void RES_USER_ENTER_FOR_SINGLE(CSession* pSession, const WCHAR* str, UINT32 userNO);

void RES_STRESS_ECHO_FOR_All(CSession* pSession, UINT16 strlen, WCHAR* str);
void RES_STRESS_ECHO_FOR_SINGLE(CSession* pSession, UINT16 strlen, char* str);
void RES_STRESS_ECHO_FOR_SINGLE(CSession* pSession, CPacket* packet);
