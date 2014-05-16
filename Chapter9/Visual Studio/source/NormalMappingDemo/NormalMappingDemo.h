#pragma once

#include "DrawableGameComponent.h"

using namespace Library;

namespace Library
{
	class Effect;
	class DirectionalLight;
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
	class NormalMappingMaterial;

	class NormalMappingDemo : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(NormalMappingDemo, DrawableGameComponent)

	public:		
		NormalMappingDemo(Game& game, Camera& camera);
		~NormalMappingDemo();

		virtual void Initialize() override;
		virtual void Update(const GameTime& gameTime) override;
		virtual void Draw(const GameTime& gameTime) override;

	private:
		NormalMappingDemo();
		NormalMappingDemo(const NormalMappingDemo& rhs);
		NormalMappingDemo& operator=(const NormalMappingDemo& rhs);

		void UpdateAmbientLight(const GameTime& gameTime);
		void UpdateDirectionalLight(const GameTime& gameTime);
		void UpdateSpecularLight(const GameTime& gameTime);

		static const float LightModulationRate;
		static const XMFLOAT2 LightRotationRate;

		Effect* mEffect;
		NormalMappingMaterial* mMaterial;		
		ID3D11ShaderResourceView* mColorTextureShaderResourceView;
		ID3D11ShaderResourceView* mNormalMapShaderResourceView;
		ID3D11Buffer* mVertexBuffer;
		UINT mVertexCount;

		Keyboard* mKeyboard;
		XMCOLOR mAmbientColor;
		DirectionalLight* mDirectionalLight;
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
