#include "game.hpp"

#include <iostream>
#include <array>
#include <optional>
#include <string>

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <gsl/gsl-lite.hpp>

auto printSDLError(std::string name) -> void {
    std::cerr << name << ": " << SDL_GetError() << '\n';
}

auto wraparound(int p, int min, int max) -> int {
    if (p >= max) {
        p = min + p % (max - min);
    }

    if (p < min) {
        p = max + p % (max - min);
    }

    return p;
}

auto loadTexture(SDL_Renderer* renderer, const std::string& filePath) -> SDL_Texture* {
    const auto surface = SDL_LoadBMP(filePath.c_str());
    if (!surface) {
        printSDLError("SDL_LoadBMP");
        return nullptr;
    }
    
    const auto texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        printSDLError("SDL_CreateTextureFromSurface");
        return nullptr;
    }

    SDL_DestroySurface(surface);

    return texture;
}

auto loadTextureFromRenderedText(SDL_Renderer* renderer, TTF_Font* font, std::string text) -> SDL_Texture* {
    auto textColor = SDL_Color{0, 0, 0, 255};
    SDL_Surface* textSurface = TTF_RenderText_Blended(font, text.c_str(), 0, textColor);
    if (!textSurface) {
        printSDLError("TTF_RenderText_Blended");
        return nullptr;
    }

    auto texture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!texture) {
        printSDLError("SDL_CreateTextureFromSurface");
        return nullptr;
    }

    SDL_DestroySurface(textSurface);

    return texture;
}

auto renderButton(SDL_Renderer* renderer, const Button& button) -> void {
    SDL_SetRenderDrawColorFloat(renderer, button.color.r, button.color.g, button.color.b, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &button.layout);
    SDL_RenderTexture(renderer, button.text, nullptr, &button.layout);
}

auto renderTextureScrolledVertically(SDL_Renderer* renderer, SDL_Texture* texture, SDL_FRect clipRect, SDL_FRect destRect) -> void {
    const float xScale = (destRect.w) / (clipRect.w);
    const float yScale = (destRect.h) / (clipRect.h);
    
    const auto upperTextureOffset = wraparound(clipRect.y, 0, texture->h);
    const auto upperDrumRenderOffset = clipRect.h - (upperTextureOffset%static_cast<int>(clipRect.h));

    const auto upperDrumClipRect = SDL_FRect{
        0,
        static_cast<float>(upperTextureOffset),
        static_cast<float>(texture->w),
        static_cast<float>(upperDrumRenderOffset)
    };
    const auto upperDrumRenderRect = SDL_FRect{
        destRect.x,
        destRect.y,
        clipRect.w * xScale,
        upperDrumRenderOffset * yScale
    };

    const auto lowerTextureOffset = wraparound(clipRect.y + upperDrumRenderOffset + 1, 0, texture->h);
    const auto lowerDrumRenderOffset = clipRect.h - upperDrumRenderOffset;

    const auto lowerDrumClipRect = SDL_FRect{
        0,
        static_cast<float>(lowerTextureOffset),
        static_cast<float>(texture->w),
        static_cast<float>(lowerDrumRenderOffset)
    };
    const auto lowerDrumRenderRect = SDL_FRect{
        destRect.x,
        destRect.y + upperDrumRenderOffset * yScale,
        clipRect.w * xScale,
        lowerDrumRenderOffset * yScale
    };

    SDL_RenderTexture(renderer, texture, &upperDrumClipRect, &upperDrumRenderRect);
    SDL_RenderTexture(renderer, texture, &lowerDrumClipRect, &lowerDrumRenderRect);
}

auto renderDrums(SDL_Renderer* renderer, SDL_Texture* texture, gsl::span<const Drum> drums) -> void {
    for (const auto& drum : drums) {
        auto clipRect = SDL_FRect{
            0, 
            static_cast<float>(drum.scrollingOffset),
            drum.layout.w,
            drum.layout.h
        };

        auto destRect = drum.layout;

        renderTextureScrolledVertically(renderer, texture, clipRect, destRect);
    }
}

auto collision(float x, float y, SDL_FRect rect) -> bool {
    return rect.x <= x && x <= rect.x + rect.w && rect.y <= y && y <= rect.y + rect.h;
}

auto renderBackground(SDL_Renderer* renderer, SDL_FColor color) -> void {
    SDL_SetRenderDrawColorFloat(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(renderer);
}

auto initGameContext() -> std::optional<GameContext> {
    GameContext result{};

    const auto windowWidth = 800;
    const auto windowHeight = 600;
    const auto windowFlags = 0;

    result.window = SDL_CreateWindow("OCTAVIAN", windowWidth, windowHeight, windowFlags);
    if (!result.window) {
        printSDLError("SDL_CreateWindow");
        SDL_Quit();
        TTF_Quit();
        return std::nullopt;
    }

    result.renderer = SDL_CreateRenderer(result.window, nullptr);
    if (!result.renderer) {
        printSDLError("SDL_CreateRenderer");
        SDL_DestroyWindow(result.window);
        SDL_Quit();
        TTF_Quit();
        return std::nullopt;
    }

    const auto basePath = std::string(SDL_GetBasePath());

    result.drumTexture = loadTexture(result.renderer, basePath + "../resources/drum.bmp");
    if (!result.drumTexture) {
        SDL_DestroyRenderer(result.renderer);
        SDL_DestroyWindow(result.window);
        SDL_Quit();
        TTF_Quit();
        return std::nullopt;
    }

    result.font = TTF_OpenFont((basePath + "../resources/JetBrainsMonoNL-Regular.ttf").c_str(), 28);
    if (!result.font) {
        printSDLError("TTF_OpenFont");
        SDL_DestroyRenderer(result.renderer);
        SDL_DestroyWindow(result.window);
        SDL_Quit();
        TTF_Quit();
    }

    result.startButton = Button{
        SDL_FRect{650, 10, 150, 60},
        GameColorPallete::startButton,
        loadTextureFromRenderedText(result.renderer, result.font, "START")
    };

    result.stopButton = Button{
        SDL_FRect{650, 80, 150, 60},
        GameColorPallete::stopButton,
        loadTextureFromRenderedText(result.renderer, result.font, "STOP")
    };

    const auto drumScrollingOffset = 0;
    const auto drumVelocity = 2;

    result.drums = {{
        {
            SDL_FRect{0, 0, 64, 64},
            drumScrollingOffset,
            drumVelocity
        },
        {
            SDL_FRect{64, 0, 64, 64},
            drumScrollingOffset,
            drumVelocity+2
        },
        {
            SDL_FRect{64*2, 0, 64, 64},
            drumScrollingOffset,
            drumVelocity+5
        },
        {
            SDL_FRect{64*3, 0, 64, 64},
            drumScrollingOffset,
            drumVelocity+7
        }
    }};

    return result;
}

auto cleanupGameContext(GameContext& context) -> void {
    SDL_DestroyTexture(context.startButton.text);
    SDL_DestroyTexture(context.stopButton.text);
    SDL_DestroyTexture(context.drumTexture);
    SDL_DestroyRenderer(context.renderer);
    SDL_DestroyWindow(context.window);
}