/**************************************************************************
    Lightspark, a free flash player implementation

    Copyright (C) 2009-2013  Alessandro Pignotti (a.pignotti@sssup.it)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/
#ifndef BACKENDS_GEOMETRY_H
#define BACKENDS_GEOMETRY_H 1

#include "compat.h"
#include "swftypes.h"
#include <list>
#include <vector>
#include <map>

namespace lightspark
{

template<class T>
class Vector2Tmpl
{

public:
	T x,y;
	Vector2Tmpl(T a=0, T b=0):x(a),y(b){}
	/* conversion Vector2 -> Vector2f is implicit */
	Vector2Tmpl(const Vector2Tmpl<int32_t>& o) : x(o.x),y(o.y) {}
	/* conversion Vector2f -> Vector2 is explicit */
	Vector2Tmpl<int32_t> round() const { return Vector2Tmpl<int32_t>(x,y); }
	bool operator==(const Vector2Tmpl<T>& v)const{return v.x==x && v.y==y;}
	bool operator!=(const Vector2Tmpl<T>& v)const{return v.x!=x || v.y!=y;}
	bool operator<(const Vector2Tmpl<T>& v) const {return (y==v.y)?(x < v.x):(y < v.y);}
	Vector2Tmpl<T> operator-() const { return Vector2Tmpl<T>(-x,-y); }
	Vector2Tmpl<T> operator-(const Vector2Tmpl<T>& v)const { return Vector2Tmpl<T>(x-v.x,y-v.y);}
	Vector2Tmpl<T> operator+(const Vector2Tmpl<T>& v)const { return Vector2Tmpl<T>(x+v.x,y+v.y);}
	Vector2Tmpl<T>& operator+=(const Vector2Tmpl<T>& v){ x+=v.x; y+=v.y; return *this;}
	Vector2Tmpl<T> operator*(int p)const { return Vector2Tmpl<T>(x*p,y*p);}
	Vector2Tmpl<T>& operator/=(T v) { x/=v; y/=v; return *this;}
	int dot(const Vector2Tmpl<T>& r) const { return x*r.x+y*r.y;}
	Vector2Tmpl<T> projectInto(const RECT& r) const
	{
		Vector2Tmpl<T> out;
		out.x = maxTmpl<T>(minTmpl<T>(x,r.Xmax),r.Xmin);
		out.y = maxTmpl<T>(minTmpl<T>(y,r.Ymax),r.Ymin);
		return out;
	}
	std::ostream& operator<<(std::ostream& s)
	{
		s << "{ "<< x << ',' << y << " }";
		return s;
	}
};

enum GEOM_TOKEN_TYPE { STRAIGHT=0, CURVE_QUADRATIC, MOVE, SET_FILL, SET_STROKE, CLEAR_FILL, CLEAR_STROKE, CURVE_CUBIC, FILL_KEEP_SOURCE, FILL_TRANSFORM_TEXTURE };

class GeomToken :public RefCountable
{
public:
	FILLSTYLE  fillStyle;
	LINESTYLE2 lineStyle;
	MATRIX textureTransform;
	GEOM_TOKEN_TYPE type;
	Vector2 p1;
	Vector2 p2;
	Vector2 p3;
	GeomToken(GEOM_TOKEN_TYPE _t):fillStyle(0xff),lineStyle(0xff),type(_t),p1(0,0),p2(0,0),p3(0,0){}
	GeomToken(GEOM_TOKEN_TYPE _t, const Vector2& _p):fillStyle(0xff),lineStyle(0xff),type(_t),p1(_p),p2(0,0),p3(0,0){}
	GeomToken(GEOM_TOKEN_TYPE _t, const Vector2& _p1, const Vector2& _p2):fillStyle(0xff),lineStyle(0xff),type(_t),p1(_p1),p2(_p2),p3(0,0){}
	GeomToken(GEOM_TOKEN_TYPE _t, const Vector2& _p1, const Vector2& _p2, const Vector2& _p3):fillStyle(0xff),lineStyle(0xff),type(_t),
			p1(_p1),p2(_p2),p3(_p3){}

	// construct from int vector encoded as uint64_t (low 32 bit = x, high 32 bit = y)
	GeomToken(GEOM_TOKEN_TYPE _t, uint64_t _p1):fillStyle(0xff),lineStyle(0xff),type(_t),
		p1(int32_t(_p1&0xffffffff),int32_t(_p1>>32)),p2(0,0),p3(0,0){}
	GeomToken(GEOM_TOKEN_TYPE _t, uint64_t _p1, uint64_t _p2):fillStyle(0xff),lineStyle(0xff),type(_t),
		p1(int32_t(_p1&0xffffffff),int32_t(_p1>>32)),p2(int32_t(_p2&0xffffffff),int32_t(_p2>>32)),p3(0,0){}
	GeomToken(GEOM_TOKEN_TYPE _t, uint64_t _p1, uint64_t _p2, uint64_t _p3):fillStyle(0xff),lineStyle(0xff),type(_t),
		p1(int32_t(_p1&0xffffffff),int32_t(_p1>>32)),p2(int32_t(_p2&0xffffffff),int32_t(_p2>>32)),p3(int32_t(_p3&0xffffffff),int32_t(_p3>>32)){}

