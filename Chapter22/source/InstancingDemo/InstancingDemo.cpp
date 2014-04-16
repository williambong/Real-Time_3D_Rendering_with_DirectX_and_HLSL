#include "InstancingDemo.h"
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
#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>
#include "ProxyModel.h"
#include "RenderStateHelper.h"
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <sstream>

namespace Rendering
{
	RTTI_DEFINITIONS(InstancingDemo)

	const float InstancingDemo::LightModulationRate = UCHAR_MAX;
	const float InstancingDemo::LightMovementRate = 10.0f;

	InstancingDemo::InstancingDemo(Game& game, Camera& camera)
		: DrawableGameComponent(game, camera), mEffect(nullptr), mMaterial(nullptr), mColorTexture(nullptr),
		  mVertexBuffers(), mIndexBuffer(nullptr), mIndexCount(0), mInstanceCount(0),
		  mKeyboard(nullptr), mAmbientColor(reinterpret_cast<const float*>(&ColorHelper::White)), mPointLight(nullptr), 
		  mSpecularColor(1.0f, 1.0f, 1.0f, 0.0f), mSpecularPower(25.0f), mProxyModel(nullptr),
		  mRenderStateHelper(nullptr), mSpriteBatch(nullptr), mSpriteFont(nullptr), mTextPosition(0.0f, 40.0f)
	{
	}

	InstancingDemo::~InstancingDemo()
	{
		DeleteObject(mSpriteFont);
		DeleteObject(mSpriteBatch);
		DeleteObject(mRenderStateHelper);
		DeleteObject(mProxyModel);
		DeleteObject(mPointLight);
		ReleaseObject(mColorTexture);
		DeleteObject(mMaterial);
		DeleteObject(mEffect);			
		ReleaseObject(mIndexBuffer);

		for (VertexBufferData& bufferData : mVertexBuffers)
		{
			ReleaseObject(bufferData.VertexBuffer);
		}
	}

	void InstancingDemo::Initialize()
	{
		SetCurrentDirectory(Utility::ExecutableDirectory().c_str());

		std::unique_ptr<Model> model(new Model(*mGame, "Content\\Models\\Sphere.obj", true));

		// Initialize the material
		mEffect = new Effect(*mGame);
		mEffect->LoadCompiledEffect(L"Content\\Effects\\Instancing.cso");
		mMaterial = new InstancingMaterial();
		mMaterial->Initialize(*mEffect);

		// Create vertex buffer
		Mesh* mesh = model->Meshes().at(0);
		ID3D11Buffer* vertexBuffer = nullptr;
		mMaterial->CreateVertexBuffer(mGame->Direct3DDevice(), *mesh, &vertexBuffer);		
		mVertexBuffers.push_back(VertexBufferData(vertexBuffer, mMaterial->VertexSize(), 0));
		
		// Create instance buffer
		std::vector<InstancingMaterial::InstanceData> instanceData;
		UINT axisInstanceCount = 5;
		float offset = 20.0f;
		for (UINT x = 0; x < axisInstanceCount; x++)
		{
			float xPosition = x * offset;

			for (UINT z = 0; z < axisInstanceCount; z++)
			{
				float zPosition = z * offset;

				instanceData.push_back(InstancingMaterial::InstanceData(XMMatrixTranslation(-xPosition, 0, -zPosition), ColorHelper::ToFloat4(mSpecularColor), mSpecularPower));
				instanceData.push_back(InstancingMaterial::InstanceData(XMMatrixTranslation(xPosition, 0, -zPosition), ColorHelper::ToFloat4(mSpecularColor), mSpecularPower));
			}
		}

		ID3D11Buffer* instanceBuffer = nullptr;
		mMaterial->CreateInstanceBuffer(mGame->Direct3DDevice(), instanceData, &instanceBuffer);
		mInstanceCount = instanceData.size();
		mVertexBuffers.push_back(VertexBufferData(instanceBuffer, mMaterial->InstanceSize(), 0));

		// Create index buffer
		mesh->CreateIndexBuffer(&mIndexBuffer);
		mIndexCount = mesh->Indices().size();

		std::wstring textureName = L"Content\\Textures\\EarthComposite.jpg";
		HRESULT hr = DirectX::CreateWICTextureFromFile(mGame->Direct3DDevice(), mGame->Direct3DDeviceContext(), textureName.c_str(), nullptr, &mColorTexture);
		if (FAILED(hr))
		{
			throw GameException("CreateWICTextureFromFile() failed.", hr);
		}

		mPointLight = new PointLight(*mGame);
		mPointLight->SetRadius(100.0f);
		mPointLight->SetPosition(5.0f, 0.0f, 10.0f);

		mKeyboard = (Keyboard*)mGame->Services().GetService(Keyboard::TypeIdClass());
		assert(mKeyboard != nullptr);

		mProxyModel = new ProxyModel(*mGame, *mCamera, "Content\\Models\\PointLightProxy.obj", 0.5f);
		mProxyModel->Initialize();

		mRenderStateHelper = new RenderStateHelper(*mGame);

		mSpriteBatch = new SpriteBatch(mGame->Direct3DDeviceContext());
		mSpriteFont = new SpriteFont(mGame->Direct3DDevice(), L"Content\\Fonts\\Arial_14_Regular.spritefont");
	}

