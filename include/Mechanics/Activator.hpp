#ifndef MECHANICS_ACTIVATOR_HPP
#define MECHANICS_ACTIVATOR_HPP

#include "BaseMechanism.hpp"
#include <glm/vec2.hpp>

class Activator : public BaseMechanism {
public:
    using BaseMechanism::BaseMechanism;
    
    virtual void Update(const glm::vec2& fireboyPos, const glm::vec2& watergirlPos) = 0;
    virtual bool IsActivated() const = 0;
};

#endif
