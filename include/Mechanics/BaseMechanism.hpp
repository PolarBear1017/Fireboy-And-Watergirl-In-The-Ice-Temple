#ifndef MECHANICS_BASE_MECHANISM_HPP
#define MECHANICS_BASE_MECHANISM_HPP

#include "Util/GameObject.hpp"
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <optional>
#include <string>

struct Collider {
    glm::vec2 center;
    glm::vec2 size;
};

inline glm::vec4 GetGroupColor(int groupId) {
    if (groupId <= 0) return {1.0f, 1.0f, 1.0f, 1.0f};
    std::string s = std::to_string(groupId);
    if (s.empty()) return {1.0f, 1.0f, 1.0f, 1.0f};
    char firstChar = s[0];
    switch (firstChar) {
        case '1': return {1.0f, 0.0f, 0.0f, 1.0f};       // Red (#ff0000)
        case '2': return {0.0f, 0.8f, 0.0f, 1.0f};       // Green (#00cc00)
        case '3': return {13.0f/255.0f, 62.0f/255.0f, 1.0f, 1.0f}; // Blue (#0D3EFF)
        case '4': return {1.0f, 1.0f, 0.0f, 1.0f};       // Yellow (#ffff00)
        case '5': return {236.0f/255.0f, 14.0f/255.0f, 241.0f/255.0f, 1.0f}; // Purple (#EC0EF1)
        case '6': return {87.0f/255.0f, 190.0f/255.0f, 1.0f, 1.0f}; // Cyan (#57beff)
        case '7': return {153.0f/255.0f, 51.0f/255.0f, 1.0f, 1.0f}; // Violet (#9933FF)
        default: return {1.0f, 1.0f, 1.0f, 1.0f};       // White/Fallback
    }
}

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
