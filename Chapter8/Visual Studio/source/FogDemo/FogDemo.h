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
	class Pass;
}

namespace DirectX
{
	class SpriteBatch;
	class SpriteFont;
}

namespace Rendering
{
	class FogMaterial;

	class FogDemo : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(FogDemo, DrawableGameComponent)

	public:
		FogDemo(Game& game, Camera& camera);
		~FogDemo();

		virtual void Initialize() override;
		virtual void Update(const GameTime& gameTime) override;
		virtual void Draw(const GameTime& gameTime) override;

	private:
		FogDemo();
		FogDemo(const FogDemo& rhs);
		FogDemo& operator=(const FogDemo& rhs);

		void UpdateAmbientLight(const GameTime& gameTime);
		void UpdateDirectionalLight(const GameTime& gameTime);
		void SetActiveTechnique();

		static const float LightModulationRate;
		static const XMFLOAT2 LightRotationRate;
		static const XMVECTORF32 FogColor;

		Effect* mEffect;
		FogMaterial* mMaterial;		
		ID3D11ShaderResourceView* mTextureShaderResourceView;
		ID3D11Buffer* mVertexBuffer;
		ID3D11Buffer* mIndexBuffer;
		UINT mIndexCount;
		
		XMCOLOR mAmbientColor;
		DirectionalLight* mDirectionalLight;
		Keyboard* mKeyboard;
		XMFLOAT4X4 mWorldMatrix;
		float mFogStart;
		float mFogRange;
		bool mFogEnabled;

		Pass* mPass;
		ID3D11InputLayout* mInputLayout;

		ProxyModel* mProxyModel;

		RenderStateHelper* mRenderStateHelper;
		SpriteBatch* mSpriteBatch;
		SpriteFont* mSpriteFont;
		XMFLOAT2 mTextPosition;
	};
}
