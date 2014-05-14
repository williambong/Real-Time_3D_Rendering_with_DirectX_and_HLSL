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
	class HelloStructsMaterial;

    class HelloStructsDemo : public DrawableGameComponent
    {
        RTTI_DECLARATIONS(HelloStructsDemo, DrawableGameComponent)

    public:
        HelloStructsDemo(Game& game, Camera& camera);
        ~HelloStructsDemo();

        virtual void Initialize() override;
        virtual void Draw(const GameTime& gameTime) override;

    private:
        HelloStructsDemo();
        HelloStructsDemo(const HelloStructsDemo& rhs);
        HelloStructsDemo& operator=(const HelloStructsDemo& rhs);	

        Effect* mEffect;
		HelloStructsMaterial* mMaterial;
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
