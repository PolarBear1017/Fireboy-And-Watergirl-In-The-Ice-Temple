#ifndef MECHANICS_BLOCK_HPP
#define MECHANICS_BLOCK_HPP

#include "BaseMechanism.hpp"
#include "SpriteAtlas.hpp"
#include <memory>

class Block : public BaseMechanism {
public:
    Block(std::shared_ptr<SpriteAtlas> atlas, const glm::vec2& pos);
    
    std::optional<Collider> GetCollider() const override;
    bool IsBlock() const override { return true; }

    void SetVelocity(const glm::vec2& vel) override { m_Velocity = vel; }
    glm::vec2 GetVelocity() const override { return m_Velocity; }
    
    void SetPosition(const glm::vec2& pos) override { m_Transform.translation = pos; }
    glm::vec2 GetPosition() const override { return m_Transform.translation; }
    
    glm::vec2 GetSize() const { return m_Size; }

    void Update();

private:
    glm::vec2 m_Velocity = {0.0f, 0.0f};
    glm::vec2 m_Size;
};

#endif
