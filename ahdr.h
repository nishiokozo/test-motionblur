
class AHdr
{
//	static	const float	pi = 3.141592;

	static	const int	TONE_MAX = 11;
	int									m_tone_cnt;
	IDirect3DTexture9*	m_tone_pTexture[TONE_MAX];
	IDirect3DSurface9*	m_tone_pSurface[TONE_MAX];
  float m_tone_rate;
//	float m_tone_dotofset[2];

  float m_bloom_rate;
  float m_bloom_s;
  int   m_bloom_cntWeight;

  float m_streak_rate;
  int   m_streak_cntArms;
  int   m_streak_cntPass;
  float m_streak_valAttenuation;
  float m_streak_radSlope;



//	float	m_dotpitch_w;
//	float	m_dotpitch_h;
	float m_dotpitch[2];

	AFilter*	m_pFilterGaussian_h;
	AFilter*	m_pFilterGaussian_v;
	AFilter*	m_pFilterStreach_hdr;
	AFilter*	m_pFilterTonemap2;
	AFilter*	m_pFilterStreak;
	AFilter*	m_pFilterBlend;
	AFilter*	m_pFilterStreach_linear;
	AFilter*	m_pFilterTonemap3;

	IDirect3DTexture9 *m_hdr_pTexture;
	IDirect3DSurface9 *m_hdr_pSurface;

	IDirect3DTexture9 *m_bloom_pTexture;
	IDirect3DSurface9 *m_bloom_pSurface;

	IDirect3DTexture9 *m_streak_pTexture;
	IDirect3DSurface9 *m_streak_pSurface;

	IDirect3DTexture9 *m_pTexture0;
	IDirect3DSurface9 *m_pSurface0;

	IDirect3DTexture9 *m_pTexture1;
	IDirect3DSurface9 *m_pSurface1;

	IDirect3DTexture9 *m_pTexture2;
	IDirect3DSurface9 *m_pSurface2;

	IDirect3DTexture9 *m_pTexture3;
	IDirect3DSurface9 *m_pSurface3;

	IDirect3DTexture9 *m_pTexture4;
	IDirect3DSurface9 *m_pSurface4;


//	IDirect3DTexture9 *m_Streak_pTexture;

public:
  enum FILTER
   {
    TONEMAP,
    BLOOM,
    STREAK,
    MAX
  };
  bool m_flgActive[MAX];


	~AHdr();
	AHdr( IDirect3DDevice9*	pd3dDevice, int width, int height, D3DFORMAT format );
//	void	GenerateBloomTexture( IDirect3DDevice9*	pd3dDevice, IDirect3DTexture9** dst_ppTexture, IDirect3DSurface9** dst_ppSurface, IDirect3DTexture9 *src_pTexture, float s, int cntWeight );
  void	GenerateBloomTexture( IDirect3DDevice9*	pd3dDevice, IDirect3DSurface9* dst_pSurface, IDirect3DTexture9 *src_pTexture, float s, int cntWeight );
	void	GenerateStreakTexture( IDirect3DDevice9*	pd3dDevice, IDirect3DSurface9* dst_pSurface, IDirect3DTexture9 *src_pTexture, int		cntArms,	int		cntPass,	float	valAttenuation,	float	radSlope );
  void	DrawFilter( IDirect3DDevice9*	pd3dDevice, IDirect3DSurface9 *dst_pSurface, IDirect3DTexture9 *src_pTexture, IDirect3DTexture9 *src_pTextureEffect );

  void	GenerateHdrTexture( IDirect3DDevice9*	pd3dDevice, IDirect3DSurface9* dst_pSurface, IDirect3DTexture9 *src_pTexture );
  void	GenerateToneTexture( IDirect3DDevice9*	pd3dDevice, IDirect3DTexture9 *src_pTexture );

  void	SetFilterTonemap( float rate );
  void	SetFilterBloom( float rate, float s, int cntWeight );
  void	SetFilterStreak( float rate, int cntArms,	int cntPass, float valAttenuation, float radSlope );
  void  DrawFilter( IDirect3DDevice9*	pd3dDevice, IDirect3DSurface9* dst_pSurface, IDirect3DTexture9* src_pTexture );

  bool  GetFlg( FILTER n );
  void  SetFlg( FILTER n, bool flg );

	
};
