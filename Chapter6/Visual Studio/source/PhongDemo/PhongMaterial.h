#pragma once

#include "Common.h"
#include "Material.h"
#include "VertexDeclarations.h"

using namespace Library;

namespace Rendering
{
	class PhongMaterial : public Material
	{
		RTTI_DECLARATIONS(PhongMaterial, Material)

		MATERIAL_VARIABLE_DECLARATION(WorldViewProjection)
		MATERIAL_VARIABLE_DECLARATION(World)
		MATERIAL_VARIABLE_DECLARATION(SpecularColor)
		MATERIAL_VARIABLE_DECLARATION(SpecularPower)
		MATERIAL_VARIABLE_DECLARATION(AmbientColor)
		MATERIAL_VARIABLE_DECLARATION(LightColor)
		MATERIAL_VARIABLE_DECLARATION(LightDirection)
		MATERIAL_VARIABLE_DECLARATION(CameraPosition)
		MATERIAL_VARIABLE_DECLARATION(ColorTexture)

	public:
		PhongMaterial();		

		virtual void Initialize(Effect& effect) override;
		virtual void CreateVertexBuffer(ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** vertexBuffer) const override;
		void CreateVertexBuffer(ID3D11Device* device, VertexPositionTextureNormal* vertices, UINT vertexCount, ID3D11Buffer** vertexBuffer) const;
		virtual UINT VertexSize() const override;
	};
}

