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
		string UIName =  "Spot Light Color";
		string UIWidget = "Color";
	> = {1.0f, 1.0f, 1.0f, 1.0f};

	float3 LightPosition : POSITION <
		string Object = "SpotLightPosition0";
		string UIName =  "Spot Light Position";
		string Space = "World";
	> = {0.0f, 0.0f, 0.0f};

	float3 LightLookAt : DIRECTION <
		string Object = "SpotLightDirection0";
		string UIName =  "Spot Light Direction";
		string Space = "World";
	> = {0.0f, 0.0f, -1.0f};

	float LightRadius <
		string UIName =  "Spot Light Radius";
		string UIWidget = "slider";
		float UIMin = 0.0;
		float UIMax = 100.0;
		float UIStep = 1.0;
	> = {10.0f};

	float SpotLightInnerAngle <
		string UIName =  "Spot Light Inner Angle";
		string UIWidget = "slider";
		float UIMin = 0.5;
		float UIMax = 1.0;
		float UIStep = 0.01;
	> = {0.75f};

	float SpotLightOuterAngle <
		string UIName =  "Spot Light Outer Angle";
		string UIWidget = "slider";
		float UIMin = 0.0;
		float UIMax = 0.5;
		float UIStep = 0.01;
	> = {0.25f};
	
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
	float3 WorldPosition : TEXCOORD1;
    float Attenuation : TEXCOORD2;
	float3 LightLookAt : TEXCOORD3;	
};

/************* Vertex Shader *************/

VS_OUTPUT vertex_shader(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;
	
    OUT.Position = mul(IN.ObjectPosition, WorldViewProjection);
    OUT.WorldPosition = mul(IN.ObjectPosition, World).xyz;
	OUT.TextureCoordinate = get_corrected_texture_coordinate(IN.TextureCoordinate); 
	OUT.Normal = normalize(mul(float4(IN.Normal, 0), World).xyz); 
		
	float3 lightDirection = LightPosition - OUT.WorldPosition;	
	OUT.Attenuation = saturate(1.0f - length(lightDirection) / LightRadius);	
	
	OUT.LightLookAt = -LightLookAt;
	
	return OUT;
}

/************* Pixel Shader *************/

float4 pixel_shader(VS_OUTPUT IN) : SV_Target
{
	float4 OUT = (float)0;
	
	float3 lightDirection = normalize(LightPosition - IN.WorldPosition);   
    float3 viewDirection = normalize(CameraPosition - IN.WorldPosition);
	
	float3 normal = normalize(IN.Normal);
	float n_dot_l = dot(normal, lightDirection);	
	float3 halfVector = normalize(lightDirection + viewDirection);
	float n_dot_h = dot(normal, halfVector);
	float3 lightLookAt = normalize(IN.LightLookAt);

	float4 color = ColorTexture.Sample(ColorSampler, IN.TextureCoordinate);	
	float4 lightCoefficients = lit(n_dot_l, n_dot_h, SpecularPower);
	
	float3 ambient = get_vector_color_contribution(AmbientColor, color.rgb);
	float3 diffuse = get_vector_color_contribution(LightColor, lightCoefficients.y * color.rgb) * IN.Attenuation;
	float3 specular = get_scalar_color_contribution(SpecularColor, min(lightCoefficients.z, color.w)) * IN.Attenuation;
	
	float spotFactor = 0.0f;
	float lightAngle = dot(lightLookAt, lightDirection);	
	if (lightAngle > 0.0f)
	{
    	spotFactor = smoothstep(SpotLightOuterAngle, SpotLightInnerAngle, lightAngle);
	}
	
	OUT.rgb = ambient + (spotFactor * (diffuse + specular));
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
