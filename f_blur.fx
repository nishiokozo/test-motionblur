float4x4 M ;
float4x4 prev_M ;

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
};

struct VS_OUTPUT
{
	float4 pos : POSITION;
	float2 uv  : TEXCOORD0;
	float2 vel  : NORMAL;
};

struct PS_INPUT
{
	float2 uv  : TEXCOORD0;
	float2 vel  : NORMAL;
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

//	float4 p = mul( v.pos,  mul(      M, mul(V,P) )  );
//	float4 q = mul( v.pos,  mul( prev_M, mul(V,P) )  );
	float4 p = mul( v.pos,  M );
	float4 q = mul( v.pos,  prev_M );

	p.xy /= p.w;
	q.xy /= q.w;

	ret.vel = q-p;


	ret.uv.x =  v.pos.x/2+0.5;
	ret.uv.y = -v.pos.y/2+0.5;
	
	return ret;
}

//-----------------------------------------------------------------------------
PS_OUTPUT fs( PS_INPUT p )
//-----------------------------------------------------------------------------
{
	PS_OUTPUT ret;

	p.uv += 0.5*dotpitch;

	ret.col = 0.0;

	int	cnt = 20;

	float2	uv = p.uv;
	float2	d = p.vel / cnt;
d.y /= 1;
d.x = -d.x;
	float		f = 1.0 / cnt;


	for ( int i = 0 ; i < cnt ; i++ )
	{  
		ret.col += f*tex2D( image0 , uv );
		uv += d;
	}

//ret.col = tex2D( image0 , p.uv );


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

