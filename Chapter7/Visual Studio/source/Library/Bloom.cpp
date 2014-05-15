#include "Bloom.h"
#include "Game.h"
#include "GameException.h"
#include "BloomMaterial.h"
#include "GaussianBlurMaterial.h"
#include "FullScreenRenderTarget.h"
#include "FullScreenQuad.h"
#include "Camera.h"
#include "VectorHelper.h"
#include "MatrixHelper.h"
#include "Utility.h"
#include "ColorHelper.h"
#include "GaussianBlur.h"

namespace Library
{
    RTTI_DEFINITIONS(Bloom)

	const std::string Bloom::DrawModeDisplayNames[] = { "Normal", "Extracted Texture", "Blurred Texture" };
	const BloomSettings Bloom::DefaultBloomSettings = { 0.45f, 2.0f, 1.25f, 1.0f, 1.0f, 1.0f };

    Bloom::Bloom(Game& game, Camera& camera)
        : DrawableGameComponent(game, camera),
          mBloomEffect(nullptr), mBloomMaterial(nullptr), mSceneTexture(nullptr), mRenderTarget(nullptr),
		  mFullScreenQuad(nullptr), mGaussianBlur(nullptr), mBloomSettings(DefaultBloomSettings), mDrawMode(BloomDrawModeNormal), mDrawFunctions()
    {
    }

    Bloom::Bloom(Game& game, Camera& camera, const BloomSettings& bloomSettings)
        : DrawableGameComponent(game, camera),
          mBloomEffect(nullptr), mBloomMaterial(nullptr), mSceneTexture(nullptr), mRenderTarget(nullptr),
		  mFullScreenQuad(nullptr), mGaussianBlur(nullptr),  mBloomSettings(bloomSettings), mDrawMode(BloomDrawModeNormal), mDrawFunctions()
    {
    }

    Bloom::~Bloom()
    {
		DeleteObject(mGaussianBlur);
        DeleteObject(mFullScreenQuad);
        DeleteObject(mRenderTarget);
		DeleteObject(mBloomMaterial);
		DeleteObject(mBloomEffect);
    }

    ID3D11ShaderResourceView* Bloom::SceneTexture()
    {
        return mSceneTexture;
    }

    void Bloom::SetSceneTexture(ID3D11ShaderResourceView& sceneTexture)
    {
        mSceneTexture = &sceneTexture;
    }

    const BloomSettings& Bloom::GetBloomSettings() const
    {
        return mBloomSettings;
    }

    void Bloom::SetBloomSettings(const BloomSettings& bloomSettings)
    {
        mBloomSettings = bloomSettings;
		mGaussianBlur->SetBlurAmount(mBloomSettings.BlurAmount);
    }

    void Bloom::Initialize()
    {
        SetCurrentDirectory(Utility::ExecutableDirectory().c_str());

        mBloomEffect = new Effect(*mGame);
        mBloomEffect->LoadCompiledEffect(L"Content\\Effects\\Bloom.cso");

        mBloomMaterial = new BloomMaterial();
        mBloomMaterial->Initialize(*mBloomEffect);

        mFullScreenQuad = new FullScreenQuad(*mGame, *mBloomMaterial);		
        mFullScreenQuad->Initialize();

        mRenderTarget = new FullScreenRenderTarget(*mGame);

		mGaussianBlur = new GaussianBlur(*mGame, *mCamera, mBloomSettings.BlurAmount);
		mGaussianBlur->SetSceneTexture(*(mRenderTarget->OutputTexture()));
		mGaussianBlur->Initialize();

		using namespace std::placeholders;
		mDrawFunctions[BloomDrawModeNormal] = std::bind(&Bloom::DrawNormal, this, _1);
		mDrawFunctions[BloomDrawModeExtractedTexture1] = std::bind(&Bloom::DrawExtractedTexture, this, _1);
		mDrawFunctions[BloomDrawModeBlurredTexture] = std::bind(&Bloom::DrawBlurredTexture, this, _1);
    }

    void Bloom::Draw(const GameTime& gameTime)
    {
		mDrawFunctions[mDrawMode](gameTime);		
    }

