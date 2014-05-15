#pragma once

#include "Common.h"
#include "Material.h"
#include "VertexDeclarations.h"

namespace Library
{
	class DistortionMappingMaterial : public Material
	{
		RTTI_DECLARATIONS(Material, DistortionMappingMaterial)
		
		MATERIAL_VARIABLE_DECLARATION(WorldViewProjection)
		MATERIAL_VARIABLE_DECLARATION(SceneTexture)
		MATERIAL_VARIABLE_DECLARATION(DistortionMap)
		MATERIAL_VARIABLE_DECLARATION(DisplacementScale)
		MATERIAL_VARIABLE_DECLARATION(Time)

	public:
		DistortionMappingMaterial();

		virtual void Initialize(Effect& effect) override;		
        virtual void CreateVertexBuffer(ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** vertexBuffer) const override;
        void CreateVertexBuffer(ID3D11Device* device, VertexPositionTexture* vertices, UINT vertexCount, ID3D11Buffer** vertexBuffer) const;
        virtual UINT VertexSize() const override;
	};
}

