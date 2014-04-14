#pragma once

#include "Common.h"
#include "DrawableGameComponent.h"

namespace Library
{
	class Effect;
	class Pass;
	class DistortionMappingMaterial;
	class FullScreenRenderTarget;
	class FullScreenQuad;
	class Mesh;

	enum DistortionTechnique
	{
		DistortionTechniqueDisplacement = 0,
		DistortionTechniqueEnd
	};

	class DistortionMapping : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(DistortionMapping, DrawableGameComponent)

	public:
		DistortionMapping(Game& game, Camera& camera);
		~DistortionMapping();

		DistortionTechnique GetDistortionTechnique() const;
		void SetDistortionTechnique(DistortionTechnique distortionTechnique);
		
		ID3D11ShaderResourceView* SceneTexture();
		void SetSceneTexture(ID3D11ShaderResourceView& sceneTexture);

		DistortionMappingMaterial* GetMaterial();

		virtual void Initialize() override;		
		virtual void Draw(const GameTime& gameTime) override;

		void BeginDistortionMap();
		void DrawMeshToDistortionMap(Mesh& mesh, CXMMATRIX worldMatrix);
		void EndDistortionMap();

	private:
		DistortionMapping();
		DistortionMapping(const DistortionMapping& rhs);
		DistortionMapping& operator=(const DistortionMapping& rhs);

		void UpdateDistortionCompositeMaterial();

		Effect* mDistortionEffect;
		DistortionMappingMaterial* mDistortionMappingMaterial;
		Pass* mDistortionPass;
		ID3D11InputLayout* mDistortionInputLayout;
		DistortionTechnique mDistortionTechnique;
		ID3D11ShaderResourceView* mSceneTexture;
		FullScreenRenderTarget* mRenderTarget;
		FullScreenQuad* mFullScreenQuad;
	};
}