	GeomToken(GEOM_TOKEN_TYPE _t, const FILLSTYLE&  _f):fillStyle(_f),lineStyle(0xff),type(_t),p1(0,0),p2(0,0),p3(0,0){}
	GeomToken(GEOM_TOKEN_TYPE _t, const LINESTYLE2& _s):fillStyle(0xff),lineStyle(_s),type(_t),p1(0,0),p2(0,0),p3(0,0){}
	GeomToken(GEOM_TOKEN_TYPE _t, const MATRIX _m):fillStyle(0xff),lineStyle(0xff),textureTransform(_m),type(_t),p1(0,0),p2(0,0),p3(0,0){}
	GeomToken(GEOM_TOKEN_TYPE _t, const MORPHLINESTYLE2& _s);
};

struct GeomToken2
{
	union
	{
		GEOM_TOKEN_TYPE type;
		struct
		{
			int32_t x;
			int32_t y;
		} vec;
		const FILLSTYLE*  fillStyle; // make sure the pointer is valid until rendering is done
		const LINESTYLE2* lineStyle; // make sure the pointer is valid until rendering is done
		const MORPHLINESTYLE2* morphlineStyle; // make sure the pointer is valid until rendering is done
		number_t value;
		uint64_t uval;// this is used to have direct access to the value as it is stored in a vector<uint64_t> for performance
	};
	GeomToken2(GEOM_TOKEN_TYPE t):type(t) {}
	GeomToken2(uint64_t v, bool isvec)
	{
		if (isvec)
		{
			vec.x = (int32_t(v&0xffffffff));
			vec.y = (int32_t(v>>32));
		}
		else
			uval = v;
	}
	GeomToken2(const FILLSTYLE& fs):fillStyle(&fs) {}
	GeomToken2(const LINESTYLE2& ls):lineStyle(&ls) {}
	GeomToken2(number_t val):value(val) {}
	GeomToken2(const MORPHLINESTYLE2& mls):morphlineStyle(&mls) {}
	GeomToken2(const Vector2& _vec)
	{
		vec.x=_vec.x;
		vec.y=_vec.y;
	}
};

struct tokensVector
{
	std::vector<_NR<GeomToken>, reporter_allocator<_NR<GeomToken>>> filltokens;
	std::vector<_NR<GeomToken>, reporter_allocator<_NR<GeomToken>>> stroketokens;
	std::vector<uint64_t> filltokens2;
	std::vector<uint64_t> stroketokens2;
	tokensVector(reporter_allocator<_NR<GeomToken>> m):filltokens(m),stroketokens(m) {}
	void clear()
	{
		filltokens.clear();
		stroketokens.clear();
		filltokens2.clear();
		stroketokens2.clear();
	}
	uint32_t size() const
	{
		return filltokens.size()+stroketokens.size()+filltokens2.size()+stroketokens2.size();
	}
	bool empty() const
	{
		return filltokens.empty() && stroketokens.empty() && filltokens2.empty() && stroketokens2.empty();
	}
};

enum SHAPE_PATH_SEGMENT_TYPE { PATH_START=0, PATH_STRAIGHT, PATH_CURVE_QUADRATIC };

class ShapePathSegment {
public:
	SHAPE_PATH_SEGMENT_TYPE type;
	uint64_t i;
	ShapePathSegment(SHAPE_PATH_SEGMENT_TYPE _t, uint64_t _i):type(_t),i(_i){}
	bool operator==(const ShapePathSegment& v)const{return v.i == i;}
};

class ShapesBuilder
{
private:
	void joinOutlines();
	static bool isOutlineEmpty(const std::vector<ShapePathSegment>& outline);
	static void extendOutlineForColor(std::map< unsigned int, std::vector< std::vector<Vector2> > >& map);
	inline uint64_t makeVertex(const Vector2& v) const { return (uint64_t(v.y)<<32) | (uint64_t(v.x)&0xffffffff); }
public:
	std::map< unsigned int, std::vector< std::vector<ShapePathSegment> > > filledShapesMap;
	std::map< unsigned int, std::vector< std::vector<ShapePathSegment> > > strokeShapesMap;
	void extendFilledOutlineForColor(std::vector<std::vector<ShapePathSegment> >* outlinesForColor, const Vector2& v1, const Vector2& v2);
	void extendFilledOutlineForColorCurve(std::vector<std::vector<ShapePathSegment> >* outlinesForColor, const Vector2& start, const Vector2& control, const Vector2& end);
	void extendStrokeOutline(std::vector<std::vector<ShapePathSegment> >* outlinesForStroke, const Vector2& v1, const Vector2& v2);
	void extendStrokeOutlineCurve(std::vector<std::vector<ShapePathSegment> >* outlinesForStroke, const Vector2& v1, const Vector2& v2, const Vector2& v3);
	/**
		Generate a sequence of cachable tokens that defines the geomtries
		@param styles This list is supposed to survive until as long as the returned tokens array
		@param tokens A vector that will be filled with tokens
	*/
	void outputTokens(const std::list<FILLSTYLE>& styles, const std::list<LINESTYLE2>& linestyles, tokensVector& tokens);
	void outputTokens2(const std::list<FILLSTYLE>& styles, const std::list<LINESTYLE2>& linestyles, tokensVector& tokens);
	void outputMorphTokens(const std::list<MORPHFILLSTYLE>& styles, const std::list<MORPHLINESTYLE2> &linestyles, tokensVector& tokens, uint16_t ratio);
	void clear();
};

std::ostream& operator<<(std::ostream& s, const Vector2& p);

}

#endif /* BACKENDS_GEOMETRY_H */
