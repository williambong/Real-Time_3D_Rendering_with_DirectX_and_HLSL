#include "GeometryShaderDemo.h"
#include "Game.h"
#include "GameException.h"
#include "Camera.h"
#include "Utility.h"
#include "Effect.h"
#include "Keyboard.h"
#include "PointSpriteMaterial.h"
#include <WICTextureLoader.h>
#include "VectorHelper.h"
#include <random>
#include <sstream>
#include <SpriteBatch.h>
#include <SpriteFont.h>

namespace Rendering
{
    RTTI_DEFINITIONS(GeometryShaderDemo)

    GeometryShaderDemo::GeometryShaderDemo(Game& game, Camera& camera)
        : DrawableGameComponent(game, camera),  
		  mKeyboard(nullptr), mMaterial(nullptr), mEffect(nullptr), mPass(nullptr), mInputLayout(nullptr),
		  mVertexBuffer(nullptr), mColorTexture(nullptr), mRenderStateHelper(game),
		  mSpriteBatch(nullptr), mSpriteFont(nullptr), mTextPosition(0.0f, 40.0f), mShowRandomPoints(true)
    {
    }

    GeometryShaderDemo::~GeometryShaderDemo()
    {
		DeleteObject(mSpriteFont);
		DeleteObject(mSpriteBatch);
		ReleaseObject(mColorTexture);
		ReleaseObject(mVertexBuffer);
        DeleteObject(mMaterial);
        DeleteObject(mEffect);
    }

    void GeometryShaderDemo::Initialize()
    {
        SetCurrentDirectory(Utility::ExecutableDirectory().c_str());
	
        // Initialize the material
        mEffect = new Effect(*mGame);
        mEffect->LoadCompiledEffect(L"Content\\Effects\\PointSprite.cso");
		mMaterial = new PointSpriteMaterial();
        mMaterial->Initialize(*mEffect);
		
		Technique* technique = mEffect->TechniquesByName().at("main11");
		mMaterial->SetCurrentTechnique(*technique);

		mPass = mMaterial->CurrentTechnique()->Passes().at(0);
		mInputLayout = mMaterial->InputLayouts().at(mPass);

		InitializeRandomPoints();
		 
		std::wstring textureName = L"Content\\Textures\\BookCover.png";
		HRESULT hr = DirectX::CreateWICTextureFromFile(mGame->Direct3DDevice(), mGame->Direct3DDeviceContext(), textureName.c_str(), nullptr, &mColorTexture);
		if (FAILED(hr))
		{
			throw GameException("CreateWICTextureFromFile() failed.", hr);
		}

		mKeyboard = (Keyboard*)mGame->Services().GetService(Keyboard::TypeIdClass());
		assert(mKeyboard != nullptr);

		mSpriteBatch = new SpriteBatch(mGame->Direct3DDeviceContext());
		mSpriteFont = new SpriteFont(mGame->Direct3DDevice(), L"Content\\Fonts\\Arial_14_Regular.spritefont");
    }

	void GeometryShaderDemo::Update(const GameTime& gameTime)
	{
		if (mKeyboard->WasKeyPressedThisFrame(DIK_SPACE))
		{
			mShowRandomPoints = !mShowRandomPoints;
			if (mShowRandomPoints)
			{
				InitializeRandomPoints();
				Technique* technique = mEffect->TechniquesByName().at("main11_strip");
				mMaterial->SetCurrentTechnique(*technique);

				mPass = technique->Passes().at(0);
				mInputLayout = mMaterial->InputLayouts().at(mPass);
			}
			else
			{
				InitializeFixedPoints();
				Technique* technique = mEffect->TechniquesByName().at("main11_nosize");
				mMaterial->SetCurrentTechnique(*technique);

				mPass = technique->Passes().at(0);
				mInputLayout = mMaterial->InputLayouts().at(mPass);
			}
		}
	}

	void GeometryShaderDemo::Draw(const GameTime& gameTime)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		direct3DDeviceContext->IASetInputLayout(mInputLayout);

		UINT stride = mMaterial->VertexSize();
		UINT offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
		
		mMaterial->ViewProjection() << mCamera->ViewMatrix() * mCamera->ProjectionMatrix();
		mMaterial->CameraPosition() << mCamera->PositionVector();
		mMaterial->CameraUp() << mCamera->UpVector();
		mMaterial->ColorTexture() << mColorTexture;

		mPass->Apply(0, direct3DDeviceContext);

		direct3DDeviceContext->Draw(mVertexCount, 0);

		direct3DDeviceContext->GSSetShader(nullptr, nullptr, 0);

		mRenderStateHelper.SaveAll();
		mSpriteBatch->Begin();

		std::wostringstream helpLabel;
		helpLabel  << L"Demo Mode (Space): " << (mShowRandomPoints ? "Random Points" : "Fixed Points");
		mSpriteFont->DrawString(mSpriteBatch, helpLabel.str().c_str(), mTextPosition);

		mSpriteBatch->End();
		mRenderStateHelper.RestoreAll();
    }

	void GeometryShaderDemo::InitializeRandomPoints()
	{
		UINT maxPoints = 100;
		float maxDistance = 10;
		float minSize = 2;
		float maxSize = 2;

		std::random_device randomDevice;
		std::default_random_engine randomGenerator(randomDevice());
		std::uniform_real_distribution<float> distanceDistribution(-maxDistance, maxDistance);
		std::uniform_real_distribution<float> sizeDistribution(minSize, maxSize);

		// Randomly generate points
		std::vector<VertexPositionSize> vertices;
		vertices.reserve(maxPoints);
		for (UINT i = 0; i < maxPoints; i++)
		{
			float x = distanceDistribution(randomGenerator);
			float y = distanceDistribution(randomGenerator);
			float z = distanceDistribution(randomGenerator);

			float size = sizeDistribution(randomGenerator);

			vertices.push_back(VertexPositionSize(XMFLOAT4(x, y, z, 1.0f), XMFLOAT2(size, size)));
		}

		mVertexCount = vertices.size();

		ReleaseObject(mVertexBuffer);
		mMaterial->CreateVertexBuffer(mGame->Direct3DDevice(), &vertices[0], mVertexCount, &mVertexBuffer);
	}

	void GeometryShaderDemo::InitializeFixedPoints()
	{
		UINT maxPoints = 10;
		float horizontalOffset = 3.0f;

		std::vector<VertexPositionSize> vertices;
		vertices.reserve(maxPoints);
		for (UINT i = 0; i < maxPoints; i++)
		{
			vertices.push_back(VertexPositionSize(XMFLOAT4(i * horizontalOffset, 0.0f, 0.0f, 1.0f), Vector2Helper::Zero));
		}

		mVertexCount = vertices.size();

		ReleaseObject(mVertexBuffer);
		mMaterial->CreateVertexBuffer(mGame->Direct3DDevice(), &vertices[0], mVertexCount, &mVertexBuffer);
	}
}