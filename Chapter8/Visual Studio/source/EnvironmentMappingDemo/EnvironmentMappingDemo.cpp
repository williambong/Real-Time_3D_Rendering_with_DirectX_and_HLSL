#include "EnvironmentMappingDemo.h"
#include "EnvironmentMappingMaterial.h"
#include "Game.h"
#include "GameException.h"
#include "VectorHelper.h"
#include "MatrixHelper.h"
#include "ColorHelper.h"
#include "Camera.h"
#include "Model.h"
#include "Mesh.h"
#include "Utility.h"
#include "Keyboard.h"
#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>
#include "RenderStateHelper.h"
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <sstream>

namespace Rendering
{
	RTTI_DEFINITIONS(EnvironmentMappingDemo)

	const XMVECTORF32 EnvironmentMappingDemo::EnvColor = ColorHelper::White;

	EnvironmentMappingDemo::EnvironmentMappingDemo(Game& game, Camera& camera)
		: DrawableGameComponent(game, camera), mEffect(nullptr), mMaterial(nullptr),
		  mColorTextureShaderResourceView(nullptr), mCubeMapShaderResourceView(nullptr),
		  mVertexBuffer(nullptr), mIndexBuffer(nullptr), mIndexCount(0), mReflectionAmount(1.0f),
		  mKeyboard(nullptr), mAmbientColor(1, 1, 1, 1), mWorldMatrix(MatrixHelper::Identity),
		  mRenderStateHelper(nullptr), mSpriteBatch(nullptr), mSpriteFont(nullptr), mTextPosition(0.0f, 40.0f)
	{
	}

	EnvironmentMappingDemo::~EnvironmentMappingDemo()
	{
		DeleteObject(mSpriteFont);
		DeleteObject(mSpriteBatch);
		DeleteObject(mRenderStateHelper);
		ReleaseObject(mColorTextureShaderResourceView);
		ReleaseObject(mCubeMapShaderResourceView);
		DeleteObject(mMaterial);
		DeleteObject(mEffect);	
		ReleaseObject(mVertexBuffer);
		ReleaseObject(mIndexBuffer);
	}

	void EnvironmentMappingDemo::Initialize()
	{
		SetCurrentDirectory(Utility::ExecutableDirectory().c_str());

		std::unique_ptr<Model> model(new Model(*mGame, "Content\\Models\\Sphere.obj", true));

		// Initialize the material
		mEffect = new Effect(*mGame);
		mEffect->CompileFromFile(L"Content\\Effects\\EnvironmentMapping.fx");

		mMaterial = new EnvironmentMappingMaterial();
		mMaterial->Initialize(*mEffect);

		Mesh* mesh = model->Meshes().at(0);
		mMaterial->CreateVertexBuffer(mGame->Direct3DDevice(), *mesh, &mVertexBuffer);
		mesh->CreateIndexBuffer(&mIndexBuffer);
		mIndexCount = mesh->Indices().size();

		std::wstring textureName = L"Content\\Textures\\Maskonaive2_1024.dds";
		HRESULT hr = DirectX::CreateDDSTextureFromFile(mGame->Direct3DDevice(), textureName.c_str(), nullptr, &mCubeMapShaderResourceView);
		if (FAILED(hr))
		{
			throw GameException("CreateDDSTextureFromFile() failed.", hr);
		}

		textureName = L"Content\\Textures\\Checkerboard.png";
		hr = DirectX::CreateWICTextureFromFile(mGame->Direct3DDevice(), mGame->Direct3DDeviceContext(), textureName.c_str(), nullptr, &mColorTextureShaderResourceView);
		if (FAILED(hr))
		{
			throw GameException("CreateWICTextureFromFile() failed.", hr);
		}

		mKeyboard = (Keyboard*)mGame->Services().GetService(Keyboard::TypeIdClass());
		assert(mKeyboard != nullptr);

		mRenderStateHelper = new RenderStateHelper(*mGame);

		mSpriteBatch = new SpriteBatch(mGame->Direct3DDeviceContext());
		mSpriteFont = new SpriteFont(mGame->Direct3DDevice(), L"Content\\Fonts\\Arial_14_Regular.spritefont");
	}

	void EnvironmentMappingDemo::Update(const GameTime& gameTime)
	{
		float elapsedTime = static_cast<float>(gameTime.ElapsedGameTime());

		if (mKeyboard->IsKeyDown(DIK_UP) && mReflectionAmount < 1.0f)
		{
			mReflectionAmount += elapsedTime;
			mReflectionAmount = min(mReflectionAmount, 1.0f);
		}
		if (mKeyboard->IsKeyDown(DIK_DOWN) && mReflectionAmount > 0.0f)
		{
			mReflectionAmount -= elapsedTime;
			mReflectionAmount = max(mReflectionAmount, 0.0f);
		}
	}

	void EnvironmentMappingDemo::Draw(const GameTime& gameTime)
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

		mMaterial->WorldViewProjection() << wvp;
		mMaterial->World() << worldMatrix;
		mMaterial->AmbientColor() << ambientColor;
		mMaterial->EnvColor() << EnvColor;
		mMaterial->ReflectionAmount() << mReflectionAmount;		
		mMaterial->ColorTexture() << mColorTextureShaderResourceView;
		mMaterial->EnvironmentMap() << mCubeMapShaderResourceView;
		mMaterial->CameraPosition() << mCamera->PositionVector();	
		
		pass->Apply(0, direct3DDeviceContext);		
		
		direct3DDeviceContext->DrawIndexed(mIndexCount, 0, 0);

		mRenderStateHelper->SaveAll();
		mSpriteBatch->Begin();

		std::wostringstream helpLabel;
		helpLabel << L"Reflection Amount (+Up/-Down): " << mReflectionAmount << "\n";

		mSpriteFont->DrawString(mSpriteBatch, helpLabel.str().c_str(), mTextPosition);

		mSpriteBatch->End();
		mRenderStateHelper->RestoreAll();
	}
}