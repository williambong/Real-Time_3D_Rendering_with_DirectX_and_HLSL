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
	class SkyboxMaterial;

	class SkyboxDemo : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(SkyboxDemo, DrawableGameComponent)

	public:		
		SkyboxDemo(Game& game, Camera& camera);
		~SkyboxDemo();

		virtual void Initialize() override;
		virtual void Update(const GameTime& gameTime) override;
		virtual void Draw(const GameTime& gameTime) override;

	private:
		SkyboxDemo();
		SkyboxDemo(const SkyboxDemo& rhs);
		SkyboxDemo& operator=(const SkyboxDemo& rhs);

		Effect* mEffect;
		SkyboxMaterial* mMaterial;		
		ID3D11ShaderResourceView* mCubeMapShaderResourceView;
		ID3D11Buffer* mVertexBuffer;
		ID3D11Buffer* mIndexBuffer;
		UINT mIndexCount;

		XMFLOAT4X4 mWorldMatrix;
		XMFLOAT4X4 mScaleMatrix;

		RenderStateHelper* mRenderStateHelper;
		SpriteBatch* mSpriteBatch;
		SpriteFont* mSpriteFont;
		XMFLOAT2 mTextPosition;
	};
}
