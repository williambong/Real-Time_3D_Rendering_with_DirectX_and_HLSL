#pragma once

#include <functional>
#include "Common.h"
#include "DrawableGameComponent.h"

namespace Library
{
	class Effect;
	class BloomMaterial;
	class FullScreenRenderTarget;
	class FullScreenQuad;
	class GaussianBlur;

	typedef struct _BloomSettings
	{
		float BloomThreshold;
		float BlurAmount;
		float BloomIntensity;
		float BloomSaturation;
		float SceneIntensity;
		float SceneSaturation;
	} BloomSettings;

	enum BloomDrawMode
	{
		BloomDrawModeNormal = 0,
		BloomDrawModeExtractedTexture1,
		BloomDrawModeBlurredTexture,
		BloomDrawModeEnd
	};

	class Bloom : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(Bloom, DrawableGameComponent)

	public:
		Bloom(Game& game, Camera& camera);
		Bloom(Game& game, Camera& camera, const BloomSettings& bloomSettings);
		~Bloom();
		
		ID3D11ShaderResourceView* SceneTexture();
		void SetSceneTexture(ID3D11ShaderResourceView& sceneTexture);

		const BloomSettings& GetBloomSettings() const;
		void SetBloomSettings(const BloomSettings& bloomSettings);

		BloomDrawMode DrawMode() const;
		std::string DrawModeString() const;
		void SetDrawMode(BloomDrawMode drawMode);

		virtual void Initialize() override;
		virtual void Draw(const GameTime& gameTime) override;

	private:
		Bloom();
		Bloom(const Bloom& rhs);
		Bloom& operator=(const Bloom& rhs);

		void DrawNormal(const GameTime& gameTime);
		void DrawExtractedTexture(const GameTime& gameTime);
		void DrawBlurredTexture(const GameTime& gameTime);

		void UpdateBloomExtractMaterial();
		void UpdateBloomCompositeMaterial();
		void UpdateNoBloomMaterial();

		static const std::string DrawModeDisplayNames[];
		static const BloomSettings DefaultBloomSettings;		

		Effect* mBloomEffect;
		BloomMaterial* mBloomMaterial;
		ID3D11ShaderResourceView* mSceneTexture;
		FullScreenRenderTarget* mRenderTarget;
		FullScreenQuad* mFullScreenQuad;
		GaussianBlur* mGaussianBlur;
		BloomSettings mBloomSettings;
		BloomDrawMode mDrawMode;
		std::function<void(const GameTime& gameTime)> mDrawFunctions[BloomDrawModeEnd];
	};
}