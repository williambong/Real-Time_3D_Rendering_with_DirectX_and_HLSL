cbuffer CBufferPerObject
{
	float4x4 WorldViewProjection : WORLDVIEWPROJECTION; 
}

RasterizerState DisableCulling
{
    CullMode = NONE;
};

float4 vertex_shader(float3 objectPosition : POSITION) : SV_Position
{
	return mul(float4(objectPosition, 1), WorldViewProjection);
}

float4 pixel_shader() : SV_Target
{
    return float4(1, 0, 0, 1);
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