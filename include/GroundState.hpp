#ifndef GROUND_STATE_HPP
#define GROUND_STATE_HPP


enum class GroundState {
    AIR,
    GROUND,
    ICE,
    SLOPE,
    MOVING_PLATFORM,
    WIND,
    DEAD
};

enum class RunningState {
    Left,
    Right,
    Idle
};


#endif //GROUND_STATE_HPP