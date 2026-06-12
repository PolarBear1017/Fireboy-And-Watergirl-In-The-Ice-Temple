#include "Mechanics/TriggerMediator.hpp"
#include "Mechanics/Receiver.hpp"
#include <algorithm>

void TriggerMediator::RegisterReceiver(int groupId, const std::shared_ptr<Receiver>& receiver) {
    m_Registry[groupId].push_back(receiver);
}

void TriggerMediator::OnActivatorStateChanged(int groupId, bool isActivated) {
    if (isActivated) {
        m_ActiveActivatorsCount[groupId]++;
    } else {
        m_ActiveActivatorsCount[groupId] = std::max(0, m_ActiveActivatorsCount[groupId] - 1);
    }

    bool shouldActivate = m_ActiveActivatorsCount[groupId] > 0;
    
    auto& list = m_Registry[groupId];
    // Clean up expired weak_ptrs and notify active ones
    for (auto it = list.begin(); it != list.end(); ) {
        if (auto receiver = it->lock()) {
            receiver->SetActivated(shouldActivate);
            ++it;
        } else {
            it = list.erase(it);
        }
    }
}

void TriggerMediator::Reset() {
    m_Registry.clear();
    m_ActiveActivatorsCount.clear();
}
