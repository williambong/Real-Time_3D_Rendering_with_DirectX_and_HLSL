#pragma once

#include "DrawableGameComponent.h"
#include "RenderStateHelper.h"

using namespace Library;

namespace Library
{
	class Keyboard;
    class Effect;
	class Pass;
}

namespace DirectX
{
	class SpriteBatch;
	class SpriteFont;
}

namespace Rendering
{
	class TriangleTessellationMaterial;
	class QuadTessellationMaterial;

    class BasicTessellationDemo : public DrawableGameComponent
    {
        RTTI_DECLARATIONS(BasicTessellationDemo, DrawableGameComponent)

    public:
        BasicTessellationDemo(Game& game, Camera& camera);
        ~BasicTessellationDemo();

        virtual void Initialize() override;
        virtual void Update(const GameTime& gameTime) override;
		virtual void Draw(const GameTime& gameTime) override;

    private:
        BasicTessellationDemo();
        BasicTessellationDemo(const BasicTessellationDemo& rhs);
        BasicTessellationDemo& operator=(const BasicTessellationDemo& rhs);	

		static const float MaxTessellationFactor;

		Keyboard* mKeyboard;
        Effect* mTriEffect;
		TriangleTessellationMaterial* mTriMaterial;
		Pass* mTriPass;
		ID3D11InputLayout* mTriInputLayout;
		ID3D11Buffer* mTriVertexBuffer;

		Effect* mQuadEffect;
		QuadTessellationMaterial* mQuadMaterial;
		Pass* mQuadPass;
		ID3D11InputLayout* mQuadInputLayout;
		ID3D11Buffer* mQuadVertexBuffer;
		
		std::vector<float> mTessellationEdgeFactors;
		std::vector<float> mTessellationInsideFactors;
		bool mUniformTessellation;
		bool mShowQuadTopology;

		RenderStateHelper mRenderStateHelper;
		SpriteBatch* mSpriteBatch;
		SpriteFont* mSpriteFont;
		XMFLOAT2 mTextPosition;
    };
}
