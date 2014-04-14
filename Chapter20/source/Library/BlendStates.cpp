#include "BlendStates.h"
#include "GameException.h"

namespace Library
{
	ID3D11BlendState* BlendStates::MultiplicativeBlending = nullptr;

	void BlendStates::Initialize(ID3D11Device* direct3DDevice)
	{
		assert(direct3DDevice != nullptr);

		D3D11_BLEND_DESC blendStateDesc;
		ZeroMemory(&blendStateDesc, sizeof(blendStateDesc));
		blendStateDesc.RenderTarget[0].BlendEnable = true;
		blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ZERO;
		blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_SRC_COLOR;
		blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
		blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		HRESULT hr = direct3DDevice->CreateBlendState(&blendStateDesc, &MultiplicativeBlending);
		if (FAILED(hr))
		{
			throw GameException("ID3D11Device::CreateBlendState() failed.", hr);
		}
	}

	void BlendStates::Release()
	{
		ReleaseObject(MultiplicativeBlending);
	}
}