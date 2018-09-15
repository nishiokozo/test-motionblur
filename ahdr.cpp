#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <stdio.h>

#include "afilter.h"
#include "ahdr.h"
extern  int	temp_IsTrigger( int val );
extern  int	temp_IsToggle( int val );
extern  int	temp_IsPress( int val );


//-----------------------------------------------------------------------------
AHdr::AHdr( IDirect3DDevice9*	pd3dDevice, int width, int height, D3DFORMAT format )
//-----------------------------------------------------------------------------
{
	memset( this, 0, sizeof(*this) );

	m_dotpitch[0] = 1.0f / width;
	m_dotpitch[1] = 1.0f / height;

	pd3dDevice->CreateTexture( width, height, 1, D3DUSAGE_RENDERTARGET, format, D3DPOOL_DEFAULT, &m_hdr_pTexture, 0 );
	m_hdr_pTexture->GetSurfaceLevel( 0, &m_hdr_pSurface );

	pd3dDevice->CreateTexture( width, height, 1, D3DUSAGE_RENDERTARGET, format, D3DPOOL_DEFAULT, &m_bloom_pTexture, 0 );
	m_bloom_pTexture->GetSurfaceLevel( 0, &m_bloom_pSurface );

	pd3dDevice->CreateTexture( width, height, 1, D3DUSAGE_RENDERTARGET, format, D3DPOOL_DEFAULT, &m_streak_pTexture, 0 );
	m_streak_pTexture->GetSurfaceLevel( 0, &m_streak_pSurface );

	pd3dDevice->CreateTexture( width, height, 1, D3DUSAGE_RENDERTARGET, format, D3DPOOL_DEFAULT, &m_pTexture0, 0 );
	m_pTexture0->GetSurfaceLevel( 0, &m_pSurface0 );

	pd3dDevice->CreateTexture( width, height, 1, D3DUSAGE_RENDERTARGET, format, D3DPOOL_DEFAULT, &m_pTexture1, 0 );
	m_pTexture1->GetSurfaceLevel( 0, &m_pSurface1 );

	pd3dDevice->CreateTexture( width, height, 1, D3DUSAGE_RENDERTARGET, format, D3DPOOL_DEFAULT, &m_pTexture2, 0 );
	m_pTexture2->GetSurfaceLevel( 0, &m_pSurface2 );

	pd3dDevice->CreateTexture( width, height, 1, D3DUSAGE_RENDERTARGET, format, D3DPOOL_DEFAULT, &m_pTexture3, 0 );
	m_pTexture3->GetSurfaceLevel( 0, &m_pSurface3 );

	pd3dDevice->CreateTexture( width, height, 1, D3DUSAGE_RENDERTARGET, format, D3DPOOL_DEFAULT, &m_pTexture4, 0 );
	m_pTexture4->GetSurfaceLevel( 0, &m_pSurface4 );


	{
		int	s = fmax(width,height);
		int	i = 0;
		for ( i = 0 ; i < TONE_MAX ; i++ )
		{
			if ( pow(2,i) > s )
			{
				m_tone_cnt = i;
				break;
			}
		}
	}
fprintf(stderr,"m_tone_cnt %d\n",m_tone_cnt);

	for ( int i = 0 ; i < m_tone_cnt ; i++ )
	{
		int	size = (int)pow(2,i);
		pd3dDevice->CreateTexture( size, size, 1, D3DUSAGE_RENDERTARGET, format, D3DPOOL_DEFAULT, &m_tone_pTexture[i], 0 );
		m_tone_pTexture[i]->GetSurfaceLevel( 0, &m_tone_pSurface[i] );
	}




	m_pFilterStreach_hdr 	= new AFilter( pd3dDevice,	"f_streach_hdr.fx" );
	m_pFilterStreach_linear = new AFilter( pd3dDevice, "f_streach_linear.fx" );
	m_pFilterGaussian_h 	= new AFilter( pd3dDevice, "f_gaussian_h.fx" );
	m_pFilterGaussian_v 	= new AFilter( pd3dDevice, "f_gaussian_v.fx" );
	m_pFilterTonemap2			= new AFilter( pd3dDevice, "f_tonemap2.fx" );
	m_pFilterStreak				= new AFilter( pd3dDevice, "f_streak.fx" );
	m_pFilterBlend				= new AFilter( pd3dDevice, "f_blend.fx" );
	m_pFilterTonemap3	  	= new AFilter( pd3dDevice, "f_tonemap3.fx" );

#if 0
  m_tone_rate  = 1.0f;;
  m_bloom_rate = 1.0;
  m_streak_rate = 1.0 ;
#else
  m_tone_rate  = 0.5f;
  m_bloom_rate = 0.8;
  m_streak_rate = 0.2 ;
#endif

  m_bloom_s = 8;
  m_bloom_cntWeight = 15;

  m_streak_cntArms = 4 ;
  m_streak_cntPass = 3;
  m_streak_valAttenuation = 0.95f;

 			float	pi = 3.141592f;

  m_streak_radSlope =  (60)*pi/180.0f;

  m_flgActive[TONEMAP] = false;
  m_flgActive[BLOOM] = false;
  m_flgActive[STREAK] = true;



}

