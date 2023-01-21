Texture2D gDiffuseMap : DiffuseMap;
Texture2D gNormalMap : NormalMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossinessMap : GlossinessMap;


float4x4 gWorldViewProj : WorldViewPorjection;
float4x4 gWorldMatrix : WorldMatrix;
float4x4 gViewInverseMatrix : ViewInverseMatrix;

static const float3 gLightDirection = normalize(float3(0.577f, -0.577f, 0.577f));
static const float PI = 3.141592653589793238f;
static const float gLightIntensity = 7.f;
static const float gShininess = 25.f;
static const float gKD = 1.f;
static const float3 gAmbientColor = float3(0.025f, 0.025f, 0.025f);

//SamplerState
SamplerState gSampler : Sampler
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Wrap; //or Mirror, Clamp, Border
	AddressV = Wrap; //or Mirror, Clamp, Border
};

BlendState gBlendState
{
	BlendEnable[0] = false;
	RenderTargetWriteMask[0] = 0x0F;
};

DepthStencilState gDepthStencilState
{
	DepthEnable = true;
	DepthWriteMask = 1;
	DepthFunc = less;
	StencilEnable = false;
};

RasterizerState gRasterizerState
{
	CullMode = none;
	FrontCounterClockwise = false; //default
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
	output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);
	output.WorldPosition = mul(float4(input.Position, 1.f), gWorldMatrix);
	output.Tangent = mul(normalize(input.Tangent), (float3x3)gWorldMatrix);
	output.Normal = mul(normalize(input.Normal), (float3x3)gWorldMatrix);
	output.UV = input.UV;
	return output;
}

//-------------------------
//	Pixel Shader
//-------------------------
float4 PS(VS_OUTPUT input) : SV_TARGET
{
	//Cache and calculate vars
	float3 viewDirection = normalize(input.WorldPosition.xyz - gViewInverseMatrix[3].xyz);
	float3 sampledNormal = input.Normal;
	const float3 binormal = cross(sampledNormal, input.Tangent);
	const float3x3 tangentSpaceAxis = { input.Tangent, normalize(binormal), sampledNormal };
	const float4 colorNormal = gNormalMap.Sample(gSampler, input.UV);
	sampledNormal = colorNormal.rgb;
	sampledNormal = (2 * sampledNormal) - float3(1.f, 1.f, 1.f);
	sampledNormal = mul(sampledNormal, tangentSpaceAxis);

	//Observed area color
	const float observedArea = saturate(dot(sampledNormal, -gLightDirection));

	//Diffuse color
	float4 diffuseColor = gDiffuseMap.Sample(gSampler, input.UV);
	diffuseColor = (diffuseColor * gKD / PI) * gLightIntensity;

	//Specular color
	const float3 reflectVector = reflect(gLightDirection, sampledNormal);
	const float reflectAngle = saturate(dot(reflectVector, -viewDirection));

	const float4 glossColor = gGlossinessMap.Sample(gSampler, input.UV);
	const float exponent = glossColor.r * gShininess;

	const float phongValue = pow(reflectAngle, exponent);
	const float4 specularColor = gSpecularMap.Sample(gSampler, input.UV) * phongValue;

	//Add each calculation to each other and convert to float4
	return float4((diffuseColor.rgb * observedArea) + specularColor.rgb + gAmbientColor, 1.f);
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