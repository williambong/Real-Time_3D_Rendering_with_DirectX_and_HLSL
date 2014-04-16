#pragma once

#include "Common.h"
#include "Material.h"
#include "VertexDeclarations.h"

using namespace Library;

namespace Rendering
{
	class ComputeShaderMaterial : public Material
	{
		RTTI_DECLARATIONS(ComputeShaderMaterial, Material)

		// Variables for the compute shader
		MATERIAL_VARIABLE_DECLARATION(TextureSize)
		MATERIAL_VARIABLE_DECLARATION(BlueColor)
		MATERIAL_VARIABLE_DECLARATION(OutputTexture)		

		// Variables for the rendering shaders
		MATERIAL_VARIABLE_DECLARATION(ColorTexture)

	public:
		ComputeShaderMaterial();		

		virtual void Initialize(Effect& effect) override;
		virtual void CreateVertexBuffer(ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** vertexBuffer) const override;
		void CreateVertexBuffer(ID3D11Device* device, VertexPositionTexture* vertices, UINT vertexCount, ID3D11Buffer** vertexBuffer) const;
		virtual UINT VertexSize() const override;
	};
}

