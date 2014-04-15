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
	class ModelTessellationMaterial;

    class ModelTessellationDemo : public DrawableGameComponent
    {
        RTTI_DECLARATIONS(ModelTessellationDemo, DrawableGameComponent)

    public:
        ModelTessellationDemo(Game& game, Camera& camera);
        ~ModelTessellationDemo();

        virtual void Initialize() override;
        virtual void Update(const GameTime& gameTime) override;
		virtual void Draw(const GameTime& gameTime) override;

    private:
        ModelTessellationDemo();
        ModelTessellationDemo(const ModelTessellationDemo& rhs);
        ModelTessellationDemo& operator=(const ModelTessellationDemo& rhs);	

		void UpdateTechnique();

		Keyboard* mKeyboard;
		Effect* mEffect;
		ModelTessellationMaterial* mMaterial;
		Pass* mPass;
		ID3D11InputLayout* mInputLayout;
		ID3D11Buffer* mVertexBuffer;
		ID3D11Buffer* mIndexBuffer;
		UINT mIndexCount;
		ID3D11ShaderResourceView* mColorTexture;
		float mDisplacementScale;
		XMFLOAT4X4 mWorldMatrix;

		std::vector<float> mTessellationEdgeFactors;
		float mTessellationInsideFactor;

		RenderStateHelper mRenderStateHelper;
		SpriteBatch* mSpriteBatch;
		SpriteFont* mSpriteFont;
		XMFLOAT2 mTextPosition;
		
		bool mShowWireframe;
		bool mDistanceModeEnabled;
		int mMaxTessellationFactor;
		float mMinTessellationDistance;
		float mMaxTessellationDistance;
    };
}
