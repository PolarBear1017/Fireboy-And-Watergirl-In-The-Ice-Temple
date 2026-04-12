#ifndef MECHANICS_RECEIVER_HPP
#define MECHANICS_RECEIVER_HPP

#include "BaseMechanism.hpp"

class Receiver : public BaseMechanism {
public:
    using BaseMechanism::BaseMechanism;

    virtual void Update() = 0;
    virtual void SetActivated(bool active) = 0;
};

#endif
