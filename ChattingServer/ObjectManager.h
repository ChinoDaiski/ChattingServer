#pragma once

#include "Singleton.h"
#include "Object.h"

class CSession;
class CPlayer;

class CObjectManager : public SingletonBase<CObjectManager> {
private:
    friend class SingletonBase<CObjectManager>;

public:
    explicit CObjectManager() noexcept;
    ~CObjectManager() noexcept;
    
    // ���� �����ڿ� ���� �����ڸ� �����Ͽ� ���� ����
    CObjectManager(const CObjectManager&) = delete;
    CObjectManager& operator=(const CObjectManager&) = delete;

public:
    void Update(float deltatime);
    void LateUpdate(float deltatime);

public:
    void RegisterObject(CObject* pObject, CSession* pCSession);

public:
    std::list<CObject*>& GetObjectList(void) { return m_ObjectList; }

private:
    std::list<CObject*> m_ObjectList;
};