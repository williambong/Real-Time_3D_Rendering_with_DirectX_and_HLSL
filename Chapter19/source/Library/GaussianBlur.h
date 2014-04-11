#pragma once

#include "Common.h"
#include "DrawableGameComponent.h"

namespace Library
{
	class Effect;
	class GaussianBlurMaterial;
	class FullScreenRenderTarget;
	class FullScreenQuad;

	class GaussianBlur : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(GaussianBlur, DrawableGameComponent)

	public:
		GaussianBlur(Game& game, Camera& camera);
		GaussianBlur(Game& game, Camera& camera, float blurAmount);
		~GaussianBlur();
		
		ID3D11ShaderResourceView* SceneTexture();
		void SetSceneTexture(ID3D11ShaderResourceView& sceneTexture);

		ID3D11ShaderResourceView* OutputTexture();

		float BlurAmount() const;
		void SetBlurAmount(float blurAmount);

		virtual void Initialize() override;
		virtual void Draw(const GameTime& gameTime) override;
		void DrawToTexture(const GameTime& gameTime);

	private:
		GaussianBlur();
		GaussianBlur(const GaussianBlur& rhs);
		GaussianBlur& operator=(const GaussianBlur& rhs);

		
		void InitializeSampleOffsets();
		void InitializeSampleWeights();		
		float GetWeight(float x) const;
		void UpdateGaussianMaterialWithHorizontalOffsets();
		void UpdateGaussianMaterialWithVerticalOffsets();
		void UpdateGaussianMaterialNoBlur();

		static const float DefaultBlurAmount;

		Effect* mEffect;
		GaussianBlurMaterial* mMaterial;
		ID3D11ShaderResourceView* mSceneTexture;
		ID3D11ShaderResourceView* mOutputTexture;
		FullScreenRenderTarget* mHorizontalBlurTarget;
		FullScreenRenderTarget* mVerticalBlurTarget;
		FullScreenQuad* mFullScreenQuad;

		std::vector<XMFLOAT2> mHorizontalSampleOffsets;
		std::vector<XMFLOAT2> mVerticalSampleOffsets;
		std::vector<float> mSampleWeights;
		float mBlurAmount;
	};
}