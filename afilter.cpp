#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <stdio.h>

#include "afilter.h"

//-----------------------------------------------------------------------------
AFilter::~AFilter()
//-----------------------------------------------------------------------------
{
	if ( m_pEffect ) m_pEffect->Release();
	if ( m_pVertexBuffer ) m_pVertexBuffer->Release();
	if ( m_pIndexBuffer ) m_pIndexBuffer->Release();
	if ( m_pDeclaration ) m_pDeclaration->Release();

}
//-----------------------------------------------------------------------------
AFilter::AFilter( IDirect3DDevice9*	pd3dDevice, const char* fnShader )
//-----------------------------------------------------------------------------
{
	memset( (this), 0, sizeof(*this) );

	Vertex tblVertex[] =
	{
		{-1.0f,-1.0f, 0.0f },
		{-1.0f, 1.0f, 0.0f },
		{ 1.0f,-1.0f, 0.0f },
		{ 1.0f, 1.0f, 0.0f }
	};

	int	tblIndex[] = 
	{
		0, 1, 2, 3,	
	};
	

	{
		pd3dDevice->CreateVertexBuffer( sizeof(tblVertex), D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &m_pVertexBuffer, NULL );

		{
			void *pVertices = NULL;
			m_pVertexBuffer->Lock( 0, 0, (void**)&pVertices, 0 );
			memcpy( pVertices, tblVertex, sizeof(tblVertex) );
			m_pVertexBuffer->Unlock();
		}
	}
	
	{
		pd3dDevice->CreateIndexBuffer( sizeof(tblIndex), D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &m_pIndexBuffer, NULL );

		{
			void *pIndex = NULL;
			m_pIndexBuffer->Lock( 0, 0, (void**)&pIndex, 0 );
			memcpy( pIndex, tblIndex, sizeof(tblIndex) );
			m_pIndexBuffer->Unlock();
		}

	}


	m_cntVertex = sizeof(tblVertex)/sizeof(Vertex);
	m_cntIndex = sizeof(tblIndex)/sizeof(int);
	

	{
		D3DVERTEXELEMENT9 VertexElements[] =
		{
		    { 0,  0, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		    D3DDECL_END()
		};

    HRESULT hr = pd3dDevice->CreateVertexDeclaration(VertexElements, &m_pDeclaration );
    if  ( hr != D3D_OK )
    {
			fprintf( stderr, "Error pDeclaration \n" );
    }
	}

		{
			HRESULT hr;
			ID3DXBuffer* pBufferErrors = NULL;

			hr = D3DXCreateEffectFromFile( 
				pd3dDevice, 
				fnShader,
				NULL, 
				NULL, 
				0, 
				NULL, 
				&m_pEffect, 
				&pBufferErrors
			);

	    if  ( hr != D3D_OK )
	    {
				if ( pBufferErrors == 0 )
				{
					fprintf( stderr, "File not found [%s]: %s(%d) in %s\n", fnShader, __func__, __LINE__, __FILE__ );
					exit(-1);
				}

				char* pStr  = (char*)pBufferErrors->GetBufferPointer();
				fprintf( stderr, "%s: %s(%d) in %s\n", pStr, __func__, __LINE__, __FILE__ );
				exit(-1);
	    }
		}

}

//-----------------------------------------------------------------------------
void	AFilter::DrawFilter( IDirect3DDevice9*	pd3dDevice )
//-----------------------------------------------------------------------------
{

	pd3dDevice->SetStreamSource( 0, m_pVertexBuffer, 0, sizeof(Vertex) );
	pd3dDevice->SetIndices(m_pIndexBuffer);
	pd3dDevice->SetVertexDeclaration(m_pDeclaration);


	UINT uPasses;
	m_pEffect->Begin( &uPasses, 0 );
	
	for( UINT uPass = 0; uPass < uPasses; ++uPass )
	{
		m_pEffect->BeginPass( uPass );

		pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLESTRIP, 0, 0, m_cntVertex, 0, m_cntIndex-2 );

		m_pEffect->EndPass();
	}
 
	m_pEffect->End();

}
//-----------------------------------------------------------------------------
void AFilter::DrawBlend( IDirect3DDevice9*	pd3dDevice, IDirect3DTexture9 *pTexture0, IDirect3DTexture9 *pTexture1 )
//-----------------------------------------------------------------------------
{
  	m_pEffect->SetTechnique( "Technique0" );
		m_pEffect->SetTexture( "tex0", pTexture0 );
		m_pEffect->SetTexture( "tex1", pTexture1 );
//  オフセットは不要
//		m_pEffect->SetFloatArray( "dotpitch", dotpitch, 2 );

		DrawFilter( pd3dDevice );
}
//-----------------------------------------------------------------------------
void AFilter::DrawAdd( IDirect3DDevice9*	pd3dDevice, IDirect3DTexture9 *pTexture0, IDirect3DTexture9 *pTexture1, float range )
//-----------------------------------------------------------------------------
{
  	m_pEffect->SetTechnique( "Technique0" );
		m_pEffect->SetTexture( "tex0", pTexture0 );
		m_pEffect->SetTexture( "tex1", pTexture1 );
		m_pEffect->SetFloat( "range1", range );
//  オフセットは不要
//		m_pEffect->SetFloatArray( "dotpitch", dotpitch, 2 );

		DrawFilter( pd3dDevice );
}

