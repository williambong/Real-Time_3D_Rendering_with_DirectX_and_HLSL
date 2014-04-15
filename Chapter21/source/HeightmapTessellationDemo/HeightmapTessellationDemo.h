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
	class QuadHeightmapTessellationMaterial;

    class HeightmapTessellationDemo : public DrawableGameComponent
    {
        RTTI_DECLARATIONS(HeightmapTessellationDemo, DrawableGameComponent)

    public:
        HeightmapTessellationDemo(Game& game, Camera& camera);
        ~HeightmapTessellationDemo();

        virtual void Initialize() override;
        virtual void Update(const GameTime& gameTime) override;
		virtual void Draw(const GameTime& gameTime) override;

    private:
        HeightmapTessellationDemo();
        HeightmapTessellationDemo(const HeightmapTessellationDemo& rhs);
        HeightmapTessellationDemo& operator=(const HeightmapTessellationDemo& rhs);	

		void UpdateDisplacementScale(const GameTime& gameTime);

		static const float MaxTessellationFactor;
		static const RECT HeightmapDestinationRectangle;
		static const XMFLOAT2 TextureModulationRate;

		Keyboard* mKeyboard;
		Effect* mEffect;
		QuadHeightmapTessellationMaterial* mMaterial;
		Pass* mPass;
		ID3D11InputLayout* mInputLayout;
		ID3D11Buffer* mVertexBuffer;
		ID3D11ShaderResourceView* mHeightmap;
		float mDisplacementScale;

		std::vector<float> mTessellationEdgeFactors;
		std::vector<float> mTessellationInsideFactors;

		RenderStateHelper mRenderStateHelper;
		SpriteBatch* mSpriteBatch;
		SpriteFont* mSpriteFont;
		XMFLOAT2 mTextPosition;

		XMFLOAT4X4 mTextureMatrix;
		XMFLOAT2 mTexturePosition;
		bool mAnimationEnabled;
    };
}
