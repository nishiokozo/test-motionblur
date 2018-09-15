//#define STRICTf
#define WIN32_LEAN_AND_MEAN


#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "afilter.h"
#include "ahdr.h"

#define	_line		fprintf( stderr,"%s(%d) in %s\n", __func__, __LINE__, __FILE__ );

#define	pi	3.14159265

static	IDirect3D9*						 g_pD3D = 0;
static	IDirect3DDevice9*			 g_pd3dDevice = 0;

static	int	g_SIZE_W	= 1;
static	int	g_SIZE_H	= 1;
static	int	g_OFB_W	= (g_SIZE_W);
static	int	g_OFB_H	= (g_SIZE_H);

static	 int	g_BLUR_DIV	= 20;
#define	FFORMAT	D3DFMT_A16B16G16R16F
//#define	FFORMAT	D3DFMT_A32B32G32R32F

static	float	bloom_rate=0.5;
static	float	bloom_s=8.0;
static	int		bloom_cntWeight=15;

static	float	streak_rate=0.5;
static	int		streak_cntArms=4;
static	int		streak_cntPass=3;
static	float	streak_valAttenuation=0.95;
static	float 	streak_radSlope=(60)*pi/180.0f ;

///////////////////////////////////////////////////////////////////////////////
class	Shape
///////////////////////////////////////////////////////////////////////////////
{
	public:

	struct VertexPNTC
	{
		float x, y, z;
		float nx, ny, nz;
		float tu, tv;
		float r,g,b;
	};



	IDirect3DTexture9*						m_pTexture0;
	IDirect3DVertexBuffer9* 			m_pVertexBuffer;
	IDirect3DIndexBuffer9*				m_pIndexBuffer;
	D3DPRIMITIVETYPE	m_type;
	int	m_cntIndex;
	int	m_cntVertex;
	int	m_sizeOfVertex;
	int m_cntFace;

};

