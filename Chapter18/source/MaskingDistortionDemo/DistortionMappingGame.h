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
	class Pass;
	class DistortionMappingMaterial;
}

namespace DirectX
{
	class SpriteBatch;
	class SpriteFont;
}

namespace Rendering
{
    class DistortionMappingGame : public Game
    {
    public:
        DistortionMappingGame(HINSTANCE instance, const std::wstring& windowClass, const std::wstring& windowTitle, int showCommand);
        ~DistortionMappingGame();

        virtual void Initialize() override;		
        virtual void Update(const GameTime& gameTime) override;
        virtual void Draw(const GameTime& gameTime) override;

    protected:
        virtual void Shutdown() override;

    private:
		void DrawMeshForDistortionCutout();
		void UpdateDistortionCompositeMaterial();
		void UpdateDrawDistortionCutoutMaterial();
		void UpdateDisplacementScale(const GameTime& gameTime);

		static const XMVECTORF32 BackgroundColor;
		static const XMVECTORF32 CutoutZeroColor;

        LPDIRECTINPUT8 mDirectInput;
        Keyboard* mKeyboard;
        Mouse* mMouse;
        FirstPersonCamera * mCamera;
        FpsComponent* mFpsComponent;
		RenderStateHelper* mRenderStateHelper;
		Skybox* mSkybox;
		Grid* mGrid;
		
		FullScreenRenderTarget* mSceneRenderTarget;
		FullScreenRenderTarget* mCutoutRenderTarget;
		FullScreenQuad* mFullScreenQuad;
		Effect* mDistortionMappingEffect;
		DistortionMappingMaterial* mDistortionMappingMaterial;
		ID3D11ShaderResourceView* mDistortionMap;
		
		ID3D11Buffer* mVertexBuffer;
		ID3D11Buffer* mIndexBuffer;
		UINT mIndexCount;
		XMFLOAT4X4 mWorldMatrix;
		Pass* mDistortionCutoutPass;
		float mDisplacementScale;
		bool mDrawDistortionCutout;

		SpriteBatch* mSpriteBatch;
		SpriteFont* mSpriteFont;
		XMFLOAT2 mTextPosition;
    };
}