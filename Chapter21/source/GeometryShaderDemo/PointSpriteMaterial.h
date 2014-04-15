#pragma once

#include "Common.h"
#include "Material.h"
#include "VertexDeclarations.h"

using namespace Library;

namespace Rendering
{
	class PointSpriteMaterial : public Material
	{
		RTTI_DECLARATIONS(PointSpriteMaterial, Material)

		MATERIAL_VARIABLE_DECLARATION(ViewProjection)
		MATERIAL_VARIABLE_DECLARATION(CameraPosition)
		MATERIAL_VARIABLE_DECLARATION(CameraUp)
		MATERIAL_VARIABLE_DECLARATION(ColorTexture)

	public:
		PointSpriteMaterial();

		virtual void Initialize(Effect& effect) override;
		virtual void CreateVertexBuffer(ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** vertexBuffer) const override;
		void CreateVertexBuffer(ID3D11Device* device, VertexPositionSize* vertices, UINT vertexCount, ID3D11Buffer** vertexBuffer) const;
		virtual UINT VertexSize() const override;
	};
}

