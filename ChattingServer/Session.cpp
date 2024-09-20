
#include "pch.h"
#include "Session.h"
#include "Packet.h"
#include "WinSockManager.h"
#include "Object.h"

CSession::CSession()
{
}

CSession::~CSession()
{
}

void CSession::RegisterObject(CObject** ppObj)
{
    pObj = *ppObj;
}
