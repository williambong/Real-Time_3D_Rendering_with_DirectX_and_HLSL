#include "NormalMappingDemo.h"
#include "NormalMappingMaterial.h"
#include "Game.h"
#include "GameException.h"
#include "VectorHelper.h"
#include "MatrixHelper.h"
#include "ColorHelper.h"
#include "Camera.h"
#include "Model.h"
#include "Mesh.h"
#include "Utility.h"
#include "DirectionalLight.h"
#include "Keyboard.h"
#include <WICTextureLoader.h>
#include "ProxyModel.h"
#include "RenderStateHelper.h"
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <sstream>

namespace Rendering
{
	RTTI_DEFINITIONS(NormalMappingDemo)

	const float NormalMappingDemo::LightModulationRate = UCHAR_MAX;
	const XMFLOAT2 NormalMappingDemo::LightRotationRate = XMFLOAT2(XM_2PI, XM_2PI);

	NormalMappingDemo::NormalMappingDemo(Game& game, Camera& camera)
		: DrawableGameComponent(game, camera), mEffect(nullptr), mMaterial(nullptr), mColorTextureShaderResourceView(nullptr),
		  mNormalMapShaderResourceView(nullptr), mVertexBuffer(nullptr), mVertexCount(0),
		  mKeyboard(nullptr), mAmbientColor(1, 1, 1, 0), mDirectionalLight(nullptr), 
		  mSpecularColor(1.0f, 1.0f, 1.0f, 0.0f), mSpecularPower(25.0f), mWorldMatrix(MatrixHelper::Identity), mProxyModel(nullptr),
		  mRenderStateHelper(nullptr), mSpriteBatch(nullptr), mSpriteFont(nullptr), mTextPosition(0.0f, 40.0f)
	{
	}

	NormalMappingDemo::~NormalMappingDemo()
	{
		DeleteObject(mSpriteFont);
		DeleteObject(mSpriteBatch);
		DeleteObject(mRenderStateHelper);
		DeleteObject(mProxyModel);
		DeleteObject(mDirectionalLight);
		ReleaseObject(mNormalMapShaderResourceView);
		ReleaseObject(mColorTextureShaderResourceView);
		DeleteObject(mMaterial);
		DeleteObject(mEffect);	
		ReleaseObject(mVertexBuffer);
	}

	void NormalMappingDemo::Initialize()
	{
		SetCurrentDirectory(Utility::ExecutableDirectory().c_str());

		// Initialize the material
		mEffect = new Effect(*mGame);
		mEffect->CompileFromFile(L"Content\\Effects\\NormalMapping.fx");

		mMaterial = new NormalMappingMaterial();
		mMaterial->Initialize(*mEffect);

		VertexPositionTextureNormalTangent vertices[] =
		{
			VertexPositionTextureNormalTangent(XMFLOAT4(-0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), Vector3Helper::Backward,Vector3Helper::Right),
			VertexPositionTextureNormalTangent(XMFLOAT4(-0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), Vector3Helper::Backward,Vector3Helper::Right),
			VertexPositionTextureNormalTangent(XMFLOAT4(0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), Vector3Helper::Backward,Vector3Helper::Right),

			VertexPositionTextureNormalTangent(XMFLOAT4(-0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), Vector3Helper::Backward,Vector3Helper::Right),
			VertexPositionTextureNormalTangent(XMFLOAT4(0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), Vector3Helper::Backward,Vector3Helper::Right),
			VertexPositionTextureNormalTangent(XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), Vector3Helper::Backward,Vector3Helper::Right),
		};

		mVertexCount = ARRAYSIZE(vertices);
		mMaterial->CreateVertexBuffer(mGame->Direct3DDevice(), vertices, mVertexCount, &mVertexBuffer);

		std::wstring textureName = L"Content\\Textures\\Blocks_COLOR_RGB.png";
		HRESULT hr = DirectX::CreateWICTextureFromFile(mGame->Direct3DDevice(), mGame->Direct3DDeviceContext(), textureName.c_str(), nullptr, &mColorTextureShaderResourceView);
		if (FAILED(hr))
		{
			throw GameException("CreateWICTextureFromFile() failed.", hr);
		}

		textureName = L"Content\\Textures\\Blocks_NORM.png";
		hr = DirectX::CreateWICTextureFromFile(mGame->Direct3DDevice(), mGame->Direct3DDeviceContext(), textureName.c_str(), nullptr, &mNormalMapShaderResourceView);
		if (FAILED(hr))
		{
			throw GameException("CreateWICTextureFromFile() failed.", hr);
		}

		mDirectionalLight = new DirectionalLight(*mGame);

		mKeyboard = (Keyboard*)mGame->Services().GetService(Keyboard::TypeIdClass());
		assert(mKeyboard != nullptr);

		mProxyModel = new ProxyModel(*mGame, *mCamera, "Content\\Models\\DirectionalLightProxy.obj", 0.5f);
		mProxyModel->Initialize();
		mProxyModel->SetPosition(10.0f, 0.0, 0.0f);
		mProxyModel->ApplyRotation(XMMatrixRotationY(XM_PIDIV2));

		mRenderStateHelper = new RenderStateHelper(*mGame);

