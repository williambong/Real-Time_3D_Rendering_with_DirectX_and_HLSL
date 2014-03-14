#pragma once

#include "DrawableGameComponent.h"

using namespace Library;

namespace Library
{
    class Effect;
    class BasicMaterial;
}

namespace Rendering
{
    class MaterialDemo : public DrawableGameComponent
    {
        RTTI_DECLARATIONS(MaterialDemo, DrawableGameComponent)

    public:
        MaterialDemo(Game& game, Camera& camera);
        ~MaterialDemo();

        virtual void Initialize() override;
        virtual void Draw(const GameTime& gameTime) override;

    private:
        MaterialDemo();
        MaterialDemo(const MaterialDemo& rhs);
        MaterialDemo& operator=(const MaterialDemo& rhs);	

        Effect* mBasicEffect;
        BasicMaterial* mBasicMaterial;
        ID3D11Buffer* mVertexBuffer;
        ID3D11Buffer* mIndexBuffer;
        UINT mIndexCount;

        XMFLOAT4X4 mWorldMatrix;	
    };
}
