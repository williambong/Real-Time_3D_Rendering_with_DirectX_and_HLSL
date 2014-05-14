#pragma once

#include "DrawableGameComponent.h"

using namespace Library;

namespace Library
{
    class Effect;
	class RenderStateHelper;
}

namespace DirectX
{
	class SpriteBatch;
	class SpriteFont;
}

namespace Rendering
{
	class TextureMappingMaterial;

    class TextureMappingDemo : public DrawableGameComponent
    {
        RTTI_DECLARATIONS(TextureMappingDemo, DrawableGameComponent)

    public:
        TextureMappingDemo(Game& game, Camera& camera);
        ~TextureMappingDemo();

        virtual void Initialize() override;
        virtual void Draw(const GameTime& gameTime) override;

    private:
        TextureMappingDemo();
        TextureMappingDemo(const TextureMappingDemo& rhs);
        TextureMappingDemo& operator=(const TextureMappingDemo& rhs);	

        Effect* mEffect;
		TextureMappingMaterial* mMaterial;
        ID3D11Buffer* mVertexBuffer;
        ID3D11Buffer* mIndexBuffer;
        UINT mIndexCount;
		ID3D11ShaderResourceView* mTextureShaderResourceView;

        XMFLOAT4X4 mWorldMatrix;

		RenderStateHelper* mRenderStateHelper;
		SpriteBatch* mSpriteBatch;
		SpriteFont* mSpriteFont;
		XMFLOAT2 mTextPosition;
    };
}