//-----------------------------------------------------------------------------
AHdr::~AHdr()
//-----------------------------------------------------------------------------
{
	if ( m_pFilterGaussian_h ) delete m_pFilterGaussian_h;
	if ( m_pFilterGaussian_v ) delete m_pFilterGaussian_v;
	if ( m_pFilterStreach_hdr ) delete m_pFilterStreach_hdr;
	if ( m_pFilterStreach_linear ) delete m_pFilterStreach_linear;
	if ( m_pFilterStreak ) delete m_pFilterStreak;
	if ( m_pFilterTonemap3 ) delete m_pFilterTonemap3;
}


//-----------------------------------------------------------------------------
static void	Swap( int& a, int& b )
//-----------------------------------------------------------------------------
{
	int c = a;		a = b;		b = c;	// swap a & b
}

//-----------------------------------------------------------------------------
void	AHdr::GenerateToneTexture( IDirect3DDevice9*	pd3dDevice, IDirect3DTexture9 *src_pTexture )
//-----------------------------------------------------------------------------
{
  pd3dDevice->SetRenderTarget( 0, m_tone_pSurface[m_tone_cnt-1] );
  m_pFilterStreach_linear->DrawStreach( pd3dDevice, src_pTexture, m_dotpitch );

   float tbl[2] = {m_dotpitch[0],m_dotpitch[1]};
	for ( int i = m_tone_cnt-1 ; i > 0  ; i-- )
	{
    tbl[0] *= 2;
    tbl[1] *= 2;
    pd3dDevice->SetRenderTarget( 0, m_tone_pSurface[i-1] );
    m_pFilterStreach_linear->DrawStreach( pd3dDevice, m_tone_pTexture[i], tbl );
	}

}

//-----------------------------------------------------------------------------
void	AHdr::GenerateStreakTexture( IDirect3DDevice9*	pd3dDevice, IDirect3DSurface9* dst_pSurface, IDirect3DTexture9 *src_pTexture, int		cntArms,	int		cntPass,	float	valAttenuation,	float	radSlope )
//-----------------------------------------------------------------------------
{

	struct TARGET
	{
		IDirect3DSurface9*	pSurface;
		IDirect3DTexture9*	pTexture;
	};
	
	TARGET ab[2] = 
	{
		{m_pSurface2,m_pTexture2},
		{m_pSurface3,m_pTexture3},
	};

	TARGET cd[2] = 
	{
		{m_pSurface4,m_pTexture4},
		{m_pSurface1,m_pTexture1},
	};

	int	a = 0;
	int	b = 1;
	int	c = 0;
	int	d = 1;




	for ( int n = 0 ; n < cntArms ; n++ )
	{
 			float	pi = 3.141592f;
	
		float	th = (float)n/(float)(cntArms) * 2*pi + radSlope;

		float	dir[] = 
		{
			m_dotpitch[0] * cos(th)/2,
			m_dotpitch[1] * sin(th)/2
		};
	
		{
			pd3dDevice->SetRenderTarget( 0, ab[a].pSurface );
			m_pFilterStreak->DrawStreak( pd3dDevice, src_pTexture, valAttenuation,  dir, 1, 1.0/cntArms, m_dotpitch );
			Swap( a, b );
		}

		for ( int p = 1 ; p < cntPass ; p++ )
		{
			pd3dDevice->SetRenderTarget( 0, ab[a].pSurface );
			m_pFilterStreak->DrawStreak( pd3dDevice, ab[b].pTexture, valAttenuation,  dir, p+1, 1.0, m_dotpitch );
			Swap( a, b );
		}

		{
      if ( n == cntArms-1 )
			  pd3dDevice->SetRenderTarget( 0, dst_pSurface );
      else
			  pd3dDevice->SetRenderTarget( 0, cd[c].pSurface );
      
			if ( n == 0 )
				m_pFilterBlend->DrawBlend( pd3dDevice, 0             , ab[b].pTexture );
			else
				m_pFilterBlend->DrawBlend( pd3dDevice, cd[d].pTexture, ab[b].pTexture );
			Swap( c, d );
		}

	}


}

