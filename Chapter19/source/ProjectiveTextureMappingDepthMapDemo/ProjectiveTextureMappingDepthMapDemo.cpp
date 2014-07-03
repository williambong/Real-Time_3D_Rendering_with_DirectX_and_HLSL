#include "ProjectiveTextureMappingDepthMapDemo.h"
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
#include "DepthMapMaterial.h"
#include "DepthMap.h"
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <sstream>
#include <iomanip>

namespace Rendering
{
	RTTI_DEFINITIONS(ProjectiveTextureMappingDepthMapDemo)

	const float ProjectiveTextureMappingDepthMapDemo::DepthBiasModulationRate = 0.01f;
	const float ProjectiveTextureMappingDepthMapDemo::LightModulationRate = UCHAR_MAX;
	const float ProjectiveTextureMappingDepthMapDemo::LightMovementRate = 10.0f;
	const XMFLOAT2 ProjectiveTextureMappingDepthMapDemo::LightRotationRate = XMFLOAT2(XM_2PI, XM_2PI);
	const UINT ProjectiveTextureMappingDepthMapDemo::DepthMapWidth = 1024U;
	const UINT ProjectiveTextureMappingDepthMapDemo::DepthMapHeight = 1024U;
	const RECT ProjectiveTextureMappingDepthMapDemo::DepthMapDestinationRectangle = { 0, 512, 256, 768 };

	ProjectiveTextureMappingDepthMapDemo::ProjectiveTextureMappingDepthMapDemo(Game& game, Camera& camera)
		: DrawableGameComponent(game, camera), mCheckerboardTexture(nullptr),
		  mPlanePositionVertexBuffer(nullptr), mPlanePositionUVNormalVertexBuffer(nullptr), mPlaneIndexBuffer(nullptr), mPlaneVertexCount(0),
		  mKeyboard(nullptr), mAmbientColor(1.0f, 1.0f, 1.0, 0.0f), mPointLight(nullptr), 
		  mSpecularColor(1.0f, 1.0f, 1.0f, 1.0f), mSpecularPower(25.0f), mPlaneWorldMatrix(MatrixHelper::Identity), mProxyModel(nullptr),
		  mProjector(nullptr), mProjectorFrustum(XMMatrixIdentity()), mRenderableProjectorFrustum(nullptr), mProjectedTexture(nullptr),
		  mProjectiveTextureMappingEffect(nullptr), mProjectiveTextureMappingMaterial(nullptr),
		  mProjectedTextureScalingMatrix(MatrixHelper::Zero), mRenderStateHelper(game),
		  mModelPositionVertexBuffer(nullptr), mModelPositionUVNormalVertexBuffer(nullptr), mModelIndexBuffer(nullptr), mModelIndexCount(0),
		  mModelWorldMatrix(MatrixHelper::Identity), mDepthMapEffect(nullptr), mDepthMapMaterial(nullptr), mDepthMap(nullptr), mDrawDepthMap(true),
		  mSpriteBatch(nullptr), mDepthBias(0.0005f), mSpriteFont(nullptr), mTextPosition(0.0f, 40.0f)
	{
	}

	ProjectiveTextureMappingDepthMapDemo::~ProjectiveTextureMappingDepthMapDemo()
	{
		DeleteObject(mSpriteFont);
		DeleteObject(mSpriteBatch);
		DeleteObject(mDepthMap);
		DeleteObject(mDepthMapMaterial);
		DeleteObject(mDepthMapEffect);
		ReleaseObject(mModelIndexBuffer);
		ReleaseObject(mModelPositionUVNormalVertexBuffer);
		ReleaseObject(mModelPositionVertexBuffer);
		DeleteObject(mProjectiveTextureMappingMaterial);
		DeleteObject(mProjectiveTextureMappingEffect);
		ReleaseObject(mProjectedTexture);
		DeleteObject(mRenderableProjectorFrustum);
		DeleteObject(mProjector);
		DeleteObject(mProxyModel);
		DeleteObject(mPointLight);
		ReleaseObject(mCheckerboardTexture);
		ReleaseObject(mPlanePositionUVNormalVertexBuffer);
		ReleaseObject(mPlanePositionVertexBuffer);
		ReleaseObject(mPlaneIndexBuffer);
	}
	
