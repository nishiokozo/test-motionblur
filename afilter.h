class	AFilter
{
	ID3DXEffect*						m_pEffect;
	IDirect3DVertexBuffer9* m_pVertexBuffer;
	IDirect3DIndexBuffer9*	m_pIndexBuffer;
	IDirect3DVertexDeclaration9*	m_pDeclaration;
	int	m_cntIndex;
	int	m_cntVertex;

	struct Vertex
	{
		float x, y, z;
	};

public:

	~AFilter();
	AFilter( IDirect3DDevice9*	pd3dDevice, const char* fnShader );

	void DrawFilter( IDirect3DDevice9*	pd3dDevice );
	void DrawBlend( IDirect3DDevice9*	pd3dDevice, IDirect3DTexture9 *pTexture0, IDirect3DTexture9 *pTexture1 );
	void DrawAdd( IDirect3DDevice9*	pd3dDevice, IDirect3DTexture9 *pTexture0, IDirect3DTexture9 *pTexture1, float range );

	void DrawStreach( IDirect3DDevice9*	pd3dDevice, IDirect3DTexture9 *pTexture0, float* dotpitch );
	void DrawGaussian( IDirect3DDevice9*	pd3dDevice, IDirect3DTexture9 *pTexture0, float s, int cntWeight, float* dotpitch );
	void DrawTonemap2( IDirect3DDevice9*	pd3dDevice, IDirect3DTexture9 *pTexture0, IDirect3DTexture9 *pTexture1, IDirect3DTexture9* tone_pTexture, float* dotpitch );
  void DrawTonemap3( IDirect3DDevice9*	pd3dDevice, IDirect3DTexture9 *pTexture0, IDirect3DTexture9 *pTexture1, IDirect3DTexture9 *pTexture2, IDirect3DTexture9* tone_pTexture, float rate1, float rate2, float rateTone, float* dotpitch );
  void DrawStreak( IDirect3DDevice9*	pd3dDevice, IDirect3DTexture9 *pTexture0, float attenuation, float* dir, int gpass, float rate, float* dotpitch );
  void DrawBlur( IDirect3DDevice9*	pd3dDevice, IDirect3DTexture9 *pTexture0, D3DXMATRIX& M, D3DXMATRIX& prev_M, float* dotpitch );

};
