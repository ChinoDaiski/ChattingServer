#pragma once

class CSession;

// 기본 CObject 클래스 정의
class CObject {
public:
    explicit CObject() noexcept;
    virtual ~CObject() = default;

    virtual void Move();

    // 업데이트 메서드 (서버 로직에서 오브젝트 상태를 갱신하는 데 사용)
    virtual void Update(float deltaTime);

public:
    CSession* m_pSession;
};
