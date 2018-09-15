///////////////////////////////////////////////////////////////////////////////
class ShapeObj  : public Shape
///////////////////////////////////////////////////////////////////////////////
{
	fpos_t	m_size;
	char*		m_pBuf;
	int			m_i;
	int			m_w;
	char		m_word[256];

	void	text_Remove();
	void	text_LoadFile( const char* fn );
	void	text_Init();
	bool	text_GetWord();

public:
	ShapeObj(
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
	);
	
};
