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
	class DistortionMappingPostMaterial;
}

namespace DirectX
{
	class SpriteBatch;
	class SpriteFont;
}

namespace Rendering
{
	class PointLightDemo;

    class DistortionMappingPostGame : public Game
    {
    public:
        DistortionMappingPostGame(HINSTANCE instance, const std::wstring& windowClass, const std::wstring& windowTitle, int showCommand);
        ~DistortionMappingPostGame();

        virtual void Initialize() override;		
        virtual void Update(const GameTime& gameTime) override;
        virtual void Draw(const GameTime& gameTime) override;

    protected:
        virtual void Shutdown() override;

    private:
		void UpdateDisplacementMaterial();
		void UpdateDisplacementScale(const GameTime& gameTime);

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
		Effect* mDistortionMappingEffect;
		DistortionMappingPostMaterial* mDistortionMappingPostMaterial;
		ID3D11ShaderResourceView* mDistortionMap;
		float mDisplacementScale;

		SpriteBatch* mSpriteBatch;
		SpriteFont* mSpriteFont;
		XMFLOAT2 mTextPosition;
    };
}