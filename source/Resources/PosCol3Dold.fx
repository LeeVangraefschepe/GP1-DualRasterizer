float4x4 gWorldViewProj : WorldViewPorjection;
Texture2D gDiffuseMap : DiffuseMap;
SamplerState gSampler : Sampler;

//SamplerState gSamState : SampleState
//{
//	Filter = MIN_MAG_MIP_POINT;
//	AddressU = Wrap; // or Mirror, Clamp, Border
//	AddressV = Wrap; // or Mirror, Clamp, Border
//};

//-------------------------
//	Input/Output Structs
//-------------------------
struct VS_INPUT
{
	float3 Position : POSITION;
	float2 UV : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float2 UV : TEXCOORD;
};

//-------------------------
//	Vertex Shader
//-------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Position = mul(float4(input.Position, 1.f),gWorldViewProj);
	output.UV = input.UV;
	return output;
}

//-------------------------
//	Pixel Shader
//-------------------------
float4 PS(VS_OUTPUT input) : SV_TARGET
{
	return gDiffuseMap.Sample(gSampler, input.UV);
}

//-------------------------
//	Technique
//-------------------------
technique11 DefaultTechnique
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}