float4x4 worldViewProj ;
float4x4 matModel ;
float4x4 matView ;
float4x4 matProj ;

//float3 	vctLight ;
//float3 	colLight ;

texture tex0;

sampler image0 = sampler_state
{
	Texture   = (tex0);
	MipFilter = POINT;
	MinFilter = POINT;
	MagFilter = POINT;
};

struct VS_INPUT
{
	float3 pos	: POSITION;
	float3 nml	: NORMAL0;
	float2 uv		: TEXCOORD0;
	float3 col	: COLOR0;
};

struct VS_OUTPUT
{
	float4 pos	: POSITION;
//	float3 nml	: NORMAL0;
	float2 uv   : TEXCOORD0;
	float3 col	: COLOR0;
};

struct PS_INPUT
{
//	float3 nml	: NORMAL0;
	float2 uv	  : TEXCOORD0;
	float3 col	: COLOR0;
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

	ret.pos = mul( float4(v.pos,1),  mul(matModel,mul(matView,matProj))  );
//	ret.pos = mul( float4(v.pos,1),  mul(matModel,matProj)  );

//	ret.nml = mul( v.nml, matModel );
	
	ret.uv = v.uv;

	ret.col = v.col;

	return ret;
}

//-----------------------------------------------------------------------------
PS_OUTPUT fs( PS_INPUT p )
//-----------------------------------------------------------------------------
{
	PS_OUTPUT ret;

	float3	C = tex2D( image0, p.uv ) * p.col;
//	float3	N = normalize( p.nml);
//	float3	L = vctLight;
//	float		d = dot( N, -L );

	ret.col = float4( C ,1);

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

