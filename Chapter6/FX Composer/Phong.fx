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
};

/************* Vertex Shader *************/

VS_OUTPUT vertex_shader(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;
	
    OUT.Position = mul(IN.ObjectPosition, WorldViewProjection);
    OUT.TextureCoordinate = get_corrected_texture_coordinate(IN.TextureCoordinate);
	OUT.Normal = normalize(mul(float4(IN.Normal, 0), World).xyz);
	OUT.LightDirection = normalize(-LightDirection);	
		
	float3 worldPosition = mul(IN.ObjectPosition, World).xyz;
	OUT.ViewDirection = normalize(CameraPosition - worldPosition);	
	
	return OUT;
}

/************* Pixel Shader *************/

float4 pixel_shader(VS_OUTPUT IN) : SV_Target
{
	float4 OUT = (float4)0;
		
	float3 normal = normalize(IN.Normal);
    float3 lightDirection = normalize(IN.LightDirection);
	float3 viewDirection = normalize(IN.ViewDirection);
	float n_dot_l = dot(lightDirection, normal);

	float4 color = ColorTexture.Sample(ColorSampler, IN.TextureCoordinate);	
	float3 ambient = AmbientColor.rgb * AmbientColor.a * color.rgb;	

	float3 diffuse = (float3)0;
	float3 specular = (float3)0;

	if (n_dot_l > 0)
	{
		diffuse = LightColor.rgb * LightColor.a * n_dot_l * color.rgb;

		// R = 2 * (N.L) * N - L
    	float3 reflectionVector = normalize(2 * n_dot_l * normal - lightDirection);  
	
		// specular = R.V^n with gloss map stored in color texture's alpha channel	
		specular = SpecularColor.rgb * SpecularColor.a * min(pow(saturate(dot(reflectionVector, viewDirection)), SpecularPower), color.w);		
	}
	
	OUT.rgb = ambient + diffuse + specular;
	OUT.a = 1.0f;
	
	return OUT;
}

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