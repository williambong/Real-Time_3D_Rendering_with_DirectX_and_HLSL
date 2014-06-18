#pragma once

#include "DrawableGameComponent.h"

using namespace Library;

namespace Library
{
	class Effect;
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
	class EnvironmentMappingMaterial;

	class EnvironmentMappingDemo : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(EnvironmentMappingDemo, DrawableGameComponent)

	public:		
		EnvironmentMappingDemo(Game& game, Camera& camera);
		~EnvironmentMappingDemo();

		virtual void Initialize() override;
		virtual void Update(const GameTime& gameTime) override;
		virtual void Draw(const GameTime& gameTime) override;

	private:
		EnvironmentMappingDemo();
		EnvironmentMappingDemo(const EnvironmentMappingDemo& rhs);
		EnvironmentMappingDemo& operator=(const EnvironmentMappingDemo& rhs);

		static const XMVECTORF32 EnvColor;

		Effect* mEffect;
		EnvironmentMappingMaterial* mMaterial;		
		ID3D11ShaderResourceView* mColorTextureShaderResourceView;
		ID3D11ShaderResourceView* mCubeMapShaderResourceView;
		ID3D11Buffer* mVertexBuffer;
		ID3D11Buffer* mIndexBuffer;
		UINT mIndexCount;

		XMCOLOR mAmbientColor;
		Keyboard* mKeyboard;
		XMFLOAT4X4 mWorldMatrix;
		float mReflectionAmount;

		RenderStateHelper* mRenderStateHelper;
		SpriteBatch* mSpriteBatch;
		SpriteFont* mSpriteFont;
		XMFLOAT2 mTextPosition;
	};
}
