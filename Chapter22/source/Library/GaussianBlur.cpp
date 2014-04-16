#include "GaussianBlur.h"
#include "Game.h"
#include "GameException.h"
#include "GaussianBlurMaterial.h"
#include "FullScreenRenderTarget.h"
#include "FullScreenQuad.h"
#include "Camera.h"
#include "VectorHelper.h"
#include "MatrixHelper.h"
#include "Utility.h"
#include "ColorHelper.h"

namespace Library
{
    RTTI_DEFINITIONS(GaussianBlur)

    const float GaussianBlur::DefaultBlurAmount = 1.0f;

    GaussianBlur::GaussianBlur(Game& game, Camera& camera)
        : DrawableGameComponent(game, camera),
          mEffect(nullptr), mMaterial(nullptr), mSceneTexture(nullptr), mOutputTexture(nullptr), mHorizontalBlurTarget(nullptr), mVerticalBlurTarget(nullptr), mFullScreenQuad(nullptr),
          mHorizontalSampleOffsets(), mVerticalSampleOffsets(), mSampleWeights(), mBlurAmount(DefaultBlurAmount)
    {
    }

    GaussianBlur::GaussianBlur(Game& game, Camera& camera, float blurAmount)
        : DrawableGameComponent(game, camera),
          mEffect(nullptr), mMaterial(nullptr), mSceneTexture(nullptr), mOutputTexture(nullptr), mHorizontalBlurTarget(nullptr), mVerticalBlurTarget(nullptr), mFullScreenQuad(nullptr),
          mHorizontalSampleOffsets(), mVerticalSampleOffsets(), mSampleWeights(), mBlurAmount(blurAmount)
    {
    }

    GaussianBlur::~GaussianBlur()
    {
        DeleteObject(mFullScreenQuad);
        DeleteObject(mVerticalBlurTarget);
		DeleteObject(mHorizontalBlurTarget);        
        DeleteObject(mMaterial);
        DeleteObject(mEffect);
    }

    ID3D11ShaderResourceView* GaussianBlur::SceneTexture()
    {
        return mSceneTexture;
    }

    void GaussianBlur::SetSceneTexture(ID3D11ShaderResourceView& sceneTexture)
    {
        mSceneTexture = &sceneTexture;
    }

	ID3D11ShaderResourceView* GaussianBlur::OutputTexture()
	{
		return mOutputTexture;
	}

    float GaussianBlur::BlurAmount() const
    {
        return mBlurAmount;
    }

    void GaussianBlur::SetBlurAmount(float blurAmount)
    {
        mBlurAmount = blurAmount;
        InitializeSampleWeights();
    }

    void GaussianBlur::Initialize()
    {
        SetCurrentDirectory(Utility::ExecutableDirectory().c_str());

        mEffect = new Effect(*mGame);
        mEffect->LoadCompiledEffect(L"Content\\Effects\\GaussianBlur.cso");

        mMaterial = new GaussianBlurMaterial();
        mMaterial->Initialize(*mEffect);

        mFullScreenQuad = new FullScreenQuad(*mGame, *mMaterial);
        mFullScreenQuad->Initialize();        

        InitializeSampleWeights();
        InitializeSampleOffsets();

        mHorizontalBlurTarget = new FullScreenRenderTarget(*mGame);
        mVerticalBlurTarget = new FullScreenRenderTarget(*mGame);
    }

