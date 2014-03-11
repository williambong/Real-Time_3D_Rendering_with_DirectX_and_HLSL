#pragma once

#include "Common.h"
#include "Game.h"

using namespace Library;

namespace DirectX
{
    class SpriteBatch;
    class SpriteFont;
}

namespace Library
{
	class FpsComponent;
	class Keyboard;
	class Mouse;
}

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

	protected:
		virtual void Shutdown() override;

    private:
        static const XMVECTORF32 BackgroundColor;

		LPDIRECTINPUT8 mDirectInput;
		Keyboard* mKeyboard;
		Mouse* mMouse;
		FpsComponent* mFpsComponent;

		SpriteBatch* mSpriteBatch;
        SpriteFont* mSpriteFont;
        XMFLOAT2 mMouseTextPosition;
    };
}