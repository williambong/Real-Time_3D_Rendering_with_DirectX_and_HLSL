#include "ColorFilteringGame.h"
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
#include "Effect.h"
#include "ColorFilterMaterial.h"
#include <sstream>
#include <iomanip>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include "PointLight.h"

#include "PointLightDemo.h"

namespace Rendering
{
	const float ColorFilteringGame::BrightnessModulationRate = 1.0f;
    const XMVECTORF32 ColorFilteringGame::BackgroundColor = ColorHelper::CornflowerBlue;

    ColorFilteringGame::ColorFilteringGame(HINSTANCE instance, const std::wstring& windowClass, const std::wstring& windowTitle, int showCommand)
        :  Game(instance, windowClass, windowTitle, showCommand),
           mFpsComponent(nullptr), mGrid(nullptr),
           mDirectInput(nullptr), mKeyboard(nullptr), mMouse(nullptr), mRenderStateHelper(nullptr), mSkybox(nullptr),
           mPointLightDemo(nullptr), mRenderTarget(nullptr), mFullScreenQuad(nullptr), mColorFilterEffect(nullptr),
		   mColorFilterMaterial(nullptr), mActiveColorFilter(ColorFilterGrayScale), mGenericColorFilter(MatrixHelper::Identity),
		   mSpriteBatch(nullptr), mSpriteFont(nullptr), mTextPosition(0.0f, 40.0f)
    {
        mDepthStencilBufferEnabled = true;
        mMultiSamplingEnabled = true;
    }

    ColorFilteringGame::~ColorFilteringGame()
    {
    }

    void ColorFilteringGame::Initialize()
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
		mColorFilterEffect = new Effect(*this);
		mColorFilterEffect->LoadCompiledEffect(L"Content\\Effects\\ColorFilter.cso");
		
		mColorFilterMaterial = new ColorFilterMaterial();
		mColorFilterMaterial->Initialize(*mColorFilterEffect);

		mFullScreenQuad = new FullScreenQuad(*this, *mColorFilterMaterial);		
		mFullScreenQuad->Initialize();
		mFullScreenQuad->SetActiveTechnique(ColorFilterTechniqueNames[mActiveColorFilter], "p0");
		mFullScreenQuad->SetCustomUpdateMaterial(std::bind(&ColorFilteringGame::UpdateColorFilterMaterial, this));

		mSpriteBatch = new SpriteBatch(mDirect3DDeviceContext);
        mSpriteFont = new SpriteFont(mDirect3DDevice, L"Content\\Fonts\\Arial_14_Regular.spritefont");
		Game::Initialize();

		mCamera->SetPosition(0.0f, 0.0f, 25.0f);
    }

    void ColorFilteringGame::Shutdown()
    {	
		DeleteObject(mSpriteFont);
        DeleteObject(mSpriteBatch);
		DeleteObject(mColorFilterMaterial);
		DeleteObject(mColorFilterEffect);
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

    void ColorFilteringGame::Update(const GameTime &gameTime)
    {
		mFpsComponent->Update(gameTime);

        if (mKeyboard->WasKeyPressedThisFrame(DIK_ESCAPE))
        {
            Exit();
        }

		if (mKeyboard->WasKeyPressedThisFrame(DIK_SPACE))
        {
            mActiveColorFilter = ColorFilter(mActiveColorFilter + 1);
			if (mActiveColorFilter >= ColorFilterEnd)
			{
				mActiveColorFilter = (ColorFilter)(0);
			}

			mFullScreenQuad->SetActiveTechnique(ColorFilterTechniqueNames[mActiveColorFilter], "p0");
        }

		UpdateGenericColorFilter(gameTime);

        Game::Update(gameTime);
    }

    void ColorFilteringGame::Draw(const GameTime &gameTime)
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
		helpLabel << std::setprecision(2) << L"Active Filter (Space Bar): " << ColorFilterDisplayNames[mActiveColorFilter].c_str();
		if (mActiveColorFilter == ColorFilterGeneric)
		{
			helpLabel << L"\nBrightness (+Comma/-Period): " << mGenericColorFilter._11 << "\n";
		}

        mSpriteFont->DrawString(mSpriteBatch, helpLabel.str().c_str(), mTextPosition);

        mSpriteBatch->End();

		mRenderStateHelper->RestoreAll();

		HRESULT hr = mSwapChain->Present(0, 0);
        if (FAILED(hr))
        {
            throw GameException("IDXGISwapChain::Present() failed.", hr);
        }
    }

	void ColorFilteringGame::UpdateColorFilterMaterial()
	{
		XMMATRIX colorFilter = XMLoadFloat4x4(&mGenericColorFilter);

		mColorFilterMaterial->ColorTexture() << mRenderTarget->OutputTexture();
		mColorFilterMaterial->ColorFilter() << colorFilter;
	}

	void ColorFilteringGame::UpdateGenericColorFilter(const GameTime& gameTime)
	{
		static float brightness = 1.0f;

		if (mKeyboard != nullptr)
		{
			if (mKeyboard->IsKeyDown(DIK_COMMA) && brightness < 1.0f)
			{
				brightness += BrightnessModulationRate * (float)gameTime.ElapsedGameTime();
				brightness = XMMin<float>(brightness, 1.0f);
				XMStoreFloat4x4(&mGenericColorFilter, XMMatrixScaling(brightness, brightness, brightness));
			}

			if (mKeyboard->IsKeyDown(DIK_PERIOD) && brightness > 0.0f)
			{
				brightness -= BrightnessModulationRate * (float)gameTime.ElapsedGameTime();
				brightness = XMMax<float>(brightness, 0.0f);
				XMStoreFloat4x4(&mGenericColorFilter, XMMatrixScaling(brightness, brightness, brightness));
			}
		}
	}
}