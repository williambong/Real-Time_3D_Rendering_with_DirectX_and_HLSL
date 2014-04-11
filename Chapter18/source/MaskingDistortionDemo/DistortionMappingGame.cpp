#include "DistortionMappingGame.h"
#include "GameException.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "FpsComponent.h"
#include "ColorHelper.h"
#include "FirstPersonCamera.h"
#include "RenderStateHelper.h"
#include "RasterizerStates.h"
#include "SamplerStates.h"
#include "Skybox.h"
#include "Grid.h"
#include "Utility.h"
#include "MatrixHelper.h"
#include "FullScreenRenderTarget.h"
#include "FullScreenQuad.h"
#include <sstream>
#include <iomanip>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <WICTextureLoader.h>
#include "DistortionMappingMaterial.h"
#include "Model.h"
#include "Mesh.h"

namespace Rendering
{
    const XMVECTORF32 DistortionMappingGame::BackgroundColor = ColorHelper::CornflowerBlue;
	const XMVECTORF32 DistortionMappingGame::CutoutZeroColor = { 0.0f, 0.0f, 0.0f, 0.0f };

    DistortionMappingGame::DistortionMappingGame(HINSTANCE instance, const std::wstring& windowClass, const std::wstring& windowTitle, int showCommand)
        :  Game(instance, windowClass, windowTitle, showCommand),
           mFpsComponent(nullptr), mGrid(nullptr),
           mDirectInput(nullptr), mKeyboard(nullptr), mMouse(nullptr), mRenderStateHelper(nullptr), mSkybox(nullptr),
           mSceneRenderTarget(nullptr), mCutoutRenderTarget(nullptr), mFullScreenQuad(nullptr), mDistortionMappingMaterial(nullptr), mDistortionMap(nullptr),
		   mVertexBuffer(nullptr), mIndexBuffer(nullptr), mIndexCount(0), mWorldMatrix(MatrixHelper::Identity),
		   mDistortionCutoutPass(nullptr), mDisplacementScale(1.0f), mDrawDistortionCutout(false),
		   mSpriteBatch(nullptr), mSpriteFont(nullptr), mTextPosition(0.0f, 40.0f)
    {
        mDepthStencilBufferEnabled = true;
        mMultiSamplingEnabled = true;
    }

    DistortionMappingGame::~DistortionMappingGame()
    {
    }

    void DistortionMappingGame::Initialize()
    {
        if (FAILED(DirectInput8Create(mInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*)&mDirectInput, nullptr)))
        {
            throw GameException("DirectInput8Create() failed");
        }

        mKeyboard = new Keyboard(*this, mDirectInput);
        mComponents.push_back(mKeyboard);
        mServices.AddService(Keyboard::TypeIdClass(), mKeyboard);
    
        mMouse = new Mouse(*this, mDirectInput);
        mComponents.push_back(mMouse);
        mServices.AddService(Mouse::TypeIdClass(), mMouse);

        mCamera = new FirstPersonCamera(*this);
        mComponents.push_back(mCamera);
        mServices.AddService(Camera::TypeIdClass(), mCamera);
    
        mFpsComponent = new FpsComponent(*this);
		mFpsComponent->Initialize();

		mSkybox = new Skybox(*this, *mCamera, L"Content\\Textures\\Maskonaive2_1024.dds", 500.0f);
		mComponents.push_back(mSkybox);

		mGrid = new Grid(*this, *mCamera);
		mComponents.push_back(mGrid);

		RasterizerStates::Initialize(mDirect3DDevice);
		SamplerStates::BorderColor = ColorHelper::Black;
		SamplerStates::Initialize(mDirect3DDevice);
		
		mRenderStateHelper = new RenderStateHelper(*this);

		mSceneRenderTarget = new FullScreenRenderTarget(*this);
		mCutoutRenderTarget = new FullScreenRenderTarget(*this);

		SetCurrentDirectory(Utility::ExecutableDirectory().c_str());
		mDistortionMappingEffect = new Effect(*this);
		mDistortionMappingEffect->LoadCompiledEffect(L"Content\\Effects\\Distortion.cso");

