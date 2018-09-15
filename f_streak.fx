float2	dir;
float		attenuation;
int			gpass;
float		rate;


texture tex0;
float2	dotpitch;


sampler image0 = sampler_state
{
	Texture   = (tex0);
	MipFilter = POINT;
	MinFilter = LINEAR;
	MagFilter = LINEAR;

		    AddressU = Clamp;
    AddressV = Clamp;

};

struct VS_INPUT
{
	float4 pos	: POSITION;
};

struct VS_OUTPUT
{
	float4 pos : POSITION;
	float2 uv  : TEXCOORD0;
};
struct PS_INPUT
{
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

#define	NUM_SAMPLES	4
//-----------------------------------------------------------------------------
PS_OUTPUT fs( PS_INPUT p )
//-----------------------------------------------------------------------------
{
	PS_OUTPUT ret;

	p.uv += 0.5*dotpitch;

	float3 col = 0;

	float	b = pow(NUM_SAMPLES,gpass);

	float2 	uv = p.uv;
	float2 	a  = dir * b;


	for( int i = 0 ; i < NUM_SAMPLES ; i++ )
	{

		float w = pow(attenuation, b * i )*rate;

		col += w * tex2D( image0, uv );

		uv +=  a;

	}

 //	col *= float3(0.27, 0.67, 0.06 );  


	ret.col = float4( col, 1 );
	
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