//-----------------------------------------------------------------------------
void	AHdr::GenerateHdrTexture( IDirect3DDevice9*	pd3dDevice, IDirect3DSurface9* dst_pSurface, IDirect3DTexture9 *src_pTexture )
//-----------------------------------------------------------------------------
{
	pd3dDevice->SetRenderTarget( 0, dst_pSurface );
	m_pFilterStreach_hdr->DrawStreach( pd3dDevice, src_pTexture, m_dotpitch );
}

//-----------------------------------------------------------------------------
void	AHdr::GenerateBloomTexture( IDirect3DDevice9*	pd3dDevice, IDirect3DSurface9* dst_pSurface, IDirect3DTexture9 *src_pTexture, float s, int cntWeight )
//-----------------------------------------------------------------------------
{
	pd3dDevice->SetRenderTarget( 0, m_pSurface1 );
	m_pFilterGaussian_h->DrawGaussian( pd3dDevice, src_pTexture, s, cntWeight, m_dotpitch );

	pd3dDevice->SetRenderTarget( 0, dst_pSurface );
	m_pFilterGaussian_v->DrawGaussian( pd3dDevice, m_pTexture1, s, cntWeight, m_dotpitch );

}

//-----------------------------------------------------------------------------
void	AHdr::DrawFilter( IDirect3DDevice9*	pd3dDevice, IDirect3DSurface9 *dst_pSurface, IDirect3DTexture9 *src_pTexture, IDirect3DTexture9 *src_pTextureEffect )
//-----------------------------------------------------------------------------
{
	pd3dDevice->SetRenderTarget( 0, dst_pSurface );

	m_pFilterBlend->DrawBlend( pd3dDevice, src_pTexture, src_pTextureEffect );

}

//-----------------------------------------------------------------------------
void	AHdr::SetFilterTonemap( float rate )
//-----------------------------------------------------------------------------
{
  m_tone_rate = rate;
}

//-----------------------------------------------------------------------------
void	AHdr::SetFilterBloom( float rate, float s, int cntWeight )
//-----------------------------------------------------------------------------
{
  m_bloom_rate = rate;
  m_bloom_s = s;
  m_bloom_cntWeight = cntWeight;
}

//-----------------------------------------------------------------------------
void	AHdr::SetFilterStreak( float rate, int cntArms,	int cntPass, float valAttenuation, float radSlope )
//-----------------------------------------------------------------------------
{
  m_streak_rate = rate;
  m_streak_cntArms = cntArms;
  m_streak_cntPass = cntPass;
  m_streak_valAttenuation = valAttenuation;
  m_streak_radSlope = radSlope;
}

//-----------------------------------------------------------------------------
bool  AHdr::GetFlg( FILTER n )
//-----------------------------------------------------------------------------
{
  return  m_flgActive[n];
}

//-----------------------------------------------------------------------------
void  AHdr::SetFlg( FILTER n, bool flg )
//-----------------------------------------------------------------------------
{
  m_flgActive[n] = flg;
}

//-----------------------------------------------------------------------------
void  AHdr::DrawFilter( IDirect3DDevice9*	pd3dDevice, IDirect3DSurface9* dst_pSurface, IDirect3DTexture9* src_pTexture )
//-----------------------------------------------------------------------------
{
  GenerateToneTexture( pd3dDevice, src_pTexture );

  GenerateHdrTexture( pd3dDevice, m_hdr_pSurface, src_pTexture );

	GenerateBloomTexture( pd3dDevice, m_bloom_pSurface, m_hdr_pTexture, m_bloom_s, m_bloom_cntWeight );

	GenerateStreakTexture( pd3dDevice, m_streak_pSurface, m_hdr_pTexture, m_streak_cntArms, m_streak_cntPass, m_streak_valAttenuation, m_streak_radSlope );

	pd3dDevice->SetRenderTarget( 0, dst_pSurface );
	m_pFilterTonemap3->DrawTonemap3( pd3dDevice, src_pTexture, m_bloom_pTexture, m_streak_pTexture, m_tone_pTexture[0],m_bloom_rate, m_streak_rate, m_tone_rate, m_dotpitch );


//	m_pFilterBlend->DrawBlend( pd3dDevice, src_pTexture, m_bloom_pTexture );

}
  