#include "HeightmapTessellationDemo.h"
#include "Game.h"
#include "GameException.h"
#include "Camera.h"
#include "Utility.h"
#include "Effect.h"
#include "Keyboard.h"
#include "QuadHeightmapTessellationMaterial.h"
#include "RasterizerStates.h"
#include <sstream>
#include <iomanip>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <WICTextureLoader.h>
#include "Grid.h"
#include "MatrixHelper.h"

namespace Rendering
{
    RTTI_DEFINITIONS(HeightmapTessellationDemo)

	const float HeightmapTessellationDemo::MaxTessellationFactor = 64.0f;
	const RECT HeightmapTessellationDemo::HeightmapDestinationRectangle = { 0, 512, 256, 768 };
	const XMFLOAT2 HeightmapTessellationDemo::TextureModulationRate = XMFLOAT2(-0.1f, 0.05f);

    HeightmapTessellationDemo::HeightmapTessellationDemo(Game& game, Camera& camera)
        : DrawableGameComponent(game, camera),  
		  mKeyboard(nullptr), mMaterial(nullptr), mEffect(nullptr), mPass(nullptr), mInputLayout(nullptr), mVertexBuffer(nullptr),
		  mHeightmap(nullptr), mDisplacementScale(1.0f), mRenderStateHelper(game), mTessellationEdgeFactors(), mTessellationInsideFactors(),
		  mSpriteBatch(nullptr), mSpriteFont(nullptr), mTextPosition(0.0f, 40.0f),
		  mTextureMatrix(MatrixHelper::Identity), mTexturePosition(0.0f, 0.0f), mAnimationEnabled(false)
    {
    }

    HeightmapTessellationDemo::~HeightmapTessellationDemo()
    {
		ReleaseObject(mHeightmap);
		DeleteObject(mSpriteFont);
		DeleteObject(mSpriteBatch);
		ReleaseObject(mVertexBuffer);
		DeleteObject(mMaterial);
		DeleteObject(mEffect);
    }

    void HeightmapTessellationDemo::Initialize()
    {
        SetCurrentDirectory(Utility::ExecutableDirectory().c_str());
	
        // Initialize the material
		mEffect = new Effect(*mGame);
		mEffect->LoadCompiledEffect(L"Content\\Effects\\QuadHeightmapTessellation.cso");
		mMaterial = new QuadHeightmapTessellationMaterial();
		mMaterial->Initialize(*mEffect);

		mPass = mMaterial->CurrentTechnique()->Passes().at(0);
		mInputLayout = mMaterial->InputLayouts().at(mPass);

		VertexPositionTexture vertices[] =
		{
			VertexPositionTexture(XMFLOAT4(-10.0f, 1.0f, -10.0f, 1.0f), XMFLOAT2(0.0f, 0.0f)), // upper-left
			VertexPositionTexture(XMFLOAT4(10.0f, 1.0f, -10.0f, 1.0f), XMFLOAT2(1.0f, 0.0f)),  // upper-right
			VertexPositionTexture(XMFLOAT4(-10.0f, 1.0f, 10.0f, 1.0f), XMFLOAT2(0.0f, 1.0f)),  // lower-left
			VertexPositionTexture(XMFLOAT4(10.0f, 1.0f, 10.0f, 1.0f), XMFLOAT2(1.0f, 1.0f))    // lower-right
		};

		mMaterial->CreateVertexBuffer(mGame->Direct3DDevice(), vertices, ARRAYSIZE(vertices), &mVertexBuffer);

		// Load the texture
		HRESULT hr;
		std::wstring textureName = L"Content\\Textures\\Heightmap.jpg";
		if (FAILED(hr = DirectX::CreateWICTextureFromFile(mGame->Direct3DDevice(), mGame->Direct3DDeviceContext(), textureName.c_str(), nullptr, &mHeightmap)))
		{
			throw GameException("CreateWICTextureFromFile() failed.", hr);
		}

		mTessellationEdgeFactors.push_back(10);
		mTessellationEdgeFactors.push_back(10);
		mTessellationEdgeFactors.push_back(10);
		mTessellationEdgeFactors.push_back(10);

		mTessellationInsideFactors.push_back(10);
		mTessellationInsideFactors.push_back(10);

		mKeyboard = (Keyboard*)mGame->Services().GetService(Keyboard::TypeIdClass());
		assert(mKeyboard != nullptr);

		mSpriteBatch = new SpriteBatch(mGame->Direct3DDeviceContext());
		mSpriteFont = new SpriteFont(mGame->Direct3DDevice(), L"Content\\Fonts\\Arial_14_Regular.spritefont");

		// Disable the grid component
		for (GameComponent* component : mGame->Components())
		{
			Grid* grid = component->As<Grid>();
			if (grid != nullptr)
			{
				grid->SetVisible(false);
			}
		}
    }

