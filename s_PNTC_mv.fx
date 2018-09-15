float4x4 matModel ;
float4x4 matView ;
float4x4 matProj ;
float4x4 prev_matModel ;
float4x4 prev_matView ;

float3 	locLight ;
float3 	colLight ;
float3	colModel ;

texture tex0;
texture tex1;

sampler image0 = sampler_state
{
	Texture   = (tex0);
	MipFilter = POINT;
	MinFilter = POINT;
	MagFilter = POINT;
};
sampler image1 = sampler_state
{
	Texture   = (tex1);
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
	float3 vel	: NORMAL3;
};

struct PS_INPUT
{
	float3 nml	: NORMAL0;
	float2 uv	  : TEXCOORD0;
	float3 col	: COLOR0;
	float3 eye	: NORMAL1;
	float3 loc	: NORMAL2;
	float3 vel	: NORMAL3;
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

//	v M V P
//	ret.pos = mul( v.pos,  mul( matModel, mul(matView,matProj) )  );

	float4 pos0 = mul( v.pos, mul( prev_matModel, prev_matView )  );
	float4 pos1 = mul( v.pos, mul( matModel, matView )  );

	float3	nml = mul( v.nml, mul( matModel, matView ) );

	float3	vel = pos1.xyz - pos0.xyz;

	float 	d = dot( nml, vel );

	if ( d > 0 ) 
	{
		ret.pos = mul(pos1, matProj);
	}
	else
	{
		ret.pos = mul(pos0, matProj);
	}
	ret.loc = ret.pos;



	ret.vel =
		 mul( v.pos,  mul( matModel, matView) )
		-mul( v.pos,  mul( prev_matModel, prev_matView) )
	;

	ret.nml = mul( v.nml, matModel );
	
	ret.uv = v.uv;

	ret.col = v.col;

	ret.eye = matView[3];
	
	ret.eye = normalize( ret.eye );

//	ret.loc = mul( v.pos,  matModel );
	ret.loc = ret.pos;
ret.loc.xyz /= ret.pos.w;
ret.loc.xyz/=2;
ret.loc.xyz+=0.5;
ret.loc.y=-ret.loc.y;
	return ret;
}

//-----------------------------------------------------------------------------
PS_OUTPUT fs( PS_INPUT p )
//-----------------------------------------------------------------------------
{
	PS_OUTPUT ret;


	float3	A = float3( 0.2,0.2,0.2);
	float3	C = tex2D( image0, p.uv ) * p.col * colModel;
	float3	L = normalize( p.loc - locLight );
	float3	Lc = colLight / dot( p.loc - locLight, p.loc - locLight );
	float		d = max( 0, dot( N, -L ) );

	float3	R = reflect( -L, N );

	int			n = 40;
	float3	Dc = (1.0-valReflectance) * C * d * Lc;
	float3	Sc =       valReflectance * Lc * ((n+2)/(8*3.14))*pow(s,n) ;




#if 0
	float	blurScale = 1.0;
	float	samples = 32;
	float	w = 1.0/samples;

	float4 a = 0;
	float i;
	for ( i = 0 ; i < samples ; i++ )
	{
		float t = i / (samples-1);
		a = a + tex2D( image0, p.loc.xy +p.vel*t )*w;
	}
	ret.col = a;

#else
	ret.col = float4( Dc + Sc + C * A,1);
#endif



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

