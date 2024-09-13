#pragma once


//#define MAX_PACKET_SIZE 10
//
//enum class PACKET_TYPE : BYTE
//{
//   SC_CREATE_MY_CHARACTER = 0,
//   SC_CREATE_OTHER_CHARACTER,
//   SC_DELETE_CHARACTER,
//
//   CS_MOVE_START = 10,
//   SC_MOVE_START,
//   CS_MOVE_STOP,
//   SC_MOVE_STOP,
//
//   CS_ATTACK1 = 20,
//   SC_ATTACK1,
//   CS_ATTACK2,
//   SC_ATTACK2,
//   CS_ATTACK3,
//   SC_ATTACK3,
//
//   SC_DAMAGE = 30,
//
//   CS_SYNC = 250,
//   SC_SYNC,
//
//   END
//};


#pragma pack(push, 1) // 1바이트 정렬

//===================================================
// 패킷 헤더
//===================================================
// 
//===================================================
// Server To Client
//===================================================



//===================================================
// Client To Server
//===================================================

#pragma pack(pop) // 저장된 정렬 상태로 복원