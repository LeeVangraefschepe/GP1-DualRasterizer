Texture2D gDiffuseMap : DiffuseMap;
float4x4 gWorldViewProj : WorldViewPorjection;

SamplerState gSampler : Sampler
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Wrap; //or Mirror, Clamp, Border
	AddressV = Wrap; //or Mirror, Clamp, Border
};

BlendState gBlendState
{
	BlendEnable[0] = true;
	SrcBlend = src_alpha;
	DestBlend = inv_src_alpha;
	BlendOp = add;
	SrcBlendAlpha = zero;
	DestBlendAlpha = zero;
	BlendOpAlpha = add;
	RenderTargetWriteMask[0] = 0x0F;
};

RasterizerState gRasterizerState
{
	CullMode = none;
	FrontCounterClockwise = false; //default
};

DepthStencilState gDepthStencilState
{
	DepthEnable = true;
	DepthWriteMask = zero;
	DepthFunc = less;
	StencilEnable = false;

	//others are redundant because
	//StencilEnable is false
	//(for demo purposes only)
	StencilReadMask = 0x0F;
	StencilWriteMask = 0x0F;

	FrontFaceStencilFunc = always;
	BackFaceStencilFunc = always;

	FrontFaceStencilDepthFail = keep;
	BackFaceStencilDepthFail = keep;

	FrontFaceStencilPass = keep;
	BackFaceStencilPass = keep;

	FrontFaceStencilFail = keep;
	BackFaceStencilFail = keep;
};

//-------------------------
//	Input/Output Structs
//-------------------------
struct VS_INPUT
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float2 UV : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float4 WorldPosition : COLOR;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float2 UV : TEXCOORD;
};

//-------------------------
//	Vertex Shader
//-------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output;
	output.Position = mul(float4(input.Position, 1.0f), gWorldViewProj);
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
		SetRasterizerState(gRasterizerState);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}