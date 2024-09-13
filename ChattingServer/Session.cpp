
#include "pch.h"
#include "Session.h"
#include "Packet.h"
#include "WinSockManager.h"
#include "Object.h"

UINT16 CSession::g_uID = 0;

CSession::CSession()
{
    // ID ºÎ¿©
    uid = g_uID;
    g_uID++;
}

CSession::~CSession()
{
}

void CSession::RegisterObject(CObject** ppObj)
{
    pObj = *ppObj;
}