///////////////////////////////////////////////////////////////////////////////
class ShapeObj  : public Shape
///////////////////////////////////////////////////////////////////////////////
{
public:
	fpos_t	m_size;
	char*		m_pBuf;
	int			m_i;
	int			m_w;
	char		m_word[256];
//-----------------------------------------------------------------------------
ShapeObj( IDirect3DDevice9*	pd3dDevice, const char* fnObj, const char* fnTexture
//-----------------------------------------------------------------------------
	,float	_tx
	,float	_ty
	,float	_tz
	,float	_rx
	,float	_ry
	,float	_rz
	,float	_sx
	,float	_sy
	,float	_sz
	,float	_cr
	,float	_cg
	,float	_cb
)
{
	memset((this),0,sizeof(*this));

//fprintf( stderr, ">>>>>> size  of ShapeObj %d\n", sizeof(*this) );

	m_type = D3DPT_TRIANGLELIST;

	m_sizeOfVertex = sizeof(VertexPNTC);

	if ( fnTexture ) 	D3DXCreateTextureFromFile( pd3dDevice, fnTexture, &m_pTexture0 );

	text_LoadFile( fnObj );

	m_cntVertex = 0;
	m_cntIndex = 0;
	m_cntFace = 0;

	text_Init();
	while( text_GetWord() )
	{
			if ( strcmp( m_word, "v" ) == 0 ) m_cntVertex++;
			if ( strcmp( m_word, "f" ) == 0 ) m_cntIndex+=3;
	}
	m_cntFace = m_cntIndex/3;

	struct _Vertex
	{
		float	x;
		float	y;
		float	z;
		float	nx;
		float	ny;
		float	nz;
	} ;
	_Vertex* tblVertex = new _Vertex[ m_cntVertex ];

	struct _Face
	{
		int	v1;
		int	v2;
		int	v3;
	};
	_Face* tblFace = new _Face[ m_cntFace ];


	fprintf( stderr, "m_cntIndex %d,  m_cntVertex %d\n", m_cntIndex ,m_cntVertex );

	{
		text_Init();
		int	i = 0;
		while( text_GetWord() )
		{
			if ( strcmp( m_word, "v" ) == 0 ) 
			{
				text_GetWord();
				float	x = atof( m_word );
				text_GetWord();
				float	y = atof( m_word );
				text_GetWord();
				float	z = atof( m_word );

				{
					D3DXMATRIX M,T,R;	
					D3DXMatrixTranslation( &T, _tx, _ty, _tz );
					D3DXMatrixRotationYawPitchRoll( &R, _ry, _rx, _rz );		
					M = R* T;
					D3DXVECTOR3 b = D3DXVECTOR3(x,y,z);
					D3DXVECTOR4 a;
					D3DXVec3Transform( &a, &b, &M );
					x = a.x;
					y = a.y;
					z = a.z;
				}

				tblVertex[i].x = x;
				tblVertex[i].y = y;
				tblVertex[i].z = z;
				i++;
			}
		}
		fprintf( stderr, "n = %d\n", i );

	}

	{

		text_Init();
		int	i = 0;
		while( text_GetWord() )
		{
			if ( strcmp( m_word, "f" ) == 0 ) 
			{
				text_GetWord();
				int	v1 = atoi( m_word )-1;
				text_GetWord();
				int	v2 = atoi( m_word )-1;
				text_GetWord();
				int	v3 = atoi( m_word )-1;
				tblFace[i].v1 = v1;
				tblFace[i].v2 = v2;
				tblFace[i].v3 = v3;
				i++;
			}

		}
		fprintf( stderr, "n = %d\n", i );
	}

	text_Remove();

	//	normal vector
	for ( int i = 0 ; i < m_cntFace ; i++ )
	{
		_Vertex& v1 = tblVertex[tblFace[i].v1];
		_Vertex& v2 = tblVertex[tblFace[i].v2];
		_Vertex& v3 = tblVertex[tblFace[i].v3];

		_Vertex	va;
		va.x = v2.x - v1.x;
		va.y = v2.y - v1.y;
		va.z = v2.z - v1.z;
		float	la = sqrt( va.x*va.x + va.y*va.y + va.z*va.z );
		va.x /= la;
		va.y /= la;
		va.z /= la;

		_Vertex	vb;
		vb.x = v3.x - v1.x;
		vb.y = v3.y - v1.y;
		vb.z = v3.z - v1.z;
		float	lb = sqrt( vb.x*vb.x + vb.y*vb.y + vb.z*vb.z );
		vb.x /= lb;
		vb.y /= lb;
		vb.z /= lb;

		float	nx = va.y*vb.z - va.z*vb.y;
		float	ny = va.z*vb.x - va.x*vb.z;
		float	nz = va.x*vb.y - va.y*vb.x;

		v1.nx += nx;
		v1.ny += ny;
		v1.nz += nz;
		v2.nx += nx;
		v2.ny += ny;
		v2.nz += nz;
		v3.nx += nx;
		v3.ny += ny;
		v3.nz += nz;
	}
	for ( int i = 0 ; i < m_cntVertex ; i++ )
	{
		_Vertex& v = tblVertex[i];
		float	l = sqrt( v.x*v.x + v.y*v.y + v.z*v.z );
		v.nx /= l;
		v.ny /= l;
		v.nz /= l;
	}

	//	vertex index buffer
	pd3dDevice->CreateVertexBuffer( sizeof(VertexPNTC)*m_cntVertex ,D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &m_pVertexBuffer, NULL );
	{
		VertexPNTC *pVertex = NULL;
		m_pVertexBuffer->Lock( 0, sizeof(VertexPNTC)*m_cntVertex, (void**)&pVertex, 0 );
		for ( int i  = 0 ; i < m_cntVertex ; i++ )
		{
			pVertex[i].x = tblVertex[i].x;
			pVertex[i].y = tblVertex[i].y;
			pVertex[i].z = tblVertex[i].z;
			pVertex[i].nx = tblVertex[i].nx;
			pVertex[i].ny = tblVertex[i].ny;
			pVertex[i].nz = tblVertex[i].nz;
			pVertex[i].tu = 0;
			pVertex[i].tv = 0;
			pVertex[i].r = 1.0;
			pVertex[i].g = 1.0;
			pVertex[i].b = 1.0;
		}
		m_pVertexBuffer->Unlock();
	}

	pd3dDevice->CreateIndexBuffer( m_cntIndex*sizeof(int), D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &m_pIndexBuffer, NULL );
	{
		int *pIndex = NULL;
		m_pIndexBuffer->Lock( 0, 0, (void**)&pIndex, 0 );
		for ( int i  = 0 ; i < m_cntFace ; i++ )
		{
			pIndex[i*3+0] = tblFace[i].v1;
			pIndex[i*3+1] = tblFace[i].v2;
			pIndex[i*3+2] = tblFace[i].v3;
		}
		m_pIndexBuffer->Unlock();
	}
	
	delete [] tblVertex;
	delete [] tblFace;
}	
//-----------------------------------------------------------------------------
void	text_Remove()
//-----------------------------------------------------------------------------
{
	if ( m_pBuf ) free( m_pBuf );
	m_pBuf = 0;	
}
//-----------------------------------------------------------------------------
void	text_LoadFile( const char* fn )
//-----------------------------------------------------------------------------
{
	{
		FILE*	fp = 0;
		
		if ( 0==(fp= fopen( fn, "rb" ) ) ) 
		{
				fprintf( stderr, "Error fopen :%s\n", fn );
	 		_line;
	 		exit(1);
		}

		fseek(fp, 0, SEEK_END);
		fgetpos(fp, &m_size);

		if ( (int)m_size == 0 )
		{
				fprintf( stderr, "Error size if 0 \n" );
	 		_line;
	 		exit(1);
		}
		fseek(fp, 0, SEEK_SET);
		
		m_pBuf = (char*)malloc( m_size );

		fread( m_pBuf, sizeof(char), m_size, fp );
		fclose(fp);
	}
}
//-----------------------------------------------------------------------------
void	text_Init()
//-----------------------------------------------------------------------------
{
	m_i = 0;
}
//-----------------------------------------------------------------------------
bool	text_GetWord()
//-----------------------------------------------------------------------------
{
	bool ret = false;

	m_w = 0;
	m_word[m_w] = 0;

	for ( ; m_i < (int)m_size ; m_i++ )
	{
			char	c = m_pBuf[ m_i ];
			if ( isgraph(c) )
			{
				m_word[m_w++] = c;
			}
			else
			{
				m_word[m_w] = 0;

				ret = true;
				m_i++;
				break;
			} 
	}

	return ret;
}

};



