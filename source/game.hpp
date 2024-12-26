#ifndef GAME_HPP
#define GAME_HPP

#include <array>
#include <optional>
#include <string>

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <gsl/gsl-lite.hpp>

namespace BaseColorPallete {
    const auto black = SDL_FColor{0, 0, 0, 1};
    const auto grey =  SDL_FColor{0.5, 0.5, 0.5, 1};
    const auto white = SDL_FColor{1, 1, 1, 1};
    const auto red =   SDL_FColor{1, 0, 0, 1};
    const auto green = SDL_FColor{0, 1, 0, 1};
    const auto blue =  SDL_FColor{0, 0, 1, 1};
};

namespace GameColorPallete {
    const auto background =     BaseColorPallete::white;
    const auto busyButton =     BaseColorPallete::grey;
    const auto startButton =    BaseColorPallete::green;
    const auto stopButton =     BaseColorPallete::red;
    const auto rewardDisplay =  BaseColorPallete::blue;
}

struct Drum {
    SDL_FRect layout{};
    int scrollingOffset{};
    float velocity{};
};

struct ColoredRect {
    SDL_FRect layout{};
    SDL_FColor color{};
};

struct Button {
    SDL_FRect layout{};
    SDL_FColor color{};
    SDL_Texture* text{};
};

struct GameContext {
    SDL_Window* window{};
    SDL_Renderer* renderer{};
    SDL_Texture* drumTexture{};
    TTF_Font* font{};
    
    Button startButton{};
    Button stopButton{};

    std::array<Drum, 4> drums{};
};

auto wraparound(int p, int min, int max) -> int;
auto collision(float x, float y, SDL_FRect rect) -> bool;
auto printSDLError(std::string name) -> void;
auto loadTexture(SDL_Renderer* renderer, const std::string& filePath) -> SDL_Texture*;
auto loadTextureFromRenderedText(SDL_Renderer* renderer, TTF_Font* font, std::string text) -> SDL_Texture*;
auto renderButton(SDL_Renderer* renderer, const Button& button) -> void;
auto renderTextureScrolledVertically(SDL_Renderer* renderer, SDL_Texture* texture, SDL_FRect clipRect, SDL_FRect destRect) -> void;
auto renderDrums(SDL_Renderer* renderer, SDL_Texture* texture, gsl::span<const Drum> drums) -> void;
auto renderBackground(SDL_Renderer* renderer, SDL_FColor color) -> void;

auto initGameContext() -> std::optional<GameContext>;
auto cleanupGameContext(GameContext& context) -> void;


#endif