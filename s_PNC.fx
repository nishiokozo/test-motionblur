float4x4 matModel ;
float4x4 matView ;
float4x4 matProj ;
float4x4 matCam ;

float3 	locLight ;
float3 	colLight ;
float3	colModel ;

float	valReflectance;
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
	float4 pos	: POSITION;
	float3 nml	: NORMAL0;
	float2 uv		: TEXCOORD0;
	float3 col	: COLOR0;
};

struct VS_OUTPUT
{
	float4 pos	: POSITION;
	float3 nml	: NORMAL0;
	float2 uv   : TEXCOORD0;
	float3 col	: COLOR0;
	float3 eye	: NORMAL1;
	float3 loc	: NORMAL2;
};

struct PS_INPUT
{
	float3 nml	: NORMAL0;
	float2 uv	  : TEXCOORD0;
	float3 col	: COLOR0;
	float3 eye	: NORMAL1;
	float3 loc	: NORMAL2;
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

	ret.pos = mul( v.pos,  mul( matModel, mul(matView,matProj) )  );

	ret.nml = mul( v.nml, matModel );
	
	ret.uv = v.uv;

	ret.col = v.col;
	
  ret.eye = matCam[3];
	
	ret.loc = mul( v.pos,  matModel );

	return ret;
}

//-----------------------------------------------------------------------------
PS_OUTPUT fs( PS_INPUT p )
//-----------------------------------------------------------------------------
{
	PS_OUTPUT ret;

	float3	A = float3( 0.2,0.2,0.2);
//	float3	C = tex2D( image0, p.uv ) * p.col * colModel;
	float3	C = p.col * colModel;
	float3	E = normalize( p.loc - p.eye);
	float3	N = normalize( p.nml);
	float3	L = normalize( p.loc - locLight );
	float3	Lc = colLight / dot( p.loc - locLight, p.loc - locLight );
	float		d = max( 0, dot( N, -L ) );

	float3	R = reflect( E, N );
	float		s = max( dot( R, -L ), 0 ) ;
	int			n = 40;


	float3	Dc = (1.0-valReflectance) * C * d * Lc;
	float3	Sc =       valReflectance * Lc * ((n+2)/(8*3.14))*pow(s,n) ;

	ret.col = float4( Dc + Sc + C * A,1);

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

