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

	float	m_athx;
	float	m_athy;
	float	m_athz;
	float	m_rotY;
	float	m_rotX;
	float	m_rotZ;
	float	m_thx;
	float	m_thy;
	float	m_thz;

	float	m_tx;
	float	m_ty;
	float	m_tz;
	float	m_rx;
	float	m_ry;
	float	m_rz;
	float	m_sx;
	float	m_sy;
	float	m_sz;
	
public:
	void setCol( float _r, float _g, float _b );
	void movePos( float _x, float _y, float _z );
	void setPos( float _x, float _y, float _z );
	void setRot( float _x, float _y, float _z );
	void setScaling( float _x, float _y, float _z );
	Model( const Shape& shape
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
	void Restore();
	void Store();
	void UpdateMatrix( float val );
};
