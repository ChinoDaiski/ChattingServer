#include "pch.h"
#include "PacketProc.h"
#include "Packet.h"
#include "MakePacket.h"

#include "Content.h"

#include "User.h"

//
//#include "SessionManager.h"
//#include "ObjectManager.h"
//

bool PacketProc(CSession* pSession, PACKET_TYPE packetType, CPacket* pPacket)
{
    CObject* pObj = pSession->pObj;

    switch (packetType)
    {
    case PACKET_TYPE::PT_REQ_LOGIN:
    {
        WCHAR szNickName[dfNICK_MAX_LEN + 1];
        pPacket->GetData((char*)szNickName, dfNICK_MAX_LEN * sizeof(WCHAR));
        szNickName[dfNICK_MAX_LEN] = '\0';

        return REQ_LOGIN(pSession, szNickName);
    }
    break;
    case PACKET_TYPE::PT_REQ_ROOM_LIST:
    {
        return REQ_ROOM_LIST(pSession);
    }
    break;
    case PACKET_TYPE::PT_REQ_ROOM_CREATE:
    {
        UINT16 strSize;
        *pPacket >> strSize;

        WCHAR szNickName[100];
        pPacket->GetData((char*)szNickName, strSize);
        szNickName[(strSize >> 1)] = L'\0';

        return REQ_ROOM_CREATE(pSession, strSize, szNickName);
    }
    break;
    case PACKET_TYPE::PT_REQ_ROOM_ENTER:
    {
        UINT32 roomNo;
        *pPacket >> roomNo;

        return REQ_ROOM_ENTER(pSession, roomNo);
    }
    break;
    case PACKET_TYPE::PT_REQ_CHAT:
    {
        UINT16 strSize;
        *pPacket >> strSize;

        WCHAR szNickName[400];
        pPacket->GetData((char*)szNickName, strSize);
        szNickName[strSize >> 1] = '\0';

        return REQ_CHAT(pSession, strSize, szNickName);
    }
    break;
    case PACKET_TYPE::PT_REQ_ROOM_LEAVE:
    {
        return REQ_ROOM_LEAVE(pSession);
    }
    break;
    default:
        return false;
        break;
    }

    return false;
}
bool REQ_LOGIN(CSession* pSession, WCHAR* szNickName)
{
    CPacket Packet;

    UINT8 result = df_RESULT_LOGIN_ETC;
    CUser* pUser;

    // �г����� ������ �ִ� ������ ��ġ�� �˻�
    std::wstring strNickName{ szNickName };

    // ��ġ�� �г����� ���ٸ�
    if (RegisterUserName(strNickName))
    {
        result = df_RESULT_LOGIN_OK;

        // ���� ����
        pUser = new CUser(strNickName);

        // ������ ���� ������ ������ �ְ�
        // ���ǵ� ���� ������ void* �������� ������ �ִ�.
        pUser->m_pSession = pSession;
        pSession->pObj = pUser;

        // ���ο� ������ ���������� �����ο��� ����ĳ��Ʈ�� �˸�
        RES_LOGIN_FOR_SINGLE(pSession, result, pUser->GetUID());

        // �ش� �������� ��� �濡 ���� ������ ����ĳ��Ʈ�� �˸�
        for (auto& iter : g_roomList)
        {
            result = df_RESULT_ROOM_CREATE_OK;
            RES_ROOM_CREATE_FOR_SINGLE(pSession, result, iter.second);
        }

        // ���� ������ ������ ������ �ٸ� ��� �����鿡�� ���ο� ������ ���������� �˸�
        RES_USER_ENTER_FOR_All(pSession, szNickName, pUser->m_uID);
    }
    else
    {
        result = df_RESULT_LOGIN_DNICK;

        // �ߺ� �г������� ���ӿ� ���������� ����ĳ��Ʈ�� �˸�
        RES_LOGIN_FOR_SINGLE(pSession, result, 0);
    }

    return true;
}
bool REQ_ROOM_LIST(CSession* pSession)
{


    return true;
}
bool REQ_ROOM_CREATE(CSession* pSession, UINT16 strSize, WCHAR* roomName)
{
    // �г����� �޾� ���� ����
    // �г��� �ִ� ũ��� �뷫 255��� ����. �� �̻� �̸��� ������� ������ ����� ���� ���� ������ ũ�� �ø� ��
    WCHAR szName[255];
    std::wstring strRoomName{ roomName };

    UINT8 result;
    CRoom* pRoom = nullptr;

    // �� �̸��� �ߺ��ȴٸ�
    if (!RegisterRoomName(strRoomName))
        result = df_RESULT_ROOM_CREATE_DNICK;

    // �� �̸��� �ߺ����� �ʴ´ٸ�
    else
    {
        result = df_RESULT_ROOM_CREATE_OK;

        // �� ����
        pRoom = new CRoom(strRoomName);
        g_roomList[pRoom->m_roomID] = pRoom;
    }

    // ���ο� ���� �����Ǿ����� ��ε�ĳ��Ʈ�� �˸�
    RES_ROOM_CREATE_FOR_All(pSession, result, pRoom);

    return true;
}
bool REQ_ROOM_ENTER(CSession* pSession, UINT16 roomNo)
{
    // �ش� ���ǰ� ����� ���� ��ü�� Ư�� ������ ����
    CUser* pUser = static_cast<CUser*>(pSession->pObj);
    UINT8 result;
    CRoom* pRoom = nullptr;

    // ���� ������ �̹� �濡 ���ٸ�
    if (pUser->m_enterRoomNo != ROBBY_ROOM_NUMBER)
    {
        result = df_RESULT_ROOM_ENTER_NOT;
    }
    else
    {
        // ���� ã��
        auto iter = g_roomList.find(roomNo);
        if (iter != g_roomList.end())
        {
            // ���� �����Ѵٸ�
            result = df_RESULT_ROOM_ENTER_OK;
            pRoom = iter->second;
        }
    }

    // ����� ���ǿ��� �濡 �����ߴٰ� �޽����� ����
    RES_ROOM_ENTER_FOR_SINGLE(pSession, result, pRoom);

    if (result == df_RESULT_ROOM_ENTER_OK)
    {
        pRoom->Participate(pUser);

        // �̹� �濡 �� �ִ� ���ǵ鿡�� ���ο� ������ �����ٰ� �˸�
        CUser* pExistingUser;
        for (auto& participant : pRoom->m_participants)
        {
            pExistingUser = participant.second;
            WCHAR szName[dfNICK_MAX_LEN + 1];
            memcpy(szName, pUser->m_strName.c_str(), dfNICK_MAX_LEN);
            szName[dfNICK_MAX_LEN] = '\0';

            RES_USER_ENTER_FOR_SINGLE(pExistingUser->m_pSession, szName, pUser->m_uID);
        }
    }

    return true;
}
bool REQ_CHAT(CSession* pSession, UINT16 strSize, WCHAR* szChat)
{
    // �ش� ���ǰ� ����� ������ �����ϴ� �� ������ ������
    CUser* pUser = static_cast<CUser*>(pSession->pObj);
    auto iter = g_roomList.find(pUser->m_enterRoomNo);

    // ���� �����Ѵٸ�
    if (iter != g_roomList.end())
    {
        // �濡 �����ϴ� �������� ���ƴٴϸ� ä�� ���� ����
        for (auto& participant : iter->second->m_participants)
        {
            // ���� �ڱ� �ڽ��̸� �Ѿ��
            if (participant.second == pUser)
                continue;

            RES_CHAT_FOR_SINGLE(participant.second->m_pSession, pUser->m_uID, strSize, szChat);
        }
    }

    return true;
}
bool REQ_ROOM_LEAVE(CSession* pSession)
{
    // �ش� ���ǰ� ����� ������ �濡�� ���� �κ�� ��
    CUser* pUser = static_cast<CUser*>(pSession->pObj);
    
    auto iter = g_roomList.find(pUser->m_enterRoomNo);
    if (iter != g_roomList.end())
    {
        // �濡 ���� �ִ� �����鿡�� �ش� ���ǰ� ����� ������ �������� �˸�
        for (const auto& participant : iter->second->m_participants)
        {
            RES_ROOM_LEAVE_FOR_SINGLE(participant.second->m_pSession, pUser->m_uID);
        }

        // �ش� ���ǰ� ����� ������ �濡�� ����
        iter->second->Withdraw(pUser);

        // ���� �濡 �ƹ��� ���� ���� �ʴٸ� ���� �ı��ϰ� ��ε�ĳ��Ʈ�� �˸�
        if (iter->second->m_participants.empty())
        {
            RES_ROOM_DELETE_FOR_All(nullptr, iter->first);
            g_roomList.erase(iter->first);
        }
    }

    return true;
}
//
//bool netPacketProc_MoveStart(CPlayer* pPlayer, void* pPacket)
//{
//    PACKET_CS_MOVE_START* pMovePacket = static_cast<PACKET_CS_MOVE_START*>(pPacket);
//
//    // �޽��� ���� �α� Ȯ��
//    // ==========================================================================================================
//    // ������ ��ġ�� ���� ��Ŷ�� ��ġ���� �ʹ� ū ���̰� ���ٸ� �������                           .
//    // �� ������ ��ǥ ����ȭ ������ �ܼ��� Ű���� ���� (Ŭ����Ʈ�� ��ó��, ������ �� �ݿ�) �������
//    // Ŭ���̾�Ʈ�� ��ǥ�� �״�� �ϴ� ����� ���ϰ� ����.
//    // ���� �¶��� �����̶�� Ŭ���̾�Ʈ���� �������� �����ϴ� ����� ���ؾ���.
//    // ������ ������ ������ �������� �ϰ� �����Ƿ� �������� ������ Ŭ���̾�Ʈ ��ǥ�� �ϵ��� �Ѵ�.
//    // ==========================================================================================================
//
//    UINT16 posX, posY;
//    pPlayer->getPosition(posX, posY);
//    if (
//        std::abs(posX - pMovePacket->x) > dfERROR_RANGE ||
//        std::abs(posY - pMovePacket->y) > dfERROR_RANGE
//        )
//    {
//        CSessionManager::NotifyClientDisconnected(pPlayer->m_pSession);
//
//        // �α� �����Ÿ� ���⼭ ���� ��
//        int gapX = std::abs(posX - pMovePacket->x);
//        int gapY = std::abs(posY - pMovePacket->y);
//        DebugBreak();
//
//        return false;
//    }
//
//    // ==========================================================================================================
//    // ������ ����. ���� �������� ���۹�ȣ�� ���Ⱚ. ���ο��� �ٶ󺸴� ���⵵ ����
//    // ==========================================================================================================
//    pPlayer->SetDirection(pMovePacket->direction);
//
//
//    // ==========================================================================================================
//    // ����ڸ� ������, ���� �������� ��� ����ڿ��� ��Ŷ�� �Ѹ�.
//    // ==========================================================================================================
//    PACKET_HEADER header;
//    PACKET_SC_MOVE_START packetSCMoveStart;
//    mpMoveStart(&header, &packetSCMoveStart, pPlayer->m_pSession->uid, pPlayer->GetDirection(), posX, posY);
//    CSessionManager::BroadcastPacket(pPlayer->m_pSession, &header, &packetSCMoveStart);
//
//
//    //=====================================================================================================================================
//    // �̵� ���� ������ �˸�
//    //=====================================================================================================================================
//    pPlayer->SetFlag(FLAG_MOVING, true);
//
//    return true;
//}
//
//bool netPacketProc_MoveStop(CPlayer* pPlayer, void* pPacket)
//{
//    // ���� ���õ� Ŭ���̾�Ʈ�� �������� �������� ���� ���̶� ��û
//    // 1. ���� ������ ó��
//    // 2. PACKET_SC_MOVE_STOP �� ��ε�ĳ����
//    // 3. ���� ������ �̵� ���� ������ �˸�
//
//    //=====================================================================================================================================
//    // 1. ���� ������ ó��
//    //=====================================================================================================================================
//    PACKET_CS_MOVE_STOP* packetCSMoveStop = static_cast<PACKET_CS_MOVE_STOP*>(pPacket);
//    pPlayer->SetDirection(packetCSMoveStop->direction);
//    pPlayer->SetPosition(packetCSMoveStop->x, packetCSMoveStop->y);
//
//
//    //=====================================================================================================================================
//    // 2. PACKET_SC_MOVE_STOP �� ��ε�ĳ����
//    //=====================================================================================================================================
//    PACKET_HEADER header;
//    PACKET_SC_MOVE_STOP packetSCMoveStop;
//    mpMoveStop(&header, &packetSCMoveStop, pPlayer->m_pSession->uid, packetCSMoveStop->direction, packetCSMoveStop->x, packetCSMoveStop->y);
//
//    CSessionManager::BroadcastPacket(pPlayer->m_pSession, &header, &packetSCMoveStop);
//
//    //=====================================================================================================================================
//    // 3. ���� ������ �̵� ���� ������ �˸�
//    //=====================================================================================================================================
//    pPlayer->SetFlag(FLAG_MOVING, false);
//
//    return true;
//}
//
//bool netPacketProc_ATTACK1(CPlayer* pPlayer, void* pPacket)
//{
//    // Ŭ���̾�Ʈ�� ���� ���� �޽����� ����.
//    // g_clientList�� ��ȸ�ϸ� ���� 1�� ������ �����ؼ� �������� �־���.
//    // 1. dfPACKET_SC_ATTACK1 �� ��ε�ĳ����
//    // 2. ���ݹ��� ĳ���͸� �˻�. �˻��� �����ϸ� 3, 4�� ���� ����
//    // 3. dfPACKET_SC_DAMAGE �� ��ε�ĳ����
//    // 4. ���� ü���� 0 ���Ϸ� �������ٸ� dfPACKET_SC_DELETE_CHARACTER �� ��ε�ĳ�����ϰ�, �������� ������ �� �ֵ��� �� -> �� �κ��� �������� ó���ϵ��� �ٲ�.
//
//    //=====================================================================================================================================
//    // 1. dfPACKET_SC_ATTACK1 �� ��ε�ĳ����
//    //=====================================================================================================================================
//    PACKET_CS_ATTACK1* packetCSAttack1 = static_cast<PACKET_CS_ATTACK1*>(pPacket);
//    pPlayer->SetPosition(packetCSAttack1->x, packetCSAttack1->y);
//
//    PACKET_HEADER header;
//    PACKET_SC_ATTACK1 packetSCAttack1;
//    mpAttack1(&header, &packetSCAttack1, pPlayer->m_pSession->uid, pPlayer->GetDirection(), packetCSAttack1->x, packetCSAttack1->y);
//    CSessionManager::BroadcastPacket(pPlayer->m_pSession, &header, &packetSCAttack1);
//
//    //=====================================================================================================================================
//    // 2. ���ݹ��� ĳ���͸� �˻�. �˻��� �����ϸ� 3, 4�� ���� ����
//    //=====================================================================================================================================
//
//    // ���� �ٶ󺸴� ���⿡ ���� ���� ������ �޶���.
//    UINT16 left, right, top, bottom;
//    UINT16 posX, posY;
//    pPlayer->getPosition(posX, posY);
//
//    // ������ �ٶ󺸰� �־��ٸ�
//    if (pPlayer->GetFacingDirection() == dfPACKET_MOVE_DIR_LL)
//    {
//        left = posX - dfATTACK1_RANGE_X;
//        right = posX;
//    }
//    // �������� �ٶ󺸰� �־��ٸ�
//    else
//    {
//        left = posX;
//        right = posX + dfATTACK1_RANGE_X;
//    }
//
//    top = posY - dfATTACK1_RANGE_Y;
//    bottom = posY + dfATTACK1_RANGE_Y;
//
//
//    CObjectManager& ObjectManager = CObjectManager::getInstance();
//    auto& ObjectList = ObjectManager.GetObjectList();
//    for (auto& Object : ObjectList)
//    {
//        if (Object == pPlayer)
//            continue;
//
//        Object->getPosition(posX, posY);
//
//        // �ٸ� �÷��̾��� ��ǥ�� ���� ������ ���� ���
//        if (posX >= left && posX <= right &&
//            posY >= top && posY <= bottom)
//        {
//            //=====================================================================================================================================
//            // 3. dfPACKET_SC_DAMAGE �� ��ε�ĳ����
//            //=====================================================================================================================================
//            // 1�� �������� �Ե��� ��
//            dynamic_cast<CPlayer*>(Object)->Damaged(dfATTACK1_DAMAGE);
//
//            PACKET_SC_DAMAGE packetDamage;
//            mpDamage(&header, &packetDamage, pPlayer->m_pSession->uid, Object->m_pSession->uid, dynamic_cast<CPlayer*>(Object)->GetHp());
//
//            CSessionManager::BroadcastPacket(nullptr, &header, &packetDamage);
//
//            /*
//            //=====================================================================================================================================
//            // 4. ���� ü���� 0 ���Ϸ� �������ٸ� dfPACKET_SC_DELETE_CHARACTER �� ��ε�ĳ�����ϰ�, �������� ������ �� �ֵ��� ��
//            //=====================================================================================================================================
//            if (packetDamage.damagedHP <= 0)
//            {
//                PACKET_SC_DELETE_CHARACTER packetDeleteCharacter;
//                mpDeleteCharacter(&header, &packetDeleteCharacter, client->uid);
//
//                BroadcastPacket(nullptr, &header, &packetDeleteCharacter);
//
//                // �������� ������ �� �ֵ��� isAlive�� false�� ����
//                client->isAlive = false;
//            }
//            */
//
//            // ���⼭ break�� ���ָ� ���� ������
//            break;
//        }
//    }
//
//    return true;
//}
//
//bool netPacketProc_ATTACK2(CPlayer* pPlayer, void* pPacket)
//{
//    // Ŭ���̾�Ʈ�� ���� ���� �޽����� ����.
//      // g_clientList�� ��ȸ�ϸ� ���� 2�� ������ �����ؼ� �������� �־���.
//      // 1. dfPACKET_SC_ATTACK2 �� ��ε�ĳ����
//      // 2. ���ݹ��� ĳ���͸� �˻�. �˻��� �����ϸ� 3, 4�� ���� ����
//      // 3. dfPACKET_SC_DAMAGE �� ��ε�ĳ����
//      // 4. ���� ü���� 0 ���Ϸ� �������ٸ� dfPACKET_SC_DELETE_CHARACTER �� ��ε�ĳ�����ϰ�, �������� ������ �� �ֵ��� �� -> �� �κ��� �������� ó���ϵ��� �ٲ�.
//
//      //=====================================================================================================================================
//      // 1. dfPACKET_SC_ATTACK2 �� ��ε�ĳ����
//      //=====================================================================================================================================
//    PACKET_CS_ATTACK2* packetCSAttack2 = static_cast<PACKET_CS_ATTACK2*>(pPacket);
//    pPlayer->SetPosition(packetCSAttack2->x, packetCSAttack2->y);
//
//    PACKET_HEADER header;
//    PACKET_SC_ATTACK2 packetSCAttack2;
//    mpAttack2(&header, &packetSCAttack2, pPlayer->m_pSession->uid, pPlayer->GetDirection(), packetCSAttack2->x, packetCSAttack2->y);
//    CSessionManager::BroadcastPacket(pPlayer->m_pSession, &header, &packetSCAttack2);
//
//    //=====================================================================================================================================
//    // 2. ���ݹ��� ĳ���͸� �˻�. �˻��� �����ϸ� 3, 4�� ���� ����
//    //=====================================================================================================================================
//
//    // ���� �ٶ󺸴� ���⿡ ���� ���� ������ �޶���.
//    UINT16 left, right, top, bottom;
//    UINT16 posX, posY;
//    pPlayer->getPosition(posX, posY);
//
//    // ������ �ٶ󺸰� �־��ٸ�
//    if (pPlayer->GetFacingDirection() == dfPACKET_MOVE_DIR_LL)
//    {
//        left = posX - dfATTACK2_RANGE_X;
//        right = posX;
//    }
//    // �������� �ٶ󺸰� �־��ٸ�
//    else
//    {
//        left = posX;
//        right = posX + dfATTACK2_RANGE_X;
//    }
//
//    top = posY - dfATTACK2_RANGE_Y;
//    bottom = posY + dfATTACK2_RANGE_Y;
//
//
//    CObjectManager& ObjectManager = CObjectManager::getInstance();
//    auto& ObjectList = ObjectManager.GetObjectList();
//    for (auto& Object : ObjectList)
//    {
//        if (Object == pPlayer)
//            continue;
//
//        Object->getPosition(posX, posY);
//
//        // �ٸ� �÷��̾��� ��ǥ�� ���� ������ ���� ���
//        if (posX >= left && posX <= right &&
//            posY >= top && posY <= bottom)
//        {
//            //=====================================================================================================================================
//            // 3. dfPACKET_SC_DAMAGE �� ��ε�ĳ����
//            //=====================================================================================================================================
//            // 1�� �������� �Ե��� ��
//            dynamic_cast<CPlayer*>(Object)->Damaged(dfATTACK2_DAMAGE);
//
//            PACKET_SC_DAMAGE packetDamage;
//            mpDamage(&header, &packetDamage, pPlayer->m_pSession->uid, Object->m_pSession->uid, dynamic_cast<CPlayer*>(Object)->GetHp());
//
//            CSessionManager::BroadcastPacket(nullptr, &header, &packetDamage);
//
//            /*
//            //=====================================================================================================================================
//            // 4. ���� ü���� 0 ���Ϸ� �������ٸ� dfPACKET_SC_DELETE_CHARACTER �� ��ε�ĳ�����ϰ�, �������� ������ �� �ֵ��� ��
//            //=====================================================================================================================================
//            if (packetDamage.damagedHP <= 0)
//            {
//                PACKET_SC_DELETE_CHARACTER packetDeleteCharacter;
//                mpDeleteCharacter(&header, &packetDeleteCharacter, client->uid);
//
//                BroadcastPacket(nullptr, &header, &packetDeleteCharacter);
//
//                // �������� ������ �� �ֵ��� isAlive�� false�� ����
//                client->isAlive = false;
//            }
//            */
//
//            // ���⼭ break�� ���ָ� ���� ������
//            break;
//        }
//    }
//
//    return true;
//}
//
//bool netPacketProc_ATTACK3(CPlayer* pPlayer, void* pPacket)
//{
//    // Ŭ���̾�Ʈ�� ���� ���� �޽����� ����.
//     // g_clientList�� ��ȸ�ϸ� ���� 3�� ������ �����ؼ� �������� �־���.
//     // 1. dfPACKET_SC_ATTACK3 �� ��ε�ĳ����
//     // 2. ���ݹ��� ĳ���͸� �˻�. �˻��� �����ϸ� 3, 4�� ���� ����
//     // 3. dfPACKET_SC_DAMAGE �� ��ε�ĳ����
//     // 4. ���� ü���� 0 ���Ϸ� �������ٸ� dfPACKET_SC_DELETE_CHARACTER �� ��ε�ĳ�����ϰ�, �������� ������ �� �ֵ��� �� -> �� �κ��� �������� ó���ϵ��� �ٲ�.
//
//     //=====================================================================================================================================
//     // 1. dfPACKET_SC_ATTACK3 �� ��ε�ĳ����
//     //=====================================================================================================================================
//    PACKET_CS_ATTACK3* packetCSAttack3 = static_cast<PACKET_CS_ATTACK3*>(pPacket);
//    pPlayer->SetPosition(packetCSAttack3->x, packetCSAttack3->y);
//
//    PACKET_HEADER header;
//    PACKET_SC_ATTACK3 packetSCAttack3;
//    mpAttack3(&header, &packetSCAttack3, pPlayer->m_pSession->uid, pPlayer->GetDirection(), packetCSAttack3->x, packetCSAttack3->y);
//    CSessionManager::BroadcastPacket(pPlayer->m_pSession, &header, &packetSCAttack3);
//
//    //=====================================================================================================================================
//    // 2. ���ݹ��� ĳ���͸� �˻�. �˻��� �����ϸ� 3, 4�� ���� ����
//    //=====================================================================================================================================
//
//    // ���� �ٶ󺸴� ���⿡ ���� ���� ������ �޶���.
//    UINT16 left, right, top, bottom;
//    UINT16 posX, posY;
//    pPlayer->getPosition(posX, posY);
//
//    // ������ �ٶ󺸰� �־��ٸ�
//    if (pPlayer->GetFacingDirection() == dfPACKET_MOVE_DIR_LL)
//    {
//        left = posX - dfATTACK3_RANGE_X;
//        right = posX;
//    }
//    // �������� �ٶ󺸰� �־��ٸ�
//    else
//    {
//        left = posX;
//        right = posX + dfATTACK3_RANGE_X;
//    }
//
//    top = posY - dfATTACK3_RANGE_Y;
//    bottom = posY + dfATTACK3_RANGE_Y;
//
//
//    CObjectManager& ObjectManager = CObjectManager::getInstance();
//    auto& ObjectList = ObjectManager.GetObjectList();
//    for (auto& Object : ObjectList)
//    {
//        if (Object == pPlayer)
//            continue;
//
//        Object->getPosition(posX, posY);
//
//        // �ٸ� �÷��̾��� ��ǥ�� ���� ������ ���� ���
//        if (posX >= left && posX <= right &&
//            posY >= top && posY <= bottom)
//        {
//            //=====================================================================================================================================
//            // 3. dfPACKET_SC_DAMAGE �� ��ε�ĳ����
//            //=====================================================================================================================================
//            // 1�� �������� �Ե��� ��
//            dynamic_cast<CPlayer*>(Object)->Damaged(dfATTACK3_DAMAGE);
//
//            PACKET_SC_DAMAGE packetDamage;
//            mpDamage(&header, &packetDamage, pPlayer->m_pSession->uid, Object->m_pSession->uid, dynamic_cast<CPlayer*>(Object)->GetHp());
//
//            CSessionManager::BroadcastPacket(nullptr, &header, &packetDamage);
//
//            /*
//            //=====================================================================================================================================
//            // 4. ���� ü���� 0 ���Ϸ� �������ٸ� dfPACKET_SC_DELETE_CHARACTER �� ��ε�ĳ�����ϰ�, �������� ������ �� �ֵ��� ��
//            //=====================================================================================================================================
//            if (packetDamage.damagedHP <= 0)
//            {
//                PACKET_SC_DELETE_CHARACTER packetDeleteCharacter;
//                mpDeleteCharacter(&header, &packetDeleteCharacter, client->uid);
//
//                BroadcastPacket(nullptr, &header, &packetDeleteCharacter);
//
//                // �������� ������ �� �ֵ��� isAlive�� false�� ����
//                client->isAlive = false;
//            }
//            */
//
//            // ���⼭ break�� ���ָ� ���� ������
//            break;
//        }
//    }
//
//    return true;
//}
