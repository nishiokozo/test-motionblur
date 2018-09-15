float4x4 worldViewProj ;
float4x4 matModel ;
float4x4 matView ;
float4x4 matProj ;

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
	float2 uv0	: TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 pos	: POSITION;
	float3 nml	: NORMAL0;
	float2 uv0  : TEXCOORD0;
};

struct PS_INPUT
{
	float3 nml	: NORMAL0;
	float2 uv0	: TEXCOORD0;
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

	ret.pos = mul( float4(v.pos,1),  mul(matModel,matProj)  );

	ret.nml = mul( v.nml, matModel );
	
	ret.uv0 = v.uv0;

	return ret;
}

//-----------------------------------------------------------------------------
PS_OUTPUT fs( PS_INPUT p )
//-----------------------------------------------------------------------------
{
	PS_OUTPUT ret;

//	float3	C = tex2D( image0, p.uv0 );
	float3	C = float3(1,1,1);
	float3	N = normalize(p.nml);
	float3	L = float3(0,0,1);
	float		d = dot( N, -L );

	ret.col = float4(C*d,1);

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

