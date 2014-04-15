#include "PointSpriteMaterial.h"
#include "GameException.h"
#include "Mesh.h"

namespace Rendering
{
	RTTI_DEFINITIONS(PointSpriteMaterial)

	PointSpriteMaterial::PointSpriteMaterial()
		: Material("main11"),
		  MATERIAL_VARIABLE_INITIALIZATION(ViewProjection), MATERIAL_VARIABLE_INITIALIZATION(CameraPosition),
		  MATERIAL_VARIABLE_INITIALIZATION(CameraUp), MATERIAL_VARIABLE_INITIALIZATION(ColorTexture)
	{
	}

	MATERIAL_VARIABLE_DEFINITION(PointSpriteMaterial, ViewProjection)
	MATERIAL_VARIABLE_DEFINITION(PointSpriteMaterial, CameraPosition)
	MATERIAL_VARIABLE_DEFINITION(PointSpriteMaterial, CameraUp)
	MATERIAL_VARIABLE_DEFINITION(PointSpriteMaterial, ColorTexture)

	void PointSpriteMaterial::Initialize(Effect& effect)
	{
		Material::Initialize(effect);

		MATERIAL_VARIABLE_RETRIEVE(ViewProjection)
		MATERIAL_VARIABLE_RETRIEVE(CameraPosition)
		MATERIAL_VARIABLE_RETRIEVE(CameraUp)
		MATERIAL_VARIABLE_RETRIEVE(ColorTexture)

		D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		for (Technique* technique : mEffect->Techniques())
		{
			for (Pass* pass : technique->Passes())
			{
				CreateInputLayout(*pass, inputElementDescriptions, ARRAYSIZE(inputElementDescriptions));
			}
		}
	}

	void PointSpriteMaterial::CreateVertexBuffer(ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** vertexBuffer) const
	{
		throw GameException("This method is unsupported for model meshes.");
	}

	void PointSpriteMaterial::CreateVertexBuffer(ID3D11Device* device, VertexPositionSize* vertices, UINT vertexCount, ID3D11Buffer** vertexBuffer) const
	{
		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
		vertexBufferDesc.ByteWidth = VertexSize() * vertexCount;
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;		
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData;
		ZeroMemory(&vertexSubResourceData, sizeof(vertexSubResourceData));
		vertexSubResourceData.pSysMem = vertices;
		if (FAILED(device->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, vertexBuffer)))
		{
			throw GameException("ID3D11Device::CreateBuffer() failed.");
		}
	}

	UINT PointSpriteMaterial::VertexSize() const
	{
		return sizeof(VertexPositionSize);
	}
}