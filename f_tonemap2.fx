texture texTone;
texture texLDR;
texture texHDR;
float2	dotpitch;
float	val;

sampler imageTone = sampler_state
{
	Texture   = (texTone);
	MipFilter = POINT;
	MinFilter = LINEAR;
	MagFilter = LINEAR;

		    AddressU = Clamp;
    AddressV = Clamp;

};
sampler imageLDR = sampler_state
{
	Texture   = (texLDR);
	MipFilter = POINT;
	MinFilter = LINEAR;
	MagFilter = LINEAR;

		    AddressU = Clamp;
    AddressV = Clamp;

};
sampler imageHDR = sampler_state
{
	Texture   = (texHDR);
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
	float4 pos : POSITION;
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

	float4	col ;
	col = clamp( tex2D( imageLDR, p.uv ), 0.0, 1.0 );
	col += tex2D( imageHDR, p.uv );





#if 1
	float3 tone = tex2D( imageTone, float2(0.5,0.5) );
	float	valu = dot( tone, float3(0.27, 0.67, 0.06 ) );

	#if 0
		col *= exp2(val);
		col = max(col - 0.004, 0.0);
		col = (col * (6.2 * col + 0.5)) / (col * (6.2 * col + 1.7) + 0.06);
	#endif

	#if 0
		col *= exp2(val);
		col /= 1.0+col;
	#endif

	#if 1
		col *= 1/(valu+0.1)*0.3;
	#endif
#endif


//	col = clamp( tex2D( imageLDR, p.uv ), 0.0, 1.0 );
//	col += tex2D( imageHDR, p.uv );
//	col = tex2D( imageTone, float2(0,0) );

//	col = tex2D( imageHDR, p.uv );


	ret.col = col;
	ret.col.a =1.0;
	
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

