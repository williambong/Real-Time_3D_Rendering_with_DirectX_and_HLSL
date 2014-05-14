#ifndef _COMMON_FXH
#define _COMMON_FXH

/************* Constants *************/

#define FLIP_TEXTURE_Y 1

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

#endif /* _COMMON_FXH */

