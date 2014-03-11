#include "include\\Common.fxh"

/************* Resources *************/

#define NUM_LIGHTS 4

cbuffer CBufferPerFrame
{
    POINT_LIGHT PointLights[NUM_LIGHTS];
    
    float4 AmbientColor : AMBIENT <
        string UIName =  "Ambient Light";
        string UIWidget = "Color";
    > = {1.0f, 1.0f, 1.0f, 0.0f};
    
    float3 CameraPosition : CAMERAPOSITION < string UIWidget="None"; >;
}

cbuffer CBufferPerObject
{
    float4x4 WorldViewProjection : WORLDVIEWPROJECTION < string UIWidget="None"; >;
    float4x4 World : WORLD < string UIWidget="None"; >;
    
    float4 SpecularColor : SPECULAR <
        string UIName =  "Specular Color";
        string UIWidget = "Color";
    > = {1.0f, 1.0f, 1.0f, 1.0f};

    float SpecularPower : SPECULARPOWER <
        string UIName =  "Specular Power";
        string UIWidget = "slider";
        float UIMin = 1.0;
        float UIMax = 255.0;
        float UIStep = 1.0;
    > = {25.0f};
}

Texture2D ColorTexture <
    string ResourceName = "default_color.dds";
    string UIName =  "Color Texture";
    string ResourceType = "2D";
>;

SamplerState ColorSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = WRAP;
    AddressV = WRAP;
};

RasterizerState DisableCulling
{
    CullMode = NONE;
};

/************* Data Structures *************/

struct VS_INPUT
{
    float4 ObjectPosition : POSITION;
    float2 TextureCoordinate : TEXCOORD;
    float3 Normal : NORMAL;
};

struct VS_OUTPUT
{
    float4 Position : SV_Position;
    float3 WorldPosition : POSITION;
    float3 Normal : NORMAL;
    float2 TextureCoordinate : TEXCOORD0;
};

/************* Vertex Shader *************/

VS_OUTPUT vertex_shader(VS_INPUT IN)
{
    VS_OUTPUT OUT = (VS_OUTPUT)0;
    
    OUT.Position = mul(IN.ObjectPosition, WorldViewProjection);
    OUT.WorldPosition = mul(IN.ObjectPosition, World).xyz;		
    OUT.TextureCoordinate = get_corrected_texture_coordinate(IN.TextureCoordinate);
    OUT.Normal = normalize(mul(float4(IN.Normal, 0), World).xyz);

    return OUT;
}

/************* Pixel Shader *************/

float4 pixel_shader(VS_OUTPUT IN, uniform int lightCount) : SV_Target
{
    float4 OUT = (float4)0;
        
    float3 normal = normalize(IN.Normal);
    float3 viewDirection = normalize(CameraPosition - IN.WorldPosition);	
    float4 color = ColorTexture.Sample(ColorSampler, IN.TextureCoordinate);
    float3 ambient = get_vector_color_contribution(AmbientColor, color.rgb);
        
    LIGHT_CONTRIBUTION_DATA lightContributionData;
    lightContributionData.Color = color;
    lightContributionData.Normal = normal;
    lightContributionData.ViewDirection = viewDirection;
    lightContributionData.SpecularColor = SpecularColor;
    lightContributionData.SpecularPower = SpecularPower;
    
    float3 totalLightContribution = (float3)0;
    
    [unroll]
    for (int i = 0; i < lightCount; i++)
    {
        lightContributionData.LightDirection = get_light_data(PointLights[i].Position, IN.WorldPosition, PointLights[i].LightRadius);	
        lightContributionData.LightColor = PointLights[i].Color;
        totalLightContribution += get_light_contribution(lightContributionData);
    }

    OUT.rgb = ambient + totalLightContribution;
    OUT.a = 1.0f;
        
    return OUT;
}

/************* Techniques *************/

technique10 Lights1
{
    pass p0
    {
        SetVertexShader(CompileShader(vs_4_0, vertex_shader()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, pixel_shader(1)));
            
        SetRasterizerState(DisableCulling);
    }
}

technique10 Lights2
{
    pass p0
    {
        SetVertexShader(CompileShader(vs_4_0, vertex_shader()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, pixel_shader(2)));
            
        SetRasterizerState(DisableCulling);
    }
}

technique10 Lights3
{
    pass p0
    {
        SetVertexShader(CompileShader(vs_4_0, vertex_shader()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, pixel_shader(3)));
            
        SetRasterizerState(DisableCulling);
    }
}

technique10 Lights4
{
    pass p0
    {
        SetVertexShader(CompileShader(vs_4_0, vertex_shader()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, pixel_shader(4)));
            
        SetRasterizerState(DisableCulling);
    }
}