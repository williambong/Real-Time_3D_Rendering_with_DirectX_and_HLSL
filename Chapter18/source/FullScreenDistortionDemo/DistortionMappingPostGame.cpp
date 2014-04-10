#include "DistortionMappingPostGame.h"
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
#include "PointLight.h"
#include "DistortionMappingPostMaterial.h"

#include "PointLightDemo.h"

namespace Rendering
{
    const XMVECTORF32 DistortionMappingPostGame::BackgroundColor = ColorHelper::CornflowerBlue;

    DistortionMappingPostGame::DistortionMappingPostGame(HINSTANCE instance, const std::wstring& windowClass, const std::wstring& windowTitle, int showCommand)
        :  Game(instance, windowClass, windowTitle, showCommand),
           mFpsComponent(nullptr), mGrid(nullptr),
           mDirectInput(nullptr), mKeyboard(nullptr), mMouse(nullptr), mRenderStateHelper(nullptr), mSkybox(nullptr),
           mPointLightDemo(nullptr), mRenderTarget(nullptr), mFullScreenQuad(nullptr), mDistortionMappingPostMaterial(nullptr), mDistortionMap(nullptr), mDisplacementScale(1.0f),
		   mSpriteBatch(nullptr), mSpriteFont(nullptr), mTextPosition(0.0f, 40.0f)
    {
        mDepthStencilBufferEnabled = true;
        mMultiSamplingEnabled = true;
    }

    DistortionMappingPostGame::~DistortionMappingPostGame()
    {
    }

    void DistortionMappingPostGame::Initialize()
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
		
		mPointLightDemo = new PointLightDemo(*this, *mCamera);
		mComponents.push_back(mPointLightDemo);

		mRenderStateHelper = new RenderStateHelper(*this);

		mRenderTarget = new FullScreenRenderTarget(*this);

		SetCurrentDirectory(Utility::ExecutableDirectory().c_str());
		mDistortionMappingEffect = new Effect(*this);
		mDistortionMappingEffect->LoadCompiledEffect(L"Content\\Effects\\DistortionPost.cso");

		mDistortionMappingPostMaterial = new DistortionMappingPostMaterial();
		mDistortionMappingPostMaterial->Initialize(*mDistortionMappingEffect);

		mFullScreenQuad = new FullScreenQuad(*this, *mDistortionMappingPostMaterial);
		mFullScreenQuad->Initialize();
		mFullScreenQuad->SetActiveTechnique("displacement", "p0");
		mFullScreenQuad->SetCustomUpdateMaterial(std::bind(&DistortionMappingPostGame::UpdateDisplacementMaterial, this));

		mSpriteBatch = new SpriteBatch(mDirect3DDeviceContext);
        mSpriteFont = new SpriteFont(mDirect3DDevice, L"Content\\Fonts\\Arial_14_Regular.spritefont");

		std::wstring textureName = L"Content\\Textures\\DistortionGlass.png";
		HRESULT hr = DirectX::CreateWICTextureFromFile(mDirect3DDevice, mDirect3DDeviceContext, textureName.c_str(), nullptr, &mDistortionMap);
		if (FAILED(hr))
		{
			throw GameException("CreateWICTextureFromFile() failed.", hr);
		}

        Game::Initialize();

		mCamera->SetPosition(0.0f, 0.0f, 25.0f);
    }

    void DistortionMappingPostGame::Shutdown()
    {	
		DeleteObject(mSpriteFont);
        DeleteObject(mSpriteBatch);

		ReleaseObject(mDistortionMap);
		DeleteObject(mDistortionMappingPostMaterial);
		DeleteObject(mDistortionMappingEffect);
		DeleteObject(mFullScreenQuad);
		DeleteObject(mRenderTarget);

		DeleteObject(mPointLightDemo);
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

    void DistortionMappingPostGame::Update(const GameTime &gameTime)
    {
		mFpsComponent->Update(gameTime);

        if (mKeyboard->WasKeyPressedThisFrame(DIK_ESCAPE))
        {
            Exit();
        }

		UpdateDisplacementScale(gameTime);

        Game::Update(gameTime);
    }

    void DistortionMappingPostGame::Draw(const GameTime &gameTime)
    {
		mRenderTarget->Begin();
		
		mDirect3DDeviceContext->ClearRenderTargetView(mRenderTarget->RenderTargetView() , reinterpret_cast<const float*>(&BackgroundColor));
        mDirect3DDeviceContext->ClearDepthStencilView(mRenderTarget->DepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		Game::Draw(gameTime);

		mRenderTarget->End();
		
		mDirect3DDeviceContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&BackgroundColor));
        mDirect3DDeviceContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	
		mFullScreenQuad->Draw(gameTime);

		mRenderStateHelper->SaveAll();
		mFpsComponent->Draw(gameTime);

		mSpriteBatch->Begin();

        std::wostringstream helpLabel;
		helpLabel << L"Ambient Intensity (+PgUp/-PgDn): " << mPointLightDemo->GetAmbientColor().a << "\n";
		helpLabel << L"Point Light Intensity (+Home/-End): " << mPointLightDemo->GetPointLight().Color().a << "\n";
		helpLabel << L"Specular Power (+Insert/-Delete): " << mPointLightDemo->GetSpecularPower() << "\n";
		helpLabel << L"Move Point Light (8/2, 4/6, 3/9)\n";
        helpLabel << std::setprecision(2) << L"Displacement Scale (+Common/-Period): " << mDisplacementScale;
		
        mSpriteFont->DrawString(mSpriteBatch, helpLabel.str().c_str(), mTextPosition);

        mSpriteBatch->End();

		mRenderStateHelper->RestoreAll();

		HRESULT hr = mSwapChain->Present(0, 0);
        if (FAILED(hr))
        {
            throw GameException("IDXGISwapChain::Present() failed.", hr);
        }
    }

	void DistortionMappingPostGame::UpdateDisplacementMaterial()
	{
		mDistortionMappingPostMaterial->SceneTexture() << mRenderTarget->OutputTexture();
		mDistortionMappingPostMaterial->DistortionMap() << mDistortionMap;
		mDistortionMappingPostMaterial->DisplacementScale() << mDisplacementScale;
	}

	void DistortionMappingPostGame::UpdateDisplacementScale(const GameTime& gameTime)
	{
		if (mKeyboard != nullptr)
		{
			if (mKeyboard->IsKeyDown(DIK_COMMA) && mDisplacementScale < 1.0f)
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
}