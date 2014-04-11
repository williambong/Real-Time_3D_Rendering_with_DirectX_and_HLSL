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
	class PointLightMaterial;

	class PointLightDemo : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(PointLightDemo, DrawableGameComponent)

	public:
		PointLightDemo(Game& game, Camera& camera);
		~PointLightDemo();

		const XMCOLOR& GetAmbientColor() const;
		const PointLight& GetPointLight() const;
		float GetSpecularPower() const;

		virtual void Initialize() override;
		virtual void Update(const GameTime& gameTime) override;
		virtual void Draw(const GameTime& gameTime) override;

	private:
		PointLightDemo();
		PointLightDemo(const PointLightDemo& rhs);
		PointLightDemo& operator=(const PointLightDemo& rhs);

		void UpdateAmbientLight(const GameTime& gameTime);
		void UpdatePointLight(const GameTime& gameTime);
		void UpdateSpecularLight(const GameTime& gameTime);

		static const float LightModulationRate;
		static const float LightMovementRate;

		Effect* mEffect;
		PointLightMaterial* mMaterial;
		ID3D11ShaderResourceView* mTextureShaderResourceView;
		ID3D11Buffer* mVertexBuffer;
		ID3D11Buffer* mIndexBuffer;
		UINT mIndexCount;

		Keyboard* mKeyboard;
		XMCOLOR mAmbientColor;
		PointLight* mPointLight;
		XMCOLOR mSpecularColor;
		float mSpecularPower;
		XMFLOAT4X4 mWorldMatrix;

		ProxyModel* mProxyModel;
	};
}
