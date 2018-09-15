float4x4 matModel ;
float4x4 matView ;
float4x4 matProj ;


struct VS_INPUT
{
	float3 pos	: POSITION;
	float3 col	: COLOR0;
};

struct VS_OUTPUT
{
	float4 pos	: POSITION;
	float3 col	: COLOR0;
};

struct PS_INPUT
{
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

	ret.pos = mul( float4(v.pos,1),  mul( matModel, mul(matView,matProj) )  );


	ret.col = v.col;
	
	return ret;
}

//-----------------------------------------------------------------------------
PS_OUTPUT fs( PS_INPUT p )
//-----------------------------------------------------------------------------
{
	PS_OUTPUT ret;

	float3	C =  p.col;

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

