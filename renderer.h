///////////////////////////////////////////////////////////////////////////////
class	Renderer
///////////////////////////////////////////////////////////////////////////////
{
public:
	ID3DXEffect*					m_pEffectPNTC;
	IDirect3DVertexDeclaration9*	m_pDeclarationPNTC;

	Renderer(
		   IDirect3DDevice9*	pd3dDevice
		 , const char* 			fnShader
	);
	void DrawModel( 
		IDirect3DDevice9*		pd3dDevice
		, Model& 				shape
		, Light 				light
		, D3DXMATRIX& 			matView
		, D3DXMATRIX& 			matProj 
		, D3DXMATRIX& 			prev_matView
		, D3DXMATRIX& 			matCam
		, IDirect3DTexture9*	pTexture 
	);
};
