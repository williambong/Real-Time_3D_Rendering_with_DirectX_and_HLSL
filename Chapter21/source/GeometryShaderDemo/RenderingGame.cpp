#include "RenderingGame.h"
#include "GameException.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "FpsComponent.h"
#include "ColorHelper.h"
#include "FirstPersonCamera.h"
#include "RenderStateHelper.h"
#include "RasterizerStates.h"
#include "BlendStates.h"
#include "SamplerStates.h"
#include "Skybox.h"
#include "Grid.h"

#include "GeometryShaderDemo.h"

namespace Rendering
{
    const XMVECTORF32 RenderingGame::BackgroundColor = ColorHelper::CornflowerBlue;

    RenderingGame::RenderingGame(HINSTANCE instance, const std::wstring& windowClass, const std::wstring& windowTitle, int showCommand)
        :  Game(instance, windowClass, windowTitle, showCommand),
           mFpsComponent(nullptr), mGrid(nullptr),
           mDirectInput(nullptr), mKeyboard(nullptr), mMouse(nullptr), mRenderStateHelper(nullptr), mSkybox(nullptr),
           mGeometryShaderDemo(nullptr)
	{
        mDepthStencilBufferEnabled = true;
        mMultiSamplingEnabled = true;
    }

    RenderingGame::~RenderingGame()
    {
    }

    void RenderingGame::Initialize()
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

		/*mSkybox = new Skybox(*this, *mCamera, L"Content\\Textures\\Maskonaive2_1024.dds", 500.0f);
		mComponents.push_back(mSkybox);*/

		mGrid = new Grid(*this, *mCamera);
		mComponents.push_back(mGrid);

		RasterizerStates::Initialize(mDirect3DDevice);
		BlendStates::Initialize(mDirect3DDevice);
		SamplerStates::BorderColor = ColorHelper::Black;
		SamplerStates::Initialize(mDirect3DDevice);

		mGeometryShaderDemo = new GeometryShaderDemo(*this, *mCamera);
		mComponents.push_back(mGeometryShaderDemo);

		mRenderStateHelper = new RenderStateHelper(*this);

		Game::Initialize();

		mCamera->SetPosition(0.0f, 0.0f, 25.0f);
    }

    void RenderingGame::Shutdown()
    {
		DeleteObject(mGeometryShaderDemo);		
        DeleteObject(mRenderStateHelper);
		DeleteObject(mKeyboard);
        DeleteObject(mMouse);
		DeleteObject(mSkybox)
		DeleteObject(mGrid);
        DeleteObject(mFpsComponent);
        DeleteObject(mCamera);

        ReleaseObject(mDirectInput);
		RasterizerStates::Release();
		BlendStates::Release();
		SamplerStates::Release();

        Game::Shutdown();
    }

    void RenderingGame::Update(const GameTime &gameTime)
    {
		mFpsComponent->Update(gameTime);

        if (mKeyboard->WasKeyPressedThisFrame(DIK_ESCAPE))
        {
            Exit();
        }

        Game::Update(gameTime);
    }

    void RenderingGame::Draw(const GameTime &gameTime)
    {
		mDirect3DDeviceContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&BackgroundColor));
        mDirect3DDeviceContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        Game::Draw(gameTime);

		mRenderStateHelper->SaveAll();
		mFpsComponent->Draw(gameTime);		
		mRenderStateHelper->RestoreAll();
        
        HRESULT hr = mSwapChain->Present(0, 0);
        if (FAILED(hr))
        {
            throw GameException("IDXGISwapChain::Present() failed.", hr);
        }
    }
}