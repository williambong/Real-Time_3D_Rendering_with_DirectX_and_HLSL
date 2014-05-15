/************* Resources *************/
static const float ZeroCorrection = 0.5f / 255.0f;

cbuffer CBufferPerObjectCutout
{
    float4x4 WorldViewProjection : WORLDVIEWPROJECTION;	
}

cbuffer CBufferPerFrameCutout
{
    float Time : TIME;
}

cbuffer CBufferPerObjectComposite
{
    float DisplacementScale = 1.0f;
}

Texture2D SceneTexture;
Texture2D DistortionMap;

SamplerState TrilinearSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = WRAP;
    AddressV = WRAP;
};

/************* Data Structures *************/

struct VS_INPUT
{
    float4 Position : POSITION;
    float2 TextureCoordinate : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 Position : SV_Position;
    float2 TextureCoordinate : TEXCOORD;
};

/************* Cutout *************/

VS_OUTPUT distortion_cutout_vertex_shader(VS_INPUT IN)
{
    VS_OUTPUT OUT = (VS_OUTPUT)0;
    
    OUT.Position = mul(IN.Position, WorldViewProjection);
    OUT.TextureCoordinate = IN.TextureCoordinate;

    return OUT;
}

float4 displacement_cutout_pixel_shader(VS_OUTPUT IN) : SV_Target
{
    float2 displacement = DistortionMap.Sample(TrilinearSampler, IN.TextureCoordinate).xy;

    return float4(displacement.xy, 0, 1);
}

/************* Compositing *************/

VS_OUTPUT distortion_composite_vertex_shader(VS_INPUT IN)
{
    VS_OUTPUT OUT = (VS_OUTPUT)0;
    
    OUT.Position = IN.Position;
    OUT.TextureCoordinate = IN.TextureCoordinate;
    
    return OUT;
}

float4 distortion_composite_pixel_shader(VS_OUTPUT IN) : SV_Target
{
    float4 OUT = (float4)0;

    float2 displacement = DistortionMap.Sample(TrilinearSampler, IN.TextureCoordinate).rg;
    if (displacement.x == 0 && displacement.y == 0)
    {
        OUT = SceneTexture.Sample(TrilinearSampler, IN.TextureCoordinate);
    }
    else
    {
        displacement -= 0.5f + ZeroCorrection;
        OUT = SceneTexture.Sample(TrilinearSampler, IN.TextureCoordinate + (DisplacementScale * displacement));
    }

    return OUT;
}

float4 no_distortion_pixel_shader(VS_OUTPUT IN) : SV_Target
{
    return SceneTexture.Sample(TrilinearSampler, IN.TextureCoordinate);
}

/************* Techniques *************/

technique11 displacement_cutout
{
    pass p0
    {
        SetVertexShader(CompileShader(vs_5_0, distortion_cutout_vertex_shader()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, displacement_cutout_pixel_shader()));
    }
}

technique11 distortion_composite
{
    pass p0
    {
        SetVertexShader(CompileShader(vs_5_0, distortion_composite_vertex_shader()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, distortion_composite_pixel_shader()));
    }
}

technique11 no_distortion
{
    pass p0
    {
        SetVertexShader(CompileShader(vs_5_0, distortion_composite_vertex_shader()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, no_distortion_pixel_shader()));
    }
}