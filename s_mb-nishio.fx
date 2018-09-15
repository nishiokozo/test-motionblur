float4x4 matModel ;
float4x4 matView ;
float4x4 matProj ;
float4x4 prev_matModel ;
float4x4 prev_matView ;

texture tex0;

int FlgVelocity;
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
	float3 nml	: NORMAL0;
	float2 uv		: TEXCOORD0;
	float3 col	: COLOR0;
};

struct VS_OUTPUT
{
	float4 pos	: POSITION;
	float2 uv   : TEXCOORD0;
	float2 vel	: NORMAL3;
};

struct PS_INPUT
{
	float2 uv	  : TEXCOORD0;
	float2 vel	: NORMAL3;
};

struct PS_OUTPUT
{
	float4 col	: COLOR;
};


//-----------------------------------------------------------------------------
VS_OUTPUT vs( VS_INPUT v )
//-----------------------------------------------------------------------------
{
	VS_OUTPUT ret;


	float4	pos0 = mul( v.pos, mul( prev_matModel, prev_matView )  );
	float4	pos1 = mul( v.pos, mul( matModel, matView )  );
	float4	p0 =mul(pos0, matProj);
	float4	p1 =mul(pos1, matProj);

	float3	nml = mul( v.nml, mul( matModel, matView ) );
	float3	vel = pos1.xyz - pos0.xyz;
	float 	d = dot( nml, vel );
	ret.pos = ( d > 0 )?p1:p0;

	p1.xy /= p1.w;
	ret.uv.x =   (p1.x/2+0.5);
	ret.uv.y = 1-(p1.y/2+0.5);
	ret.vel = 0;

	p0.xy /= p0.w;
	ret.vel.x =   p1.x - p0.x;
	ret.vel.y = -(p1.y - p0.y);


	return ret;
}

//-----------------------------------------------------------------------------
PS_OUTPUT fs( PS_INPUT p )
//-----------------------------------------------------------------------------
{
	PS_OUTPUT ret;
	ret.col = 0;

		int	div = 32;
		float	alpha = 1.0/div;

		float2	d = p.vel / div;

		for ( int i = 0 ; i < div ; i++ )
		{
			ret.col += alpha * tex2D( image0, p.uv - d*i );
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
	}
}

