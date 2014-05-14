#pragma once

#include "Common.h"
#include "Material.h"

using namespace Library;

namespace Rendering
{
	struct HelloShadersVertex
	{
		XMFLOAT3 Position;

		HelloShadersVertex() { }

		HelloShadersVertex(XMFLOAT3 position)
			: Position(position) { }
	};

	class HelloShadersMaterial : public Material
	{
		RTTI_DECLARATIONS(HelloShadersMaterial, Material)

		MATERIAL_VARIABLE_DECLARATION(WorldViewProjection)

	public:
		HelloShadersMaterial();

		virtual void Initialize(Effect& effect) override;
		virtual void CreateVertexBuffer(ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** vertexBuffer) const override;
		void CreateVertexBuffer(ID3D11Device* device, HelloShadersVertex* vertices, UINT vertexCount, ID3D11Buffer** vertexBuffer) const;
		virtual UINT VertexSize() const override;
	};
}