		mSpriteBatch = new SpriteBatch(mGame->Direct3DDeviceContext());
		mSpriteFont = new SpriteFont(mGame->Direct3DDevice(), L"Content\\Fonts\\Arial_14_Regular.spritefont");

		XMStoreFloat4x4(&mWorldMatrix, XMMatrixScaling(10.0f, 10.0f, 10.0f));
	}

	void NormalMappingDemo::Update(const GameTime& gameTime)
	{
		UpdateAmbientLight(gameTime);
		UpdateDirectionalLight(gameTime);
		UpdateSpecularLight(gameTime);

		mProxyModel->Update(gameTime);
	}

	void NormalMappingDemo::Draw(const GameTime& gameTime)
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
		mMaterial->LightColor() << mDirectionalLight->ColorVector();
		mMaterial->LightDirection() << mDirectionalLight->DirectionVector();
		mMaterial->ColorTexture() << mColorTextureShaderResourceView;
		mMaterial->NormalMap() << mNormalMapShaderResourceView;
		mMaterial->CameraPosition() << mCamera->PositionVector();
		
		pass->Apply(0, direct3DDeviceContext);

		direct3DDeviceContext->Draw(mVertexCount, 0);

		mProxyModel->Draw(gameTime);

		mRenderStateHelper->SaveAll();
		mSpriteBatch->Begin();

		std::wostringstream helpLabel;
		helpLabel << L"Ambient Intensity (+PgUp/-PgDn): " << mAmbientColor.a << "\n";
		helpLabel << L"Directional Light Intensity (+Home/-End): " << mDirectionalLight->Color().a << "\n";
		helpLabel << L"Specular Power (+Insert/-Delete): " << mSpecularPower << "\n";
		helpLabel << L"Rotate Directional Light (Arrow Keys)\n";

		mSpriteFont->DrawString(mSpriteBatch, helpLabel.str().c_str(), mTextPosition);

		mSpriteBatch->End();
		mRenderStateHelper->RestoreAll();
	}

	void NormalMappingDemo::UpdateSpecularLight(const GameTime& gameTime)
	{
		static float specularIntensity = mSpecularPower;

		if (mKeyboard != nullptr)
		{
			if (mKeyboard->IsKeyDown(DIK_INSERT) && specularIntensity < UCHAR_MAX)
			{
				specularIntensity += LightModulationRate * (float)gameTime.ElapsedGameTime();
				specularIntensity = XMMin<float>(specularIntensity, UCHAR_MAX);

				mSpecularPower = specularIntensity;;
			}

			if (mKeyboard->IsKeyDown(DIK_DELETE) && specularIntensity > 0)
			{
				specularIntensity -= LightModulationRate * (float)gameTime.ElapsedGameTime();
				specularIntensity = XMMax<float>(specularIntensity, 0);

				mSpecularPower = specularIntensity;
			}
		}
	}

	void NormalMappingDemo::UpdateDirectionalLight(const GameTime& gameTime)
	{
		static float directionalIntensity = mDirectionalLight->Color().a;
		float elapsedTime = (float)gameTime.ElapsedGameTime();

		// Update directional light intensity		
		if (mKeyboard->IsKeyDown(DIK_HOME) && directionalIntensity < UCHAR_MAX)
		{
			directionalIntensity += LightModulationRate * elapsedTime;

			XMCOLOR directionalLightColor = mDirectionalLight->Color();
			directionalLightColor.a = (UCHAR)XMMin<float>(directionalIntensity, UCHAR_MAX);
			mDirectionalLight->SetColor(directionalLightColor);
		}
		if (mKeyboard->IsKeyDown(DIK_END) && directionalIntensity > 0)
		{
			directionalIntensity -= LightModulationRate * elapsedTime;

			XMCOLOR directionalLightColor = mDirectionalLight->Color();
			directionalLightColor.a = (UCHAR)XMMax<float>(directionalIntensity, 0.0f);
			mDirectionalLight->SetColor(directionalLightColor);
		}

		// Rotate directional light
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
			XMMATRIX lightRotationAxisMatrix = XMMatrixRotationAxis(mDirectionalLight->RightVector(), rotationAmount.y);
			lightRotationMatrix *= lightRotationAxisMatrix;
		}

		if (rotationAmount.x != 0.0f || rotationAmount.y != 0.0f)
		{
			mDirectionalLight->ApplyRotation(lightRotationMatrix);
			mProxyModel->ApplyRotation(lightRotationMatrix);
		}
	}

	void NormalMappingDemo::UpdateAmbientLight(const GameTime& gameTime)
	{
		static float ambientIntensity = mAmbientColor.a;

		if (mKeyboard != nullptr)
		{
			if (mKeyboard->IsKeyDown(DIK_PGUP) && ambientIntensity < UCHAR_MAX)
			{
				ambientIntensity += LightModulationRate * (float)gameTime.ElapsedGameTime();
				mAmbientColor.a = (UCHAR)XMMin<float>(ambientIntensity, UCHAR_MAX);
			}

			if (mKeyboard->IsKeyDown(DIK_PGDN) && ambientIntensity > 0)
			{
				ambientIntensity -= LightModulationRate * (float)gameTime.ElapsedGameTime();
				mAmbientColor.a = (UCHAR)XMMax<float>(ambientIntensity, 0);
			}
		}
	}
}