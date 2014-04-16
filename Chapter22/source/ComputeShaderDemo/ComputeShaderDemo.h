#pragma once

#include "DrawableGameComponent.h"
#include "RenderStateHelper.h"

using namespace Library;

namespace Library
{
	class Effect;
	class Pass;
	class FullScreenQuad;
}

namespace DirectX
{
	class SpriteBatch;
	class SpriteFont;
}

namespace Rendering
{
	class ComputeShaderMaterial;

	class ComputeShaderDemo : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(ComputeShaderDemo, DrawableGameComponent)

	public:		
		ComputeShaderDemo(Game& game, Camera& camera);
		~ComputeShaderDemo();

		virtual void Initialize() override;
		virtual void Update(const GameTime& gameTime) override;
		virtual void Draw(const GameTime& gameTime) override;

	private:
		ComputeShaderDemo();
		ComputeShaderDemo(const ComputeShaderDemo& rhs);
		ComputeShaderDemo& operator=(const ComputeShaderDemo& rhs);

		static const UINT ThreadsPerGroup;

		void UpdateRenderingMaterial();

		Effect* mEffect;
		ComputeShaderMaterial* mMaterial;		
		Pass* mComputePass;
		ID3D11UnorderedAccessView* mOutputTexture;		
		XMFLOAT2 mTextureSize;
		float mBlueColor;

		FullScreenQuad* mFullScreenQuad;
		ID3D11ShaderResourceView* mColorTexture;

		RenderStateHelper mRenderStateHelper;
		SpriteBatch* mSpriteBatch;
		SpriteFont* mSpriteFont;
		XMFLOAT2 mTextPosition;

		XMUINT2 mThreadGroupCount;
	};
}
