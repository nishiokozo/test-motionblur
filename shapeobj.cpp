//頂点データローダー .obj(maya形式)モデルデータ用
#define WIN32_LEAN_AND_MEAN


#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "shape.h"
#include "shapeobj.h"

#define	_line		fprintf( stderr,"%s(%d) in %s\n", __func__, __LINE__, __FILE__ );


	//-----------------------------------------------------------------------------
void	ShapeObj::text_Remove()
	//-----------------------------------------------------------------------------
	{
		if ( m_pBuf ) free( m_pBuf );
		m_pBuf = 0;	
	}
	//-----------------------------------------------------------------------------
	void	ShapeObj::text_LoadFile( const char* fn )
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
	void	ShapeObj::text_Init()
	//-----------------------------------------------------------------------------
	{
		m_i = 0;
	}
	//-----------------------------------------------------------------------------
	bool	ShapeObj::text_GetWord()
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

	//-----------------------------------------------------------------------------
	ShapeObj::ShapeObj(
	//-----------------------------------------------------------------------------
		 IDirect3DDevice9*	pd3dDevice
		, const char* 		fnObj
		, const char* 		fnTexture
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
