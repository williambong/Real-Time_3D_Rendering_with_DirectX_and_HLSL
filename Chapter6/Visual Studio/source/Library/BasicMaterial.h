#pragma once

#include "Common.h"
#include "Material.h"
#include "VertexDeclarations.h"

namespace Library
{
    class BasicMaterial : public Material
    {
        RTTI_DECLARATIONS(BasicMaterial, Material)

        MATERIAL_VARIABLE_DECLARATION(WorldViewProjection)

    public:
        BasicMaterial();

        virtual void Initialize(Effect& effect) override;		
        virtual void CreateVertexBuffer(ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** vertexBuffer) const override;
        void CreateVertexBuffer(ID3D11Device* device, VertexPositionColor* vertices, UINT vertexCount, ID3D11Buffer** vertexBuffer) const;
        virtual UINT VertexSize() const override;
    };
}