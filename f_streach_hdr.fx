texture tex0;
float2	dotpitch;

sampler image0 = sampler_state
{
	Texture   = (tex0);
	MipFilter = POINT;
	MinFilter = POINT;
	MagFilter = POINT;

		    AddressU = Clamp;
    AddressV = Clamp;

};

struct VS_INPUT
{
	float4 pos	: POSITION;
//	float2 uv : TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 pos : POSITION;
	float2 uv  : TEXCOORD0;
};
struct PS_INPUT
{
//	float4 pos : POSITION;
	float2 uv  : TEXCOORD0;
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

	ret.pos = v.pos;
	ret.uv.x = v.pos.x/2+0.5;
	ret.uv.y = -v.pos.y/2+0.5;
	
	return ret;
}

//-----------------------------------------------------------------------------
PS_OUTPUT fs( PS_INPUT p )
//-----------------------------------------------------------------------------
{
	PS_OUTPUT ret;

	p.uv += 0.5*dotpitch;

	ret.col = max( tex2D( image0, p.uv )-float4(1,1,1,0), 0 );

	ret.col.a = 1.0;
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

    CullMode = None;
    AlphaBlendEnable = false;
    ZWriteEnable = false;
	}
}

