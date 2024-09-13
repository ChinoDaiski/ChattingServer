#pragma once

class CSession;

// �⺻ CObject Ŭ���� ����
class CObject {
public:
    explicit CObject() noexcept;
    virtual ~CObject() = default;

    virtual void Move();

    // ������Ʈ �޼��� (���� �������� ������Ʈ ���¸� �����ϴ� �� ���)
    virtual void Update(float deltaTime);

public:
    CSession* m_pSession;
};