///////////////////////////////////////////////////////////////////////////////
class Model : public Shape
///////////////////////////////////////////////////////////////////////////////
{
public:

	D3DXMATRIX m_matModel;
	D3DXMATRIX m_prev_matModel;

	float	m_r;
	float	m_g;
	float	m_b;

	float	valReflectance;
	bool	flgShowVelocity;
	
	//----

	float	m_back_thx;
	float	m_back_thy;
	float	m_back_thz;
	float	m_back_rx;
	float	m_back_ry;
	float	m_back_rz;

	float m_athx;
	float m_athy;
	float m_athz;
	float	m_rotY;
	float	m_rotX;
	float	m_rotZ;
	float m_thx;
	float m_thy;
	float m_thz;

	float	m_tx;
	float	m_ty;
	float	m_tz;
	float	m_rx;
	float	m_ry;
	float	m_rz;
	float	m_sx;
	float	m_sy;
	float	m_sz;
	

	void setCol( float _r, float _g, float _b )
	{
		m_r = _r;
		m_g = _g;
		m_b = _b;
	}
	void movePos( float _x, float _y, float _z )
	{
		m_tx += _x;
		m_ty += _y;
		m_tz += _z;
	}
	void setPos( float _x, float _y, float _z )
	{
		m_tx = _x;
		m_ty = _y;
		m_tz = _z;
	}
	void setRot( float _x, float _y, float _z )
	{
		m_rx = _x;
		m_ry = _y;
		m_rz = _z;
	}
	void setScaling( float _x, float _y, float _z )
	{
		m_sx = _x;
		m_sy = _y;
		m_sz = _z;
	}

//-----------------------------------------------------------------------------
Model( const Shape& shape
//-----------------------------------------------------------------------------
	,float	_tx
	,float	_ty
	,float	_tz
	,float	_rx
	,float	_ry
	,float	_rz
	,float	_sx
	,float	_sy
	,float	_sz
	,float	_cr
	,float	_cg
	,float	_cb
) : Shape(shape)
{
//fprintf( stderr, ">>>>>> size  of Model %d\n", sizeof((*this)) );
	valReflectance = 0.5;

	D3DXMatrixIdentity( &m_matModel );
	m_tx = _tx;
	m_ty = _ty;
	m_tz = _tz;
	m_rx = _rx;
	m_ry = _ry;
	m_rz = _rz;
	m_sx = _sx;
	m_sy = _sy;
	m_sz = _sz;
	m_r  = _cr;
	m_g  = _cg;
	m_b  = _cb;
}	

//-----------------------------------------------------------------------------
void Restore()
//-----------------------------------------------------------------------------
{
		m_thx = m_back_thx;
		m_thy = m_back_thy;
		m_thz = m_back_thz;
		m_rx  = m_back_rx;
		m_ry  = m_back_ry;
		m_rz  = m_back_rz;

		m_matModel = m_prev_matModel;
}
//-----------------------------------------------------------------------------
void Store()
//-----------------------------------------------------------------------------
{
		m_back_thx = m_thx;
		m_back_thy = m_thy;
		m_back_thz = m_thz;
		m_back_rx = m_rx;
		m_back_ry = m_ry;
		m_back_rz = m_rz;
		m_prev_matModel = m_matModel;
}
//-----------------------------------------------------------------------------
void UpdateMatrix( float val )
//-----------------------------------------------------------------------------
{
		m_thx += m_athx*val;
		m_thy += m_athy*val;
		m_thz += m_athz*val;
		m_rx += m_rotX*val;
		m_ry += m_rotY*val;
		m_rz += m_rotZ*val;

		m_tx = 1.0*sin(m_thx);
		m_ty = 1.0*sin(m_thy);
		m_tz = 2.0*sin(m_thz);

	//---

	{

		D3DXMATRIX matTrans;
		D3DXMATRIX matScale;
		D3DXMATRIX matRot;

		D3DXMatrixIdentity( &m_matModel );
		D3DXMatrixTranslation( &matTrans, m_tx, m_ty, m_tz );
		D3DXMatrixScaling( &matScale, m_sx, m_sy, m_sz );
		D3DXMatrixRotationYawPitchRoll( &matRot, m_ry, m_rx,  m_rz );
		m_matModel *= matScale;
		m_matModel *= matRot;
		m_matModel *= matTrans;
	}

}

};

///////////////////////////////////////////////////////////////////////////////
class Light
///////////////////////////////////////////////////////////////////////////////
{
public:
		struct 
		{
			float	x;
			float	y;
			float	z;
		} loc ;

		struct 
		{
			float	r;
			float	g;
			float	b;
		} col ;

		Light( float x, float y, float z, float r, float g, float b)
		{
			memset((this),0,sizeof(*this));

			loc.x = x;
			loc.y = y;
			loc.z = z;
			col.r = r;
			col.g = g;
			col.b = b;

		}
};


