#pragma once

#include "DrawableGameComponent.h"

using namespace Library;

namespace Library
{
	class Effect;
	class SpotLight;
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
	class SpotLightMaterial;

	class SpotLightDemo : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(SpotLightDemo, DrawableGameComponent)

	public:		
		SpotLightDemo(Game& game, Camera& camera);
		~SpotLightDemo();

		virtual void Initialize() override;
		virtual void Update(const GameTime& gameTime) override;
		virtual void Draw(const GameTime& gameTime) override;

	private:
		SpotLightDemo();
		SpotLightDemo(const SpotLightDemo& rhs);
		SpotLightDemo& operator=(const SpotLightDemo& rhs);

		void UpdateAmbientLight(const GameTime& gameTime);
		void UpdateSpotLight(const GameTime& gameTime);
		void UpdateSpecularLight(const GameTime& gameTime);

		static const float LightModulationRate;
		static const float LightMovementRate;
		static const XMFLOAT2 LightRotationRate;

		Effect* mEffect;
		SpotLightMaterial* mMaterial;		
		ID3D11ShaderResourceView* mTextureShaderResourceView;
		ID3D11Buffer* mVertexBuffer;
		ID3D11Buffer* mIndexBuffer;
		UINT mVertexCount;

		Keyboard* mKeyboard;
		XMCOLOR mAmbientColor;
		SpotLight* mSpotLight;		
		XMCOLOR mSpecularColor;
		float mSpecularPower;
		XMFLOAT4X4 mWorldMatrix;

		ProxyModel* mProxyModel;

		RenderStateHelper* mRenderStateHelper;
		SpriteBatch* mSpriteBatch;
		SpriteFont* mSpriteFont;
		XMFLOAT2 mTextPosition;
	};
}
