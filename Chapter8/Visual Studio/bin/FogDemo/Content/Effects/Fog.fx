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

	float3 LightDirection : DIRECTION <
		string Object = "DirectionalLight0";
		string UIName =  "Light Direction";
		string Space = "World";
	> = {0.0f, 0.0f, -1.0f};
	
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
	float3 Normal : NORMAL;
	float2 TextureCoordinate : TEXCOORD0;	
	float3 LightDirection : TEXCOORD1;
	float3 ViewDirection : TEXCOORD2;
	float FogAmount: TEXCOORD3;
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
	OUT.LightDirection = normalize(-LightDirection);

	float3 worldPosition = mul(IN.ObjectPosition, World).xyz;
	float3 viewDirection = CameraPosition - worldPosition;
	OUT.ViewDirection = normalize(viewDirection);

	if (fogEnabled)
	{
		OUT.FogAmount = get_fog_amount(viewDirection, FogStart, FogRange);
	}
	
	return OUT;
}

/************* Pixel Shader *************/

float4 pixel_shader(VS_OUTPUT IN, uniform bool fogEnabled) : SV_Target
{
	float4 OUT = (float4)0;
	
	float3 normal = normalize(IN.Normal);
	float3 viewDirection = normalize(IN.ViewDirection);
	float4 color = ColorTexture.Sample(ColorSampler, IN.TextureCoordinate);
	float3 ambient = get_vector_color_contribution(AmbientColor, color.rgb);
	
	LIGHT_CONTRIBUTION_DATA lightContributionData;
	lightContributionData.Color = color;
	lightContributionData.Normal = normal;
	lightContributionData.ViewDirection = viewDirection;
	lightContributionData.LightDirection = float4(IN.LightDirection, 1);
	lightContributionData.SpecularColor = SpecularColor;
	lightContributionData.SpecularPower = SpecularPower;
	lightContributionData.LightColor = LightColor;
	float3 light_contribution = get_light_contribution(lightContributionData);

	OUT.rgb = ambient + light_contribution;
	OUT.a = 1.0f;	

	if (fogEnabled)
	{
		OUT.rgb = lerp(OUT.rgb, FogColor, IN.FogAmount);
	}
	
	return OUT;
}

technique10 fogEnabled
{
    pass p0
	{
        SetVertexShader(CompileShader(vs_4_0, vertex_shader(true)));
		SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, pixel_shader(true)));                
			
		SetRasterizerState(DisableCulling);
    }
}

technique10 fogDisabled
{
    pass p0
	{
        SetVertexShader(CompileShader(vs_4_0, vertex_shader(false)));
		SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, pixel_shader(false)));                
			
		SetRasterizerState(DisableCulling);
    }
}