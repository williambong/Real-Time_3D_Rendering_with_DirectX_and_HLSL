#include "ProjectiveTextureMappingSimpleDemo.h"
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
#include "Projector.h"
#include "RenderableFrustum.h"
#include "ProjectiveTextureMappingMaterial.h"
#include "RenderStateHelper.h"
#include <sstream>
#include <iomanip>
#include <SpriteBatch.h>
#include <SpriteFont.h>

namespace Rendering
{
	RTTI_DEFINITIONS(ProjectiveTextureMappingSimpleDemo)

	const float ProjectiveTextureMappingSimpleDemo::LightModulationRate = UCHAR_MAX;
	const float ProjectiveTextureMappingSimpleDemo::LightMovementRate = 10.0f;
	const XMFLOAT2 ProjectiveTextureMappingSimpleDemo::LightRotationRate = XMFLOAT2(XM_2PI, XM_2PI);

	ProjectiveTextureMappingSimpleDemo::ProjectiveTextureMappingSimpleDemo(Game& game, Camera& camera)
		: DrawableGameComponent(game, camera),
		  mKeyboard(nullptr), mAmbientColor(1.0f, 1.0f, 1.0, 0.0f), mPointLight(nullptr), 
		  mSpecularColor(1.0f, 1.0f, 1.0f, 1.0f), mSpecularPower(25.0f), 
		  mProxyModel(nullptr), mProjector(nullptr), mProjectorFrustum(XMMatrixIdentity()), mRenderableProjectorFrustum(nullptr), mProjectedTexture(nullptr),
		  mProjectiveTextureMappingEffect(nullptr), mProjectiveTextureMappingMaterial(nullptr), mPlaneVertexBuffer(nullptr),
		  mPlaneVertexCount(0), mPlaneWorldMatrix(MatrixHelper::Identity),
		  mProjectedTextureScalingMatrix(MatrixHelper::Zero), mCheckerboardTexture(nullptr), mUseNoReverseTechnique(false),
		  mRenderStateHelper(nullptr), mSpriteBatch(nullptr), mSpriteFont(nullptr), mTextPosition(0.0f, 40.0f)
	{
	}

	ProjectiveTextureMappingSimpleDemo::~ProjectiveTextureMappingSimpleDemo()
	{
		DeleteObject(mSpriteFont);
		DeleteObject(mSpriteBatch);
		DeleteObject(mRenderStateHelper);
		ReleaseObject(mProjectedTexture);
		DeleteObject(mRenderableProjectorFrustum);
		DeleteObject(mProjector);
		DeleteObject(mProjector);
		DeleteObject(mProxyModel);
		DeleteObject(mPointLight);
		ReleaseObject(mCheckerboardTexture);
		DeleteObject(mProjectiveTextureMappingMaterial);
		DeleteObject(mProjectiveTextureMappingEffect);
		ReleaseObject(mPlaneVertexBuffer);
	}

	void ProjectiveTextureMappingSimpleDemo::Initialize()
	{
		SetCurrentDirectory(Utility::ExecutableDirectory().c_str());

		// Initialize material
		mProjectiveTextureMappingEffect = new Effect(*mGame);
		mProjectiveTextureMappingEffect->LoadCompiledEffect(L"Content\\Effects\\ProjectiveTextureMapping.cso");

		mProjectiveTextureMappingMaterial = new ProjectiveTextureMappingMaterial();
		mProjectiveTextureMappingMaterial->Initialize(*mProjectiveTextureMappingEffect);

		// Create vertex buffers
		VertexPositionTextureNormal vertices[] = 
        {
			VertexPositionTextureNormal(XMFLOAT4(-0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), Vector3Helper::Backward),
			VertexPositionTextureNormal(XMFLOAT4(-0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), Vector3Helper::Backward),
			VertexPositionTextureNormal(XMFLOAT4(0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), Vector3Helper::Backward),

			VertexPositionTextureNormal(XMFLOAT4(-0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), Vector3Helper::Backward),
			VertexPositionTextureNormal(XMFLOAT4(0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), Vector3Helper::Backward),
			VertexPositionTextureNormal(XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), Vector3Helper::Backward),
        };

		mPlaneVertexCount = ARRAYSIZE(vertices);	
		mProjectiveTextureMappingMaterial->CreateVertexBuffer(mGame->Direct3DDevice(), vertices, mPlaneVertexCount, &mPlaneVertexBuffer);

