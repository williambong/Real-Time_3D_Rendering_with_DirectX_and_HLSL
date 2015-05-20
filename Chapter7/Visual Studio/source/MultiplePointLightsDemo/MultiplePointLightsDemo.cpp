#include "MultiplePointLightsDemo.h"
#include "MultiplePointLightsMaterial.h"
#include "Game.h"
#include "GameException.h"
#include "VectorHelper.h"
#include "MatrixHelper.h"
#include "ColorHelper.h"
#include "Camera.h"
#include "Model.h"
#include "Mesh.h"
#include "Utility.h"
#include "PointLight.h"
#include "Keyboard.h"
#include <DDSTextureLoader.h>
#include "ProxyModel.h"
#include "RenderStateHelper.h"
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <sstream>

namespace Rendering
{
	RTTI_DEFINITIONS(MultiplePointLightsDemo)

	const float MultiplePointLightsDemo::LightModulationRate = UCHAR_MAX;
	const float MultiplePointLightsDemo::LightMovementRate = 10.0f;
	const int MultiplePointLightsDemo::PointLightCount = 4;

	MultiplePointLightsDemo::MultiplePointLightsDemo(Game& game, Camera& camera)
		: DrawableGameComponent(game, camera), mEffect(nullptr), mMaterial(nullptr), mTextureShaderResourceView(nullptr),
		  mVertexBuffer(nullptr), mIndexBuffer(nullptr), mIndexCount(0),
		  mKeyboard(nullptr), mAmbientColor(1, 1, 1, 0), mPointLights(), mProxyModels(),
		  mSpecularColor(1.0f, 1.0f, 1.0f, 1.0f), mSpecularPower(25.0f), mWorldMatrix(MatrixHelper::Identity),
		  mRenderStateHelper(nullptr), mSpriteBatch(nullptr), mSpriteFont(nullptr), mTextPosition(0.0f, 40.0f)
	{
	}

	MultiplePointLightsDemo::~MultiplePointLightsDemo()
	{
		for (PointLight* pointLight : mPointLights)
		{
			DeleteObject(pointLight);
		}

		for (ProxyModel* proxyModel : mProxyModels)
		{
			DeleteObject(proxyModel);
		}

		DeleteObject(mSpriteFont);
		DeleteObject(mSpriteBatch);
		DeleteObject(mRenderStateHelper);		
		ReleaseObject(mTextureShaderResourceView);
		DeleteObject(mMaterial);
		DeleteObject(mEffect);	
		ReleaseObject(mVertexBuffer);
		ReleaseObject(mIndexBuffer);
	}

	void MultiplePointLightsDemo::Initialize()
	{
		SetCurrentDirectory(Utility::ExecutableDirectory().c_str());

		std::unique_ptr<Model> model(new Model(*mGame, "Content\\Models\\Sphere.obj", true));

		// Initialize the material
		mEffect = new Effect(*mGame);
		mEffect->CompileFromFile(L"Content\\Effects\\MultiplePointLights.fx");

		mMaterial = new MultiplePointLightsMaterial();
		mMaterial->Initialize(*mEffect);

		Mesh* mesh = model->Meshes().at(0);
		mMaterial->CreateVertexBuffer(mGame->Direct3DDevice(), *mesh, &mVertexBuffer);
		mesh->CreateIndexBuffer(&mIndexBuffer);
		mIndexCount = mesh->Indices().size();

		std::wstring textureName = L"Content\\Textures\\Earthatday.dds";
		HRESULT hr = DirectX::CreateDDSTextureFromFile(mGame->Direct3DDevice(), textureName.c_str(), nullptr, &mTextureShaderResourceView);
		if (FAILED(hr))
		{
			throw GameException("CreateDDSTextureFromFile() failed.", hr);
		}

		mPointLights.reserve(PointLightCount);
		mProxyModels.reserve(PointLightCount);
		
		// Light 1
		PointLight* pointLight = new PointLight(*mGame);
		pointLight->SetRadius(500.0f);
		pointLight->SetColor(ColorHelper::White);
		pointLight->SetPosition(10.0f, 0.0f, 0.0f);

		ProxyModel* proxyModel = new ProxyModel(*mGame, *mCamera, "Content\\Models\\PointLightProxy.obj", 0.5f);
		proxyModel->Initialize();

		mPointLights.push_back(pointLight);
		mProxyModels.push_back(proxyModel);
		
		// Light 2
		pointLight = new PointLight(*mGame);
		pointLight->SetRadius(500.0f);
		pointLight->SetColor(ColorHelper::White);
		pointLight->SetPosition(-10.0f, 0.0f, 0.0f);

		proxyModel = new ProxyModel(*mGame, *mCamera, "Content\\Models\\PointLightProxy.obj", 0.5f);
		proxyModel->Initialize();

		mPointLights.push_back(pointLight);
		mProxyModels.push_back(proxyModel);

		// Light 3
		pointLight = new PointLight(*mGame);
		pointLight->SetRadius(500.0f);
		pointLight->SetColor(ColorHelper::White);
		pointLight->SetPosition(0.0f, 0.0f, 10.0f);

		proxyModel = new ProxyModel(*mGame, *mCamera, "Content\\Models\\PointLightProxy.obj", 0.5f);
		proxyModel->Initialize();

		mPointLights.push_back(pointLight);
		mProxyModels.push_back(proxyModel);

		// Light 4
		pointLight = new PointLight(*mGame);
		pointLight->SetRadius(500.0f);
		pointLight->SetColor(ColorHelper::White);
		pointLight->SetPosition(0.0f, 0.0f, -10.0f);

		proxyModel = new ProxyModel(*mGame, *mCamera, "Content\\Models\\PointLightProxy.obj", 0.5f);
		proxyModel->Initialize();

		mPointLights.push_back(pointLight);
		mProxyModels.push_back(proxyModel);

		for (std::vector<PointLight*>::size_type i = 0; i < mPointLights.size(); i++)
		{
			mProxyModels[i]->SetPosition(mPointLights[i]->Position());
		}

		mKeyboard = (Keyboard*)mGame->Services().GetService(Keyboard::TypeIdClass());
		assert(mKeyboard != nullptr);

		mRenderStateHelper = new RenderStateHelper(*mGame);

		mSpriteBatch = new SpriteBatch(mGame->Direct3DDeviceContext());
		mSpriteFont = new SpriteFont(mGame->Direct3DDevice(), L"Content\\Fonts\\Arial_14_Regular.spritefont");
	}

