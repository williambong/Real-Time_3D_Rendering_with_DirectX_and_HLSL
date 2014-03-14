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
	class DiffuseLightingMaterial;

	class DiffuseLightingDemo : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(DiffuseLightingDemo, DrawableGameComponent)

	public:
		DiffuseLightingDemo(Game& game, Camera& camera);
		~DiffuseLightingDemo();

		virtual void Initialize() override;
		virtual void Update(const GameTime& gameTime) override;
		virtual void Draw(const GameTime& gameTime) override;

	private:
		DiffuseLightingDemo();
		DiffuseLightingDemo(const DiffuseLightingDemo& rhs);
		DiffuseLightingDemo& operator=(const DiffuseLightingDemo& rhs);

		void UpdateAmbientLight(const GameTime& gameTime);
		void UpdateDirectionalLight(const GameTime& gameTime);

		static const float LightModulationRate;
		static const XMFLOAT2 LightRotationRate;

		Effect* mEffect;
		DiffuseLightingMaterial* mMaterial;		
		ID3D11ShaderResourceView* mTextureShaderResourceView;
		ID3D11Buffer* mVertexBuffer;
		ID3D11Buffer* mIndexBuffer;
		UINT mIndexCount;
		
		XMCOLOR mAmbientColor;
		DirectionalLight* mDirectionalLight;
		Keyboard* mKeyboard;
		XMFLOAT4X4 mWorldMatrix;

		ProxyModel* mProxyModel;

		RenderStateHelper* mRenderStateHelper;
		SpriteBatch* mSpriteBatch;
		SpriteFont* mSpriteFont;
		XMFLOAT2 mTextPosition;
	};
}
