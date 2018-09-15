#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "shape.h"
#include "shapeobj.h"
#include "model.h"
#include "light.h"
#include "renderer.h"

#define	_line		fprintf( stderr,"%s(%d) in %s\n", __func__, __LINE__, __FILE__ );

//-----------------------------------------------------------------------------
Renderer::Renderer(
//-----------------------------------------------------------------------------
	   IDirect3DDevice9*	pd3dDevice
	 , const char* 			fnShader
)
{
	memset( this, 0, sizeof(*this) );

	// rendererSorce
	if ( m_pDeclarationPNTC == NULL )
	{

		{
			D3DVERTEXELEMENT9 VertexElementsPNTC[] =
			{
			 	{ 0,  0, D3DDECLTYPE_FLOAT3,  D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
			 	{ 0, 12, D3DDECLTYPE_FLOAT3,  D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
			 	{ 0, 24, D3DDECLTYPE_FLOAT2,  D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
			 	{ 0, 32, D3DDECLTYPE_FLOAT3,  D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
			 	D3DDECL_END()
			};

	 	HRESULT hr = pd3dDevice->CreateVertexDeclaration(VertexElementsPNTC, &m_pDeclarationPNTC );
	 	if ( hr != D3D_OK )
	 	{
				fprintf( stderr, "Error pDeclaration \n" );
	 		_line;
	 	}
		}
	}	

	{

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
				&m_pEffectPNTC, 
				&pBufferErrors
			);

	 	if ( hr != D3D_OK )
	 	{
				if ( pBufferErrors == 0 )
				{
					fprintf( stderr, "File not found [%s]: %s(%d) in %s\n", fnShader, __func__, __LINE__, __FILE__ );
					exit(-1);
				}

				char* pStr	= (char*)pBufferErrors->GetBufferPointer();
				fprintf( stderr, "%s: %s(%d) in %s\n", pStr, __func__, __LINE__, __FILE__ );
				exit(-1);
	 	}
		}
	}
}

//-----------------------------------------------------------------------------
void Renderer::DrawModel( 
//-----------------------------------------------------------------------------
	IDirect3DDevice9*		pd3dDevice
	, Model& 				shape
	, Light 				light
	, D3DXMATRIX& 			matView
	, D3DXMATRIX& 			matProj 
	, D3DXMATRIX& 			prev_matView
	, D3DXMATRIX& 			matCam
	, IDirect3DTexture9*	pTexture 
)
{

	m_pEffectPNTC->SetTechnique( "Technique0" );
	m_pEffectPNTC->SetMatrix( "matModel", &shape.m_matModel );
	m_pEffectPNTC->SetMatrix( "matView", &matView );
	m_pEffectPNTC->SetMatrix( "matProj", &matProj );
	m_pEffectPNTC->SetMatrix( "prev_matModel", &shape.m_prev_matModel );
	m_pEffectPNTC->SetMatrix( "prev_matView", &prev_matView );
	m_pEffectPNTC->SetMatrix( "matCam", &matCam );
	if ( pTexture )
	{
		if ( shape.m_pTexture0 ) m_pEffectPNTC->SetTexture( "tex0", pTexture );
	}
	else
	{
		if ( shape.m_pTexture0 ) m_pEffectPNTC->SetTexture( "tex0", shape.m_pTexture0 );
	}
	m_pEffectPNTC->SetFloatArray( "locLight", (float*)&light.loc, 3 );
	m_pEffectPNTC->SetFloatArray( "colLight", (float*)&light.col, 3 );
	float colModel[3] = { shape.m_r, shape.m_g, shape.m_b };
	m_pEffectPNTC->SetFloatArray( "colModel", colModel, 3 );
	m_pEffectPNTC->SetInt( "flgShowVelocity", shape.flgShowVelocity );
	m_pEffectPNTC->SetFloat( "valReflectance", shape.valReflectance );


	pd3dDevice->SetStreamSource( 0, shape.m_pVertexBuffer, 0, shape.m_sizeOfVertex );
	pd3dDevice->SetIndices(shape.m_pIndexBuffer);
	pd3dDevice->SetVertexDeclaration(m_pDeclarationPNTC);

	{

		UINT uPasses;
		m_pEffectPNTC->Begin( &uPasses, 0 );
		
		for( UINT uPass = 0; uPass < uPasses; ++uPass )
		{
			m_pEffectPNTC->BeginPass( uPass );

			pd3dDevice->DrawIndexedPrimitive( shape.m_type, 0, 0, shape.m_cntVertex, 0, shape.m_cntFace );

			m_pEffectPNTC->EndPass();
		}
	 
		m_pEffectPNTC->End();
	}


}

