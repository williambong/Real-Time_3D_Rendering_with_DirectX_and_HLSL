#include "include\\Common.fxh"

/************* Resources *************/

cbuffer CBufferPerFrame
{
    float4 AmbientColor = {1.0f, 1.0f, 1.0f, 0.0f};
    float4 LightColor = {1.0f, 1.0f, 1.0f, 1.0f};
    float3 LightPosition = {0.0f, 0.0f, 0.0f};
    float LightRadius = 10.0f;
    float3 CameraPosition : CAMERAPOSITION;
}

cbuffer CBufferPerObject
{
    float4x4 ViewProjection : VIEWPROJECTION;
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
    float2 TextureCoordinate : TEXCOORD;
    float3 Normal : NORMAL;
    row_major float4x4 World : WORLD;
    float4 SpecularColor : SPECULARCOLOR;
    float SpecularPower : SPECULARPOWER;
};

struct VS_OUTPUT
{
    float4 Position : SV_Position;
    float3 Normal : NORMAL;
    float2 TextureCoordinate : TEXCOORD;
    float3 WorldPosition : WORLD;
    float Attenuation : ATTENUATION;
    float4 SpecularColor : SPECULAR;
    float SpecularPower : SPECULARPOWER;
};

/************* Vertex Shader *************/

VS_OUTPUT vertex_shader(VS_INPUT IN)
{
    VS_OUTPUT OUT = (VS_OUTPUT)0;
    
    OUT.WorldPosition = mul(IN.ObjectPosition, IN.World).xyz;
    OUT.Position = mul(float4(OUT.WorldPosition, 1.0f), ViewProjection);
    OUT.Normal = normalize(mul(float4(IN.Normal, 0), IN.World).xyz);
    OUT.TextureCoordinate = IN.TextureCoordinate;

    float3 lightDirection = LightPosition - OUT.WorldPosition;
    OUT.Attenuation = saturate(1.0f - (length(lightDirection) / LightRadius));
    OUT.SpecularColor = IN.SpecularColor;
    OUT.SpecularPower = IN.SpecularPower;

    return OUT;
}

/************* Pixel Shader *************/

float4 pixel_shader(VS_OUTPUT IN) : SV_Target
{
    float4 OUT = (float4)0;
    
    float3 lightDirection = LightPosition - IN.WorldPosition;
    lightDirection = normalize(lightDirection);

    float3 viewDirection = normalize(CameraPosition - IN.WorldPosition);
    
    float3 normal = normalize(IN.Normal);	
    float n_dot_l = dot(normal, lightDirection);	
    float3 halfVector = normalize(lightDirection + viewDirection);
    float n_dot_h = dot(normal, halfVector);

    float4 color = ColorTexture.Sample(TrilinearSampler, IN.TextureCoordinate);
    float4 lightCoefficients = lit(n_dot_l, n_dot_h, IN.SpecularPower);

    float3 ambient = get_vector_color_contribution(AmbientColor, color.rgb);
    float3 diffuse = get_vector_color_contribution(LightColor,  lightCoefficients.y * color.rgb) * IN.Attenuation;
    float3 specular = get_scalar_color_contribution(IN.SpecularColor, min(lightCoefficients.z, color.w)) * IN.Attenuation;

    OUT.rgb = ambient + diffuse + specular;
    OUT.a = 1.0f;
    
    return OUT;
}

/************* Techniques *************/

technique11 main11
{
    pass p0
    {
        SetVertexShader(CompileShader(vs_5_0, vertex_shader()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, pixel_shader()));
    }
}
