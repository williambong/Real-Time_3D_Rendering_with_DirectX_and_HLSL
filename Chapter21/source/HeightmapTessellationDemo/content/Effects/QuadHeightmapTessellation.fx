/************* Resources *************/
static const float4 ColorWheat = { 0.961f, 0.871f, 0.702f, 1.0f };

cbuffer CBufferPerFrame
{
	float TessellationEdgeFactors[4];
	float TessellationInsideFactors[2];
}

cbuffer CBufferPerObject
{
	float4x4 WorldViewProjection;
	float4x4 TextureMatrix;

	float DisplacementScale;
}

Texture2D Heightmap;

SamplerState TrilinearSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};

/************* Data Structures *************/

struct VS_INPUT
{
	float4 ObjectPosition : POSITION;
	float2 TextureCoordinate: TEXCOORD;
};

struct VS_OUTPUT
{
	float4 ObjectPosition : POSITION;
	float2 TextureCoordinate: TEXCOORD;
};

struct HS_CONSTANT_OUTPUT
{
	float EdgeFactors[4] : SV_TessFactor;
	float InsideFactors[2] : SV_InsideTessFactor;
};

struct HS_OUTPUT
{
	float4 ObjectPosition : POSITION;
	float2 TextureCoordinate: TEXCOORD;
};

struct DS_OUTPUT
{
	float4 Position : SV_Position;
};

/************* Vertex Shader *************/

VS_OUTPUT vertex_shader(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;

	OUT.ObjectPosition = IN.ObjectPosition;
	OUT.TextureCoordinate = mul(float4(IN.TextureCoordinate, 0, 1), TextureMatrix).xy;

	return OUT;
}

/************* Hull Shaders *************/

HS_CONSTANT_OUTPUT constant_hull_shader(InputPatch<VS_OUTPUT, 4> patch, uint patchID : SV_PrimitiveID)
{
	HS_CONSTANT_OUTPUT OUT = (HS_CONSTANT_OUTPUT)0;

	[unroll]
	for (int i = 0; i < 4; i++)
	{
		OUT.EdgeFactors[i] = TessellationEdgeFactors[i];
	}

	OUT.InsideFactors[0] = TessellationInsideFactors[0];
	OUT.InsideFactors[1] = TessellationInsideFactors[1];

	return OUT;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("constant_hull_shader")]
HS_OUTPUT hull_shader(InputPatch<VS_OUTPUT, 4> patch, uint controlPointID : SV_OutputControlPointID)
{
	HS_OUTPUT OUT = (HS_OUTPUT)0;

	OUT.ObjectPosition = patch[controlPointID].ObjectPosition;
	OUT.TextureCoordinate = patch[controlPointID].TextureCoordinate;

	return OUT;
}

/************* Domain Shader *************/

[domain("quad")]
DS_OUTPUT domain_shader(HS_CONSTANT_OUTPUT IN, float2 uv : SV_DomainLocation, const OutputPatch<HS_OUTPUT, 4> patch)
{
	DS_OUTPUT OUT;

	float4 v0 = lerp(patch[0].ObjectPosition, patch[1].ObjectPosition, uv.x);
	float4 v1 = lerp(patch[2].ObjectPosition, patch[3].ObjectPosition, uv.x);
	float4 objectPosition = lerp(v0, v1, uv.y);

	float2 texCoord0 = lerp(patch[0].TextureCoordinate, patch[1].TextureCoordinate, uv.x);
	float2 texCoord1 = lerp(patch[2].TextureCoordinate, patch[3].TextureCoordinate, uv.x);
	float2 textureCoordinate = lerp(texCoord0, texCoord1, uv.y);

	objectPosition.y = (2 * Heightmap.SampleLevel(TrilinearSampler, textureCoordinate, 0).x - 1) * DisplacementScale;
	
	OUT.Position = mul(float4(objectPosition.xyz, 1.0f), WorldViewProjection);

	return OUT;
}

/************* Pixel Shader *************/

float4 pixel_shader(DS_OUTPUT IN) : SV_Target
{
	return ColorWheat;
}

/************* Techniques *************/

technique11 main11
{
	pass p0
	{
		SetVertexShader(CompileShader(vs_5_0, vertex_shader()));
		SetHullShader(CompileShader(hs_5_0, hull_shader()));
		SetDomainShader(CompileShader(ds_5_0, domain_shader()));
		SetPixelShader(CompileShader(ps_5_0, pixel_shader()));
	}
}