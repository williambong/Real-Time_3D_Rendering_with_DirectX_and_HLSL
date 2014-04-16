#include "ComputeShaderDemo.h"
#include "Game.h"
#include "GameException.h"
#include "Camera.h"
#include "Utility.h"
#include "ComputeShaderMaterial.h"
#include "FullScreenQuad.h"
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <sstream>
#include <iomanip>

namespace Rendering
{
	RTTI_DEFINITIONS(ComputeShaderDemo)

	const UINT ComputeShaderDemo::ThreadsPerGroup = 32;

	ComputeShaderDemo::ComputeShaderDemo(Game& game, Camera& camera)
		: DrawableGameComponent(game, camera), mEffect(nullptr), mMaterial(nullptr), mComputePass(nullptr),
		mOutputTexture(nullptr), mTextureSize(0.0f, 0.0f), mBlueColor(0.0f), mFullScreenQuad(nullptr), mColorTexture(nullptr),
		mRenderStateHelper(game), mSpriteBatch(nullptr), mSpriteFont(nullptr), mTextPosition(0.0f, 40.0f), mThreadGroupCount(0, 0)
	{
	}

	ComputeShaderDemo::~ComputeShaderDemo()
	{
		DeleteObject(mSpriteFont);
		DeleteObject(mSpriteBatch);
		ReleaseObject(mColorTexture);
		DeleteObject(mFullScreenQuad);
		ReleaseObject(mOutputTexture);
		DeleteObject(mMaterial);
		DeleteObject(mEffect);			
	}

	void ComputeShaderDemo::Initialize()
	{
		SetCurrentDirectory(Utility::ExecutableDirectory().c_str());

		// Initialize the material
		mEffect = new Effect(*mGame);
		mEffect->LoadCompiledEffect(L"Content\\Effects\\ComputeShader.cso");
		mMaterial = new ComputeShaderMaterial();
		mMaterial->Initialize(*mEffect);

		mComputePass = mEffect->TechniquesByName().at("compute")->PassesByName().at("p0");

		D3D11_TEXTURE2D_DESC textureDesc;
		ZeroMemory(&textureDesc, sizeof(textureDesc));
		textureDesc.Width = mGame->ScreenWidth();
		textureDesc.Height = mGame->ScreenHeight();
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;

		HRESULT hr;
		ID3D11Texture2D* texture = nullptr;
		if (FAILED(hr = mGame->Direct3DDevice()->CreateTexture2D(&textureDesc, nullptr, &texture)))
		{
			throw GameException("IDXGIDevice::CreateTexture2D() failed.", hr);
		}

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		ZeroMemory(&uavDesc, sizeof(uavDesc));
		uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;

		if (FAILED(hr = mGame->Direct3DDevice()->CreateUnorderedAccessView(texture, &uavDesc, &mOutputTexture)))
		{
			ReleaseObject(texture);
			throw GameException("IDXGIDevice::CreateUnorderedAccessView() failed.", hr);
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDesc;
		ZeroMemory(&resourceViewDesc, sizeof(resourceViewDesc));
		resourceViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		resourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		resourceViewDesc.Texture2D.MipLevels = 1;

		if (FAILED(hr = mGame->Direct3DDevice()->CreateShaderResourceView(texture, &resourceViewDesc, &mColorTexture)))
		{
			ReleaseObject(texture);
			throw GameException("IDXGIDevice::CreateShaderResourceView() failed.", hr);
		}

		ReleaseObject(texture);

		mTextureSize.x = static_cast<float>(textureDesc.Width);
		mTextureSize.y = static_cast<float>(textureDesc.Height);

		mThreadGroupCount.x = textureDesc.Width / ThreadsPerGroup;
		mThreadGroupCount.y = textureDesc.Height / ThreadsPerGroup;

		mFullScreenQuad = new FullScreenQuad(*mGame, *mMaterial);
		mFullScreenQuad->SetActiveTechnique("render", "p0");
		mFullScreenQuad->SetCustomUpdateMaterial(std::bind(&ComputeShaderDemo::UpdateRenderingMaterial, this));
		mFullScreenQuad->Initialize();

		mSpriteBatch = new SpriteBatch(mGame->Direct3DDeviceContext());
		mSpriteFont = new SpriteFont(mGame->Direct3DDevice(), L"Content\\Fonts\\Arial_14_Regular.spritefont");
	}

	void ComputeShaderDemo::Update(const GameTime& gameTime)
	{
		mBlueColor = (0.5f) * static_cast<float>(sin(gameTime.TotalGameTime())) + 0.5f;
	}

	void ComputeShaderDemo::Draw(const GameTime& gameTime)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mGame->Direct3DDeviceContext();
		
		// Update the compute shader's material
		mMaterial->TextureSize() << XMLoadFloat2(&mTextureSize);
		mMaterial->BlueColor() << mBlueColor;
		mMaterial->OutputTexture() << mOutputTexture;	
		mComputePass->Apply(0, direct3DDeviceContext);
		
		// Dispatch the compute shader
		direct3DDeviceContext->Dispatch(mThreadGroupCount.x, mThreadGroupCount.y, 1);

		// Unbind the UAV from the compute shader, so we can bind the same underlying resource as an SRV
		static ID3D11UnorderedAccessView* emptyUAV = nullptr;
		direct3DDeviceContext->CSSetUnorderedAccessViews(0, 1, &emptyUAV, nullptr);

		// Draw the texture written by the compute shader
		mFullScreenQuad->Draw(gameTime);
		mGame->UnbindPixelShaderResources(0, 1);

		mRenderStateHelper.SaveAll();
		mSpriteBatch->Begin();

		std::wostringstream helpLabel;
		helpLabel << std::setprecision(2) << "Color Offset: " << mBlueColor;

		mSpriteFont->DrawString(mSpriteBatch, helpLabel.str().c_str(), mTextPosition);

		mSpriteBatch->End();
		mRenderStateHelper.RestoreAll();
	}

	void ComputeShaderDemo::UpdateRenderingMaterial()
	{
		mMaterial->ColorTexture() << mColorTexture;
	}
}