	void ProjectiveTextureMappingDepthMapDemo::Initialize()
	{
		SetCurrentDirectory(Utility::ExecutableDirectory().c_str());

		// Initialize materials
		mProjectiveTextureMappingEffect = new Effect(*mGame);
		mProjectiveTextureMappingEffect->LoadCompiledEffect(L"Content\\Effects\\ProjectiveTextureMapping.cso");

		mProjectiveTextureMappingMaterial = new ProjectiveTextureMappingMaterial();
		mProjectiveTextureMappingMaterial->Initialize(*mProjectiveTextureMappingEffect);
		mProjectiveTextureMappingMaterial->SetCurrentTechnique(*mProjectiveTextureMappingMaterial->GetEffect()->TechniquesByName().at("project_texture_w_depthmap"));

		mDepthMapEffect = new Effect(*mGame);
		mDepthMapEffect->LoadCompiledEffect(L"Content\\Effects\\DepthMap.cso");

		mDepthMapMaterial = new DepthMapMaterial();
		mDepthMapMaterial->Initialize(*mDepthMapEffect);

		// Plane vertex buffers
		VertexPositionTextureNormal positionUVNormalVertices[] = 
        {
			VertexPositionTextureNormal(XMFLOAT4(-0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), Vector3Helper::Backward),
			VertexPositionTextureNormal(XMFLOAT4(-0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), Vector3Helper::Backward),
			VertexPositionTextureNormal(XMFLOAT4(0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), Vector3Helper::Backward),

			VertexPositionTextureNormal(XMFLOAT4(-0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), Vector3Helper::Backward),
			VertexPositionTextureNormal(XMFLOAT4(0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), Vector3Helper::Backward),
			VertexPositionTextureNormal(XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), Vector3Helper::Backward),
        };

		mPlaneVertexCount = ARRAYSIZE(positionUVNormalVertices);
		std::vector<VertexPositionNormal> positionNormalVertices;
		positionNormalVertices.reserve(mPlaneVertexCount);
		std::vector<VertexPosition> positionVertices;
		positionVertices.reserve(mPlaneVertexCount);
		for (UINT i = 0; i < mPlaneVertexCount; i++)
		{
			positionNormalVertices.push_back(VertexPositionNormal(positionUVNormalVertices[i].Position, positionUVNormalVertices[i].Normal));
			positionVertices.push_back(VertexPosition(positionUVNormalVertices[i].Position));
		}
		
		mDepthMapMaterial->CreateVertexBuffer(mGame->Direct3DDevice(), &positionVertices[0], mPlaneVertexCount, &mPlanePositionVertexBuffer);
		mProjectiveTextureMappingMaterial->CreateVertexBuffer(mGame->Direct3DDevice(), positionUVNormalVertices, mPlaneVertexCount, &mPlanePositionUVNormalVertexBuffer);

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

		InitializeProjectedTextureScalingMatrix();

		// Vertex and index buffers for a second model to render
		std::unique_ptr<Model> model(new Model(*mGame, "Content\\Models\\teapot.obj", true));

		Mesh* mesh = model->Meshes().at(0);
		mDepthMapMaterial->CreateVertexBuffer(mGame->Direct3DDevice(), *mesh, &mModelPositionVertexBuffer);
		mProjectiveTextureMappingMaterial->CreateVertexBuffer(mGame->Direct3DDevice(), *mesh, &mModelPositionUVNormalVertexBuffer);
		mesh->CreateIndexBuffer(&mModelIndexBuffer);
		mModelIndexCount = mesh->Indices().size();
		
