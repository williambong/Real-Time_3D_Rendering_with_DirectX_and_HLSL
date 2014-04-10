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
	class FullScreenQuad;
	class Effect;
	class GaussianBlur;
}

namespace DirectX
{
	class SpriteBatch;
	class SpriteFont;
}

namespace Rendering
{
	class PointLightDemo;
	class ColorFilterMaterial;

	enum ColorFilter
	{
		ColorFilterGrayScale = 0,
		ColorFilterInverse,
		ColorFilterSepia,
		ColorFilterGeneric,
		ColorFilterEnd
	};

	const std::string ColorFilterTechniqueNames[] = { "grayscale_filter", "inverse_filter", "sepia_filter", "generic_filter" };
	const std::string ColorFilterDisplayNames[] = { "Grayscale", "Inverse", "Sepia", "Generic" };

    class ColorFilteringGame : public Game
    {
    public:
        ColorFilteringGame(HINSTANCE instance, const std::wstring& windowClass, const std::wstring& windowTitle, int showCommand);
        ~ColorFilteringGame();

        virtual void Initialize() override;		
        virtual void Update(const GameTime& gameTime) override;
        virtual void Draw(const GameTime& gameTime) override;

    protected:
        virtual void Shutdown() override;

    private:
        void UpdateColorFilterMaterial();
		void ColorFilteringGame::UpdateGenericColorFilter(const GameTime& gameTime);

		static const float BrightnessModulationRate;
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
		
		FullScreenRenderTarget* mRenderTarget;
		FullScreenQuad* mFullScreenQuad;
		Effect* mColorFilterEffect;
		ColorFilterMaterial* mColorFilterMaterial;
		ColorFilter mActiveColorFilter;
		XMFLOAT4X4 mGenericColorFilter;		

		SpriteBatch* mSpriteBatch;
		SpriteFont* mSpriteFont;
		XMFLOAT2 mTextPosition;
    };
}