		mDistortionMappingMaterial = new DistortionMappingMaterial();
		mDistortionMappingMaterial->Initialize(*mDistortionMappingEffect);

		mFullScreenQuad = new FullScreenQuad(*this, *mDistortionMappingMaterial);
		mFullScreenQuad->Initialize();		

		mSpriteBatch = new SpriteBatch(mDirect3DDeviceContext);
        mSpriteFont = new SpriteFont(mDirect3DDevice, L"Content\\Fonts\\Arial_14_Regular.spritefont");

		std::wstring textureName = L"Content\\Textures\\DistortionGlass.png";
		HRESULT hr = DirectX::CreateWICTextureFromFile(mDirect3DDevice, mDirect3DDeviceContext, textureName.c_str(), nullptr, &mDistortionMap);
		if (FAILED(hr))
		{
			throw GameException("CreateWICTextureFromFile() failed.", hr);
		}

		// Load the model
		std::unique_ptr<Model> model(new Model(*this, "Content\\Models\\sphere.obj", true));

		Mesh* mesh = model->Meshes().at(0);
		mDistortionMappingMaterial->CreateVertexBuffer(mDirect3DDevice, *mesh, &mVertexBuffer);
		mesh->CreateIndexBuffer(&mIndexBuffer);
		mIndexCount = mesh->Indices().size();

		mDistortionCutoutPass = mDistortionMappingMaterial->GetEffect()->TechniquesByName().at("displacement_cutout")->PassesByName().at("p0");

        Game::Initialize();

