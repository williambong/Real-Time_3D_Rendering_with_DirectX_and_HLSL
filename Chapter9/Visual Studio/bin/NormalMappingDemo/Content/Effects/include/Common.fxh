#ifndef _COMMON_FXH
#define _COMMON_FXH

/************* Constants *************/

#define FLIP_TEXTURE_Y 1

/************* Data Structures *************/

struct POINT_LIGHT
{
    float3 Position;
    float LightRadius;
    float4 Color;
};

struct LIGHT_CONTRIBUTION_DATA
{
    float4 Color;
    float3 Normal;
    float3 ViewDirection;
    float4 LightColor;
    float4 LightDirection;	
    float4 SpecularColor;
    float SpecularPower;	
};

/************* Utility Functions *************/

float2 get_corrected_texture_coordinate(float2 textureCoordinate)
{
    #if FLIP_TEXTURE_Y
        return float2(textureCoordinate.x, 1.0 - textureCoordinate.y); 
    #else
        return textureCoordinate; 
    #endif
}

float3 get_vector_color_contribution(float4 light, float3 color)
{
    // Color (.rgb) * Intensity (.a)
    return light.rgb * light.a * color;
}

float3 get_scalar_color_contribution(float4 light, float color)
{
    // Color (.rgb) * Intensity (.a)
    return light.rgb * light.a * color;
}

float4 get_light_data(float3 lightPosition, float3 worldPosition, float lightRadius)
{
    float4 lightData;
    float3 lightDirection = lightPosition - worldPosition;

    lightData.xyz = normalize(lightDirection);
    lightData.w = saturate(1.0f - length(lightDirection) / lightRadius); // Attenuation

    return lightData;
}

float3 get_light_contribution(LIGHT_CONTRIBUTION_DATA IN)
{
    float3 lightDirection = IN.LightDirection.xyz;	
    float n_dot_l = dot(IN.Normal, lightDirection);	
    float3 halfVector = normalize(lightDirection + IN.ViewDirection);
    float n_dot_h = dot(IN.Normal, halfVector);
    
    float4 lightCoefficients = lit(n_dot_l, n_dot_h, IN.SpecularPower);
    float3 diffuse = get_vector_color_contribution(IN.LightColor, lightCoefficients.y * IN.Color.rgb) * IN.LightDirection.w;
    float3 specular = get_scalar_color_contribution(IN.SpecularColor, min(lightCoefficients.z, IN.Color.w)) * IN.LightDirection.w * IN.LightColor.w;
    
    return (diffuse + specular);
}

float get_fog_amount(float3 viewDirection, float fogStart, float fogRange)
{
    return saturate((length(viewDirection) - fogStart) / (fogRange));
}

#endif /* _COMMON_FXH */
