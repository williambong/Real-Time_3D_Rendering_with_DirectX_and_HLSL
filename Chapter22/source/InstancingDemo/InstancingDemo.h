#pragma once

#include "DrawableGameComponent.h"
#include "InstancingMaterial.h"

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
	class InstancingDemo : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(InstancingDemo, DrawableGameComponent)

	public:		
		InstancingDemo(Game& game, Camera& camera);
		~InstancingDemo();

		virtual void Initialize() override;
		virtual void Update(const GameTime& gameTime) override;
		virtual void Draw(const GameTime& gameTime) override;

	private:
		struct VertexBufferData
		{
			ID3D11Buffer* VertexBuffer;
			UINT Stride;
			UINT Offset;

			VertexBufferData(ID3D11Buffer* vertexBuffer, UINT stride, UINT offset)
				: VertexBuffer(vertexBuffer), Stride(stride), Offset(offset) { }
		};

		InstancingDemo();
		InstancingDemo(const InstancingDemo& rhs);
		InstancingDemo& operator=(const InstancingDemo& rhs);

		void UpdateAmbientLight(const GameTime& gameTime);
		void UpdatePointLight(const GameTime& gameTime);

		static const float LightModulationRate;
		static const float LightMovementRate;

		Effect* mEffect;
		InstancingMaterial* mMaterial;		
		ID3D11ShaderResourceView* mColorTexture;
		std::vector<VertexBufferData> mVertexBuffers;
		ID3D11Buffer* mIndexBuffer;
		UINT mIndexCount;
		UINT mInstanceCount;

		Keyboard* mKeyboard;
		XMCOLOR mAmbientColor;
		PointLight* mPointLight;		
		XMCOLOR mSpecularColor;
		float mSpecularPower;

		ProxyModel* mProxyModel;

		RenderStateHelper* mRenderStateHelper;
		SpriteBatch* mSpriteBatch;
		SpriteFont* mSpriteFont;
		XMFLOAT2 mTextPosition;
	};
}
