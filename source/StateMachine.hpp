#ifndef STATE_MACHINE_HPP
#define STATE_MACHINE_HPP

#include <chrono>
#include <memory>

#include "game.hpp"

enum class States {
    Idle,
    Rolling,
    EndRolling,
    RewardPresentation
};

struct BaseGameState {
    virtual auto enter(GameContext& context) -> void = 0;
    virtual auto exit(GameContext& context) -> void = 0;
    virtual auto handleEvent(GameContext& context, SDL_Event& event) -> void = 0;
    virtual auto update(GameContext& context) -> void = 0;
    virtual auto render(GameContext& context) -> void = 0;
    virtual auto transition(GameContext& context) -> States = 0;
    virtual ~BaseGameState() = default;
};

struct Idle : BaseGameState {
    auto enter(GameContext& context) -> void override;
    auto exit(GameContext& context) -> void override;
    auto handleEvent(GameContext& context, SDL_Event& event) ->void override;
    auto update(GameContext& context) -> void override;
    auto render(GameContext& context) -> void override;
    auto transition(GameContext& context) -> States override;

    bool isStartButtonPushed = false;
};

struct Rolling : BaseGameState {
    auto enter(GameContext& context) -> void override;
    auto exit(GameContext& context) -> void override;
    auto handleEvent(GameContext& context, SDL_Event& event) ->void override;
    auto update(GameContext& context) -> void override;
    auto render(GameContext& context) -> void override;
    auto transition(GameContext& context) -> States override;

    bool isStopButtonPushed = false;
    bool isTimerUp = false;
    std::chrono::time_point<std::chrono::system_clock> startTime;
    std::chrono::seconds timerDuration = std::chrono::seconds(3);
};

struct EndRolling : BaseGameState {
    auto enter(GameContext& context) -> void override;
    auto exit(GameContext& context) -> void override;
    auto handleEvent(GameContext& context, SDL_Event& event) ->void override;
    auto update(GameContext& context) -> void override;
    auto render(GameContext& context) -> void override;
    auto transition(GameContext& context) -> States override;

    bool isDrumsStoped = false;
};

struct RewardPresentation : BaseGameState {
    auto enter(GameContext& context) -> void override;
    auto exit(GameContext& context) -> void override;
    auto handleEvent(GameContext& context, SDL_Event& event) ->void override;
    auto update(GameContext& context) -> void override;
    auto render(GameContext& context) -> void override;
    auto transition(GameContext& context) -> States override;

    int reward = 0;
    bool shouldBeginNextCycle = false;
};

struct StateMachine {
    Idle idleState;
    Rolling rollingState;
    EndRolling endRollingState;
    RewardPresentation rewardPresentationState;

    auto transition(States nextState) -> BaseGameState* {
        switch(nextState) {
            case States::Idle:
            return &idleState;
            break;

            case States::Rolling:
            return &rollingState;
            break;
            
            case States::EndRolling:
            return &endRollingState;
            break;
            
            case States::RewardPresentation:
            return &rewardPresentationState;
            break;
            
            default:
            return nullptr;
            break;
        }
    }
};

#endif