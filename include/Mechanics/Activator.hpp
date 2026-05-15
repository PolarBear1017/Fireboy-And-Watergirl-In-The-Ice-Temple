#ifndef MECHANICS_ACTIVATOR_HPP
#define MECHANICS_ACTIVATOR_HPP

#include "BaseMechanism.hpp"
#include <glm/vec2.hpp>

#include <vector>

class Activator : public BaseMechanism {
public:
    using BaseMechanism::BaseMechanism;
    
    virtual void Update(const std::vector<glm::vec2>& interactorPositions) = 0;
    virtual bool IsActivated() const = 0;
};

#endif
