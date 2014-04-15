#pragma once

#include "Common.h"
#include "Material.h"
#include "VertexDeclarations.h"

using namespace Library;

namespace Rendering
{
	class QuadHeightmapTessellationMaterial : public Material
	{
		RTTI_DECLARATIONS(QuadHeightmapTessellationMaterial, Material)

		MATERIAL_VARIABLE_DECLARATION(WorldViewProjection)
		MATERIAL_VARIABLE_DECLARATION(TextureMatrix)
		MATERIAL_VARIABLE_DECLARATION(DisplacementScale)
		MATERIAL_VARIABLE_DECLARATION(TessellationEdgeFactors)
		MATERIAL_VARIABLE_DECLARATION(TessellationInsideFactors)
		MATERIAL_VARIABLE_DECLARATION(Heightmap)

	public:
		QuadHeightmapTessellationMaterial();

		virtual void Initialize(Effect& effect) override;
		virtual void CreateVertexBuffer(ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** vertexBuffer) const override;
		void CreateVertexBuffer(ID3D11Device* device, VertexPositionTexture* vertices, UINT vertexCount, ID3D11Buffer** vertexBuffer) const;
		virtual UINT VertexSize() const override;
	};
}