//-----------------------------------------------------------------------------
void AFilter::DrawStreach( IDirect3DDevice9*	pd3dDevice, IDirect3DTexture9 *pTexture0, float* dotpitch )
//-----------------------------------------------------------------------------
{
  	m_pEffect->SetTechnique( "Technique0" );
		m_pEffect->SetTexture( "tex0", pTexture0 );
		m_pEffect->SetFloatArray( "dotpitch", dotpitch, 2 );

		DrawFilter( pd3dDevice );
}

//-----------------------------------------------------------------------------
void AFilter::DrawBlur( IDirect3DDevice9*	pd3dDevice, IDirect3DTexture9 *pTexture0, D3DXMATRIX& M, D3DXMATRIX& prev_M, float* dotpitch )
//-----------------------------------------------------------------------------
{
		m_pEffect->SetTechnique( "Technique0" );
		m_pEffect->SetTexture( "tex0", pTexture0 );
		m_pEffect->SetMatrix( "M", &M );
		m_pEffect->SetMatrix( "prev_M", &prev_M );
		m_pEffect->SetFloatArray( "dotpitch", dotpitch, 2 );

		DrawFilter( pd3dDevice );

}

//-----------------------------------------------------------------------------
void AFilter::DrawStreak( IDirect3DDevice9*	pd3dDevice, IDirect3DTexture9 *pTexture0, float attenuation, float* dir, int gpass, float rate, float* dotpitch )
//-----------------------------------------------------------------------------
{
  	m_pEffect->SetTechnique( "Technique0" );
		m_pEffect->SetTexture( "tex0", pTexture0 );
		m_pEffect->SetFloat( "attenuation", attenuation );
		m_pEffect->SetFloatArray( "dir", dir, 2 );
		m_pEffect->SetInt( "gpass",	gpass ); 
		m_pEffect->SetFloat( "rate",	rate ); 
		m_pEffect->SetFloatArray( "dotpitch", dotpitch, 2 );

		DrawFilter( pd3dDevice );

}

//-----------------------------------------------------------------------------
void AFilter::DrawTonemap2( IDirect3DDevice9*	pd3dDevice, IDirect3DTexture9 *pTexture0, IDirect3DTexture9 *pTexture1, IDirect3DTexture9* tone_pTexture, float* dotpitch )
//-----------------------------------------------------------------------------
{
  	m_pEffect->SetTechnique( "Technique0" );
		m_pEffect->SetTexture( "texLDR", pTexture0 );
		m_pEffect->SetTexture( "texHDR", pTexture1 );
		m_pEffect->SetTexture( "texTone", tone_pTexture );
		m_pEffect->SetFloatArray( "dotpitch", dotpitch, 2 );

		DrawFilter( pd3dDevice );
}
//-----------------------------------------------------------------------------
void AFilter::DrawTonemap3( IDirect3DDevice9*	pd3dDevice, IDirect3DTexture9 *pTexture0, IDirect3DTexture9 *pTexture1, IDirect3DTexture9 *pTexture2, IDirect3DTexture9* tone_pTexture, float rate1, float rate2, float rateTone, float* dotpitch )
//-----------------------------------------------------------------------------
{
  	m_pEffect->SetTechnique( "Technique0" );
		m_pEffect->SetTexture( "tex0", pTexture0 );
		m_pEffect->SetTexture( "tex1", pTexture1 );
		m_pEffect->SetTexture( "tex2", pTexture2 );
		m_pEffect->SetTexture( "texTone", tone_pTexture );
		m_pEffect->SetFloat( "rate1",	rate1 ); 
		m_pEffect->SetFloat( "rate2",	rate2 ); 
		m_pEffect->SetFloat( "rateTone",	rateTone ); 
		m_pEffect->SetFloatArray( "dotpitch", dotpitch, 2 );

		DrawFilter( pd3dDevice );
}
//-----------------------------------------------------------------------------
void AFilter::DrawGaussian( IDirect3DDevice9*	pd3dDevice, IDirect3DTexture9 *pTexture0, float s, int cntWeight, float* dotpitch )
//-----------------------------------------------------------------------------
{
		float	tblWeight[cntWeight];

		{
			float	pi = 3.141592f;
			float	sum = 0.0f;
			for ( int x = 0 ; x < cntWeight ; x++ )
			{
				double g = 1/(sqrt(2*pi)*s)*exp(-x*x/(2*s*s));
				sum += g;
				tblWeight[x] = g;
			}

			sum = tblWeight[0];
			for ( int x = 1 ; x < cntWeight ; x++ )
			{
				sum += tblWeight[x]*2;
			}

			for ( int x = 0 ; x < cntWeight ; x++ )
			{
				tblWeight[x] /= sum;
			}
		}

  	m_pEffect->SetTechnique( "Technique0" );
		m_pEffect->SetTexture( "tex0", pTexture0 );
		{
			D3DSURFACE_DESC	desc;
			pTexture0->GetLevelDesc( 0, &desc );
			m_pEffect->SetFloatArray( "dotpitch", dotpitch, 2 );
			m_pEffect->SetFloatArray( "weight", tblWeight, cntWeight ); 
			m_pEffect->SetInt( "params",	cntWeight ); 
		}

		DrawFilter( pd3dDevice );

}
		
