#define WIN32_LEAN_AND_MEAN


#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "light.h"

Light::Light( float x, float y, float z, float r, float g, float b)
{
	memset((this),0,sizeof(*this));

	loc.x = x;
	loc.y = y;
	loc.z = z;
	col.r = r;
	col.g = g;
	col.b = b;

}
