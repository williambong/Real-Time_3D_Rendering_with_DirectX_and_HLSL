#pragma once

#include "DrawableGameComponent.h"

using namespace Library;

namespace Library
{
	class Effect;
	class PointLight;
	class Keyboard;
	class ProxyModel;
	class RenderStateHelper;
}

namespace DirectX
{
	class SpriteBatch;
	class SpriteFont;
}

namespace Rendering
{
	class MultiplePointLightsMaterial;

	class MultiplePointLightsDemo : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(MultiplePointLightsDemo, DrawableGameComponent)

	public:		
		MultiplePointLightsDemo(Game& game, Camera& camera);
		~MultiplePointLightsDemo();

		virtual void Initialize() override;
		virtual void Update(const GameTime& gameTime) override;
		virtual void Draw(const GameTime& gameTime) override;

	private:
		MultiplePointLightsDemo();
		MultiplePointLightsDemo(const MultiplePointLightsDemo& rhs);
		MultiplePointLightsDemo& operator=(const MultiplePointLightsDemo& rhs);

		void UpdateAmbientLight(const GameTime& gameTime);
		void UpdatePointLights(const GameTime& gameTime);
		void UpdateSpecularLight(const GameTime& gameTime);

		static const float LightModulationRate;
		static const float LightMovementRate;
		static const int PointLightCount;

		Effect* mEffect;
		MultiplePointLightsMaterial* mMaterial;		
		ID3D11ShaderResourceView* mTextureShaderResourceView;
		ID3D11Buffer* mVertexBuffer;
		ID3D11Buffer* mIndexBuffer;
		UINT mIndexCount;

		Keyboard* mKeyboard;
		XMCOLOR mAmbientColor;		
		XMCOLOR mSpecularColor;
		float mSpecularPower;
		XMFLOAT4X4 mWorldMatrix;
		std::vector<PointLight*> mPointLights;
		std::vector<ProxyModel*> mProxyModels;

		RenderStateHelper* mRenderStateHelper;
		SpriteBatch* mSpriteBatch;
		SpriteFont* mSpriteFont;
		XMFLOAT2 mTextPosition;
	};
}
