#include "include\\Common.fxh"

/************* Resources *************/

cbuffer CBufferPerFrame
{
    float4 AmbientColor : AMBIENT <
        string UIName =  "Ambient Light";
        string UIWidget = "Color";
    > = {1.0f, 1.0f, 1.0f, 0.0f};

    float4 LightColor : COLOR <
        string Object = "LightColor0";
        string UIName =  "Light Color";
        string UIWidget = "Color";
    > = {1.0f, 1.0f, 1.0f, 1.0f};

    float3 LightPosition : POSITION <
        string Object = "PointLight0";
        string UIName =  "Light Position";
        string Space = "World";
    > = {0.0f, 0.0f, 0.0f};

    float LightRadius <
        string UIName =  "Light Radius";
        string UIWidget = "slider";
        float UIMin = 0.0;
        float UIMax = 100.0;
        float UIStep = 1.0;
    > = {10.0f};

    float3 FogColor <
        string UIName =  "Fog Color";
        string UIWidget = "Color";
    > = {0.5f, 0.5f, 0.5f};

    float FogStart = { 20.0f };
    float FogRange = { 40.0f };
    
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

Texture2D TransparencyMap <
    string UIName =  "Transparency Map";
    string ResourceType = "2D";
>;

SamplerState TrilinearSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = WRAP;
    AddressV = WRAP;
};

RasterizerState DisableCulling
{
    CullMode = NONE;
};

BlendState EnableAlphaBlending
{
    BlendEnable[0] = True;
    SrcBlend = SRC_ALPHA;
    DestBlend = INV_SRC_ALPHA;
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
    float3 Normal : NORMAL;
    float2 TextureCoordinate : TEXCOORD0;	
    float4 LightDirection : TEXCOORD1;
    float3 ViewDirection : TEXCOORD2;
    float FogAmount : TEXCOORD3;
};

/************* Utility Functions *************/

float get_fog_amount(float3 viewDirection, float fogStart, float fogRange)
{
    return saturate((length(viewDirection) - fogStart) / (fogRange));
}

/************* Vertex Shader *************/

VS_OUTPUT vertex_shader(VS_INPUT IN, uniform bool fogEnabled)
{
    VS_OUTPUT OUT = (VS_OUTPUT)0;
    
    OUT.Position = mul(IN.ObjectPosition, WorldViewProjection);
    OUT.TextureCoordinate = get_corrected_texture_coordinate(IN.TextureCoordinate);
	OUT.Normal = normalize(mul(float4(IN.Normal, 0), World).xyz);
    
    float3 worldPosition = mul(IN.ObjectPosition, World).xyz;	
    
    OUT.LightDirection = get_light_data(LightPosition, worldPosition, LightRadius);
    float3 viewDirection = CameraPosition - worldPosition;	
    
    if (fogEnabled)
    {		
        OUT.FogAmount = get_fog_amount(viewDirection, FogStart, FogRange);
    }
    
    OUT.ViewDirection = normalize(viewDirection);

    return OUT;
}

/************* Pixel Shader *************/

float4 pixel_shader(VS_OUTPUT IN, uniform bool fogEnabled) : SV_Target
{
    float4 OUT = (float4)0;
        
    float3 normal = normalize(IN.Normal);
    float3 viewDirection = normalize(IN.ViewDirection);
    float4 color = ColorTexture.Sample(TrilinearSampler, IN.TextureCoordinate);
    float3 ambient = get_vector_color_contribution(AmbientColor, color.rgb);

    LIGHT_CONTRIBUTION_DATA lightContributionData;
    lightContributionData.Color = color;
    lightContributionData.Normal = normal;
    lightContributionData.ViewDirection = viewDirection;
    lightContributionData.SpecularColor = SpecularColor;
    lightContributionData.SpecularPower = SpecularPower;	
    lightContributionData.LightDirection = IN.LightDirection;
    lightContributionData.LightColor = LightColor;
    float3 light_contribution = get_light_contribution(lightContributionData);
    
    OUT.rgb = ambient + light_contribution;
    OUT.a = TransparencyMap.Sample(TrilinearSampler, IN.TextureCoordinate).a;

    if (fogEnabled)
    {
        OUT.rgb = lerp(OUT.rgb, FogColor, IN.FogAmount);
    }
    
    return OUT;
}

technique10 alphaBlendingWithFog
{
    pass p0
    {
        SetVertexShader(CompileShader(vs_4_0, vertex_shader(true)));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, pixel_shader(true)));         
        
        SetRasterizerState(DisableCulling);
        //SetBlendState(EnableAlphaBlending, (float4)0, 0xFFFFFFFF); // Not supported by Effects 11 library
    }
}

technique10 alphaBlendingWithoutFog
{
    pass p0
    {
        SetVertexShader(CompileShader(vs_4_0, vertex_shader(false)));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, pixel_shader(false)));                
            
        SetRasterizerState(DisableCulling);
        //SetBlendState(EnableAlphaBlending, (float4)0, 0xFFFFFFFF); // Not supported by Effects 11 library
    }
}
