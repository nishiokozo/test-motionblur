#define WIN32_LEAN_AND_MEAN


#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "shape.h"
#include "model.h"

#define	_line		fprintf( stderr,"%s(%d) in %s\n", __func__, __LINE__, __FILE__ );


void Model::setCol( float _r, float _g, float _b )
{
	m_r = _r;
	m_g = _g;
	m_b = _b;
}
void Model::movePos( float _x, float _y, float _z )
{
	m_tx += _x;
	m_ty += _y;
	m_tz += _z;
}
void Model::setPos( float _x, float _y, float _z )
{
	m_tx = _x;
	m_ty = _y;
	m_tz = _z;
}
void Model::setRot( float _x, float _y, float _z )
{
	m_rx = _x;
	m_ry = _y;
	m_rz = _z;
}
void Model::setScaling( float _x, float _y, float _z )
{
	m_sx = _x;
	m_sy = _y;
	m_sz = _z;
}

//-----------------------------------------------------------------------------
Model::Model( const Shape& shape
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
void Model::Restore()
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
void Model::Store()
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
void Model::UpdateMatrix( float val )
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
