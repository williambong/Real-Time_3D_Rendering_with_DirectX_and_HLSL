/************* Resources *************/

static const float2 QuadUVs[4] = {	float2(0.0f, 1.0f), // v0, lower-left
                                    float2(0.0f, 0.0f), // v1, upper-left
                                    float2(1.0f, 0.0f), // v2, upper-right
                                    float2(1.0f, 1.0f)  // v3, lower-right
                                 };

static const float2 QuadStripUVs[4] = { float2(0.0f, 1.0f), // v0, lower-left
                                        float2(0.0f, 0.0f), // v1, upper-left
                                        float2(1.0f, 1.0f), // v2, lower-right
                                        float2(1.0f, 0.0f)  // v3, upper-right										
                                      };

cbuffer CBufferPerFrame
{
    float3 CameraPosition : CAMERAPOSITION;
	float3 CameraUp;
}

cbuffer CBufferPerObject
{
    float4x4 ViewProjection;
}

Texture2D ColorTexture;

SamplerState ColorSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = WRAP;
    AddressV = WRAP;
};

/************* Data Structures *************/

struct VS_INPUT
{
    float4 Position : POSITION;
    float2 Size : SIZE;
};

struct VS_OUTPUT
{
    float4 Position : POSITION;
    float2 Size : SIZE;
};

struct VS_NOSIZE_OUTPUT
{
	float4 Position : POSITION;
};

struct GS_OUTPUT
{
    float4 Position : SV_Position;
    float2 TextureCoordinate : TEXCOORD;
};

/************* Vertex Shader *************/

VS_OUTPUT vertex_shader(VS_INPUT IN)
{
    VS_OUTPUT OUT = (VS_OUTPUT)0;
    
    OUT.Position = IN.Position;
    OUT.Size = IN.Size;
    
    return OUT;
}

VS_OUTPUT vertex_shader_nosize(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;

	OUT.Position = IN.Position;

	return OUT;
}

/************* Geometry Shader *************/

[maxvertexcount(6)]
void geometry_shader(point VS_OUTPUT IN[1],  inout TriangleStream<GS_OUTPUT> triStream)
{
    GS_OUTPUT OUT = (GS_OUTPUT)0;

    float2 halfSize = IN[0].Size / 2.0f;
    float3 direction = CameraPosition - IN[0].Position.xyz;
    float3 right = cross(normalize(direction), CameraUp);

    float3 offsetX = halfSize.x * right;
    float3 offsetY = halfSize.y * CameraUp;

    float4 vertices[4];
    vertices[0] = float4(IN[0].Position.xyz + offsetX - offsetY, 1.0f); // lower-left
    vertices[1] = float4(IN[0].Position.xyz + offsetX + offsetY, 1.0f); // upper-left
    vertices[2] = float4(IN[0].Position.xyz - offsetX + offsetY, 1.0f); // upper-right
    vertices[3] = float4(IN[0].Position.xyz - offsetX - offsetY, 1.0f); // lower-right
    
    // tri: 0, 1, 2
    OUT.Position = mul(vertices[0], ViewProjection);
    OUT.TextureCoordinate = QuadUVs[0];
    triStream.Append(OUT);

    OUT.Position = mul(vertices[1], ViewProjection);
    OUT.TextureCoordinate = QuadUVs[1];
    triStream.Append(OUT);

    OUT.Position = mul(vertices[2], ViewProjection);
    OUT.TextureCoordinate = QuadUVs[2];
    triStream.Append(OUT);
    triStream.RestartStrip();

    // tri: 0, 2, 3
    OUT.Position = mul(vertices[0], ViewProjection);
    OUT.TextureCoordinate = QuadUVs[0];
    triStream.Append(OUT);

    OUT.Position = mul(vertices[2], ViewProjection);
    OUT.TextureCoordinate = QuadUVs[2];
    triStream.Append(OUT);

    OUT.Position = mul(vertices[3], ViewProjection);
    OUT.TextureCoordinate = QuadUVs[3];
    triStream.Append(OUT);
}

