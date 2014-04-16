#pragma once

#include "Common.h"
#include "Material.h"
#include "VertexDeclarations.h"

using namespace Library;

namespace Rendering
{
	class InstancingMaterial : public Material
	{
		RTTI_DECLARATIONS(InstancingMaterial, Material)

		MATERIAL_VARIABLE_DECLARATION(ViewProjection)
		MATERIAL_VARIABLE_DECLARATION(AmbientColor)
		MATERIAL_VARIABLE_DECLARATION(LightColor)
		MATERIAL_VARIABLE_DECLARATION(LightPosition)
		MATERIAL_VARIABLE_DECLARATION(LightRadius)
		MATERIAL_VARIABLE_DECLARATION(CameraPosition)
		MATERIAL_VARIABLE_DECLARATION(ColorTexture)

	public:
		struct InstanceData
		{
			XMFLOAT4X4 World;
			XMFLOAT4 SpecularColor;
			float SpecularPower;

			InstanceData() { }

			InstanceData(const XMFLOAT4X4& world, const XMFLOAT4& specularColor, float specularPower)
				: World(world), SpecularColor(specularColor), SpecularPower(specularPower)
			{
			}

			InstanceData(CXMMATRIX world, const XMFLOAT4& specularColor, float specularPower)
				: World(), SpecularColor(specularColor), SpecularPower(specularPower)
			{
				XMStoreFloat4x4(&World, world);
			}
		};

		InstancingMaterial();		

		virtual void Initialize(Effect& effect) override;
		virtual void CreateVertexBuffer(ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** vertexBuffer) const override;
		void CreateVertexBuffer(ID3D11Device* device, VertexPositionTextureNormal* vertices, UINT vertexCount, ID3D11Buffer** vertexBuffer) const;
		virtual UINT VertexSize() const override;

		void CreateInstanceBuffer(ID3D11Device* device, std::vector<InstanceData>& instanceData, ID3D11Buffer** instanceBuffer) const;
		void CreateInstanceBuffer(ID3D11Device* device, InstanceData* instanceData, UINT instanceCount, ID3D11Buffer** instanceBuffer) const;
		UINT InstanceSize() const;
	};
}

