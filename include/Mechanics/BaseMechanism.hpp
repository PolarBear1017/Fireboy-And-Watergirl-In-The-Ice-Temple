#ifndef MECHANICS_BASE_MECHANISM_HPP
#define MECHANICS_BASE_MECHANISM_HPP

#include "Util/GameObject.hpp"
#include <glm/vec2.hpp>
#include <optional>

struct Collider {
    glm::vec2 center;
    glm::vec2 size;
};

class BaseMechanism : public Util::GameObject {
public:
    explicit BaseMechanism(int groupId) : m_GroupId(groupId) {}
    virtual ~BaseMechanism() = default;

    int GetGroupId() const { return m_GroupId; }
    virtual std::optional<Collider> GetCollider() const { return std::nullopt; }

protected:
    int m_GroupId;
};

#endif
