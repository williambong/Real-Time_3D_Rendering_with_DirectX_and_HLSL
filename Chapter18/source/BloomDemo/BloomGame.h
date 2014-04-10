#pragma once

#include "Common.h"
#include "Game.h"

using namespace Library;

namespace Library
{
    class Keyboard;
    class Mouse;
    class FirstPersonCamera;
    class FpsComponent;
	class RenderStateHelper;
	class Skybox;
	class Grid;
	class FullScreenRenderTarget;
	class Bloom;
}

namespace DirectX
{
	class SpriteBatch;
	class SpriteFont;
}

namespace Rendering
{
	class PointLightDemo;

	class BloomGame : public Game
    {
    public:
        BloomGame(HINSTANCE instance, const std::wstring& windowClass, const std::wstring& windowTitle, int showCommand);
        ~BloomGame();

        virtual void Initialize() override;		
        virtual void Update(const GameTime& gameTime) override;
        virtual void Draw(const GameTime& gameTime) override;

    protected:
        virtual void Shutdown() override;

    private:
		void UpdateBloomSettings(const GameTime& gameTime);

		static const float BloomModulationRate;
		static const XMVECTORF32 BackgroundColor;

        LPDIRECTINPUT8 mDirectInput;
        Keyboard* mKeyboard;
        Mouse* mMouse;
        FirstPersonCamera * mCamera;
        FpsComponent* mFpsComponent;
		RenderStateHelper* mRenderStateHelper;
		Skybox* mSkybox;
		Grid* mGrid;

		PointLightDemo* mPointLightDemo;
		FullScreenRenderTarget* mFullScreenRenderTarget;
		Bloom* mBloom;
		bool mBloomEnabled;

		SpriteBatch* mSpriteBatch;
		SpriteFont* mSpriteFont;
		XMFLOAT2 mTextPosition;		
    };
}