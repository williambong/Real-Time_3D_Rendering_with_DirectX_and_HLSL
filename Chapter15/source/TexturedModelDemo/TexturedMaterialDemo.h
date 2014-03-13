#pragma once

#include "DrawableGameComponent.h"

using namespace Library;

namespace Library
{
    class Effect;    
}

namespace Rendering
{
	class TextureMappingMaterial;

    class TexturedMaterialDemo : public DrawableGameComponent
    {
        RTTI_DECLARATIONS(TexturedMaterialDemo, DrawableGameComponent)

    public:
        TexturedMaterialDemo(Game& game, Camera& camera);
        ~TexturedMaterialDemo();

        virtual void Initialize() override;
        virtual void Draw(const GameTime& gameTime) override;

    private:
        TexturedMaterialDemo();
        TexturedMaterialDemo(const TexturedMaterialDemo& rhs);
        TexturedMaterialDemo& operator=(const TexturedMaterialDemo& rhs);	

        Effect* mEffect;
        TextureMappingMaterial* mMaterial;
        ID3D11Buffer* mVertexBuffer;
        ID3D11Buffer* mIndexBuffer;
        UINT mIndexCount;

        XMFLOAT4X4 mWorldMatrix;
		ID3D11ShaderResourceView* mTextureShaderResourceView;
    };
}