///////////////////////////////////////////////////////////////////////////////
class	Renderer
///////////////////////////////////////////////////////////////////////////////
{
public:
	ID3DXEffect*									m_pEffectPNTC;
	IDirect3DVertexDeclaration9*	m_pDeclarationPNTC;

//-----------------------------------------------------------------------------
Renderer( IDirect3DDevice9*	pd3dDevice, const char* fnShader )
//-----------------------------------------------------------------------------
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
void DrawModel( IDirect3DDevice9*	pd3dDevice, Model& shape, Light light, D3DXMATRIX& matView, D3DXMATRIX& matProj , D3DXMATRIX& prev_matView, D3DXMATRIX& matCam, IDirect3DTexture9 *pTexture )
//-----------------------------------------------------------------------------
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
};

///////////////////////////////////////////////////////////////////////////////
class ModelNanika : public Model
///////////////////////////////////////////////////////////////////////////////
{
public:

	Renderer*	m_p_rendererSorce;
	Renderer*	m_p_rendererBlurSimon;
	Renderer*	m_p_rendererBlurNishio;
	Renderer*	m_p_rendererBlurKozo;
	AFilter*	m_p_filterAdd;

	float m_dotpitch[2];

	IDirect3DTexture9 *pTextureA;
	IDirect3DSurface9 *pSurfaceA;

	IDirect3DTexture9 *pTextureB;
	IDirect3DSurface9 *pSurfaceB;

	IDirect3DTexture9 *pTextureC;
	IDirect3DSurface9 *pSurfaceC;



//-----------------------------------------------------------------------------
ModelNanika( IDirect3DDevice9*	pd3dDevice, const Model& model ) : Model(model)
//-----------------------------------------------------------------------------
{
//fprintf( stderr, ">>>>>> size  of ModelNanika %d\n", sizeof((*this)) );

	m_p_rendererSorce 			= new Renderer( pd3dDevice, "s_PNC.fx" );
	m_p_rendererBlurSimon 	= new Renderer( pd3dDevice, "s_mb-simon.fx" );
	m_p_rendererBlurNishio 	= new Renderer( pd3dDevice, "s_mb-nishio.fx" );
	m_p_rendererBlurKozo 		= new Renderer( pd3dDevice, "s_mb-kozo.fx" );
	m_p_filterAdd 					= new AFilter( pd3dDevice, "f_add.fx" );

	m_dotpitch[0] = 1.0f/g_OFB_W;
	m_dotpitch[1] = 1.0f/g_OFB_H;

	pd3dDevice->CreateTexture( g_OFB_W, g_OFB_H, 1, D3DUSAGE_RENDERTARGET, FFORMAT, D3DPOOL_DEFAULT, &pTextureA, 0 );
	pTextureA->GetSurfaceLevel( 0, &pSurfaceA );

	pd3dDevice->CreateTexture( g_OFB_W, g_OFB_H, 1, D3DUSAGE_RENDERTARGET, FFORMAT, D3DPOOL_DEFAULT, &pTextureB, 0 );
	pTextureB->GetSurfaceLevel( 0, &pSurfaceB );

	pd3dDevice->CreateTexture( g_OFB_W, g_OFB_H, 1, D3DUSAGE_RENDERTARGET, FFORMAT, D3DPOOL_DEFAULT, &pTextureC, 0 );
	pTextureC->GetSurfaceLevel( 0, &pSurfaceC );

}


//-----------------------------------------------------------------------------
void	DrawSimonBlur( IDirect3DDevice9*	pd3dDevice, const Light& light, D3DXMATRIX& matView, D3DXMATRIX& matProj , D3DXMATRIX& prev_matView, D3DXMATRIX& matCam  )
//-----------------------------------------------------------------------------
{
	//--
	IDirect3DSurface9 *prim_pSurface = 0;
	pd3dDevice->GetRenderTarget( 0, &prim_pSurface );

	{
		UpdateMatrix( 1.0 );

		pd3dDevice->SetRenderTarget( 0, pSurfaceA );
		pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_COLORVALUE(0.2f,0.6f,0.8f,1.0f), 1.0f, 0);
		m_p_rendererSorce->DrawModel( pd3dDevice, (*this), light, matView, matProj, prev_matView, matCam, 0 ); 

		pd3dDevice->SetRenderTarget( 0, pSurfaceC );
		pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_COLORVALUE(0.2f,0.6f,0.8f,1.0f), 1.0f, 0);
		m_p_rendererBlurSimon->DrawModel( pd3dDevice, (*this), light, matView, matProj, prev_matView, matCam, pTextureA );

		(*this).flgShowVelocity = true;
		pd3dDevice->SetRenderTarget( 0, pSurfaceB );
		pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_COLORVALUE(0.2f,0.6f,0.8f,1.0f), 1.0f, 0);
		m_p_rendererBlurSimon->DrawModel( pd3dDevice, (*this), light, matView, matProj, prev_matView, matCam, 0 );
		(*this).flgShowVelocity = false;
	}

	pd3dDevice->SetRenderTarget( 0, prim_pSurface );
	if ( prim_pSurface ) prim_pSurface->Release();

}

