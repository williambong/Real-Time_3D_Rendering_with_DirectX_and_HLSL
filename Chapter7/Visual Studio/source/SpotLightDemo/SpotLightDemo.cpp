#include "SpotLightDemo.h"
#include "SpotLightMaterial.h"
#include "Game.h"
#include "GameException.h"
#include "VectorHelper.h"
#include "MatrixHelper.h"
#include "ColorHelper.h"
#include "Camera.h"
#include "Model.h"
#include "Mesh.h"
#include "Utility.h"
#include "SpotLight.h"
#include "Keyboard.h"
#include <WICTextureLoader.h>
#include "ProxyModel.h"
#include "RenderStateHelper.h"
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <sstream>

namespace Rendering
{
	RTTI_DEFINITIONS(SpotLightDemo)

	const float SpotLightDemo::LightModulationRate = UCHAR_MAX;
	const float SpotLightDemo::LightMovementRate = 10.0f;
	const XMFLOAT2 SpotLightDemo::LightRotationRate = XMFLOAT2(XM_2PI, XM_2PI);

	SpotLightDemo::SpotLightDemo(Game& game, Camera& camera)
		: DrawableGameComponent(game, camera), mEffect(nullptr), mMaterial(nullptr), mTextureShaderResourceView(nullptr),
		  mVertexBuffer(nullptr), mVertexCount(0),
		  mKeyboard(nullptr), mAmbientColor(1.0f, 1.0f, 1.0, 0.0f), mSpotLight(nullptr), 
		  mSpecularColor(1.0f, 1.0f, 1.0f, 1.0f), mSpecularPower(25.0f), mWorldMatrix(MatrixHelper::Identity), mProxyModel(nullptr),
		  mRenderStateHelper(nullptr), mSpriteBatch(nullptr), mSpriteFont(nullptr), mTextPosition(0.0f, 40.0f)
	{
	}

	SpotLightDemo::~SpotLightDemo()
	{
		DeleteObject(mSpriteFont);
		DeleteObject(mSpriteBatch);
		DeleteObject(mRenderStateHelper);
		DeleteObject(mProxyModel);
		DeleteObject(mSpotLight);
		ReleaseObject(mTextureShaderResourceView);
		DeleteObject(mMaterial);
		DeleteObject(mEffect);	
		ReleaseObject(mVertexBuffer);
	}

	void SpotLightDemo::Initialize()
	{
		SetCurrentDirectory(Utility::ExecutableDirectory().c_str());

		// Initialize the material
		mEffect = new Effect(*mGame);
		mEffect->CompileFromFile(L"Content\\Effects\\SpotLight.fx");

		mMaterial = new SpotLightMaterial();
		mMaterial->Initialize(*mEffect);

		VertexPositionTextureNormal vertices[] =
        {
			VertexPositionTextureNormal(XMFLOAT4(-0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), Vector3Helper::Backward),
			VertexPositionTextureNormal(XMFLOAT4(-0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), Vector3Helper::Backward),
			VertexPositionTextureNormal(XMFLOAT4(0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), Vector3Helper::Backward),

			VertexPositionTextureNormal(XMFLOAT4(-0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), Vector3Helper::Backward),
			VertexPositionTextureNormal(XMFLOAT4(0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), Vector3Helper::Backward),
			VertexPositionTextureNormal(XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), Vector3Helper::Backward),
        };

		mVertexCount= ARRAYSIZE(vertices);
		mMaterial->CreateVertexBuffer(mGame->Direct3DDevice(), vertices, mVertexCount, &mVertexBuffer);		

		std::wstring textureName = L"Content\\Textures\\Checkerboard.png";
		HRESULT hr = DirectX::CreateWICTextureFromFile(mGame->Direct3DDevice(), mGame->Direct3DDeviceContext(), textureName.c_str(), nullptr, &mTextureShaderResourceView);
		if (FAILED(hr))
		{
			throw GameException("CreateWICTextureFromFile() failed.", hr);
		}

		mSpotLight = new SpotLight(*mGame);
		mSpotLight->SetRadius(50.0f);
		mSpotLight->SetPosition(0.0f, 5.0f, 2.0f);

		mKeyboard = (Keyboard*)mGame->Services().GetService(Keyboard::TypeIdClass());
		assert(mKeyboard != nullptr);

		mProxyModel = new ProxyModel(*mGame, *mCamera, "Content\\Models\\SpotLightProxy.obj", 0.5f);
		mProxyModel->Initialize();
		mProxyModel->ApplyRotation(XMMatrixRotationX(XM_PIDIV2));

		mRenderStateHelper = new RenderStateHelper(*mGame);

		mSpriteBatch = new SpriteBatch(mGame->Direct3DDeviceContext());
		mSpriteFont = new SpriteFont(mGame->Direct3DDevice(), L"Content\\Fonts\\Arial_14_Regular.spritefont");

