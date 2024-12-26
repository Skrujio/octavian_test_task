#include <iostream>
#include <string>
#include <chrono>

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "game.hpp"
#include "StateMachine.hpp"

auto main(int argc, char** argv) -> int {
    const auto sdlInitFlags = SDL_INIT_VIDEO;

    if (!SDL_Init(sdlInitFlags)) {
        printSDLError("SDL_Init");
        return 1;
    }

    if (!TTF_Init()) {
        printSDLError("TTF_Init");
        return 1;
    }

    const auto gameContextOptional = initGameContext();
    if (!gameContextOptional) {
        return 1;
    }

    auto context = *gameContextOptional;

    auto isRunning = true;
    SDL_Event event{};
    
    auto stateMachine = StateMachine{};

    auto currentState = stateMachine.transition(States::Idle);
    currentState->enter(context);

    while (isRunning) {
        while(SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                isRunning = false;
            }

            currentState->handleEvent(context, event);
        }
        
        currentState->update(context);
        currentState->render(context);
        SDL_RenderPresent(context.renderer);

        auto nextState = stateMachine.transition(currentState->transition(context));
        if (nextState != currentState) {
            currentState->exit(context);
            currentState = nextState;
            currentState->enter(context);
        }
    }

    cleanupGameContext(context);
    TTF_Quit();
    SDL_Quit();
}