	void InstancingDemo::Update(const GameTime& gameTime)
	{
		UpdateAmbientLight(gameTime);
		UpdatePointLight(gameTime);

		mProxyModel->Update(gameTime);
	}

	void InstancingDemo::Draw(const GameTime& gameTime)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		Pass* pass = mMaterial->CurrentTechnique()->Passes().at(0);
		ID3D11InputLayout* inputLayout = mMaterial->InputLayouts().at(pass);
		direct3DDeviceContext->IASetInputLayout(inputLayout);
	
		ID3D11Buffer* vertexBuffers[2] = { mVertexBuffers[0].VertexBuffer, mVertexBuffers[1].VertexBuffer };
		UINT strides[2] = { mVertexBuffers[0].Stride, mVertexBuffers[1].Stride };
		UINT offsets[2] = { mVertexBuffers[0].Offset, mVertexBuffers[1].Offset };
		
		direct3DDeviceContext->IASetVertexBuffers(0, 2, vertexBuffers, strides, offsets);
		direct3DDeviceContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		mMaterial->ViewProjection() << mCamera->ViewMatrix() * mCamera->ProjectionMatrix();
		mMaterial->AmbientColor() << XMLoadColor(&mAmbientColor);
		mMaterial->LightColor() << mPointLight->ColorVector();
		mMaterial->LightPosition() << mPointLight->PositionVector();
		mMaterial->LightRadius() << mPointLight->Radius();
		mMaterial->ColorTexture() << mColorTexture;
		mMaterial->CameraPosition() << mCamera->PositionVector();
	
		pass->Apply(0, direct3DDeviceContext);		
		
		direct3DDeviceContext->DrawIndexedInstanced(mIndexCount, mInstanceCount, 0, 0, 0);

		mProxyModel->Draw(gameTime);		

		mRenderStateHelper->SaveAll();
		mSpriteBatch->Begin();

		std::wostringstream helpLabel;
		helpLabel << L"Ambient Intensity (+PgUp/-PgDn): " << mAmbientColor.a << "\n";
		helpLabel << L"Point Light Intensity (+Home/-End): " << mPointLight->Color().a << "\n";
		helpLabel << L"Move Point Light (8/2, 4/6, 3/9)\n";

		mSpriteFont->DrawString(mSpriteBatch, helpLabel.str().c_str(), mTextPosition);

		mSpriteBatch->End();
		mRenderStateHelper->RestoreAll();
	}

	void InstancingDemo::UpdatePointLight(const GameTime& gameTime)
	{
		static float pointLightIntensity = mPointLight->Color().a;
		float elapsedTime = (float)gameTime.ElapsedGameTime();

		// Update point light intensity		
		if (mKeyboard->IsKeyDown(DIK_HOME) && pointLightIntensity < UCHAR_MAX)
		{
			pointLightIntensity += LightModulationRate * elapsedTime;

			XMCOLOR pointLightLightColor = mPointLight->Color();
			pointLightLightColor.a = (UCHAR)XMMin<float>(pointLightIntensity, UCHAR_MAX);
			mPointLight->SetColor(pointLightLightColor);
		}
		if (mKeyboard->IsKeyDown(DIK_END) && pointLightIntensity > 0)
		{
			pointLightIntensity -= LightModulationRate * elapsedTime;

			XMCOLOR pointLightLightColor = mPointLight->Color();
			pointLightLightColor.a = (UCHAR)XMMax<float>(pointLightIntensity, 0.0f);
			mPointLight->SetColor(pointLightLightColor);
		}

		// Move point light
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
		mPointLight->SetPosition(mPointLight->PositionVector() + movement);
		mProxyModel->SetPosition(mPointLight->Position());
	}

	void InstancingDemo::UpdateAmbientLight(const GameTime& gameTime)
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