[maxvertexcount(4)]
void geometry_shader_strip(point VS_OUTPUT IN[1], inout TriangleStream<GS_OUTPUT> triStream)
{
    GS_OUTPUT OUT = (GS_OUTPUT)0;

    float2 halfSize = IN[0].Size / 2.0f;
    float3 direction = CameraPosition - IN[0].Position.xyz;
    float3 right = cross(normalize(direction), CameraUp);

    float3 offsetX = halfSize.x * right;
    float3 offsetY = halfSize.y * CameraUp;

    float4 vertices[4];
    vertices[0] = float4(IN[0].Position.xyz + offsetX - offsetY, 1.0f); // lower-left
    vertices[1] = float4(IN[0].Position.xyz + offsetX + offsetY, 1.0f); // upper-left	
    vertices[2] = float4(IN[0].Position.xyz - offsetX - offsetY, 1.0f); // lower-right
    vertices[3] = float4(IN[0].Position.xyz - offsetX + offsetY, 1.0f); // upper-right

    [unroll]
    for (int i = 0; i < 4; i++)
    {
        OUT.Position = mul(vertices[i], ViewProjection);
        OUT.TextureCoordinate = QuadStripUVs[i];

        triStream.Append(OUT);
    }
}

[maxvertexcount(4)]
void geometry_shader_nosize(point VS_NOSIZE_OUTPUT IN[1], uint primitiveID : SV_PrimitiveID, inout TriangleStream<GS_OUTPUT> triStream)
{
    GS_OUTPUT OUT = (GS_OUTPUT)0;

	float size = primitiveID + 1.0f;
    
    float2 halfSize = size / 2.0f;
    float3 direction = CameraPosition - IN[0].Position.xyz;
    float3 right = cross(normalize(direction), CameraUp);

    float3 offsetX = halfSize.x * right;
    float3 offsetY = halfSize.y * CameraUp;

    float4 vertices[4];
    vertices[0] = float4(IN[0].Position.xyz + offsetX - offsetY, 1.0f); // lower-left
    vertices[1] = float4(IN[0].Position.xyz + offsetX + offsetY, 1.0f); // upper-left	
    vertices[2] = float4(IN[0].Position.xyz - offsetX - offsetY, 1.0f); // lower-right
    vertices[3] = float4(IN[0].Position.xyz - offsetX + offsetY, 1.0f); // upper-right

    [unroll]
    for (int i = 0; i < 4; i++)
    {
        OUT.Position = mul(vertices[i], ViewProjection);
        OUT.TextureCoordinate = QuadStripUVs[i];

        triStream.Append(OUT);
    }
}

/************* Pixel Shader *************/

float4 pixel_shader(GS_OUTPUT IN) : SV_Target
{
    return ColorTexture.Sample(ColorSampler, IN.TextureCoordinate);
}

/************* Techniques *************/

technique11 main11
{
    pass p0
    {
        SetVertexShader(CompileShader(vs_5_0, vertex_shader()));
        SetGeometryShader(CompileShader(gs_5_0, geometry_shader()));
        SetPixelShader(CompileShader(ps_5_0, pixel_shader()));
    }
}

technique11 main11_strip
{
	pass p0
	{
		SetVertexShader(CompileShader(vs_5_0, vertex_shader()));
		SetGeometryShader(CompileShader(gs_5_0, geometry_shader_strip()));
		SetPixelShader(CompileShader(ps_5_0, pixel_shader()));
	}
}

technique11 main11_nosize
{
	pass p0
	{
		SetVertexShader(CompileShader(vs_5_0, vertex_shader_nosize()));
		SetGeometryShader(CompileShader(gs_5_0, geometry_shader_nosize()));
		SetPixelShader(CompileShader(ps_5_0, pixel_shader()));
	}
}
