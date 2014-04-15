/************* Resources *************/
static const float4 ColorWheat = { 0.961f, 0.871f, 0.702f, 1.0f };

cbuffer CBufferPerFrame
{
    float TessellationEdgeFactors[3];
    float TessellationInsideFactor;
}

cbuffer CBufferPerObject
{
    float4x4 WorldViewProjection;
}

/************* Data Structures *************/

struct VS_INPUT
{
    float4 ObjectPosition : POSITION;
};

struct VS_OUTPUT
{
    float4 ObjectPosition : POSITION;
};

struct HS_CONSTANT_OUTPUT
{
    float EdgeFactors[3] : SV_TessFactor;
    float InsideFactor : SV_InsideTessFactor;
};

struct HS_OUTPUT
{
    float4 ObjectPosition : POSITION;
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

    return OUT;
}

/************* Hull Shaders *************/

HS_CONSTANT_OUTPUT constant_hull_shader(InputPatch<VS_OUTPUT, 3> patch)
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

    return OUT;
}

/************* Domain Shader *************/

[domain("tri")]
DS_OUTPUT domain_shader(HS_CONSTANT_OUTPUT IN, float3 uvw : SV_DomainLocation, const OutputPatch<HS_OUTPUT, 3> patch)
{
    DS_OUTPUT OUT = (DS_OUTPUT)0;

    float3 objectPosition = uvw.x * patch[0].ObjectPosition.xyz + uvw.y * patch[1].ObjectPosition.xyz + uvw.z * patch[2].ObjectPosition.xyz;

    OUT.Position = mul(float4(objectPosition, 1.0f), WorldViewProjection);

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