#include "GaussianBlurGame.h"
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
#include "GaussianBlur.h"
#include <sstream>
#include <iomanip>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include "PointLight.h">

#include "PointLightDemo.h"

namespace Rendering
{
	const float GaussianBlurGame::BlurModulationRate = 1.0f;
    const XMVECTORF32 GaussianBlurGame::BackgroundColor = ColorHelper::CornflowerBlue;

    GaussianBlurGame::GaussianBlurGame(HINSTANCE instance, const std::wstring& windowClass, const std::wstring& windowTitle, int showCommand)
        :  Game(instance, windowClass, windowTitle, showCommand),
           mFpsComponent(nullptr), mGrid(nullptr),
           mDirectInput(nullptr), mKeyboard(nullptr), mMouse(nullptr), mRenderStateHelper(nullptr), mSkybox(nullptr),
           mPointLightDemo(nullptr),  mRenderTarget(nullptr), mGaussianBlur(nullptr),
		   mSpriteBatch(nullptr), mSpriteFont(nullptr), mTextPosition(0.0f, 40.0f)
    {
        mDepthStencilBufferEnabled = true;
        mMultiSamplingEnabled = true;
    }

    GaussianBlurGame::~GaussianBlurGame()
    {
    }

    void GaussianBlurGame::Initialize()
    {
		SetCurrentDirectory(Utility::ExecutableDirectory().c_str());

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

        Game::Initialize();

		mCamera->SetPosition(0.0f, 0.0f, 25.0f);

		mRenderTarget = new FullScreenRenderTarget(*this);
		mGaussianBlur = new GaussianBlur(*this, *mCamera);
		mGaussianBlur->SetSceneTexture(*(mRenderTarget->OutputTexture()));
		mGaussianBlur->Initialize();

		mSpriteBatch = new SpriteBatch(mDirect3DDeviceContext);
        mSpriteFont = new SpriteFont(mDirect3DDevice, L"Content\\Fonts\\Arial_14_Regular.spritefont");
    }

    void GaussianBlurGame::Shutdown()
    {			
		DeleteObject(mSpriteFont);
        DeleteObject(mSpriteBatch);
		DeleteObject(mGaussianBlur);
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

    void GaussianBlurGame::Update(const GameTime &gameTime)
    {
		mFpsComponent->Update(gameTime);

        if (mKeyboard->WasKeyPressedThisFrame(DIK_ESCAPE))
        {
            Exit();
        }

		UpdateBlurAmount(gameTime);

        Game::Update(gameTime);
    }

    void GaussianBlurGame::Draw(const GameTime &gameTime)
    {
		mRenderTarget->Begin();
		
		mDirect3DDeviceContext->ClearRenderTargetView(mRenderTarget->RenderTargetView() , reinterpret_cast<const float*>(&BackgroundColor));
        mDirect3DDeviceContext->ClearDepthStencilView(mRenderTarget->DepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		Game::Draw(gameTime);

		mRenderTarget->End();

		mDirect3DDeviceContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&BackgroundColor));
        mDirect3DDeviceContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		mGaussianBlur->Draw(gameTime);
	
		mRenderStateHelper->SaveAll();
		mFpsComponent->Draw(gameTime);

		mSpriteBatch->Begin();

		std::wostringstream helpLabel;
		helpLabel << L"Ambient Intensity (+PgUp/-PgDn): " << mPointLightDemo->GetAmbientColor().a << "\n";
		helpLabel << L"Point Light Intensity (+Home/-End): " << mPointLightDemo->GetPointLight().Color().a << "\n";
		helpLabel << L"Specular Power (+Insert/-Delete): " << mPointLightDemo->GetSpecularPower() << "\n";
		helpLabel << L"Move Point Light (8/2, 4/6, 3/9)\n";
		helpLabel << std::setprecision(2) << L"Blur Amount (+J/-K): " << mGaussianBlur->BlurAmount();
		mSpriteFont->DrawString(mSpriteBatch, helpLabel.str().c_str(), mTextPosition);

        mSpriteBatch->End();

		mRenderStateHelper->RestoreAll();

		HRESULT hr = mSwapChain->Present(0, 0);
        if (FAILED(hr))
        {
            throw GameException("IDXGISwapChain::Present() failed.", hr);
        }
    }

	void GaussianBlurGame::UpdateBlurAmount(const GameTime& gameTime)
	{
		static float blurAmount = mGaussianBlur->BlurAmount();

		if (mKeyboard != nullptr)
		{
			if (mKeyboard->IsKeyDown(DIK_J))
			{
				blurAmount += BlurModulationRate * (float)gameTime.ElapsedGameTime();
				mGaussianBlur->SetBlurAmount(blurAmount);
			}

			if (mKeyboard->IsKeyDown(DIK_K) && blurAmount > 0)
			{
				blurAmount -= BlurModulationRate * (float)gameTime.ElapsedGameTime();
				blurAmount = XMMax<float>(blurAmount, 0);
				mGaussianBlur->SetBlurAmount(blurAmount);
			}
		}
	}
}