//-----------------------------------------------------------------------------
void	Draw3DBlur( IDirect3DDevice9*	pd3dDevice, const Light& light, D3DXMATRIX& matView, D3DXMATRIX& matProj , D3DXMATRIX& prev_matView, D3DXMATRIX& matCam  )
//-----------------------------------------------------------------------------
{
	IDirect3DSurface9 *prim_pSurface = 0;
	pd3dDevice->GetRenderTarget( 0, &prim_pSurface );
	{
		pd3dDevice->SetRenderTarget( 0, pSurfaceB );
		pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_COLORVALUE(0.0f,0.0f,0.0f,1.0f), 1.0f, 0);
		pd3dDevice->SetRenderTarget( 0, pSurfaceC );
		pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_COLORVALUE(0.0f,0.0f,0.0f,1.0f), 1.0f, 0);

		int	div = g_BLUR_DIV;
		for ( int i = 0 ; i < div ; i++ )
		{
			(*this).UpdateMatrix( 1.0/div );

			pd3dDevice->SetRenderTarget( 0, pSurfaceA );
			pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_COLORVALUE(0.2f,0.6f,0.8f,1.0f), 1.0f, 0);
			m_p_rendererSorce->DrawModel( pd3dDevice, (*this), light, matView, matProj, prev_matView, matCam, 0 ); 

			if ( i & 0x1 )
			{
				pd3dDevice->SetRenderTarget( 0, pSurfaceC );
				m_p_filterAdd->DrawAdd( pd3dDevice, pTextureB, pTextureA, 1.0/div );
			}
			else
			{
				pd3dDevice->SetRenderTarget( 0, pSurfaceB );
				m_p_filterAdd->DrawAdd( pd3dDevice, pTextureC, pTextureA, 1.0/div );
			}
		}
	}
	pd3dDevice->SetRenderTarget( 0, prim_pSurface );
	if ( prim_pSurface ) prim_pSurface->Release();

}
//-----------------------------------------------------------------------------
void	DrawNormal( IDirect3DDevice9*	pd3dDevice, const Light& light, D3DXMATRIX& matView, D3DXMATRIX& matProj , D3DXMATRIX& prev_matView, D3DXMATRIX& matCam  )
//-----------------------------------------------------------------------------
{
	IDirect3DSurface9 *prim_pSurface = 0;
	pd3dDevice->GetRenderTarget( 0, &prim_pSurface );
	{
		pd3dDevice->SetRenderTarget( 0, pSurfaceC );
		pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_COLORVALUE(0.0f,0.0f,0.0f,1.0f), 1.0f, 0);

			(*this).UpdateMatrix( 1.0 );

			pd3dDevice->SetRenderTarget( 0, pSurfaceC );
			pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_COLORVALUE(0.2f,0.6f,0.8f,1.0f), 1.0f, 0);
			m_p_rendererSorce->DrawModel( pd3dDevice, (*this), light, matView, matProj, prev_matView, matCam, 0 ); 

	}
	pd3dDevice->SetRenderTarget( 0, prim_pSurface );
	if ( prim_pSurface ) prim_pSurface->Release();

}

};

static	ModelNanika	*g_pModel = 0;




//-----------------------------------------------------------------------------
int	temp_IsToggle( int val )
//-----------------------------------------------------------------------------
{
	static	int flg[256];
	static	int prev[256]; 
	{
		int	key = GetKeyState( val );
		if ( key < 0 && prev[val] >=0 ) 
		{
			flg[val] = !flg[val];
		}
		prev[val] = key;
	}
	return	flg[ val ];
}

//-----------------------------------------------------------------------------
int	temp_IsTrigger( int val )
//-----------------------------------------------------------------------------
{
	static	int flg[256];
	static	int prev[256]; 
	{
		int	key = GetKeyState( val );
		if ( key < 0 && prev[val] >=0 ) 
		{
			flg[val] = true;
		}
		else
		{
			flg[val] = false;
		}
		prev[val] = key;
	}
	return	flg[ val ];
}


//-----------------------------------------------------------------------------
int	temp_IsPress( int val )
//-----------------------------------------------------------------------------
{
	return  GetKeyState( val ) & 0xfe ;

}













//-----------------------------------------------------------------------------
void dx_Remove( IDirect3D9** ppD3D, IDirect3DDevice9** ppd3dDevice )
//-----------------------------------------------------------------------------
{
	if( (*ppd3dDevice) != NULL )
			(*ppd3dDevice)->Release();

	if( (*ppD3D) != NULL )
			(*ppD3D)->Release();
}

//-----------------------------------------------------------------------------
void dx_Create( HWND& hWnd, IDirect3D9** ppD3D, IDirect3DDevice9** ppd3dDevice )
//-----------------------------------------------------------------------------
{

	(*ppD3D) = Direct3DCreate9( D3D_SDK_VERSION );

	D3DDISPLAYMODE d3ddm;

	(*ppD3D)->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm );

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory( &d3dpp, sizeof(d3dpp) );

	d3dpp.Windowed							 = TRUE;
	d3dpp.SwapEffect						 = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat			 = d3ddm.Format;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	d3dpp.PresentationInterval	 = D3DPRESENT_INTERVAL_IMMEDIATE;

	(*ppD3D)->CreateDevice( 
		D3DADAPTER_DEFAULT, 
		D3DDEVTYPE_HAL, 
		hWnd,

//		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&d3dpp, 
		&(*ppd3dDevice) 
	);