		std::wstring textureName = L"Content\\Textures\\Checkerboard.png";
		HRESULT hr = DirectX::CreateWICTextureFromFile(mGame->Direct3DDevice(), mGame->Direct3DDeviceContext(), textureName.c_str(), nullptr, &mCheckerboardTexture);
		if (FAILED(hr))
		{
			throw GameException("CreateWICTextureFromFile() failed.", hr);
		}

		mPointLight = new PointLight(*mGame);
		mPointLight->SetRadius(50.0f);
		mPointLight->SetPosition(0.0f, 5.0f, 2.0f);

		mKeyboard = (Keyboard*)mGame->Services().GetService(Keyboard::TypeIdClass());
		assert(mKeyboard != nullptr);

		mProxyModel = new ProxyModel(*mGame, *mCamera, "Content\\Models\\PointLightProxy.obj", 0.5f);
		mProxyModel->Initialize();

		XMStoreFloat4x4(&mPlaneWorldMatrix, XMMatrixScaling(10.0f, 10.0f, 10.0f));

		mProjector = new Projector(*mGame);
		mProjector->Initialize();

		mProjectorFrustum.SetMatrix(mProjector->ViewProjectionMatrix());

		mRenderableProjectorFrustum = new RenderableFrustum(*mGame, *mCamera);
		mRenderableProjectorFrustum->Initialize();
		mRenderableProjectorFrustum->InitializeGeometry(mProjectorFrustum);

		textureName = L"Content\\Textures\\ProjectedTexture.png";
		if (FAILED(hr = DirectX::CreateWICTextureFromFile(mGame->Direct3DDevice(), mGame->Direct3DDeviceContext(), textureName.c_str(), nullptr, &mProjectedTexture)))
		{
			throw GameException("CreateWICTextureFromFile() failed.", hr);
		}

		ID3D11Resource* projectedTexture = nullptr;
		mProjectedTexture->GetResource(&projectedTexture);

		D3D11_TEXTURE2D_DESC projectedTextureDesc;
		static_cast<ID3D11Texture2D*>(projectedTexture)->GetDesc(&projectedTextureDesc);
		ReleaseObject(projectedTexture);

		InitializeProjectedTextureScalingMatrix(projectedTextureDesc.Width, projectedTextureDesc.Height);

		mRenderStateHelper = new RenderStateHelper(*mGame);

