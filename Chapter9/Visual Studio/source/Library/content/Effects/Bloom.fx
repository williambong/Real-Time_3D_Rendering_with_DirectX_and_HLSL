/************* Resources *************/

static const float3 GrayScaleIntensity = { 0.299f, 0.587f, 0.114f };

Texture2D ColorTexture;
Texture2D BloomTexture;

cbuffer CBufferPerObject
{
    float BloomThreshold = 0.3f;
    float BloomIntensity = 1.25f;
    float BloomSaturation = 1.0f;
    float SceneIntensity = 1.0f;
    float SceneSaturation = 1.0f;
};

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

/************* Utility Functions *************/

float4 AdjustSaturation(float4 color, float saturation)
{
    float intensity = dot(color.rgb, GrayScaleIntensity);
    
    return float4(lerp(intensity.rrr, color.rgb, saturation), color.a);
}

/************* Vertex Shader *************/

VS_OUTPUT vertex_shader(VS_INPUT IN)
{
    VS_OUTPUT OUT = (VS_OUTPUT)0;
    
    OUT.Position = IN.Position;
    OUT.TextureCoordinate = IN.TextureCoordinate;
    
    return OUT;
}

/************* Pixel Shaders *************/

float4 bloom_extract_pixel_shader(VS_OUTPUT IN) : SV_Target
{
    float4 color = ColorTexture.Sample(TrilinearSampler, IN.TextureCoordinate);

    return saturate((color - BloomThreshold) / (1 - BloomThreshold));
}

float4 bloom_composite_pixel_shader(VS_OUTPUT IN) : SV_Target
{
    float4 sceneColor = ColorTexture.Sample(TrilinearSampler, IN.TextureCoordinate);
    float4 bloomColor = BloomTexture.Sample(TrilinearSampler, IN.TextureCoordinate);
    
    sceneColor = AdjustSaturation(sceneColor, SceneSaturation) * SceneIntensity;
    bloomColor = AdjustSaturation(bloomColor, BloomSaturation) * BloomIntensity;
    
    sceneColor *= (1 - saturate(bloomColor));	

    return sceneColor + bloomColor;
}

float4 no_bloom_pixel_shader(VS_OUTPUT IN) : SV_Target
{
    return ColorTexture.Sample(TrilinearSampler, IN.TextureCoordinate);
}

/************* Techniques *************/

technique11 bloom_extract
{
    pass p0
    {
        SetVertexShader(CompileShader(vs_5_0, vertex_shader()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, bloom_extract_pixel_shader()));
    }
}

technique11 bloom_composite
{
    pass p0
    {
        SetVertexShader(CompileShader(vs_5_0, vertex_shader()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, bloom_composite_pixel_shader()));
    }
}

technique11 no_bloom
{
    pass p0
    {
        SetVertexShader(CompileShader(vs_5_0, vertex_shader()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, no_bloom_pixel_shader()));
    }
}
