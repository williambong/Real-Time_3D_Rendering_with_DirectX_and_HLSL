#pragma once

#include "DrawableGameComponent.h"

using namespace Library;

namespace Library
{
	class Keyboard;
}

namespace Rendering
{
	class CubeDemo : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(CubeDemo, DrawableGameComponent)

	public:
		CubeDemo(Game& game, Camera& camera);
		~CubeDemo();

		virtual void Initialize() override;
		virtual void Draw(const GameTime& gameTime) override;

	private:
		typedef struct _BasicEffectVertex
        {
            XMFLOAT4 Position;
            XMFLOAT4 Color;

            _BasicEffectVertex() { }

            _BasicEffectVertex(XMFLOAT4 position, XMFLOAT4 color)
                : Position(position), Color(color) { }
        } BasicEffectVertex;

		CubeDemo();
		CubeDemo(const CubeDemo& rhs);
		CubeDemo& operator=(const CubeDemo& rhs);

		ID3DX11Effect* mEffect;
        ID3DX11EffectTechnique* mTechnique;
        ID3DX11EffectPass* mPass;
        ID3DX11EffectMatrixVariable* mWvpVariable;

        ID3D11InputLayout* mInputLayout;		
        ID3D11Buffer* mVertexBuffer;
		ID3D11Buffer* mIndexBuffer;

		XMFLOAT4X4 mWorldMatrix;	
	};
}
