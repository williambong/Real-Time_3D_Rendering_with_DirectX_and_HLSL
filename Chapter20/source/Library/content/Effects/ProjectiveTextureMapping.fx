#include "include\\Common.fxh"

/************* Resources *************/
static const float4 ColorWhite = { 1, 1, 1, 1 };

cbuffer CBufferPerFrame
{
    float4 AmbientColor = { 1.0f, 1.0f, 1.0f, 0.0f };
    float4 LightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    float3 LightPosition = { 0.0f, 0.0f, 0.0f };
    float LightRadius = 10.0f;
    float3 CameraPosition;
    float DepthBias = 0.005;
}

cbuffer CBufferPerObject
{
    float4x4 WorldViewProjection : WORLDVIEWPROJECTION;
    float4x4 World : WORLD;
    float4 SpecularColor : SPECULAR = { 1.0f, 1.0f, 1.0f, 1.0f };
    float SpecularPower : SPECULARPOWER  = 25.0f;

    float4x4 ProjectiveTextureMatrix;
}

Texture2D ColorTexture;
Texture2D ProjectedTexture;
Texture2D DepthMap;

SamplerState DepthMapSampler
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = BORDER;
    AddressV = BORDER;
    BorderColor = ColorWhite;
};

SamplerState ProjectedTextureSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = BORDER;
    AddressV = BORDER;
    BorderColor = ColorWhite;
};

SamplerState ColorSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = WRAP;
    AddressV = WRAP;
};

RasterizerState BackFaceCulling
{
    CullMode = BACK;
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
    float3 WorldPosition : TEXCOORD1;
    float Attenuation : TEXCOORD2;
    float4 ProjectedTextureCoordinate : TEXCOORD3;
};

/************* Vertex Shader *************/

VS_OUTPUT project_texture_vertex_shader(VS_INPUT IN)
{
    VS_OUTPUT OUT = (VS_OUTPUT)0;      

    OUT.Position = mul(IN.ObjectPosition, WorldViewProjection);
    OUT.WorldPosition = mul(IN.ObjectPosition, World).xyz;
    OUT.TextureCoordinate = IN.TextureCoordinate;
    OUT.Normal = normalize(mul(float4(IN.Normal, 0), World).xyz);

    float3 lightDirection = LightPosition - OUT.WorldPosition;
    OUT.Attenuation = saturate(1.0f - (length(lightDirection) / LightRadius));

    OUT.ProjectedTextureCoordinate = mul(IN.ObjectPosition, ProjectiveTextureMatrix);

    return OUT;
}

/************* Pixel Shaders *************/

float4 project_texture_pixel_shader(VS_OUTPUT IN) : SV_Target
{
    float4 OUT = (float4)0;

    float3 lightDirection = LightPosition - IN.WorldPosition;
    lightDirection = normalize(lightDirection);

    float3 viewDirection = normalize(CameraPosition - IN.WorldPosition);

    float3 normal = normalize(IN.Normal);
    float n_dot_l = dot(normal, lightDirection);
    float3 halfVector = normalize(lightDirection + viewDirection);
    float n_dot_h = dot(normal, halfVector);

    float4 color = ColorTexture.Sample(ColorSampler, IN.TextureCoordinate);
    float4 lightCoefficients = lit(n_dot_l, n_dot_h, SpecularPower);

    float3 ambient = get_vector_color_contribution(AmbientColor, color.rgb);
    float3 diffuse = get_vector_color_contribution(LightColor, lightCoefficients.y * color.rgb) * IN.Attenuation;
    float3 specular = get_scalar_color_contribution(SpecularColor, min(lightCoefficients.z, color.w)) * IN.Attenuation;

    OUT.rgb = ambient + diffuse + specular;
    OUT.a = 1.0f;

    IN.ProjectedTextureCoordinate.xy /= IN.ProjectedTextureCoordinate.w;
    float3 projectedColor = ProjectedTexture.Sample(ProjectedTextureSampler, IN.ProjectedTextureCoordinate.xy).rgb;

    OUT.rgb *= projectedColor;
    
    return OUT;
}

