struct VS_INPUT
{
	float3 pos	: POSITION;
};

struct VS_OUTPUT
{
	float4 pos : POSITION;
	float4 col : COLOR0;
};
struct PS_INPUT
{
	float4 col : COLOR0;
};

struct PS_OUTPUT
{
	float4 col : COLOR0;
};


//-----------------------------------------------------------------------------
VS_OUTPUT vs( VS_INPUT v )
//-----------------------------------------------------------------------------
{
	VS_OUTPUT ret;

	ret.pos = float4(v.pos, 1);
	ret.col = float4( normalize(v.pos)/2+0.5, 1);

	return ret;
}

//-----------------------------------------------------------------------------
PS_OUTPUT fs( PS_INPUT p )
//-----------------------------------------------------------------------------
{
	PS_OUTPUT ret;

	ret.col = float4(1,1,1,1);
	ret.col = p.col;

	return ret;
}

//-----------------------------------------------------------------------------
technique Technique0
//-----------------------------------------------------------------------------
{
	pass Pass0
	{

		VertexShader = compile vs_3_0 vs();
		PixelShader  = compile ps_3_0 fs();
	}
}

