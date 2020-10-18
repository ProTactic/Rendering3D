struct VS_INPUT {
	float3 inPos: POSITION;
	float3 inColor: COLOR;
};

struct VS_OUTPUT {
	float4 outPos: SV_POSITION;
	float3 outColor: COLOR;
};

cbuffer constantBuffer : register(b0)
{
	float4x4 wvp;
};


VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	output.outPos = float4(input.inPos, 1.0f);
	output.outPos = mul(output.outPos, wvp);
	output.outColor = input.inColor;
	return output;
}