float4 project_texture_no_reverse_pixel_shader(VS_OUTPUT IN) : SV_Target
{
    float4 OUT = (float4)0;

    float3 lightDirection = LightPosition - IN.WorldPosition;
    lightDirection = normalize(lightDirection);

    float3 viewDirection = normalize(CameraPosition - IN.WorldPosition);

    float3 normal = normalize(IN.Normal);
    float n_dot_l = dot(normal, lightDirection);
    float3 halfVector = normalize(lightDirection + viewDirection);
    float n_dot_h = dot(normal, halfVector);

    float4 color = ColorTexture.Sample(ColorSampler, IN.TextureCoordinate);
    float4 lightCoefficients = lit(n_dot_l, n_dot_h, SpecularPower);

    float3 ambient = get_vector_color_contribution(AmbientColor, color.rgb);
    float3 diffuse = get_vector_color_contribution(LightColor, lightCoefficients.y * color.rgb) * IN.Attenuation;
    float3 specular = get_scalar_color_contribution(SpecularColor, min(lightCoefficients.z, color.w)) * IN.Attenuation;

    OUT.rgb = ambient + diffuse + specular;
    OUT.a = 1.0f;

    if (IN.ProjectedTextureCoordinate.w >= 0.0f)
    {
        IN.ProjectedTextureCoordinate.xy /= IN.ProjectedTextureCoordinate.w;
        float3 projectedColor = ProjectedTexture.Sample(ProjectedTextureSampler, IN.ProjectedTextureCoordinate.xy).rgb;

        OUT.rgb *= projectedColor;        
    }

    return OUT;
}

float4 project_texture_w_depthmap_pixel_shader(VS_OUTPUT IN) : SV_Target
{
    float4 OUT = (float4)0;

    float3 lightDirection = LightPosition - IN.WorldPosition;
    lightDirection = normalize(lightDirection);

    float3 viewDirection = normalize(CameraPosition - IN.WorldPosition);

    float3 normal = normalize(IN.Normal);
    float n_dot_l = dot(normal, lightDirection);
    float3 halfVector = normalize(lightDirection + viewDirection);
    float n_dot_h = dot(normal, halfVector);

    float4 color = ColorTexture.Sample(ColorSampler, IN.TextureCoordinate);
    float4 lightCoefficients = lit(n_dot_l, n_dot_h, SpecularPower);

    float3 ambient = get_vector_color_contribution(AmbientColor, color.rgb);
    float3 diffuse = get_vector_color_contribution(LightColor, lightCoefficients.y * color.rgb) * IN.Attenuation;
    float3 specular = get_scalar_color_contribution(SpecularColor, min(lightCoefficients.z, color.w)) * IN.Attenuation;

    OUT.rgb = ambient + diffuse + specular;
    OUT.a = 1.0f;

    if (IN.ProjectedTextureCoordinate.w >= 0.0f)
    {
        IN.ProjectedTextureCoordinate.xyz /= IN.ProjectedTextureCoordinate.w;		
        float pixelDepth = IN.ProjectedTextureCoordinate.z;
        float sampledDepth = DepthMap.Sample(DepthMapSampler, IN.ProjectedTextureCoordinate.xy).x + DepthBias;

        float3 projectedColor = (pixelDepth > sampledDepth ? ColorWhite.rgb : ProjectedTexture.Sample(ProjectedTextureSampler, IN.ProjectedTextureCoordinate.xy).rgb);
        OUT.rgb *= projectedColor;
    }

    return OUT;
}

/************* Techniques *************/

technique11 project_texture
{
    pass p0
    {
        SetVertexShader(CompileShader(vs_5_0, project_texture_vertex_shader()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, project_texture_pixel_shader()));

        SetRasterizerState(BackFaceCulling);
    }
}

technique11 project_texture_no_reverse
{
    pass p0
    {
        SetVertexShader(CompileShader(vs_5_0, project_texture_vertex_shader()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, project_texture_no_reverse_pixel_shader()));

        SetRasterizerState(BackFaceCulling);
    }
}

technique11 project_texture_w_depthmap
{
    pass p0
    {
        SetVertexShader(CompileShader(vs_5_0, project_texture_vertex_shader()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, project_texture_w_depthmap_pixel_shader()));

        SetRasterizerState(BackFaceCulling);
    }
}