		XMStoreFloat4x4(&mWorldMatrix, XMMatrixScaling(10.0f, 10.0f, 10.0f));
	}

	void SpotLightDemo::Update(const GameTime& gameTime)
	{
		UpdateAmbientLight(gameTime);
		UpdateSpotLight(gameTime);
		UpdateSpecularLight(gameTime);

		mProxyModel->Update(gameTime);
	}

	void SpotLightDemo::Draw(const GameTime& gameTime)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		Pass* pass = mMaterial->CurrentTechnique()->Passes().at(0);
		ID3D11InputLayout* inputLayout = mMaterial->InputLayouts().at(pass);
		direct3DDeviceContext->IASetInputLayout(inputLayout);
	
		UINT stride = mMaterial->VertexSize();
		UINT offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);

		XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
		XMMATRIX wvp = worldMatrix * mCamera->ViewMatrix() * mCamera->ProjectionMatrix();
		XMVECTOR ambientColor = XMLoadColor(&mAmbientColor);
		XMVECTOR specularColor = XMLoadColor(&mSpecularColor);

		mMaterial->WorldViewProjection() << wvp;
		mMaterial->World() << worldMatrix;
		mMaterial->SpecularColor() << specularColor;
		mMaterial->SpecularPower() << mSpecularPower;
		mMaterial->AmbientColor() << ambientColor;
		mMaterial->LightColor() << mSpotLight->ColorVector();
		mMaterial->LightPosition() << mSpotLight->PositionVector();
		mMaterial->LightLookAt() << mSpotLight->DirectionVector();
		mMaterial->LightRadius() << mSpotLight->Radius();
		mMaterial->SpotLightInnerAngle() << mSpotLight->InnerAngle();
		mMaterial->SpotLightOuterAngle() << mSpotLight->OuterAngle();
		mMaterial->ColorTexture() << mTextureShaderResourceView;
		mMaterial->CameraPosition() << mCamera->PositionVector();		
		
		pass->Apply(0, direct3DDeviceContext);		
		
		direct3DDeviceContext->Draw(mVertexCount, 0);

		mProxyModel->Draw(gameTime);		

		mRenderStateHelper->SaveAll();
		mSpriteBatch->Begin();

		std::wostringstream helpLabel;
		helpLabel << L"Ambient Intensity (+PgUp/-PgDn): " << mAmbientColor.a << "\n";
		helpLabel << L"Spot Light Intensity (+Home/-End): " << mSpotLight->Color().a << "\n";
		helpLabel << L"Specular Power (+Insert/-Delete): " << mSpecularPower << "\n";
		helpLabel << L"Move Spot Light (8/2, 4/6, 3/9)\n";
		helpLabel << L"Rotate Spot Light (Arrow Keys)\n";
		helpLabel << L"Spot Light Radius (+B/-N): " << mSpotLight->Radius() << "\n";
		helpLabel << L"Spot Light Inner Angle(+Z/-X): " << mSpotLight->InnerAngle() << "\n";
		helpLabel << L"Spot Light Outer Angle(+C/-V): " << mSpotLight->OuterAngle() << "\n";

		mSpriteFont->DrawString(mSpriteBatch, helpLabel.str().c_str(), mTextPosition);

		mSpriteBatch->End();
		mRenderStateHelper->RestoreAll();
	}

	void SpotLightDemo::UpdateSpecularLight(const GameTime& gameTime)
	{
		static float specularIntensity = mSpecularPower;

		if (mKeyboard != nullptr)
		{
			if (mKeyboard->IsKeyDown(DIK_INSERT) && specularIntensity < UCHAR_MAX)
			{
				specularIntensity += LightModulationRate * (float)gameTime.ElapsedGameTime();
				mSpecularPower = specularIntensity;
			}

			if (mKeyboard->IsKeyDown(DIK_DELETE) && specularIntensity > 1)
			{
				specularIntensity -= LightModulationRate * (float)gameTime.ElapsedGameTime();
				mSpecularPower = specularIntensity;
			}
		}
	}

	void SpotLightDemo::UpdateSpotLight(const GameTime& gameTime)
	{
		static float spotLightIntensity = mSpotLight->Color().a;
		float elapsedTime = (float)gameTime.ElapsedGameTime();

		// Upddate directional light intensity		
		if (mKeyboard->IsKeyDown(DIK_HOME) && spotLightIntensity < UCHAR_MAX)
		{			
			spotLightIntensity += LightModulationRate * elapsedTime;

			XMCOLOR directionalLightColor = mSpotLight->Color();
			directionalLightColor.a = static_cast<UCHAR>(XMMin<float>(spotLightIntensity, UCHAR_MAX));
			mSpotLight->SetColor(directionalLightColor);
		}
		if (mKeyboard->IsKeyDown(DIK_END) && spotLightIntensity > 0)
		{
			spotLightIntensity -= LightModulationRate * elapsedTime;

			XMCOLOR directionalLightColor = mSpotLight->Color();
			directionalLightColor.a = static_cast<UCHAR>(XMMax<float>(spotLightIntensity, 0));
			mSpotLight->SetColor(directionalLightColor);
		}

		// Move spot light
		XMFLOAT3 movementAmount = Vector3Helper::Zero;
        if (mKeyboard != nullptr)
        {
			if (mKeyboard->IsKeyDown(DIK_NUMPAD4))
            {
                movementAmount.x = -1.0f;
            }

            if (mKeyboard->IsKeyDown(DIK_NUMPAD6))
            {
                movementAmount.x = 1.0f;
            }

			if (mKeyboard->IsKeyDown(DIK_NUMPAD9))
            {
                movementAmount.y = 1.0f;
            }

            if (mKeyboard->IsKeyDown(DIK_NUMPAD3))
            {
                movementAmount.y = -1.0f;
            }

			if (mKeyboard->IsKeyDown(DIK_NUMPAD8))
            {
                movementAmount.z = -1.0f;
            }

            if (mKeyboard->IsKeyDown(DIK_NUMPAD2))
            {
                movementAmount.z = 1.0f;
            }
        }

		XMVECTOR movement = XMLoadFloat3(&movementAmount) * LightMovementRate * elapsedTime;
		mSpotLight->SetPosition(mSpotLight->PositionVector() + movement);
		mProxyModel->SetPosition(mSpotLight->Position());

		// Rotate spot light
		XMFLOAT2 rotationAmount = Vector2Helper::Zero;
		if (mKeyboard->IsKeyDown(DIK_LEFTARROW))
		{
			rotationAmount.x += LightRotationRate.x * elapsedTime;
		}
		if (mKeyboard->IsKeyDown(DIK_RIGHTARROW))
		{
			rotationAmount.x -= LightRotationRate.x * elapsedTime;
		}
		if (mKeyboard->IsKeyDown(DIK_UPARROW))
		{
			rotationAmount.y += LightRotationRate.y * elapsedTime;
		}
		if (mKeyboard->IsKeyDown(DIK_DOWNARROW))
		{
			rotationAmount.y -= LightRotationRate.y * elapsedTime;
		}

		XMMATRIX lightRotationMatrix = XMMatrixIdentity();
		if (rotationAmount.x != 0)
		{
			lightRotationMatrix = XMMatrixRotationY(rotationAmount.x);
		}

		if (rotationAmount.y != 0)
		{
			XMMATRIX lightRotationAxisMatrix = XMMatrixRotationAxis(mSpotLight->RightVector(), rotationAmount.y);
			lightRotationMatrix *= lightRotationAxisMatrix;
		}

		if (rotationAmount.x != Vector2Helper::Zero.x || rotationAmount.y != Vector2Helper::Zero.y)
		{
			mSpotLight->ApplyRotation(lightRotationMatrix);
			mProxyModel->ApplyRotation(lightRotationMatrix);
		}

		// Update the light's radius
		if (mKeyboard->IsKeyDown(DIK_B))
		{
			float radius = mSpotLight->Radius() + LightModulationRate * elapsedTime;
			mSpotLight->SetRadius(radius);
		}

		if (mKeyboard->IsKeyDown(DIK_N))
		{
			float radius = mSpotLight->Radius() - LightModulationRate * elapsedTime;
			radius = max(radius, 0.0f);
			mSpotLight->SetRadius(radius);
		}

		// Update inner and outer angles
		static float innerAngle = mSpotLight->InnerAngle();
		if (mKeyboard->IsKeyDown(DIK_Z) && innerAngle < 1.0f)
		{
			innerAngle += elapsedTime;
			innerAngle = min(innerAngle, 1.0f);

			mSpotLight->SetInnerAngle(innerAngle);
		}
		if (mKeyboard->IsKeyDown(DIK_X) && innerAngle > 0.5f)
		{
			innerAngle -= elapsedTime;
			innerAngle = max(innerAngle, 0.5f);

			mSpotLight->SetInnerAngle(innerAngle);
		}

		static float outerAngle = mSpotLight->OuterAngle();
		if (mKeyboard->IsKeyDown(DIK_C) && outerAngle < 0.5f)
		{
			outerAngle += elapsedTime;
			outerAngle = min(outerAngle, 0.5f);

			mSpotLight->SetOuterAngle(outerAngle);
		}
		if (mKeyboard->IsKeyDown(DIK_V) && outerAngle > 0.0f)
		{
			outerAngle -= elapsedTime;
			outerAngle = max(outerAngle, 0.0f);

			mSpotLight->SetOuterAngle(outerAngle);
		}
	}

	void SpotLightDemo::UpdateAmbientLight(const GameTime& gameTime)
	{
		static float ambientIntensity = mAmbientColor.a;

		if (mKeyboard != nullptr)
		{
			if (mKeyboard->IsKeyDown(DIK_PGUP) && ambientIntensity < UCHAR_MAX)
			{
				ambientIntensity += LightModulationRate * (float)gameTime.ElapsedGameTime();
				mAmbientColor.a = static_cast<UCHAR>(XMMin<float>(ambientIntensity, UCHAR_MAX));
			}

			if (mKeyboard->IsKeyDown(DIK_PGDN) && ambientIntensity > 0)
			{
				ambientIntensity -= LightModulationRate * (float)gameTime.ElapsedGameTime();
				mAmbientColor.a = static_cast<UCHAR>(XMMax<float>(ambientIntensity, 0));
			}
		}
	}
}