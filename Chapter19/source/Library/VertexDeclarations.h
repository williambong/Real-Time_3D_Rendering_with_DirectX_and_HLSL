#pragma once

#include "Common.h"

namespace Library
{
	typedef struct _VertexPosition
    {
        XMFLOAT4 Position;

        _VertexPosition() { }

        _VertexPosition(XMFLOAT4 position)
            : Position(position) { }
    } VertexPosition;

	typedef struct _VertexPositionColor
    {
        XMFLOAT4 Position;
        XMFLOAT4 Color;

        _VertexPositionColor() { }

        _VertexPositionColor(XMFLOAT4 position, XMFLOAT4 color)
            : Position(position), Color(color) { }
    } VertexPositionColor;

	typedef struct _VertexPositionTexture
	{
		XMFLOAT4 Position;
		XMFLOAT2 TextureCoordinates;

		_VertexPositionTexture() { }

		_VertexPositionTexture(XMFLOAT4 position, XMFLOAT2 textureCoordinates)
			: Position(position), TextureCoordinates(textureCoordinates) { }
	} VertexPositionTexture;

	typedef struct _VertexPositionNormal
    {
        XMFLOAT4 Position;
        XMFLOAT3 Normal;

        _VertexPositionNormal() { }

        _VertexPositionNormal(XMFLOAT4 position, XMFLOAT3 normal)
            : Position(position), Normal(normal) { }
    } VertexPositionNormal;

	typedef struct _VertexPositionTextureNormal
	{
		XMFLOAT4 Position;
		XMFLOAT2 TextureCoordinates;
		XMFLOAT3 Normal;

		_VertexPositionTextureNormal() { }

		_VertexPositionTextureNormal(XMFLOAT4 position, XMFLOAT2 textureCoordinates, XMFLOAT3 normal)
			: Position(position), TextureCoordinates(textureCoordinates), Normal(normal) { }
	} VertexPositionTextureNormal;
}
