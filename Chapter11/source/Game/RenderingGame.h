#pragma once

#include "Common.h"
#include "Game.h"

using namespace Library;

namespace Rendering
{
    class RenderingGame : public Game
    {
    public:
        RenderingGame(HINSTANCE instance, const std::wstring& windowClass, const std::wstring& windowTitle, int showCommand);
        ~RenderingGame();

        virtual void Initialize() override;
        virtual void Update(const GameTime& gameTime) override;
        virtual void Draw(const GameTime& gameTime) override;

    private:
        static const XMVECTORF32 BackgroundColor;
    };
}