#if 0
	{
		D3DCAPS9 Caps;
		(*ppd3dDevice)->GetDeviceCaps( &Caps );
		DWORD MaxRT = Caps.NumSimultaneousRTs;
			fprintf(stderr, "number fo render target 	%d \n", (int)MaxRT );
	}
#endif


}



//-----------------------------------------------------------------------------
void dx_Paint( IDirect3DDevice9*	pd3dDevice)
//-----------------------------------------------------------------------------
{

	D3DXMATRIX matProj;	
	D3DXMatrixPerspectiveFovLH( &matProj, D3DXToRadian( 30.0f ), (float)g_OFB_W / (float)g_OFB_H, 0.1f, 10000.0f );

//	static	ModelNanika& mdl_obj = *new ModelNanika( pd3dDevice, *new Model( ShapeObj( pd3dDevice,  "bunny.obj", "test.bmp",   0, 0, 0,			0,	pi,	0,		1.0, 1.0, 1.0,   1.0, 1.0, 1.0 ),   0, 0,  0,			0,	0,	0,		1.0, 1.0, 1.0,   1.0, 1.0, 1.0 ) );
//	static	ModelNanika& mdl_obj = *new ModelNanika( pd3dDevice, *new Model( ShapeObj( pd3dDevice,  "buddha.obj", "test.bmp",   0, 0, 0,			0,	pi,	0,		1.0, 1.0, 1.0,   1.0, 1.0, 1.0 ),   0, 0,  0,			0,	0,	0,		1.0, 1.0, 1.0,   1.0, 1.0, 1.0 ) );
//	static	ModelNanika& mdl_obj = *new ModelNanika( pd3dDevice, *new Model( ShapeObj( pd3dDevice,  "dragon.obj", "test.bmp",   0, 0, 0,			0,	pi,	0,		1.0, 1.0, 1.0,   1.0, 1.0, 1.0 ),   0, 0,  0,			0,	0,	0,		1.0, 1.0, 1.0,   1.0, 1.0, 1.0 ) );

	ModelNanika& mdl_obj = *g_pModel;

	static	Light		light( 0, 3, -2,  35,25,45 );


	
	if ( temp_IsTrigger( VK_LEFT ) )	mdl_obj.m_rotY += D3DXToRadian(2.1);
	if ( temp_IsTrigger( VK_RIGHT ) )	mdl_obj.m_rotY -= D3DXToRadian(2.1);
	if ( temp_IsTrigger( VK_UP ) )		mdl_obj.m_rotX += D3DXToRadian(2.1);
	if ( temp_IsTrigger( VK_DOWN ) )	mdl_obj.m_rotX -= D3DXToRadian(2.1);
	if ( temp_IsTrigger( 'Z' ) )		mdl_obj.m_rotZ += D3DXToRadian(1.1);
	if ( temp_IsTrigger( 'X' ) )		mdl_obj.m_rotZ -= D3DXToRadian(1.1);

	if ( temp_IsTrigger( 'A' ) ) mdl_obj.m_athx -= D3DXToRadian(1);
	if ( temp_IsTrigger( 'D' ) ) mdl_obj.m_athx += D3DXToRadian(1);
	if ( temp_IsTrigger( 'W' ) ) mdl_obj.m_athy += D3DXToRadian(1);
	if ( temp_IsTrigger( 'S' ) ) mdl_obj.m_athy -= D3DXToRadian(1);
	if ( temp_IsTrigger( 'F' ) ) mdl_obj.m_athz += D3DXToRadian(1);
	if ( temp_IsTrigger( 'V' ) ) mdl_obj.m_athz -= D3DXToRadian(1);
	

	static	bool flgPause = false;
	if ( temp_IsTrigger( VK_SPACE ) ) flgPause=!flgPause;

	bool	flgStep = false;

	if ( temp_IsTrigger( VK_RETURN ) ) flgStep = true;



	static	D3DXMATRIX prev_matView;
	static	bool flgFirst = true;
	if ( flgFirst )
	{
		flgFirst = false ;
		D3DXMatrixIdentity( &prev_matView ); 
	}


	D3DXMATRIX matView;
	D3DXMATRIX matCam;
	{
		if ( !temp_IsToggle( VK_F1 ) )
		{
			D3DXMATRIX matRot;
			D3DXMATRIX matTrans;
			D3DXMatrixIdentity( &matCam ); 
			D3DXMatrixIdentity( &matRot ); 
			D3DXMatrixIdentity( &matTrans ); 
			D3DXMatrixTranslation( &matTrans, 0, 0, -6 );
			D3DXMatrixRotationYawPitchRoll( &matRot, D3DXToRadian(0), D3DXToRadian(0), 0.0f );		
			matCam = matRot* matTrans;
	 	D3DXMatrixInverse( &matView, NULL, &matCam );
		}
		else
		{
			D3DXMATRIX matRot;
			D3DXMATRIX matTrans;
			D3DXMatrixIdentity( &matCam ); 
			D3DXMatrixIdentity( &matRot ); 
			D3DXMatrixIdentity( &matTrans ); 
			D3DXMatrixTranslation( &matTrans, 0, 6, 0 );
			D3DXMatrixRotationYawPitchRoll( &matRot, D3DXToRadian(0), D3DXToRadian(90), 0.0f );		
			matCam = matRot* matTrans;
	 	D3DXMatrixInverse( &matView, NULL, &matCam );
		}
	}
	

	static enum
	{
		NONE,
		SIMON,
		BLUR3D
	}	alg = BLUR3D;

	if ( temp_IsTrigger( '0' ) ) alg = NONE;
	if ( temp_IsTrigger( '9' ) ) alg = SIMON;
	if ( temp_IsTrigger( '8' ) ) alg = BLUR3D;


	if ( flgPause && !flgStep )		mdl_obj.Restore();
	mdl_obj.Store();

	static	AHdr		hdr( pd3dDevice, g_SIZE_W/2, g_SIZE_H/2, FFORMAT );
 	hdr.SetFilterTonemap( 1.0 );


	pd3dDevice->BeginScene();
	{
		switch( alg )
		{
		case NONE:
			mdl_obj.DrawNormal( pd3dDevice, light, matView, matProj, prev_matView, matCam );
			break;

		case SIMON:
			mdl_obj.DrawSimonBlur( pd3dDevice, light, matView, matProj, prev_matView, matCam );
			break;

		case BLUR3D:
			mdl_obj.Draw3DBlur( pd3dDevice, light, matView, matProj, prev_matView, matCam );
		break;
			
		}		

		IDirect3DSurface9 *prim_pSurface = 0;
		pd3dDevice->GetRenderTarget( 0, &prim_pSurface );
		pd3dDevice->StretchRect( mdl_obj.pSurfaceA, 0,	prim_pSurface, 0,	D3DTEXF_LINEAR );


/* 	if ( temp_IsTrigger( '1' ) ) 
 	{
		hdr.SetFlg( AHdr::TONEMAP, false );
		hdr.SetFlg( AHdr::BLOOM, false );
		hdr.SetFlg( AHdr::STREAK, false );
 	}
 	if ( temp_IsTrigger( '3' ) ) hdr.SetFlg( AHdr::BLOOM,   !hdr.GetFlg( AHdr::BLOOM ) );
 	if ( temp_IsTrigger( '4' ) ) hdr.SetFlg( AHdr::STREAK,  !hdr.GetFlg( AHdr::STREAK ) );
*/
 	if ( !temp_IsToggle( '2' ) ) 
 	{
 	hdr.SetFilterTonemap( 1.0 );
 	}
 	else
 	{
 	hdr.SetFilterTonemap( 0.0 );
 	}

 	if ( temp_IsToggle( 'G' ) ) 
 	{
		hdr.SetFilterTonemap( 0.0 );
		hdr.SetFilterBloom( bloom_rate, bloom_s, bloom_cntWeight );
		hdr.SetFilterStreak( streak_rate, streak_cntArms, streak_cntPass, streak_valAttenuation, streak_radSlope);
		hdr.DrawFilter( pd3dDevice, prim_pSurface, mdl_obj.pTextureC );
 	}
 	else
 	{
			pd3dDevice->StretchRect( mdl_obj.pSurfaceC, 0,	prim_pSurface, 0,	D3DTEXF_POINT );
 	}

		if ( prim_pSurface ) prim_pSurface->Release();

	}
	pd3dDevice->EndScene();



	prev_matView = matView;


	pd3dDevice->Present( NULL, NULL, NULL, NULL );


}



