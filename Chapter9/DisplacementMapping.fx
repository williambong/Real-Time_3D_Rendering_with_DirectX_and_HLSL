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
	
	float DisplacementScale <
		string UIName =  "Displacement Scale";
		string UIWidget = "slider";
		float UIMin = 0.0;
		float UIMax = 2.0;
		float UIStep = 0.01;
	> = {0.0f};
}

Texture2D ColorTexture <
    string ResourceName = "default_color.dds";
    string UIName =  "Color Texture";
    string ResourceType = "2D";
>;

Texture2D DisplacementMap <
    string UIName =  "Displacement Map";
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

/************* Data Structures *************/

struct VS_INPUT
{
    float4 ObjectPosition : POSITION0;
	float2 TextureCoordinate : TEXCOORD;	
	float3 Normal : NORMAL;
};

struct VS_OUTPUT
{
    float4 Position : SV_Position;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
    float2 TextureCoordinate : TEXCOORD0; 
	float3 ViewDirection : TEXCOORD1;
	float4 LightDirection : TEXCOORD2;
};

/************* Vertex Shader *************/

VS_OUTPUT vertex_shader(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;
	
	float2 textureCoordinate = get_corrected_texture_coordinate(IN.TextureCoordinate);
	
	[flatten]
	if (DisplacementScale > 0.0f)
	{
		float displacement = DisplacementMap.SampleLevel(TrilinearSampler, textureCoordinate, 0);
		IN.ObjectPosition.xyz += IN.Normal * DisplacementScale * (displacement - 1);
	}

	OUT.Position = mul(IN.ObjectPosition, WorldViewProjection);
	OUT.TextureCoordinate = textureCoordinate; 
	OUT.Normal = normalize(mul(float4(IN.Normal, 0), World).xyz);

	float3 worldPosition = normalize(mul(IN.ObjectPosition, World)).xyz;
	OUT.ViewDirection = normalize(CameraPosition - worldPosition);

	OUT.LightDirection = get_light_data(LightPosition, worldPosition, LightRadius);	

	return OUT;
}

/************* Pixel Shader *************/

float4 pixel_shader(VS_OUTPUT IN) : SV_Target
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
	lightContributionData.LightDirection = IN.LightDirection;
	lightContributionData.SpecularColor = SpecularColor;
	lightContributionData.SpecularPower = SpecularPower;
	lightContributionData.LightColor = LightColor;
	float3 light_contribution = get_light_contribution(lightContributionData);

	OUT.rgb = ambient + light_contribution;
	OUT.a = 1.0f;
	
	return OUT;
}

/************* Techniques *************/

technique10 main10
{
    pass p0
	{
        SetVertexShader(CompileShader(vs_4_0, vertex_shader()));
		SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, pixel_shader()));                
			
		SetRasterizerState(DisableCulling);
    }
}