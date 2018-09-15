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

	IDirect3DTexture9*		m_pTexture0;
	IDirect3DVertexBuffer9* m_pVertexBuffer;
	IDirect3DIndexBuffer9*	m_pIndexBuffer;
	D3DPRIMITIVETYPE		m_type;
	int						m_cntIndex;
	int						m_cntVertex;
	int						m_sizeOfVertex;
	int						m_cntFace;

};
