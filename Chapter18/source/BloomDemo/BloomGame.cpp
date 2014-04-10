#include "BloomGame.h"
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
#include "VectorHelper.h"
#include "FullScreenRenderTarget.h"
#include "Bloom.h"
#include <sstream>
#include <iomanip>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include "PointLight.h"

#include "PointLightDemo.h"

namespace Rendering
{
	const float BloomGame::BloomModulationRate = 1.0f;
    const XMVECTORF32 BloomGame::BackgroundColor = ColorHelper::CornflowerBlue;

    BloomGame::BloomGame(HINSTANCE instance, const std::wstring& windowClass, const std::wstring& windowTitle, int showCommand)
        :  Game(instance, windowClass, windowTitle, showCommand),
           mFpsComponent(nullptr), mGrid(nullptr),
           mDirectInput(nullptr), mKeyboard(nullptr), mMouse(nullptr), mRenderStateHelper(nullptr), mSkybox(nullptr),
           mPointLightDemo(nullptr),  mFullScreenRenderTarget(nullptr), mBloom(nullptr), mBloomEnabled(true),
		   mSpriteBatch(nullptr), mSpriteFont(nullptr), mTextPosition(0.0f, 40.0f)
    {
        mDepthStencilBufferEnabled = true;
        mMultiSamplingEnabled = true;
    }

    BloomGame::~BloomGame()
    {
    }

    void BloomGame::Initialize()
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

		mFullScreenRenderTarget = new FullScreenRenderTarget(*this);
		mBloom = new Bloom(*this, *mCamera);
		mBloom->SetSceneTexture(*(mFullScreenRenderTarget->OutputTexture()));
		mBloom->Initialize();

