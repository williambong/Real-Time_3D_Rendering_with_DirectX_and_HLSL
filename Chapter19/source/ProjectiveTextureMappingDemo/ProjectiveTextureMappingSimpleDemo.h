#pragma once

#include "DrawableGameComponent.h"
#include "Frustum.h"

using namespace Library;

namespace Library
{
	class Effect;
	class PointLight;
	class Keyboard;
	class ProxyModel;
	class Projector;
	class RenderableFrustum;
	class ProjectiveTextureMappingMaterial;
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

	class ProjectiveTextureMappingSimpleDemo : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(ProjectiveTextureMappingSimpleDemo, DrawableGameComponent)

	public:		
		ProjectiveTextureMappingSimpleDemo(Game& game, Camera& camera);
		~ProjectiveTextureMappingSimpleDemo();

		virtual void Initialize() override;
		virtual void Update(const GameTime& gameTime) override;
		virtual void Draw(const GameTime& gameTime) override;

	private:
		ProjectiveTextureMappingSimpleDemo();
		ProjectiveTextureMappingSimpleDemo(const ProjectiveTextureMappingSimpleDemo& rhs);
		ProjectiveTextureMappingSimpleDemo& operator=(const ProjectiveTextureMappingSimpleDemo& rhs);

		void UpdateTechnique();
		void UpdateAmbientLight(const GameTime& gameTime);
		void UpdatePointLightAndProjector(const GameTime& gameTime);
		void UpdateSpecularLight(const GameTime& gameTime);
		void InitializeProjectedTextureScalingMatrix(UINT textureWidth, UINT textureHeight);

		static const float LightModulationRate;		
		static const float LightMovementRate;
		static const XMFLOAT2 LightRotationRate;

		Keyboard* mKeyboard;
		XMCOLOR mAmbientColor;
		PointLight* mPointLight;		
		XMCOLOR mSpecularColor;
		float mSpecularPower;		

		ProxyModel* mProxyModel;
		Projector* mProjector;
		Frustum mProjectorFrustum;
		RenderableFrustum* mRenderableProjectorFrustum;
		ID3D11ShaderResourceView* mProjectedTexture;
		
		Effect* mProjectiveTextureMappingEffect;
		ProjectiveTextureMappingMaterial * mProjectiveTextureMappingMaterial;		
		ID3D11Buffer* mPlaneVertexBuffer;
		ID3D11Buffer* mPlaneIndexBuffer;
		UINT mPlaneVertexCount;
		XMFLOAT4X4 mPlaneWorldMatrix;
		XMFLOAT4X4 mProjectedTextureScalingMatrix;
		ID3D11ShaderResourceView* mCheckerboardTexture;
		
		bool mUseNoReverseTechnique;

		RenderStateHelper* mRenderStateHelper;
		SpriteBatch* mSpriteBatch;
		SpriteFont* mSpriteFont;
		XMFLOAT2 mTextPosition;
	};
}
