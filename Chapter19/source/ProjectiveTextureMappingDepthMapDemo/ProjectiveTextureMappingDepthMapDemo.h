#pragma once

#include "DrawableGameComponent.h"
#include "Frustum.h"
#include "RenderStateHelper.h"

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
	class DepthMapMaterial;
	class DepthMap;
}

namespace DirectX
{
	class SpriteBatch;
	class SpriteFont;
}

namespace Rendering
{
	class ProjectiveTextureMappingDepthMapDemo : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(ProjectiveTextureMappingDepthMapDemo, DrawableGameComponent)

	public:		
		ProjectiveTextureMappingDepthMapDemo(Game& game, Camera& camera);
		~ProjectiveTextureMappingDepthMapDemo();

		virtual void Initialize() override;
		virtual void Update(const GameTime& gameTime) override;
		virtual void Draw(const GameTime& gameTime) override;

	private:
		ProjectiveTextureMappingDepthMapDemo();
		ProjectiveTextureMappingDepthMapDemo(const ProjectiveTextureMappingDepthMapDemo& rhs);
		ProjectiveTextureMappingDepthMapDemo& operator=(const ProjectiveTextureMappingDepthMapDemo& rhs);

		void UpdateDepthBias(const GameTime& gameTime);
		void UpdateAmbientLight(const GameTime& gameTime);
		void UpdatePointLightAndProjector(const GameTime& gameTime);
		void UpdateSpecularLight(const GameTime& gameTime);
		void InitializeProjectedTextureScalingMatrix();

		static const float DepthBiasModulationRate;
		static const float LightModulationRate;
		static const float LightMovementRate;
		static const XMFLOAT2 LightRotationRate;
		static const UINT DepthMapWidth;
		static const UINT DepthMapHeight;
		static const RECT DepthMapDestinationRectangle;

		Keyboard* mKeyboard;
		XMCOLOR mAmbientColor;
		PointLight* mPointLight;
		XMCOLOR mSpecularColor;
		float mSpecularPower;
		ProxyModel* mProxyModel;
		RenderStateHelper mRenderStateHelper;
		Projector* mProjector;
		Frustum mProjectorFrustum;
		RenderableFrustum* mRenderableProjectorFrustum;
		ID3D11ShaderResourceView* mProjectedTexture;
		
		ID3D11Buffer* mPlanePositionVertexBuffer;
		ID3D11Buffer* mPlanePositionUVNormalVertexBuffer;
		ID3D11Buffer* mPlaneIndexBuffer;		
		UINT mPlaneVertexCount;
		XMFLOAT4X4 mPlaneWorldMatrix;
		ID3D11ShaderResourceView* mCheckerboardTexture;

		Effect* mProjectiveTextureMappingEffect;
		ProjectiveTextureMappingMaterial * mProjectiveTextureMappingMaterial;
		ID3D11Buffer* mModelPositionVertexBuffer;
		ID3D11Buffer* mModelPositionUVNormalVertexBuffer;		
		ID3D11Buffer* mModelIndexBuffer;
		UINT mModelIndexCount;
		XMFLOAT4X4 mModelWorldMatrix;
		XMFLOAT4X4 mProjectedTextureScalingMatrix;

		Effect* mDepthMapEffect;
		DepthMapMaterial* mDepthMapMaterial;
		DepthMap* mDepthMap;
		bool mDrawDepthMap;
		SpriteBatch* mSpriteBatch;
		float mDepthBias;
		SpriteFont* mSpriteFont;
		XMFLOAT2 mTextPosition;
	};
}
