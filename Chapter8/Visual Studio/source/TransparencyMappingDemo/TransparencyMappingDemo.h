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
	class TransparencyMappingMaterial;

	class TransparencyMappingDemo : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(TransparencyMappingDemo, DrawableGameComponent)

	public:		
		TransparencyMappingDemo(Game& game, Camera& camera);
		~TransparencyMappingDemo();

		virtual void Initialize() override;
		virtual void Update(const GameTime& gameTime) override;
		virtual void Draw(const GameTime& gameTime) override;

	private:
		TransparencyMappingDemo();
		TransparencyMappingDemo(const TransparencyMappingDemo& rhs);
		TransparencyMappingDemo& operator=(const TransparencyMappingDemo& rhs);

		void UpdateAmbientLight(const GameTime& gameTime);
		void UpdatePointLight(const GameTime& gameTime);
		void UpdateSpecularLight(const GameTime& gameTime);

		static const float LightModulationRate;
		static const float LightMovementRate;

		Effect* mEffect;
		TransparencyMappingMaterial* mMaterial;		
		ID3D11ShaderResourceView* mColorTextureShaderResourceView;
		ID3D11ShaderResourceView* mTransparencyMapShaderResourceView;
		ID3D11Buffer* mVertexBuffer;
		UINT mVertexCount;

		Keyboard* mKeyboard;
		XMCOLOR mAmbientColor;
		PointLight* mPointLight;		
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