	void HeightmapTessellationDemo::Update(const GameTime& gameTime)
	{
		if (mAnimationEnabled)
		{
			mTexturePosition.x += TextureModulationRate.x * static_cast<float>(gameTime.ElapsedGameTime());
			mTexturePosition.y += TextureModulationRate.y * static_cast<float>(gameTime.ElapsedGameTime());
			XMStoreFloat4x4(&mTextureMatrix, XMMatrixTranslation(mTexturePosition.x, mTexturePosition.y, 0.0f));
		}

		if (mKeyboard->WasKeyPressedThisFrame(DIK_SPACE))
		{
			mAnimationEnabled = !mAnimationEnabled;
		}

		if (mKeyboard->WasKeyPressedThisFrame(DIK_UPARROW))
		{
			float tesselletionFactor = mTessellationEdgeFactors[0] + 1;
			tesselletionFactor = min(tesselletionFactor, MaxTessellationFactor);

			for (UINT i = 0; i < mTessellationEdgeFactors.size(); i++)
			{
				mTessellationEdgeFactors[i] = tesselletionFactor;
			}

			for (UINT i = 0; i < mTessellationInsideFactors.size(); i++)
			{
				mTessellationInsideFactors[i] = tesselletionFactor;
			}
		}

		if (mKeyboard->WasKeyPressedThisFrame(DIK_DOWNARROW))
		{
			float tesselletionFactor = mTessellationEdgeFactors[0] - 1;
			tesselletionFactor = max(tesselletionFactor, 1);

			for (UINT i = 0; i < mTessellationEdgeFactors.size(); i++)
			{
				mTessellationEdgeFactors[i] = tesselletionFactor;
			}

			for (UINT i = 0; i < mTessellationInsideFactors.size(); i++)
			{
				mTessellationInsideFactors[i] = tesselletionFactor;
			}
		}

		if (mKeyboard->WasKeyPressedThisFrame(DIK_PGUP))
		{
			float tesselletionFactor = mTessellationEdgeFactors[0] + 5;
			tesselletionFactor = min(tesselletionFactor, MaxTessellationFactor);

			for (UINT i = 0; i < mTessellationEdgeFactors.size(); i++)
			{
				mTessellationEdgeFactors[i] = tesselletionFactor;
			}

			for (UINT i = 0; i < mTessellationInsideFactors.size(); i++)
			{
				mTessellationInsideFactors[i] = tesselletionFactor;
			}
		}

		if (mKeyboard->WasKeyPressedThisFrame(DIK_PGDN))
		{
			float tesselletionFactor = mTessellationEdgeFactors[0] - 5;
			tesselletionFactor = max(tesselletionFactor, 1);

			for (UINT i = 0; i < mTessellationEdgeFactors.size(); i++)
			{
				mTessellationEdgeFactors[i] = tesselletionFactor;
			}

			for (UINT i = 0; i < mTessellationInsideFactors.size(); i++)
			{
				mTessellationInsideFactors[i] = tesselletionFactor;
			}
		}

		UpdateDisplacementScale(gameTime);

		if (mKeyboard->WasKeyPressedThisFrame(DIK_G))
		{
			for (GameComponent* component : mGame->Components())
			{
				Grid* grid = component->As<Grid>();
				if (grid != nullptr)
				{
					grid->SetVisible(!grid->Visible());
				}
			}
		}
	}

	void HeightmapTessellationDemo::Draw(const GameTime& gameTime)
	{
		mRenderStateHelper.SaveRasterizerState();

		ID3D11DeviceContext* direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->RSSetState(RasterizerStates::Wireframe);

		direct3DDeviceContext->IASetInputLayout(mInputLayout);
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

		UINT stride = mMaterial->VertexSize();
		UINT offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);

		mMaterial->WorldViewProjection() << mCamera->ViewMatrix() * mCamera->ProjectionMatrix();
		mMaterial->TextureMatrix() << XMLoadFloat4x4(&mTextureMatrix);
		mMaterial->TessellationEdgeFactors() << mTessellationEdgeFactors;
		mMaterial->TessellationInsideFactors() << mTessellationInsideFactors;
		mMaterial->Heightmap() << mHeightmap;
		mMaterial->DisplacementScale() << mDisplacementScale;
		mPass->Apply(0, direct3DDeviceContext);

		direct3DDeviceContext->Draw(4, 0);

		direct3DDeviceContext->HSSetShader(nullptr, nullptr, 0);
		direct3DDeviceContext->DSSetShader(nullptr, nullptr, 0);

		mRenderStateHelper.RestoreAll();

		mRenderStateHelper.SaveAll();
		mSpriteBatch->Begin();

		mSpriteBatch->Draw(mHeightmap, HeightmapDestinationRectangle);

		std::wostringstream helpLabel;
		helpLabel << std::setprecision(2) << "\nTessellation Edge Factors: (+Up/-Down +PgUp/-PgDn)[" << mTessellationEdgeFactors[0] << ", " << mTessellationEdgeFactors[1] << ", " << mTessellationEdgeFactors[2] << ", " << mTessellationEdgeFactors[3] << "]"
			<< "\nTessellation Inside Factor: [" << mTessellationInsideFactors[0] << ", " << mTessellationInsideFactors[1] << "]"
			<< "\nDisplacement Scale (+Right/-Left): " << mDisplacementScale << "\nAnimation Enabled (Space): " << (mAnimationEnabled ? "True" : "False");

		mSpriteFont->DrawString(mSpriteBatch, helpLabel.str().c_str(), mTextPosition);

		mSpriteBatch->End();
		mRenderStateHelper.RestoreAll();
    }

	void HeightmapTessellationDemo::UpdateDisplacementScale(const GameTime& gameTime)
	{
		if (mKeyboard->IsKeyDown(DIK_RIGHTARROW) && mDisplacementScale < UCHAR_MAX)
		{
			mDisplacementScale += (float)gameTime.ElapsedGameTime();
			mDisplacementScale = XMMin<float>(mDisplacementScale, UCHAR_MAX);
		}

		if (mKeyboard->IsKeyDown(DIK_LEFTARROW))
		{
			mDisplacementScale -= (float)gameTime.ElapsedGameTime();
			mDisplacementScale = XMMax<float>(mDisplacementScale, -UCHAR_MAX);
		}
	}
}