		XMStoreFloat4x4(&mModelWorldMatrix, XMMatrixScaling(0.1f, 0.1f, 0.1f) * XMMatrixTranslation(0.0f, 5.0f, 2.5f));		

		mDepthMap = new DepthMap(*mGame, DepthMapWidth, DepthMapHeight);
		mSpriteBatch = new SpriteBatch(mGame->Direct3DDeviceContext());
		mSpriteFont = new SpriteFont(mGame->Direct3DDevice(), L"Content\\Fonts\\Arial_14_Regular.spritefont");
	}

	void ProjectiveTextureMappingDepthMapDemo::Update(const GameTime& gameTime)
	{
		if (mKeyboard != nullptr && mKeyboard->WasKeyPressedThisFrame(DIK_SPACE))
		{
			mDrawDepthMap = !mDrawDepthMap;
		}

		UpdateDepthBias(gameTime);
		UpdateAmbientLight(gameTime);
		UpdatePointLightAndProjector(gameTime);
		UpdateSpecularLight(gameTime);

		mProxyModel->Update(gameTime);
		mProjector->Update(gameTime);
		mRenderableProjectorFrustum->Update(gameTime);
	}

	void ProjectiveTextureMappingDepthMapDemo::Draw(const GameTime& gameTime)
	{
		// Depth map pass (render the teapot model only)
		mRenderStateHelper.SaveRasterizerState();
		mDepthMap->Begin();

		ID3D11DeviceContext* direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		direct3DDeviceContext->ClearDepthStencilView(mDepthMap->DepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		Pass* pass = mDepthMapMaterial->CurrentTechnique()->Passes().at(0);
		ID3D11InputLayout* inputLayout = mDepthMapMaterial->InputLayouts().at(pass);
		direct3DDeviceContext->IASetInputLayout(inputLayout);

		UINT stride = mDepthMapMaterial->VertexSize();
		UINT offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, &mModelPositionVertexBuffer, &stride, &offset);
		direct3DDeviceContext->IASetIndexBuffer(mModelIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		XMMATRIX modelWorldMatrix = XMLoadFloat4x4(&mModelWorldMatrix);
		mDepthMapMaterial->WorldLightViewProjection() << modelWorldMatrix * mProjector->ViewMatrix() * mProjector->ProjectionMatrix();

		pass->Apply(0, direct3DDeviceContext);
		
		direct3DDeviceContext->DrawIndexed(mModelIndexCount, 0, 0);		

		mDepthMap->End();
		mRenderStateHelper.RestoreRasterizerState();

		// Projective texture mapping pass
		pass = mProjectiveTextureMappingMaterial->CurrentTechnique()->Passes().at(0);		
		inputLayout = mProjectiveTextureMappingMaterial->InputLayouts().at(pass);
		direct3DDeviceContext->IASetInputLayout(inputLayout);

		// Draw plane
		stride = mProjectiveTextureMappingMaterial->VertexSize();
		direct3DDeviceContext->IASetVertexBuffers(0, 1, &mPlanePositionUVNormalVertexBuffer, &stride, &offset);
		direct3DDeviceContext->IASetIndexBuffer(mPlaneIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

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
		mProjectiveTextureMappingMaterial->DepthMap() << mDepthMap->OutputTexture();
		mProjectiveTextureMappingMaterial->DepthBias() << mDepthBias;
		
		pass->Apply(0, direct3DDeviceContext);
		
		direct3DDeviceContext->Draw(mPlaneVertexCount, 0);
		mGame->UnbindPixelShaderResources(0, 3);

		// Draw teapot model
		direct3DDeviceContext->IASetVertexBuffers(0, 1, &mModelPositionUVNormalVertexBuffer, &stride, &offset);
		direct3DDeviceContext->IASetIndexBuffer(mModelIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		
		XMMATRIX modelWVP = modelWorldMatrix * mCamera->ViewMatrix() * mCamera->ProjectionMatrix();
		projectiveTextureMatrix = modelWorldMatrix * mProjector->ViewMatrix() * mProjector->ProjectionMatrix() * XMLoadFloat4x4(&mProjectedTextureScalingMatrix);

		mProjectiveTextureMappingMaterial->WorldViewProjection() << modelWVP;
		mProjectiveTextureMappingMaterial->World() << modelWorldMatrix;
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
		mProjectiveTextureMappingMaterial->DepthMap() << mDepthMap->OutputTexture();
		mProjectiveTextureMappingMaterial->DepthBias() << mDepthBias;
		
		pass->Apply(0, direct3DDeviceContext);
		
		direct3DDeviceContext->DrawIndexed(mModelIndexCount, 0, 0);
		mGame->UnbindPixelShaderResources(0, 3);

		mProxyModel->Draw(gameTime);
		mRenderableProjectorFrustum->Draw(gameTime);
		
		mRenderStateHelper.SaveAll();
		mSpriteBatch->Begin();

		if (mDrawDepthMap)
		{
			mSpriteBatch->Draw(mDepthMap->OutputTexture(), DepthMapDestinationRectangle);
		}

		std::wostringstream helpLabel;
		helpLabel << L"Ambient Intensity (+PgUp/-PgDn): " << mAmbientColor.a << "\n";
		helpLabel << L"Point Light Intensity (+Home/-End): " << mPointLight->Color().a << "\n";
		helpLabel << L"Specular Power (+Insert/-Delete): " << mSpecularPower << "\n";
		helpLabel << L"Move Projector/Light (8/2, 4/6, 3/9)\n";
		helpLabel << L"Rotate Projector (Arrow Keys)\n";
		helpLabel << L"Show Depth Map (Space): " << (mDrawDepthMap ? "Yes" : "No") << "\n";
		helpLabel << std::setprecision(5) << L"Depth Bias Amount (+O/-P): " << mDepthBias;
		mSpriteFont->DrawString(mSpriteBatch, helpLabel.str().c_str(), mTextPosition);

		mSpriteBatch->End();
		mRenderStateHelper.RestoreAll();
	}

	void ProjectiveTextureMappingDepthMapDemo::UpdateDepthBias(const GameTime& gameTime)
	{
		if (mKeyboard != nullptr)
		{
			if (mKeyboard->IsKeyDown(DIK_O) && mDepthBias < 1.0f)
			{
				mDepthBias += DepthBiasModulationRate * (float)gameTime.ElapsedGameTime();
				mDepthBias = XMMin<float>(mDepthBias, 1.0f);
			}

			if (mKeyboard->IsKeyDown(DIK_P) && mDepthBias > 0)
			{
				mDepthBias -= DepthBiasModulationRate * (float)gameTime.ElapsedGameTime();
				mDepthBias = XMMax<float>(mDepthBias, 0);
			}
		}
	}

	void ProjectiveTextureMappingDepthMapDemo::UpdateAmbientLight(const GameTime& gameTime)
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

	void ProjectiveTextureMappingDepthMapDemo::UpdateSpecularLight(const GameTime& gameTime)
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

	void ProjectiveTextureMappingDepthMapDemo::UpdatePointLightAndProjector(const GameTime& gameTime)
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

	void ProjectiveTextureMappingDepthMapDemo::InitializeProjectedTextureScalingMatrix()
	{
		mProjectedTextureScalingMatrix._11 = 0.5f;
		mProjectedTextureScalingMatrix._22 = -0.5f;		
		mProjectedTextureScalingMatrix._33 = 1.0f;
		mProjectedTextureScalingMatrix._41 = 0.5f;
		mProjectedTextureScalingMatrix._42 = 0.5f;
		mProjectedTextureScalingMatrix._44 = 1.0f;
	}
}