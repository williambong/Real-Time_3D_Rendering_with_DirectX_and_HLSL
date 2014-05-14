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
	class HelloShadersMaterial;

    class HelloShadersDemo : public DrawableGameComponent
    {
        RTTI_DECLARATIONS(HelloShadersDemo, DrawableGameComponent)

    public:
        HelloShadersDemo(Game& game, Camera& camera);
        ~HelloShadersDemo();

        virtual void Initialize() override;
        virtual void Draw(const GameTime& gameTime) override;

    private:
        HelloShadersDemo();
        HelloShadersDemo(const HelloShadersDemo& rhs);
        HelloShadersDemo& operator=(const HelloShadersDemo& rhs);	

        Effect* mEffect;
		HelloShadersMaterial* mMaterial;
        ID3D11Buffer* mVertexBuffer;
        ID3D11Buffer* mIndexBuffer;
        UINT mIndexCount;

        XMFLOAT4X4 mWorldMatrix;

		RenderStateHelper* mRenderStateHelper;
		SpriteBatch* mSpriteBatch;
		SpriteFont* mSpriteFont;
		XMFLOAT2 mTextPosition;
    };
}
