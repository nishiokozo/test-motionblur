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
//	float2 uv 	: TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 pos : POSITION;
	float2 uv : TEXCOORD0;
};
struct PS_INPUT
{
	float2 uv : TEXCOORD0;
};

struct PS_OUTPUT
{
	float4 col : COLOR;
};

#if 0
float offset[5] = { 0.0, 1.0, 2.0, 3.0, 4.0 };
float	weight[5] = { 0.2270270270, 0.1945945946, 0.1216216216,  0.0540540541, 0.0162162162 };
#else
int		params;
float	offset[16];
float	weight[16];
#endif

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

	ret.col = tex2D( image0, p.uv ) * weight[0];


	float	ofs = dotpitch.x;
	for (int i=1; i<params; i++) 
	{
		ret.col += tex2D( image0, ( p.uv + float2( ofs, 0.0 ) ) ) * weight[i];
		ret.col += tex2D( image0, ( p.uv - float2( ofs, 0.0 ) ) ) * weight[i];
		ofs += dotpitch.x;
	}

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

