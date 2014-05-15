#include "TransparencyMappingMaterial.h"
#include "GameException.h"
#include "Mesh.h"

namespace Rendering
{
	RTTI_DEFINITIONS(TransparencyMappingMaterial)

	TransparencyMappingMaterial::TransparencyMappingMaterial()
		: Material("alphaBlendingWithoutFog"),
		  MATERIAL_VARIABLE_INITIALIZATION(WorldViewProjection), MATERIAL_VARIABLE_INITIALIZATION(World),
		  MATERIAL_VARIABLE_INITIALIZATION(SpecularColor), MATERIAL_VARIABLE_INITIALIZATION(SpecularPower),
		  MATERIAL_VARIABLE_INITIALIZATION(AmbientColor), MATERIAL_VARIABLE_INITIALIZATION(LightColor),
		  MATERIAL_VARIABLE_INITIALIZATION(LightPosition), MATERIAL_VARIABLE_INITIALIZATION(LightRadius),
		  MATERIAL_VARIABLE_INITIALIZATION(FogColor), MATERIAL_VARIABLE_INITIALIZATION(FogStart),
		  MATERIAL_VARIABLE_INITIALIZATION(FogRange), MATERIAL_VARIABLE_INITIALIZATION(CameraPosition),
		  MATERIAL_VARIABLE_INITIALIZATION(ColorTexture), MATERIAL_VARIABLE_INITIALIZATION(TransparencyMap)
	{
	}

	MATERIAL_VARIABLE_DEFINITION(TransparencyMappingMaterial, WorldViewProjection)
	MATERIAL_VARIABLE_DEFINITION(TransparencyMappingMaterial, World)
	MATERIAL_VARIABLE_DEFINITION(TransparencyMappingMaterial, SpecularColor)
	MATERIAL_VARIABLE_DEFINITION(TransparencyMappingMaterial, SpecularPower)
	MATERIAL_VARIABLE_DEFINITION(TransparencyMappingMaterial, AmbientColor)
	MATERIAL_VARIABLE_DEFINITION(TransparencyMappingMaterial, LightColor)
	MATERIAL_VARIABLE_DEFINITION(TransparencyMappingMaterial, LightPosition)
	MATERIAL_VARIABLE_DEFINITION(TransparencyMappingMaterial, LightRadius)
	MATERIAL_VARIABLE_DEFINITION(TransparencyMappingMaterial, FogColor)
	MATERIAL_VARIABLE_DEFINITION(TransparencyMappingMaterial, FogStart)
	MATERIAL_VARIABLE_DEFINITION(TransparencyMappingMaterial, FogRange)
	MATERIAL_VARIABLE_DEFINITION(TransparencyMappingMaterial, CameraPosition)
	MATERIAL_VARIABLE_DEFINITION(TransparencyMappingMaterial, ColorTexture)
	MATERIAL_VARIABLE_DEFINITION(TransparencyMappingMaterial, TransparencyMap)

	void TransparencyMappingMaterial::Initialize(Effect& effect)
	{
		Material::Initialize(effect);

		MATERIAL_VARIABLE_RETRIEVE(WorldViewProjection)
		MATERIAL_VARIABLE_RETRIEVE(World)
		MATERIAL_VARIABLE_RETRIEVE(SpecularColor)
		MATERIAL_VARIABLE_RETRIEVE(SpecularPower)
		MATERIAL_VARIABLE_RETRIEVE(AmbientColor)
		MATERIAL_VARIABLE_RETRIEVE(LightColor)
		MATERIAL_VARIABLE_RETRIEVE(LightPosition)
		MATERIAL_VARIABLE_RETRIEVE(LightRadius)
		MATERIAL_VARIABLE_RETRIEVE(FogColor)
		MATERIAL_VARIABLE_RETRIEVE(FogStart)
		MATERIAL_VARIABLE_RETRIEVE(FogRange)
		MATERIAL_VARIABLE_RETRIEVE(CameraPosition)
		MATERIAL_VARIABLE_RETRIEVE(ColorTexture)
		MATERIAL_VARIABLE_RETRIEVE(TransparencyMap)

		D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		CreateInputLayout("alphaBlendingWithoutFog", "p0", inputElementDescriptions, ARRAYSIZE(inputElementDescriptions));
		CreateInputLayout("alphaBlendingWithFog", "p0", inputElementDescriptions, ARRAYSIZE(inputElementDescriptions));
	}

	void TransparencyMappingMaterial::CreateVertexBuffer(ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** vertexBuffer) const
	{
		const std::vector<XMFLOAT3>& sourceVertices = mesh.Vertices();
		std::vector<XMFLOAT3>* textureCoordinates = mesh.TextureCoordinates().at(0);
		assert(textureCoordinates->size() == sourceVertices.size());
		const std::vector<XMFLOAT3>& normals = mesh.Normals();
		assert(textureCoordinates->size() == sourceVertices.size());

		std::vector<VertexPositionTextureNormal> vertices;
		vertices.reserve(sourceVertices.size());
		for (UINT i = 0; i < sourceVertices.size(); i++)
		{
			XMFLOAT3 position = sourceVertices.at(i);
			XMFLOAT3 uv = textureCoordinates->at(i);
			XMFLOAT3 normal = normals.at(i);
			vertices.push_back(VertexPositionTextureNormal(XMFLOAT4(position.x, position.y, position.z, 1.0f), XMFLOAT2(uv.x, uv.y), normal));
		}

		CreateVertexBuffer(device, &vertices[0], vertices.size(), vertexBuffer);
	}

	void TransparencyMappingMaterial::CreateVertexBuffer(ID3D11Device* device, VertexPositionTextureNormal* vertices, UINT vertexCount, ID3D11Buffer** vertexBuffer) const
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

	UINT TransparencyMappingMaterial::VertexSize() const
	{
		return sizeof(VertexPositionTextureNormal);
	}
}