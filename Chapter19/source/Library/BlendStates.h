#pragma once

#include "Common.h"

namespace Library
{
	class BlendStates
	{
	public:
		static ID3D11BlendState* MultiplicativeBlending;		

		static void Initialize(ID3D11Device* direct3DDevice);
		static void Release();

	private:
		BlendStates();
		BlendStates(const BlendStates& rhs);
		BlendStates& operator=(const BlendStates& rhs);
	};
}
