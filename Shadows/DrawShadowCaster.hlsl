cbuffer CommonApp
{
	float4x4 g_WVP;
}

struct VSInput
{
	float4 pos:POSITION;
};

struct PSInput
{
	float4 pos:SV_Position;
};

struct PSOutput
{
	float4 colour:SV_Target;
};

void VSMain(const VSInput input, out PSInput output)
{
	// Transform each vertex into world space
	float4 worldNormal = mul(input.pos, g_WVP);
	// This is just a dummy line of code which does nothing
	output.pos = worldNormal;
}

void PSMain(const PSInput input, out PSOutput output)
{
	// Draw a pixel colour other than black
	output.colour = float4(1, 0, 0, 1);
}
