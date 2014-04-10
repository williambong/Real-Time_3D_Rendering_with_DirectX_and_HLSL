/************* Resources *************/
static const float ZeroCorrection = 0.5f / 255.0f;

cbuffer CBufferPerObject
{
    float DisplacementScale = 1.0f;
}

Texture2D ColorTexture <
    string ResourceName = "default_color.dds";
    string UIName =  "Color Texture";
    string ResourceType = "2D";
>;

Texture2D SceneTexture;
Texture2D DistortionMap;

SamplerState TrilinearSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

/************* Data Structures *************/

struct VS_INPUT
{
    float4 ObjectPosition : POSITION;
    float2 TextureCoordinate : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 Position : SV_Position;
    float2 TextureCoordinate : TEXCOORD;  
};

/************* Vertex Shader *************/

VS_OUTPUT vertex_shader(VS_INPUT IN)
{
    VS_OUTPUT OUT = (VS_OUTPUT)0;
    
    OUT.Position = IN.ObjectPosition;
    OUT.TextureCoordinate = IN.TextureCoordinate;
    
    return OUT;
}

/************* Displacement *************/

float4 displacement_pixel_shader(VS_OUTPUT IN) : SV_Target
{
    float4 OUT = (float4)0;

    float2 displacement = DistortionMap.Sample(TrilinearSampler, IN.TextureCoordinate).xy - 0.5 + ZeroCorrection;	
    OUT = SceneTexture.Sample(TrilinearSampler, IN.TextureCoordinate + (DisplacementScale * displacement));

    return OUT;
}

/************* Techniques *************/

technique11 displacement
{
    pass p0
    {
        SetVertexShader(CompileShader(vs_5_0, vertex_shader()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, displacement_pixel_shader()));
    }
}