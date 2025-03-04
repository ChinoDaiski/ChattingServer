#pragma once

#define NOMINMAX
#include <iostream>
#include <stdexcept>

#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#include <Windows.h>
#pragma comment(lib, "Winmm.lib")


//===================================================
// 자료구조
//===================================================
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <unordered_set>
//===================================================
#include <string>
#include <codecvt>
#include <locale>

//===================================================
// 링버퍼 사용
//===================================================
#include "RingBuffer.h"
//===================================================

//===================================================
// 프로토콜 정보
//===================================================
#include "Protocol.h"
//===================================================

//===================================================
// 세션 정보
//===================================================
#include "Session.h"
//===================================================
