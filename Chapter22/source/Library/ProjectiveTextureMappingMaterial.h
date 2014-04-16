#pragma once

#include "Common.h"
#include "Material.h"
#include "VertexDeclarations.h"

namespace Library
{
    class ProjectiveTextureMappingMaterial : public Material
    {
        RTTI_DECLARATIONS(ProjectiveTextureMappingMaterial, Material)

        MATERIAL_VARIABLE_DECLARATION(WorldViewProjection)
		MATERIAL_VARIABLE_DECLARATION(World)
		MATERIAL_VARIABLE_DECLARATION(SpecularColor)
		MATERIAL_VARIABLE_DECLARATION(SpecularPower)
		MATERIAL_VARIABLE_DECLARATION(AmbientColor)
		MATERIAL_VARIABLE_DECLARATION(LightColor)
		MATERIAL_VARIABLE_DECLARATION(LightPosition)
		MATERIAL_VARIABLE_DECLARATION(LightRadius)
		MATERIAL_VARIABLE_DECLARATION(CameraPosition)

		MATERIAL_VARIABLE_DECLARATION(ColorTexture)
		MATERIAL_VARIABLE_DECLARATION(ProjectiveTextureMatrix)
		MATERIAL_VARIABLE_DECLARATION(ProjectedTexture)
		MATERIAL_VARIABLE_DECLARATION(DepthMap)
		MATERIAL_VARIABLE_DECLARATION(DepthBias)

    public:
        ProjectiveTextureMappingMaterial();

        virtual void Initialize(Effect& effect) override;		
        virtual void CreateVertexBuffer(ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** vertexBuffer) const override;
        void CreateVertexBuffer(ID3D11Device* device, VertexPositionTextureNormal* vertices, UINT vertexCount, ID3D11Buffer** vertexBuffer) const;
        virtual UINT VertexSize() const override;
    };
}