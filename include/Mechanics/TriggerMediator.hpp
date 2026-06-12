#ifndef MECHANICS_TRIGGER_MEDIATOR_HPP
#define MECHANICS_TRIGGER_MEDIATOR_HPP

#include <unordered_map>
#include <vector>
#include <memory>

class Receiver; // Forward declaration

/**
 * @brief Mediator class that coordinates trigger events between Activators and Receivers (Mediator Pattern).
 * Decouples game logic from individual trigger links by mapping groupIds.
 */
class TriggerMediator : public std::enable_shared_from_this<TriggerMediator> {
private:
    std::unordered_map<int, std::vector<std::weak_ptr<Receiver>>> m_Registry;
    std::unordered_map<int, int> m_ActiveActivatorsCount;

public:
    /**
     * @brief Registers a receiver to a specific groupId channel.
     */
    void RegisterReceiver(int groupId, const std::shared_ptr<Receiver>& receiver);

    /**
     * @brief Called by Activators when their activation state changes.
     */
    void OnActivatorStateChanged(int groupId, bool isActivated);

    /**
     * @brief Resets all registered connections.
     */
    void Reset();
};

#endif // MECHANICS_TRIGGER_MEDIATOR_HPP
