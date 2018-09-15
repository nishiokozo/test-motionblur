float4x4 matModel ;
float4x4 matView ;
float4x4 matProj ;

float3 	locLight ;
float3 	colLight ;
float3	colModel ;

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

	ret.pos = mul( float4(v.pos,1),  mul( matModel, mul(matView,matProj) )  );


	ret.nml = mul( v.nml, matModel );
	
//	ret.uv = v.uv;

	ret.uv.x =     ret.pos.x / ret.pos.w *0.5 +0.5; 
	ret.uv.y = 1-( ret.pos.y / ret.pos.w *0.5 +0.5); 


	ret.col = v.col;
	


ret.eye = matView[3];
	
	ret.eye = normalize( ret.eye );

	ret.loc = mul( float4(v.pos,1),  matModel );

	return ret;
}

//-----------------------------------------------------------------------------
PS_OUTPUT fs( PS_INPUT p )
//-----------------------------------------------------------------------------
{
	PS_OUTPUT ret;

	float3	A = float3( 0.2,0.2,0.2);
//	float3	C = tex2D( image0, p.uv ) * p.col * colModel;
//	float3	C = p.col * colModel;
	float3	C = p.col * float3(0,1,0);
	float3	N = normalize( p.nml);
	float3	L = normalize( p.loc - locLight );
	float		d = max( 0, dot( N, -L ) );

	float3	R = reflect( -L, N );
	float		s = max( dot( R, p.eye ), 0 ) ;
	float3	spe = colLight * pow(s,40) ;

	float3 col = C * d * colLight + spe + C * A;






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
	}
}

