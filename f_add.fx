texture tex0;
texture tex1;
//float2	dotpitch;

float	range1;

sampler image0 = sampler_state
{
	Texture   = (tex0);
	MipFilter = POINT;
//	MinFilter = LINEAR;
//	MagFilter = LINEAR;
	MinFilter = POINT;
	MagFilter = POINT;

		    AddressU = Clamp;
    AddressV = Clamp;

};
sampler image1 = sampler_state
{
	Texture   = (tex1);
	MipFilter = POINT;
//	MinFilter = LINEAR;
	//MagFilter = LINEAR;
	MinFilter = POINT;
	MagFilter = POINT;
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

	float3	col ;
	col =  tex2D( image0, p.uv );
	col += tex2D( image1, p.uv )*range1;


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

