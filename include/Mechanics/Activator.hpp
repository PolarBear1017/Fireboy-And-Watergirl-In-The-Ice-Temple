#ifndef MECHANICS_ACTIVATOR_HPP
#define MECHANICS_ACTIVATOR_HPP

#include "BaseMechanism.hpp"
#include <glm/vec2.hpp>
#include <vector>
#include <memory>

class TriggerMediator; // Forward declaration

class Activator : public BaseMechanism {
protected:
    std::shared_ptr<TriggerMediator> m_Mediator;

public:
    Activator(int groupId, std::shared_ptr<TriggerMediator> mediator)
        : BaseMechanism(groupId), m_Mediator(std::move(mediator)) {}
    virtual ~Activator() = default;
    
    virtual void Update(const std::vector<glm::vec2>& interactorPositions) = 0;
    virtual bool IsActivated() const = 0;
};

#endif