	void MultiplePointLightsDemo::Update(const GameTime& gameTime)
	{
		UpdateAmbientLight(gameTime);
		UpdatePointLights(gameTime);
		UpdateSpecularLight(gameTime);

		for (ProxyModel* proxyModel : mProxyModels)
		{
			proxyModel->Update(gameTime);
		}
	}

	void MultiplePointLightsDemo::Draw(const GameTime& gameTime)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		Pass* pass = mMaterial->CurrentTechnique()->Passes().at(0);
		ID3D11InputLayout* inputLayout = mMaterial->InputLayouts().at(pass);
		direct3DDeviceContext->IASetInputLayout(inputLayout);
	
		UINT stride = mMaterial->VertexSize();
		UINT offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
		direct3DDeviceContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
		XMMATRIX wvp = worldMatrix * mCamera->ViewMatrix() * mCamera->ProjectionMatrix();
		XMVECTOR ambientColor = XMLoadColor(&mAmbientColor);
		XMVECTOR specularColor = XMLoadColor(&mSpecularColor);

		mMaterial->WorldViewProjection() << wvp;
		mMaterial->World() << worldMatrix;
		mMaterial->SpecularColor() << specularColor;
		mMaterial->SpecularPower() << mSpecularPower;
		mMaterial->AmbientColor() << ambientColor;
		mMaterial->ColorTexture() << mTextureShaderResourceView;
		mMaterial->CameraPosition() << mCamera->PositionVector();

		for (std::vector<PointLight*>::size_type i = 0; i < mPointLights.size(); i++)
		{
			PointLight* pointLight = mPointLights[i];

			mMaterial->PointLights()[i].Position << pointLight->PositionVector();
			mMaterial->PointLights()[i].LightRadius << pointLight->Radius();
			mMaterial->PointLights()[i].Color << pointLight->ColorVector();
		}

		pass->Apply(0, direct3DDeviceContext);		
		
		direct3DDeviceContext->DrawIndexed(mIndexCount, 0, 0);

		for (ProxyModel* proxyModel : mProxyModels)
		{
			proxyModel->Draw(gameTime);
		}

		mRenderStateHelper->SaveAll();
		mSpriteBatch->Begin();

		std::wostringstream helpLabel;
		helpLabel << L"Ambient Intensity (+PgUp/-PgDn): " << mAmbientColor.a << "\n";
		helpLabel << L"Specular Power (+Insert/-Delete): " << mSpecularPower << "\n";

		mSpriteFont->DrawString(mSpriteBatch, helpLabel.str().c_str(), mTextPosition);

		mSpriteBatch->End();
		mRenderStateHelper->RestoreAll();
	}


	void MultiplePointLightsDemo::UpdatePointLights(const GameTime& gameTime)
	{
	}

	void MultiplePointLightsDemo::UpdateSpecularLight(const GameTime& gameTime)
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

	void MultiplePointLightsDemo::UpdateAmbientLight(const GameTime& gameTime)
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