	void Bloom::DrawNormal(const GameTime& gameTime)
	{
		if (mBloomSettings.BloomThreshold < 1.0f)
        {
			// Extract the bright spots in the scene
			mRenderTarget->Begin();
            mGame->Direct3DDeviceContext()->ClearRenderTargetView(mRenderTarget->RenderTargetView(), reinterpret_cast<const float*>(&ColorHelper::Purple));
            mGame->Direct3DDeviceContext()->ClearDepthStencilView(mRenderTarget->DepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
			mFullScreenQuad->SetMaterial(*mBloomMaterial, "bloom_extract", "p0");
            mFullScreenQuad->SetCustomUpdateMaterial(std::bind(&Bloom::UpdateBloomExtractMaterial, this));
            mFullScreenQuad->Draw(gameTime);
			mRenderTarget->End();
			mGame->UnbindPixelShaderResources(0, 1);

			// Blur the bright spots in the scene
			mGaussianBlur->DrawToTexture(gameTime);
			mGame->UnbindPixelShaderResources(0, 1);
			
			// Combine the original scene with the blurred bright spot image
			mFullScreenQuad->SetMaterial(*mBloomMaterial, "bloom_composite", "p0");
			mFullScreenQuad->SetCustomUpdateMaterial(std::bind(&Bloom::UpdateBloomCompositeMaterial, this));
			mFullScreenQuad->Draw(gameTime);
			mGame->UnbindPixelShaderResources(0, 2);
        }
        else
        {
			mFullScreenQuad->SetMaterial(*mBloomMaterial, "no_bloom", "p0");
            mFullScreenQuad->SetCustomUpdateMaterial(std::bind(&Bloom::UpdateNoBloomMaterial, this));
            mFullScreenQuad->Draw(gameTime);
        }
	}

	void Bloom::DrawExtractedTexture(const GameTime& gameTime)
	{
		mFullScreenQuad->SetMaterial(*mBloomMaterial, "bloom_extract", "p0");
        mFullScreenQuad->SetCustomUpdateMaterial(std::bind(&Bloom::UpdateBloomExtractMaterial, this));
        mFullScreenQuad->Draw(gameTime);
	}

	void Bloom::DrawBlurredTexture(const GameTime& gameTime)
	{
		// Extract the bright spots in the scene
		mRenderTarget->Begin();
        mGame->Direct3DDeviceContext()->ClearRenderTargetView(mRenderTarget->RenderTargetView() , reinterpret_cast<const float*>(&ColorHelper::Purple));
        mGame->Direct3DDeviceContext()->ClearDepthStencilView(mRenderTarget->DepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		mFullScreenQuad->SetMaterial(*mBloomMaterial, "bloom_extract", "p0");
        mFullScreenQuad->SetCustomUpdateMaterial(std::bind(&Bloom::UpdateBloomExtractMaterial, this));
        mFullScreenQuad->Draw(gameTime);			
		mRenderTarget->End();
		mGame->UnbindPixelShaderResources(0, 1);

		mGaussianBlur->Draw(gameTime);
	}

	void Bloom::UpdateBloomExtractMaterial()
	{
		mBloomMaterial->ColorTexture() << mSceneTexture;
		mBloomMaterial->BloomThreshold() << mBloomSettings.BloomThreshold;
	}

	void Bloom::UpdateBloomCompositeMaterial()
	{	
		mBloomMaterial->ColorTexture() << mSceneTexture;
		mBloomMaterial->BloomTexture() << mGaussianBlur->OutputTexture();
		mBloomMaterial->BloomIntensity() << mBloomSettings.BloomIntensity;
		mBloomMaterial->BloomSaturation() << mBloomSettings.BloomSaturation;
		mBloomMaterial->SceneIntensity() << mBloomSettings.SceneIntensity;
		mBloomMaterial->SceneSaturation() << mBloomSettings.SceneSaturation;
	}

	void Bloom::UpdateNoBloomMaterial()
	{
		mBloomMaterial->ColorTexture() << mSceneTexture;
	}

	BloomDrawMode Bloom::DrawMode() const
	{
		return mDrawMode;
	}

	std::string Bloom::DrawModeString() const
	{
		return DrawModeDisplayNames[(int)mDrawMode];
	}

	void Bloom::SetDrawMode(BloomDrawMode drawMode)
	{
		mDrawMode = drawMode;
	}
}