		mSpriteBatch = new SpriteBatch(mGame->Direct3DDeviceContext());
		mSpriteFont = new SpriteFont(mGame->Direct3DDevice(), L"Content\\Fonts\\Arial_14_Regular.spritefont");
	}

	void ProjectiveTextureMappingSimpleDemo::Update(const GameTime& gameTime)
	{
		UpdateTechnique();
		UpdateAmbientLight(gameTime);
		UpdatePointLightAndProjector(gameTime);
		UpdateSpecularLight(gameTime);

		mProxyModel->Update(gameTime);
		mProjector->Update(gameTime);
		mRenderableProjectorFrustum->Update(gameTime);
	}

	void ProjectiveTextureMappingSimpleDemo::Draw(const GameTime& gameTime)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		Pass* pass = mProjectiveTextureMappingMaterial->CurrentTechnique()->Passes().at(0);
		ID3D11InputLayout* inputLayout = mProjectiveTextureMappingMaterial->InputLayouts().at(pass);
		direct3DDeviceContext->IASetInputLayout(inputLayout);

		UINT stride = mProjectiveTextureMappingMaterial->VertexSize();
		UINT offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, &mPlaneVertexBuffer, &stride, &offset);

		XMMATRIX planeWorldMatrix = XMLoadFloat4x4(&mPlaneWorldMatrix);
		XMMATRIX planeWVP = planeWorldMatrix * mCamera->ViewMatrix() * mCamera->ProjectionMatrix();
		XMMATRIX projectiveTextureMatrix = planeWorldMatrix * mProjector->ViewMatrix() * mProjector->ProjectionMatrix() * XMLoadFloat4x4(&mProjectedTextureScalingMatrix);
		XMVECTOR ambientColor = XMLoadColor(&mAmbientColor);
		XMVECTOR specularColor = XMLoadColor(&mSpecularColor);

		mProjectiveTextureMappingMaterial->WorldViewProjection() << planeWVP;
		mProjectiveTextureMappingMaterial->World() << planeWorldMatrix;
		mProjectiveTextureMappingMaterial->SpecularColor() << specularColor;
		mProjectiveTextureMappingMaterial->SpecularPower() << mSpecularPower;
		mProjectiveTextureMappingMaterial->AmbientColor() << ambientColor;
		mProjectiveTextureMappingMaterial->LightColor() << mPointLight->ColorVector();
		mProjectiveTextureMappingMaterial->LightPosition() << mPointLight->PositionVector();
		mProjectiveTextureMappingMaterial->LightRadius() << mPointLight->Radius();
		mProjectiveTextureMappingMaterial->ColorTexture() << mCheckerboardTexture;
		mProjectiveTextureMappingMaterial->CameraPosition() << mCamera->PositionVector();
		mProjectiveTextureMappingMaterial->ProjectiveTextureMatrix() << projectiveTextureMatrix;
		mProjectiveTextureMappingMaterial->ProjectedTexture() << mProjectedTexture;
		
		pass->Apply(0, direct3DDeviceContext);
		
		direct3DDeviceContext->Draw(mPlaneVertexCount, 0);

		mProxyModel->Draw(gameTime);
		mRenderableProjectorFrustum->Draw(gameTime);

		mRenderStateHelper->SaveAll();
		mSpriteBatch->Begin();

		std::wostringstream helpLabel;
		helpLabel << L"Ambient Intensity (+PgUp/-PgDn): " << mAmbientColor.a << "\n";
		helpLabel << L"Point Light Intensity (+Home/-End): " << mPointLight->Color().a << "\n";
		helpLabel << L"Specular Power (+Insert/-Delete): " << mSpecularPower << "\n";
		helpLabel << L"Move Projector/Light (8/2, 4/6, 3/9)\n";
		helpLabel << L"Rotate Projector (Arrow Keys)\n";
		helpLabel << L"Reverse Projection? (Space): " << (mUseNoReverseTechnique ? "Off" : "On");

		mSpriteFont->DrawString(mSpriteBatch, helpLabel.str().c_str(), mTextPosition);

		mSpriteBatch->End();
		mRenderStateHelper->RestoreAll();
	}

	void ProjectiveTextureMappingSimpleDemo::UpdateTechnique()
	{
		if (mKeyboard != nullptr && mKeyboard->WasKeyPressedThisFrame(DIK_SPACE))
		{
			mUseNoReverseTechnique = !mUseNoReverseTechnique;
			std::string techniqueName = (mUseNoReverseTechnique ? "project_texture_no_reverse" : "project_texture");
			mProjectiveTextureMappingMaterial->SetCurrentTechnique(*mProjectiveTextureMappingMaterial->GetEffect()->TechniquesByName().at(techniqueName));
		}
	}

	void ProjectiveTextureMappingSimpleDemo::UpdateAmbientLight(const GameTime& gameTime)
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

	void ProjectiveTextureMappingSimpleDemo::UpdateSpecularLight(const GameTime& gameTime)
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

	void ProjectiveTextureMappingSimpleDemo::UpdatePointLightAndProjector(const GameTime& gameTime)
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

		// Move point light and projector
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
		mProjector->SetPosition(mPointLight->Position());
		mRenderableProjectorFrustum->SetPosition(mPointLight->Position());

		// Rotate projector
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

		XMMATRIX projectorRotationMatrix = XMMatrixIdentity();
		if (rotationAmount.x != 0)
		{
			projectorRotationMatrix = XMMatrixRotationY(rotationAmount.x);
		}

		if (rotationAmount.y != 0)
		{
			XMMATRIX projectorRotationAxisMatrix = XMMatrixRotationAxis(mProjector->RightVector(), rotationAmount.y);
			projectorRotationMatrix *= projectorRotationAxisMatrix;
		}

		if (rotationAmount.x != Vector2Helper::Zero.x || rotationAmount.y != Vector2Helper::Zero.y)
		{
			mProjector->ApplyRotation(projectorRotationMatrix);
			mRenderableProjectorFrustum->ApplyRotation(projectorRotationMatrix);
		}
	}

	void ProjectiveTextureMappingSimpleDemo::InitializeProjectedTextureScalingMatrix(UINT textureWidth, UINT textureHeight)
	{
		float scalingBiasX = 0.5f + (0.5f / textureWidth);
		float scalingBiasY = 0.5f + (0.5f / textureHeight);

		mProjectedTextureScalingMatrix._11 = 0.5f;
		mProjectedTextureScalingMatrix._22 = -0.5f;		
		mProjectedTextureScalingMatrix._33 = 1.0f;
		mProjectedTextureScalingMatrix._41 = scalingBiasX;
		mProjectedTextureScalingMatrix._42 = scalingBiasY;
		mProjectedTextureScalingMatrix._44 = 1.0f;
	}
}