//-----------------------------------------------------------------------------
LRESULT CALLBACK WindowProc( HWND	 hWnd, 
//-----------------------------------------------------------------------------
							 UINT	 msg, 
							 WPARAM wParam, 
							 LPARAM lParam )
{

		switch( msg )
	{
		
		case WM_PAINT:
			{
					static LARGE_INTEGER liFreq, liNow, liPrev;
					static	double time = 0;
					QueryPerformanceFrequency(&liFreq);
					QueryPerformanceCounter(&liNow);
					time += (double)(liNow.QuadPart - liPrev.QuadPart) / liFreq.QuadPart;
					QueryPerformanceCounter(&liPrev);

					if ( time > 1.0 / 60.0 )
					{
						time = 0;
						if ( g_pd3dDevice ) dx_Paint( g_pd3dDevice );
					}
			}
			return 0;
			break;
		case WM_CLOSE:
			{
				PostQuitMessage(0);	
			}
			break;

		case WM_DESTROY:
			{
					PostQuitMessage(0);
			}
			break;
		
		default:
			{
				return DefWindowProc( hWnd, msg, wParam, lParam );
			}
			break;
	}

	return 0;
}
//-----------------------------------------------------------------------------
int WINAPI WinMain(	HINSTANCE hInstance,
//-----------------------------------------------------------------------------
					HINSTANCE hPrevInstance,
					LPSTR		 lpCmdLine,
					int			 nCmdShow )
{

	{
		LPTSTR cl = GetCommandLine();
		char*	cr=strrchr(cl,' ');

		if ( cr==0 ) cr=(char*)"-middle"; else cr++;


		if ( strcmp(cr,"-large")==0)
		{
			g_SIZE_W	= 1024;
			g_SIZE_H	= 1024;
			g_BLUR_DIV=48;
		}
		else
		if ( strcmp(cr,"-middle")==0)
		{
			g_SIZE_W	= 512;
			g_SIZE_H	= 512;
			g_BLUR_DIV=32;
		}
		else
		if ( strcmp(cr,"-small")==0)
		{
			g_SIZE_W	= 256;
			g_SIZE_H	= 256;
			g_BLUR_DIV=20;
		}


		fprintf( stderr, "---------------\n" );
		fprintf( stderr, "commandline:[%s]\n",cr);
		fprintf( stderr, "switch:\n" );
		fprintf( stderr, "  main.exe  -large : 1024x1024x48div\n" );
		fprintf( stderr, "  main.exe -middle:   512x 512x32div (default)\n" );
		fprintf( stderr, "  main.exe -small :   256x 256x20div\n" );
		fprintf( stderr, "  main.exe -file:filename.obj :  default:bunny.obj\n" );
		fprintf( stderr, "---------------\n" );
		fprintf( stderr, "param:\n" );
		fprintf( stderr, "  W=%d H=%d DIV=%d\n",g_SIZE_W, g_SIZE_H, g_BLUR_DIV);
		fprintf( stderr, "---------------\n" );
		fprintf( stderr, "key control:\n" );
		fprintf( stderr, "  up  /down : rotate x\n" );
		fprintf( stderr, "  left/right: rotate y\n" );
		fprintf( stderr, "  Z/X       : rotate z\n" );
		fprintf( stderr, "  A/D       : move x axis\n" );
		fprintf( stderr, "  W/S       : move y axis\n" );
		fprintf( stderr, "  F/V       : move z axis\n" );
		fprintf( stderr, "   G        : swith grea filter\n" );
		fprintf( stderr, "   0        : blur NONE\n" );
		fprintf( stderr, "   9        : blur 2.5D\n" );
		fprintf( stderr, "   8        : blur 3D\n" );
		fprintf( stderr, "---------------\n" );
	}

	g_OFB_W	= (g_SIZE_W);
	g_OFB_H	= (g_SIZE_H);


	HWND	hWnd						 = NULL;

	WNDCLASSEX winClass;
	MSG				uMsg;

	memset(&uMsg,0,sizeof(uMsg));
		
	winClass.lpszClassName	= "MY_WINDOWS_CLASS";
	winClass.cbSize			= sizeof(WNDCLASSEX);
	winClass.style			= CS_HREDRAW | CS_VREDRAW;
	winClass.lpfnWndProc	= WindowProc;
	winClass.hInstance		= hInstance;
	winClass.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_APPLICATION);
	winClass.hIconSm		= LoadIcon(hInstance, (LPCTSTR)IDI_APPLICATION);
	winClass.hCursor		= LoadCursor(NULL, IDC_ARROW);
	winClass.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
	winClass.lpszMenuName	= NULL;
	winClass.cbClsExtra		= 0;
	winClass.cbWndExtra		= 0;

	if( !RegisterClassEx(&winClass) )
		return E_FAIL;

	hWnd = CreateWindowEx( 
		0, 
		"MY_WINDOWS_CLASS", 
		"nishio kozo",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		880, 
		0, 
		g_OFB_W, 
		g_OFB_H, 
		NULL, 
		NULL, 
		hInstance, 
		NULL 
	);

	{
		RECT	rc;
		SetRect(&rc, 0, 0, g_OFB_W, g_OFB_H);
		AdjustWindowRectEx(&rc, WS_OVERLAPPEDWINDOW, FALSE, 0);
		SetWindowPos( hWnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER );
	}

	{
		RECT	rc;
		GetWindowRect(hWnd, &rc);
		fprintf(stderr, "windows size	%d %d\n", (int)rc.right,		(int)rc.bottom );
	}

	if( hWnd == NULL ) return E_FAIL;

	dx_Create( hWnd, &g_pD3D, &g_pd3dDevice );

	{
		RECT	rc;
		SetRect(&rc, 0, 0, g_SIZE_W, g_SIZE_H);
		AdjustWindowRectEx(&rc, WS_OVERLAPPEDWINDOW, FALSE, 0);
		SetWindowPos( hWnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER );
	}

	//---------------

	g_pModel = new ModelNanika( g_pd3dDevice, *new Model( ShapeObj( g_pd3dDevice, "bunny.obj", "test.bmp",   0, 0, 0,			0,	pi,	0,		1.0, 1.0, 1.0,   1.0, 1.0, 1.0 ),   0, 0,  0,			0,	0,	0,		1.0, 1.0, 1.0,   1.0, 1.0, 1.0 ) );
	ShowWindow( hWnd, nCmdShow );
	UpdateWindow( hWnd );

	//---------------
	

	while( uMsg.message != WM_QUIT )
	{
		if( PeekMessage( &uMsg, NULL, 0, 0, PM_REMOVE ) )
		{ 
			TranslateMessage( &uMsg );
			DispatchMessage( &uMsg );
		}
		else
		{

		}
	}

	dx_Remove( &g_pD3D, &g_pd3dDevice );

	return uMsg.wParam;
}