    void GaussianBlur::Draw(const GameTime& gameTime)
    {
		mOutputTexture = nullptr;

        if (mBlurAmount > 0.0f)
        {
            // Horizontal blur
            mHorizontalBlurTarget->Begin();
            mGame->Direct3DDeviceContext()->ClearRenderTargetView(mHorizontalBlurTarget->RenderTargetView(), reinterpret_cast<const float*>(&ColorHelper::Purple));
            mGame->Direct3DDeviceContext()->ClearDepthStencilView(mHorizontalBlurTarget->DepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
			mFullScreenQuad->SetActiveTechnique("blur", "p0");
            mFullScreenQuad->SetCustomUpdateMaterial(std::bind(&GaussianBlur::UpdateGaussianMaterialWithHorizontalOffsets, this));
            mFullScreenQuad->Draw(gameTime);
            mHorizontalBlurTarget->End();

            // Vertical blur for the final image
            mFullScreenQuad->SetCustomUpdateMaterial(std::bind(&GaussianBlur::UpdateGaussianMaterialWithVerticalOffsets, this));
            mFullScreenQuad->Draw(gameTime);
        }
        else
        {
			mFullScreenQuad->SetActiveTechnique("no_blur", "p0");
            mFullScreenQuad->SetCustomUpdateMaterial(std::bind(&GaussianBlur::UpdateGaussianMaterialNoBlur, this));
            mFullScreenQuad->Draw(gameTime);
        }
    }

	void GaussianBlur::DrawToTexture(const GameTime& gameTime)
	{
		if (mBlurAmount > 0.0f)
        {
            // Horizontal blur
            mHorizontalBlurTarget->Begin();
            mGame->Direct3DDeviceContext()->ClearRenderTargetView(mHorizontalBlurTarget->RenderTargetView(), reinterpret_cast<const float*>(&ColorHelper::Purple));
            mGame->Direct3DDeviceContext()->ClearDepthStencilView(mHorizontalBlurTarget->DepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
			mFullScreenQuad->SetActiveTechnique("blur", "p0");
            mFullScreenQuad->SetCustomUpdateMaterial(std::bind(&GaussianBlur::UpdateGaussianMaterialWithHorizontalOffsets, this));
            mFullScreenQuad->Draw(gameTime);
            mHorizontalBlurTarget->End();

            mGame->UnbindPixelShaderResources(0, 1);

			// Vertical blur for the final image
            mVerticalBlurTarget->Begin();
            mGame->Direct3DDeviceContext()->ClearRenderTargetView(mVerticalBlurTarget->RenderTargetView(), reinterpret_cast<const float*>(&ColorHelper::Purple));
            mGame->Direct3DDeviceContext()->ClearDepthStencilView(mVerticalBlurTarget->DepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
            mFullScreenQuad->SetCustomUpdateMaterial(std::bind(&GaussianBlur::UpdateGaussianMaterialWithVerticalOffsets, this));
            mFullScreenQuad->Draw(gameTime);
            mVerticalBlurTarget->End();

			mGame->UnbindPixelShaderResources(0, 1);
			mOutputTexture = mVerticalBlurTarget->OutputTexture();
        }
        else
        {
			mHorizontalBlurTarget->Begin();
			mFullScreenQuad->SetActiveTechnique("no_blur", "p0");
            mFullScreenQuad->SetCustomUpdateMaterial(std::bind(&GaussianBlur::UpdateGaussianMaterialNoBlur, this));
            mFullScreenQuad->Draw(gameTime);
			mHorizontalBlurTarget->End();

			mGame->UnbindPixelShaderResources(0, 1);
			mOutputTexture = mHorizontalBlurTarget->OutputTexture();
        }		
	}

	void GaussianBlur::InitializeSampleOffsets()
	{
		float horizontalPixelSize = 1.0f / mGame->ScreenWidth();
		float verticalPixelSize = 1.0f / mGame->ScreenHeight();

		UINT sampleCount = mMaterial->SampleOffsets().TypeDesc().Elements;

		mHorizontalSampleOffsets.resize(sampleCount);
		mVerticalSampleOffsets.resize(sampleCount);
		mHorizontalSampleOffsets[0] = Vector2Helper::Zero;
		mVerticalSampleOffsets[0] = Vector2Helper::Zero;

		for (UINT i = 0; i < sampleCount / 2; i++)
		{
			float sampleOffset = i * 2 + 1.5f;
			float horizontalOffset = horizontalPixelSize * sampleOffset;
			float verticalOffset = verticalPixelSize * sampleOffset;

			mHorizontalSampleOffsets[i * 2 + 1] = XMFLOAT2(horizontalOffset, 0.0f);
			mHorizontalSampleOffsets[i * 2 + 2] = XMFLOAT2(-horizontalOffset, 0.0f);

			mVerticalSampleOffsets[i * 2 + 1] = XMFLOAT2(0.0f, verticalOffset);
			mVerticalSampleOffsets[i * 2 + 2] = XMFLOAT2(0.0f, -verticalOffset);
		}
	}

	void GaussianBlur::InitializeSampleWeights()
	{
		UINT sampleCount = mMaterial->SampleOffsets().TypeDesc().Elements;

		mSampleWeights.resize(sampleCount);
		mSampleWeights[0] = GetWeight(0);

		float totalWeight = mSampleWeights[0];
		for (UINT i = 0; i < sampleCount / 2; i++)
		{
			float weight = GetWeight((float)i + 1);
			mSampleWeights[i * 2 + 1] = weight;
			mSampleWeights[i * 2 + 2] = weight;
			totalWeight += weight * 2;
		}

		// Normalize the weights so that they sum to one
		for (UINT i = 0; i < mSampleWeights.size(); i++)
		{
			mSampleWeights[i] /= totalWeight;
		}
	}

	float GaussianBlur::GetWeight(float x) const
	{
		return (float)(exp(-(x * x) / (2 * mBlurAmount * mBlurAmount)));
	}

	void GaussianBlur::UpdateGaussianMaterialWithHorizontalOffsets()
	{
		mMaterial->ColorTexture() << mSceneTexture;
		mMaterial->SampleWeights() << mSampleWeights;
		mMaterial->SampleOffsets() << mHorizontalSampleOffsets;
	}

	void GaussianBlur::UpdateGaussianMaterialWithVerticalOffsets()
	{
		mMaterial->ColorTexture() << mHorizontalBlurTarget->OutputTexture();
		mMaterial->SampleWeights() << mSampleWeights;
		mMaterial->SampleOffsets() << mVerticalSampleOffsets;
	}

	void GaussianBlur::UpdateGaussianMaterialNoBlur()
	{
		mMaterial->ColorTexture() << mSceneTexture;
	}
}