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

#include "afilter.h"
#include "ahdr.h"

#define	_line		fprintf( stderr,"%s(%d) in %s\n", __func__, __LINE__, __FILE__ );

#define	pi	3.14159265

static	IDirect3D9*					 g_pD3D = 0;
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

static	float	streak_rate				= 0.5;
static	int		streak_cntArms			= 4;
static	int		streak_cntPass			= 3;
static	float	streak_valAttenuation	= 0.95;
static	float 	streak_radSlope			= (60)*pi/180.0f;



///////////////////////////////////////////////////////////////////////////////
class ModelNanika : public Model
///////////////////////////////////////////////////////////////////////////////
{
public:

	Renderer*	m_p_rendererSorce;
	Renderer*	m_p_rendererBlurSimon;
//	Renderer*	m_p_rendererBlurNishio;
//	Renderer*	m_p_rendererBlurKozo;
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

		m_p_rendererSorce 		= new Renderer( pd3dDevice, "s_PNC.fx" );
		m_p_rendererBlurSimon 	= new Renderer( pd3dDevice, "s_mb-simon.fx" );
	//	m_p_rendererBlurNishio 	= new Renderer( pd3dDevice, "s_mb-nishio.fx" );
	//	m_p_rendererBlurKozo	= new Renderer( pd3dDevice, "s_mb-kozo.fx" );
		m_p_filterAdd 			= new AFilter( pd3dDevice, "f_add.fx" );

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

};

static	ModelNanika	*g_pModel = 0;




//-----------------------------------------------------------------------------
int	key_IsToggle( int val )
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
int	key_IsTrigger( int val )
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
int	key_IsPress( int val )
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


	
	if ( key_IsTrigger( VK_LEFT ) )	mdl_obj.m_rotY += D3DXToRadian(2.1);
	if ( key_IsTrigger( VK_RIGHT ) )	mdl_obj.m_rotY -= D3DXToRadian(2.1);
	if ( key_IsTrigger( VK_UP ) )		mdl_obj.m_rotX += D3DXToRadian(2.1);
	if ( key_IsTrigger( VK_DOWN ) )	mdl_obj.m_rotX -= D3DXToRadian(2.1);
	if ( key_IsTrigger( 'Z' ) )		mdl_obj.m_rotZ += D3DXToRadian(1.1);
	if ( key_IsTrigger( 'X' ) )		mdl_obj.m_rotZ -= D3DXToRadian(1.1);

	if ( key_IsTrigger( 'A' ) ) mdl_obj.m_athx -= D3DXToRadian(1);
	if ( key_IsTrigger( 'D' ) ) mdl_obj.m_athx += D3DXToRadian(1);
	if ( key_IsTrigger( 'W' ) ) mdl_obj.m_athy += D3DXToRadian(1);
	if ( key_IsTrigger( 'S' ) ) mdl_obj.m_athy -= D3DXToRadian(1);
	if ( key_IsTrigger( 'F' ) ) mdl_obj.m_athz += D3DXToRadian(1);
	if ( key_IsTrigger( 'V' ) ) mdl_obj.m_athz -= D3DXToRadian(1);
	

	static	bool flgPause = false;
	if ( key_IsTrigger( VK_SPACE ) ) flgPause=!flgPause;

	bool	flgStep = false;

	if ( key_IsTrigger( VK_RETURN ) ) flgStep = true;



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
		if ( !key_IsToggle( VK_F1 ) )
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
		ALG_NONE,
		ALG_SIMON,
		ALG_BLUR3D
	}	alg = ALG_BLUR3D;
//	alg = ALG_SIMON;

	if ( key_IsTrigger( '0' ) ) alg = ALG_NONE;
	if ( key_IsTrigger( '9' ) ) alg = ALG_SIMON;
	if ( key_IsTrigger( '8' ) ) alg = ALG_BLUR3D;


	if ( flgPause && !flgStep )		mdl_obj.Restore();
	mdl_obj.Store();

	static	AHdr		hdr( pd3dDevice, g_SIZE_W/2, g_SIZE_H/2, FFORMAT );
 	hdr.SetFilterTonemap( 1.0 );


	pd3dDevice->BeginScene();
	{
		switch( alg )
		{
		case ALG_NONE:
			mdl_obj.DrawNormal( pd3dDevice, light, matView, matProj, prev_matView, matCam );
			break;

		case ALG_SIMON:
			mdl_obj.DrawSimonBlur( pd3dDevice, light, matView, matProj, prev_matView, matCam );

			break;

		case ALG_BLUR3D:
			mdl_obj.Draw3DBlur( pd3dDevice, light, matView, matProj, prev_matView, matCam );
		break;
			
		}		

		IDirect3DSurface9 *prim_pSurface = 0;
		pd3dDevice->GetRenderTarget( 0, &prim_pSurface );
		pd3dDevice->StretchRect( mdl_obj.pSurfaceA, 0,	prim_pSurface, 0,	D3DTEXF_LINEAR );


/* 	if ( key_IsTrigger( '1' ) ) 
 	{
		hdr.SetFlg( AHdr::TONEMAP, false );
		hdr.SetFlg( AHdr::BLOOM, false );
		hdr.SetFlg( AHdr::STREAK, false );
 	}
 	if ( key_IsTrigger( '3' ) ) hdr.SetFlg( AHdr::BLOOM,   !hdr.GetFlg( AHdr::BLOOM ) );
 	if ( key_IsTrigger( '4' ) ) hdr.SetFlg( AHdr::STREAK,  !hdr.GetFlg( AHdr::STREAK ) );
*/

 	if ( !key_IsToggle( '2' ) ) 
 	{
	 	hdr.SetFilterTonemap( 1.0 );
 	}
 	else
 	{
 		hdr.SetFilterTonemap( 0.0 );
 	}

 	if ( key_IsToggle( 'G' ) ) 
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
		fprintf( stderr, "  main.exe -large : 1024x1024x48div\n" );
		fprintf( stderr, "  main.exe -middle:  512x 512x32div (default)\n" );
		fprintf( stderr, "  main.exe -small :  256x 256x20div\n" );
//		fprintf( stderr, "  main.exe -file:filename.obj :  default:bunny.obj\n" );
		fprintf( stderr, "---------------\n" );
		fprintf( stderr, "param:\n" );
		fprintf( stderr, "  W=%d H=%d DIV=%d\n",g_SIZE_W, g_SIZE_H, g_BLUR_DIV);
		fprintf( stderr, "---------------\n" );
		fprintf( stderr, "   up / down : rotate x\n" );
		fprintf( stderr, " left / right: rotate y\n" );
		fprintf( stderr, "    Z / X    : rotate z\n" );
		fprintf( stderr, "    A / D    : move x axis\n" );
		fprintf( stderr, "    W / S    : move y axis\n" );
		fprintf( stderr, "    F / V    : move z axis\n" );
		fprintf( stderr, "      G      : swith grea filter\n" );
		fprintf( stderr, "      0      : blur NONE\n" );
		fprintf( stderr, "      9      : blur 2.5D\n" );
		fprintf( stderr, "      8      : blur 3D\n" );
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

	g_pModel = new ModelNanika( 
		g_pd3dDevice, 
		*new Model( 
			ShapeObj( g_pd3dDevice, "bunny.obj", "test.bmp",   0, 0, 0,			0,	pi,	0,		1.0, 1.0, 1.0,   1.0, 1.0, 1.0 ),   
			0, 	0,  0,			
			0,	0,	0,		
			1.0, 1.0, 1.0,   
			1.0, 1.0, 1.0 
		) 
	);

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


