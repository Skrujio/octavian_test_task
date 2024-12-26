#include "StateMachine.hpp"

#include <iostream>
#include <memory>

#include "game.hpp"

auto Idle::enter(GameContext& context) -> void {
    context.startButton.color = GameColorPallete::startButton;
    context.stopButton.color = GameColorPallete::stopButton;
}

auto Idle::exit(GameContext& context) -> void {
    isStartButtonPushed = false;
}

auto Idle::handleEvent(GameContext& context, SDL_Event& event) -> void {
    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        isStartButtonPushed = collision(event.button.x, event.button.y, context.startButton.layout);
    }
}

auto Idle::update(GameContext& context) -> void {}

auto Idle::render(GameContext& context) -> void {
    renderBackground(context.renderer, GameColorPallete::background);
    renderButton(context.renderer, context.startButton);
    renderButton(context.renderer, context.stopButton);
    renderDrums(context.renderer, context.drumTexture, context.drums);
}

auto Idle::transition(GameContext& context) -> States {
    if (isStartButtonPushed) {
        return States::Rolling;
    }

    return States::Idle;
}

auto Rolling::enter(GameContext& context) -> void {
    startTime = std::chrono::system_clock::now();
    context.startButton.color = GameColorPallete::busyButton;
}

auto Rolling::exit(GameContext& context) -> void {
    isStopButtonPushed = false;
    context.stopButton.color = GameColorPallete::busyButton;
}

auto Rolling::handleEvent(GameContext& context, SDL_Event& event) -> void {
    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        isStopButtonPushed = collision(event.button.x, event.button.y, context.stopButton.layout);
    }
}

auto Rolling::update(GameContext& context) -> void {
    isTimerUp = (std::chrono::system_clock::now() - startTime) >= timerDuration;

    for (auto& drum : context.drums) {
        drum.scrollingOffset = wraparound(drum.scrollingOffset + drum.velocity, 0, context.drumTexture->h);
    }
}

auto Rolling::render(GameContext& context) -> void {
    renderButton(context.renderer, context.startButton);
    renderDrums(context.renderer, context.drumTexture, context.drums);
}

auto Rolling::transition(GameContext& context) -> States {
    if (isStopButtonPushed || isTimerUp) {
        return States::EndRolling;
    }

    return States::Rolling;
}

auto EndRolling::enter(GameContext& context) -> void {}

auto EndRolling::exit(GameContext& context) -> void {
    isDrumsStoped = false;
}

auto EndRolling::handleEvent(GameContext& context, SDL_Event& event) -> void {}

auto EndRolling::update(GameContext& context) -> void {
    isDrumsStoped = true;
    for (auto& drum : context.drums) {
        if (drum.scrollingOffset % context.drumTexture->w != 0) {
            drum.scrollingOffset = wraparound(drum.scrollingOffset + drum.velocity, 0, context.drumTexture->h);
        }

        isDrumsStoped &= drum.scrollingOffset % context.drumTexture->w == 0;
    }
}

auto EndRolling::render(GameContext& context) -> void {
    renderButton(context.renderer, context.stopButton);
    renderDrums(context.renderer, context.drumTexture, context.drums);
}

auto EndRolling::transition(GameContext& context) -> States {
    if (isDrumsStoped) {
        return States::RewardPresentation;
    }

    return States::EndRolling;
}

auto RewardPresentation::enter(GameContext& context) -> void {
    context.stopButton.color = GameColorPallete::busyButton;
}

auto RewardPresentation::exit(GameContext& context) -> void {
    shouldBeginNextCycle = false;
}

auto RewardPresentation::handleEvent(GameContext& context, SDL_Event& event) -> void {
    shouldBeginNextCycle = event.type == SDL_EVENT_MOUSE_BUTTON_DOWN;
}

auto RewardPresentation::update(GameContext& context) -> void {}

auto RewardPresentation::transition(GameContext& context) -> States {
    if (shouldBeginNextCycle) {
        return States::Idle;
    }

    return States::RewardPresentation;
}

auto RewardPresentation::render(GameContext& context) -> void {
    // renderColoredRect(context.renderer, context.rewardDisplay);
    auto xOffset = 24;

    for (const auto& drum : context.drums) {
        auto result = std::to_string(drum.scrollingOffset / context.drumTexture->w + 1);
        auto text = loadTextureFromRenderedText(context.renderer, context.font, result.c_str());
        auto textRect = SDL_FRect{static_cast<float>(xOffset), 64, static_cast<float>(text->w), static_cast<float>(text->h)};
        xOffset += context.drumTexture->w;
        SDL_RenderTexture(context.renderer, text, nullptr, &textRect);
        SDL_DestroyTexture(text);
    }
    
}