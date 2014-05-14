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
	class PhongMaterial;

	class PhongDemo : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(PhongDemo, DrawableGameComponent)

	public:		
		PhongDemo(Game& game, Camera& camera);
		~PhongDemo();

		virtual void Initialize() override;
		virtual void Update(const GameTime& gameTime) override;
		virtual void Draw(const GameTime& gameTime) override;

	private:
		PhongDemo();
		PhongDemo(const PhongDemo& rhs);
		PhongDemo& operator=(const PhongDemo& rhs);

		void UpdateAmbientLight(const GameTime& gameTime);
		void UpdateDirectionalLight(const GameTime& gameTime);
		void UpdateSpecularLight(const GameTime& gameTime);

		static const float LightModulationRate;
		static const XMFLOAT2 LightRotationRate;

		Effect* mEffect;
		PhongMaterial* mMaterial;		
		ID3D11ShaderResourceView* mTextureShaderResourceView;
		ID3D11Buffer* mVertexBuffer;
		ID3D11Buffer* mIndexBuffer;
		UINT mIndexCount;

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