		mSpriteBatch = new SpriteBatch(mDirect3DDeviceContext);
        mSpriteFont = new SpriteFont(mDirect3DDevice, L"Content\\Fonts\\Arial_14_Regular.spritefont");
    }

    void BloomGame::Shutdown()
    {			
		DeleteObject(mSpriteFont);
        DeleteObject(mSpriteBatch);
		DeleteObject(mBloom);
		DeleteObject(mFullScreenRenderTarget);

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

    void BloomGame::Update(const GameTime &gameTime)
    {
		mFpsComponent->Update(gameTime);

        if (mKeyboard->WasKeyPressedThisFrame(DIK_ESCAPE))
        {
            Exit();
        }

		if (mKeyboard->WasKeyPressedThisFrame(DIK_SPACE))
        {
            mBloomEnabled = !mBloomEnabled;
        }

		if (mKeyboard->WasKeyPressedThisFrame(DIK_RETURN))
        {
			BloomDrawMode drawMode = BloomDrawMode(mBloom->DrawMode() + 1);
			if (drawMode >= BloomDrawModeEnd)
			{
				drawMode = (BloomDrawMode)0;
			}

            mBloom->SetDrawMode(drawMode);
        }

		UpdateBloomSettings(gameTime);

        Game::Update(gameTime);
    }

    void BloomGame::Draw(const GameTime &gameTime)
    {
		if (mBloomEnabled)
		{
			mFullScreenRenderTarget->Begin();
		
			mDirect3DDeviceContext->ClearRenderTargetView(mFullScreenRenderTarget->RenderTargetView() , reinterpret_cast<const float*>(&BackgroundColor));
			mDirect3DDeviceContext->ClearDepthStencilView(mFullScreenRenderTarget->DepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

			Game::Draw(gameTime);

			mFullScreenRenderTarget->End();

			mDirect3DDeviceContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&BackgroundColor));
			mDirect3DDeviceContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

			mBloom->Draw(gameTime);
		}
		else
		{
			mDirect3DDeviceContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&BackgroundColor));
			mDirect3DDeviceContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

			Game::Draw(gameTime);
		}

		mRenderStateHelper->SaveAll();
		mFpsComponent->Draw(gameTime);

		mSpriteBatch->Begin();

		std::wostringstream helpLabel;
		helpLabel << L"Ambient Intensity (+PgUp/-PgDn): " << mPointLightDemo->GetAmbientColor().a << "\n"
			      << L"Point Light Intensity (+Home/-End): " << mPointLightDemo->GetPointLight().Color().a << "\n"
				  << L"Specular Power (+Insert/-Delete): " << mPointLightDemo->GetSpecularPower() << "\n"
				  << L"Move Point Light (8/2, 4/6, 3/9)\n";

		const BloomSettings& bloomSettings = mBloom->GetBloomSettings();		
		helpLabel << std::setprecision(2) << "\nBloom Enabled (Space Bar): " << (mBloomEnabled ? L"True" : L"False") << "\n"
			      << L"Draw Mode (Enter): " << mBloom->DrawModeString().c_str() << "\n"
			      << L"Bloom Threshold (+U/-I): " << bloomSettings.BloomThreshold << "\n"
				  << L"Blur Amount (+J/-K): " << bloomSettings.BlurAmount << "\n"
				  << L"Bloom Intensity (+N/-M): " << bloomSettings.BloomIntensity << "\n"
			      << L"Bloom Saturation (+G/-H): " << bloomSettings.BloomSaturation << "\n";
		mSpriteFont->DrawString(mSpriteBatch, helpLabel.str().c_str(), mTextPosition);

		mSpriteBatch->End();

		mRenderStateHelper->RestoreAll();

        

		HRESULT hr = mSwapChain->Present(0, 0);
        if (FAILED(hr))
        {
            throw GameException("IDXGISwapChain::Present() failed.", hr);
        }
    }

	void BloomGame::UpdateBloomSettings(const GameTime& gameTime)
	{
		static BloomSettings bloomSettings = mBloom->GetBloomSettings();

		if (mKeyboard != nullptr)
		{
			if (mKeyboard->IsKeyDown(DIK_J))
			{
				bloomSettings.BlurAmount += BloomModulationRate * (float)gameTime.ElapsedGameTime();
				mBloom->SetBloomSettings(bloomSettings);
			}

			if (mKeyboard->IsKeyDown(DIK_K) && bloomSettings.BlurAmount > 0.0f)
			{
				bloomSettings.BlurAmount -= BloomModulationRate * (float)gameTime.ElapsedGameTime();
				bloomSettings.BlurAmount = XMMax<float>(bloomSettings.BlurAmount, 0);
				mBloom->SetBloomSettings(bloomSettings);
			}

			if (mKeyboard->IsKeyDown(DIK_U) && bloomSettings.BloomThreshold < 1.0f)
			{
				bloomSettings.BloomThreshold += BloomModulationRate * (float)gameTime.ElapsedGameTime();
				bloomSettings.BloomThreshold = XMMin<float>(bloomSettings.BloomThreshold, 1.0f);
				mBloom->SetBloomSettings(bloomSettings);
			}

			if (mKeyboard->IsKeyDown(DIK_I) && bloomSettings.BloomThreshold > 0.0f)
			{
				bloomSettings.BloomThreshold -= BloomModulationRate * (float)gameTime.ElapsedGameTime();
				bloomSettings.BloomThreshold = XMMax<float>(bloomSettings.BloomThreshold, 0);
				mBloom->SetBloomSettings(bloomSettings);
			}

			if (mKeyboard->IsKeyDown(DIK_N))
			{
				bloomSettings.BloomIntensity += BloomModulationRate * (float)gameTime.ElapsedGameTime();
				mBloom->SetBloomSettings(bloomSettings);
			}

			if (mKeyboard->IsKeyDown(DIK_M) && bloomSettings.BloomIntensity > 0.0f)
			{
				bloomSettings.BloomIntensity -= BloomModulationRate * (float)gameTime.ElapsedGameTime();
				bloomSettings.BloomIntensity = XMMax<float>(bloomSettings.BloomIntensity, 0);
				mBloom->SetBloomSettings(bloomSettings);
			}

			if (mKeyboard->IsKeyDown(DIK_G))
			{
				bloomSettings.BloomSaturation += BloomModulationRate * (float)gameTime.ElapsedGameTime();
				mBloom->SetBloomSettings(bloomSettings);
			}

			if (mKeyboard->IsKeyDown(DIK_H) && bloomSettings.BloomSaturation > 0.0f)
			{
				bloomSettings.BloomSaturation -= BloomModulationRate * (float)gameTime.ElapsedGameTime();
				bloomSettings.BloomSaturation = XMMax<float>(bloomSettings.BloomSaturation, 0);
				mBloom->SetBloomSettings(bloomSettings);
			}
		}
	}
}