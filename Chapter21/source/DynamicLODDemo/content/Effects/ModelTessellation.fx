/************* Resources *************/
static const float4 ColorWheat = { 0.961f, 0.871f, 0.702f, 1.0f };

cbuffer CBufferPerFrame
{
    float TessellationEdgeFactors[3];
    float TessellationInsideFactor;
    float3 CameraPosition : CAMERAPOSITION;
    int MaxTessellationFactor = 64;
    float MinTessellationDistance = 2.0f;
    float MaxTessellationDistance = 20.0f;
}

cbuffer CBufferPerObject
{
    float4x4 WorldViewProjection : WORLDVIEWPROJECTION;
    float4x4 World : WORLD;
}

Texture2D ColorTexture;

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
    float EdgeFactors[3] : SV_TessFactor;
    float InsideFactor : SV_InsideTessFactor;
};

struct HS_OUTPUT
{
    float4 ObjectPosition : POSITION;
    float2 TextureCoordinate: TEXCOORD;
};

struct DS_OUTPUT
{
    float4 Position : SV_Position;
    float2 TextureCoordinate: TEXCOORD;
};

/************* Vertex Shader *************/

VS_OUTPUT vertex_shader(VS_INPUT IN)
{
    VS_OUTPUT OUT = (VS_OUTPUT)0;
    
    OUT.ObjectPosition = IN.ObjectPosition;
    OUT.TextureCoordinate = IN.TextureCoordinate;

    return OUT;
}

/************* Hull Shaders *************/

HS_CONSTANT_OUTPUT constant_hull_shader(InputPatch<VS_OUTPUT, 3> patch, uint patchID : SV_PrimitiveID)
{
    HS_CONSTANT_OUTPUT OUT = (HS_CONSTANT_OUTPUT)0;

    [unroll]
    for (int i = 0; i < 3; i++)
    {
        OUT.EdgeFactors[i] = TessellationEdgeFactors[i];
    }

    OUT.InsideFactor = TessellationInsideFactor;

    return OUT;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("constant_hull_shader")]
HS_OUTPUT hull_shader(InputPatch<VS_OUTPUT, 3> patch, uint controlPointID : SV_OutputControlPointID)
{
    HS_OUTPUT OUT = (HS_OUTPUT)0;

    OUT.ObjectPosition = patch[controlPointID].ObjectPosition;
    OUT.TextureCoordinate = patch[controlPointID].TextureCoordinate;

    return OUT;
}

HS_CONSTANT_OUTPUT distance_constant_hull_shader(InputPatch<VS_OUTPUT, 3> patch, uint patchID : SV_PrimitiveID)
{
    HS_CONSTANT_OUTPUT OUT = (HS_CONSTANT_OUTPUT)0;

    // Caclulate the center of the patch
    float3 objectCenter = (patch[0].ObjectPosition.xyz + patch[1].ObjectPosition.xyz + patch[2].ObjectPosition.xyz) / 3.0f;
    float3 worldCenter = mul(float4(objectCenter, 1.0f), World).xyz;

    // Calculate uniform tessellation factor based on distance from the camera
    float tessellationFactor = max(min(MaxTessellationFactor, (MaxTessellationDistance - distance(worldCenter, CameraPosition)) / (MaxTessellationDistance - MinTessellationDistance) * MaxTessellationFactor), 1);

    [unroll]
    for (int i = 0; i < 3; i++)
    {
        OUT.EdgeFactors[i] = tessellationFactor;
    }

    OUT.InsideFactor = tessellationFactor;

    return OUT;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("distance_constant_hull_shader")]
HS_OUTPUT distance_hull_shader(InputPatch<VS_OUTPUT, 3> patch, uint controlPointID : SV_OutputControlPointID)
{
    HS_OUTPUT OUT = (HS_OUTPUT)0;

    OUT.ObjectPosition = patch[controlPointID].ObjectPosition;
    OUT.TextureCoordinate = patch[controlPointID].TextureCoordinate;

    return OUT;
}

/************* Domain Shader *************/

[domain("tri")]
DS_OUTPUT domain_shader(HS_CONSTANT_OUTPUT IN, float3 uvw : SV_DomainLocation, const OutputPatch<HS_OUTPUT, 3> patch)
{
    DS_OUTPUT OUT = (DS_OUTPUT)0;
    
    float3 objectPosition = uvw.x * patch[0].ObjectPosition.xyz + uvw.y * patch[1].ObjectPosition.xyz + uvw.z * patch[2].ObjectPosition.xyz;
    float2 textureCoordinate = uvw.x * patch[0].TextureCoordinate + uvw.y * patch[1].TextureCoordinate + uvw.z * patch[2].TextureCoordinate;

    OUT.Position = mul(float4(objectPosition, 1.0f), WorldViewProjection);
    OUT.TextureCoordinate = textureCoordinate;

    return OUT;
}

/************* Pixel Shader *************/

float4 pixel_shader(DS_OUTPUT IN) : SV_Target
{
    return ColorTexture.Sample(TrilinearSampler, IN.TextureCoordinate);
}

float4 solid_color_pixel_shader(DS_OUTPUT IN) : SV_Target
{
    return ColorWheat;
}

/************* Techniques *************/

technique11 textured_manual_factors
{
    pass p0
    {
        SetVertexShader(CompileShader(vs_5_0, vertex_shader()));
        SetHullShader(CompileShader(hs_5_0, hull_shader()));
        SetDomainShader(CompileShader(ds_5_0, domain_shader()));
        SetPixelShader(CompileShader(ps_5_0, pixel_shader()));
    }
}

technique11 solid_color_manual_factors
{
    pass p0
    {
        SetVertexShader(CompileShader(vs_5_0, vertex_shader()));
        SetHullShader(CompileShader(hs_5_0, hull_shader()));
        SetDomainShader(CompileShader(ds_5_0, domain_shader()));
        SetPixelShader(CompileShader(ps_5_0, solid_color_pixel_shader()));
    }
}

technique11 solid_color_distance_factors
{
    pass p0
    {
        SetVertexShader(CompileShader(vs_5_0, vertex_shader()));
        SetHullShader(CompileShader(hs_5_0, distance_hull_shader()));
        SetDomainShader(CompileShader(ds_5_0, domain_shader()));
        SetPixelShader(CompileShader(ps_5_0, solid_color_pixel_shader()));
    }
}