		mCamera->SetPosition(0.0f, 0.0f, 25.0f);
    }

    void DistortionMappingGame::Shutdown()
    {	
		DeleteObject(mSpriteFont);
        DeleteObject(mSpriteBatch);

		ReleaseObject(mVertexBuffer);
		ReleaseObject(mIndexBuffer);

		ReleaseObject(mDistortionMap);
		DeleteObject(mDistortionMappingMaterial);
		DeleteObject(mDistortionMappingEffect);
		DeleteObject(mFullScreenQuad);
		DeleteObject(mCutoutRenderTarget);
		DeleteObject(mSceneRenderTarget);

		DeleteObject(mRenderStateHelper);
		DeleteObject(mKeyboard);
        DeleteObject(mMouse);
		DeleteObject(mSkybox);
		DeleteObject(mGrid);
        DeleteObject(mFpsComponent);
        DeleteObject(mCamera);

        ReleaseObject(mDirectInput);
		RasterizerStates::Release();
		SamplerStates::Release();

        Game::Shutdown();
    }

    void DistortionMappingGame::Update(const GameTime &gameTime)
    {
		mFpsComponent->Update(gameTime);

        if (mKeyboard->WasKeyPressedThisFrame(DIK_ESCAPE))
        {
            Exit();
        }

		if (mKeyboard->WasKeyPressedThisFrame(DIK_G))
		{
			mGrid->SetVisible(!mGrid->Visible());
		}

		UpdateDisplacementScale(gameTime);

		if (mKeyboard->WasKeyPressedThisFrame(DIK_SPACE))
		{
			mDrawDistortionCutout = !mDrawDistortionCutout;
		}

        Game::Update(gameTime);
    }

    void DistortionMappingGame::Draw(const GameTime &gameTime)
    {
		// Draw un-distorted background objects
		mSceneRenderTarget->Begin();
		
		mDirect3DDeviceContext->ClearRenderTargetView(mSceneRenderTarget->RenderTargetView() , reinterpret_cast<const float*>(&BackgroundColor));
        mDirect3DDeviceContext->ClearDepthStencilView(mSceneRenderTarget->DepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		Game::Draw(gameTime);

		mSceneRenderTarget->End();

		// Create distortion map "cutout"
		mCutoutRenderTarget->Begin();
		
		mDirect3DDeviceContext->ClearRenderTargetView(mCutoutRenderTarget->RenderTargetView() , reinterpret_cast<const float*>(&CutoutZeroColor));
        mDirect3DDeviceContext->ClearDepthStencilView(mCutoutRenderTarget->DepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		DrawMeshForDistortionCutout();

		mCutoutRenderTarget->End();

		// Final Draw
		mDirect3DDeviceContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&BackgroundColor));
		mDirect3DDeviceContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		if (mDrawDistortionCutout)
		{
			// Draw distortion map cutout
			mFullScreenQuad->SetActiveTechnique("no_distortion", "p0");
			mFullScreenQuad->SetCustomUpdateMaterial(std::bind(&DistortionMappingGame::UpdateDrawDistortionCutoutMaterial, this));
			mFullScreenQuad->Draw(gameTime);
		}
		else
		{
			// Draw composited scene texture with distortion map cutout	
			mFullScreenQuad->SetActiveTechnique("distortion_composite", "p0");
			mFullScreenQuad->SetCustomUpdateMaterial(std::bind(&DistortionMappingGame::UpdateDistortionCompositeMaterial, this));
			mFullScreenQuad->Draw(gameTime);
		}

		UnbindPixelShaderResources(0, 2);

		mRenderStateHelper->SaveAll();
		mFpsComponent->Draw(gameTime);

		mSpriteBatch->Begin();

        std::wostringstream helpLabel;
		helpLabel << std::setprecision(2) << L"Displacement Scale (+Common/-Period): " << mDisplacementScale << "\n"
		          << std::setprecision(2) << L"Display Mode (Space): " << (mDrawDistortionCutout ? "Distortion Mask" : "Normal");
		
        mSpriteFont->DrawString(mSpriteBatch, helpLabel.str().c_str(), mTextPosition);

        mSpriteBatch->End();

		mRenderStateHelper->RestoreAll();

		HRESULT hr = mSwapChain->Present(0, 0);
        if (FAILED(hr))
        {
            throw GameException("IDXGISwapChain::Present() failed.", hr);
        }
    }

	void DistortionMappingGame::DrawMeshForDistortionCutout()
    {
        mDirect3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        
        ID3D11InputLayout* inputLayout = mDistortionMappingMaterial->InputLayouts().at(mDistortionCutoutPass);
        mDirect3DDeviceContext->IASetInputLayout(inputLayout);

        UINT stride = mDistortionMappingMaterial->VertexSize();
        UINT offset = 0;
        mDirect3DDeviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);		
        mDirect3DDeviceContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
        
		XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
		XMMATRIX wvp = worldMatrix * mCamera->ViewMatrix() * mCamera->ProjectionMatrix();
        mDistortionMappingMaterial->WorldViewProjection() << wvp;
		mDistortionMappingMaterial->DistortionMap() << mDistortionMap;
		mDistortionMappingMaterial->Time() << static_cast<float>(mGameTime.TotalGameTime());

        mDistortionCutoutPass->Apply(0, mDirect3DDeviceContext);

        mDirect3DDeviceContext->DrawIndexed(mIndexCount, 0, 0);
    }

	void DistortionMappingGame::UpdateDistortionCompositeMaterial()
	{
		mDistortionMappingMaterial->SceneTexture() << mSceneRenderTarget->OutputTexture();
		mDistortionMappingMaterial->DistortionMap() << mCutoutRenderTarget->OutputTexture();
		mDistortionMappingMaterial->DisplacementScale() << mDisplacementScale;
	}

	void DistortionMappingGame::UpdateDisplacementScale(const GameTime& gameTime)
	{
		if (mKeyboard != nullptr)
		{
			if (mKeyboard->IsKeyDown(DIK_COMMA) && mDisplacementScale < 3.0f)
			{
				mDisplacementScale += (float)gameTime.ElapsedGameTime();
				if (mDisplacementScale >= UCHAR_MAX)
				{
					mDisplacementScale = UCHAR_MAX;
				}
			}

			if (mKeyboard->IsKeyDown(DIK_PERIOD) && mDisplacementScale > 0)
			{
				mDisplacementScale -= (float)gameTime.ElapsedGameTime();
				if (mDisplacementScale <= 0.0f)
				{
					mDisplacementScale = 0.0f;
				}
			}
		}
	}

	void DistortionMappingGame::UpdateDrawDistortionCutoutMaterial()
	{
		mDistortionMappingMaterial->SceneTexture() << mCutoutRenderTarget->OutputTexture();		
	}
}