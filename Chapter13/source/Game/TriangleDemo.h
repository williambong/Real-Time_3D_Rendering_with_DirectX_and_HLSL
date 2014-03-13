#pragma once

#include "DrawableGameComponent.h"

using namespace Library;

namespace Rendering
{
    class TriangleDemo : public DrawableGameComponent
    {
        RTTI_DECLARATIONS(TriangleDemo, DrawableGameComponent)

    public:
        TriangleDemo(Game& game, Camera& camera);
        ~TriangleDemo();

        virtual void Initialize() override;
        virtual void Update(const GameTime& gameTime) override;
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

        TriangleDemo();
        TriangleDemo(const TriangleDemo& rhs);
        TriangleDemo& operator=(const TriangleDemo& rhs);

        ID3DX11Effect* mEffect;
        ID3DX11EffectTechnique* mTechnique;
        ID3DX11EffectPass* mPass;
        ID3DX11EffectMatrixVariable* mWvpVariable;

        ID3D11InputLayout* mInputLayout;		
        ID3D11Buffer* mVertexBuffer;

        XMFLOAT4X4 mWorldMatrix;
		float mAngle;
    };
}
