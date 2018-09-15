texture tex0;
texture tex1;
texture tex2;
texture texTone;
float2	dotpitch;
float	rate1;
float	rate2;
float	rateTone;

sampler imageTone = sampler_state
{
	Texture   = (texTone);
	MipFilter = POINT;
	MinFilter = POINT;
	MagFilter = POINT;

		    AddressU = Clamp;
    AddressV = Clamp;

};
sampler image0 = sampler_state
{
	Texture   = (tex0);
	MipFilter = POINT;
	MinFilter = POINT;
	MagFilter = POINT;

		    AddressU = Clamp;
    AddressV = Clamp;

};
sampler image1 = sampler_state
{
	Texture   = (tex1);
	MipFilter = POINT;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
		    AddressU = Clamp;
    AddressV = Clamp;

};
sampler image2 = sampler_state
{
	Texture   = (tex2);
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

//  オフセットは不要
//	p.uv += 0.5*dotpitch;

	float4	col ;
	col = clamp( tex2D( image0, p.uv ), 0.0, 1.0 );
	col += tex2D( image1, p.uv )*rate1;
	col += tex2D( image2, p.uv )*rate2;





#if 1
	float3 tone = tex2D( imageTone, float2(0.5,0.5) );
	float	valu = dot( tone, float3(0.27, 0.67, 0.06 ) );

	#if 0
		col *= exp2(bias);
		col = max(col - 0.004, 0.0);
		col = (col * (6.2 * col + 0.5)) / (col * (6.2 * col + 1.7) + 0.06);
	#endif

	#if 0
		col *= exp2(bias);
		col /= 1.0+col;
	#endif

	#if 1
		float t = (1/(valu+0.1)*0.3);
		col = (col * t * rateTone) + ( col * (1-rateTone) );
	#endif
#endif


//	col = clamp( tex2D( image0, p.uv ), 0.0, 1.0 );
//	col += tex2D( image1, p.uv );
//	col = tex2D( imageTone, float2(0,0) );

//	col = tex2D( image1, p.uv );



//	col = clamp( tex2D( image0, p.uv ), 0.0, 1.0 );

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

