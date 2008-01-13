#ifndef WORLDLOT_H
#define WORLDLOT_H

#include "stdafx.h"
#include "CellParams.h"
#include "Triangulate.h"

struct LotBoundaryPoint
{
	bool			_roadAccess;
	Ogre::Vector3	_pos;

	LotBoundaryPoint(const bool ra, const Ogre::Vector3 &p)
		: _roadAccess(ra), _pos(p) {}
};
typedef std::vector<LotBoundaryPoint> LotBoundary;


class WorldLot
{
private:
	bool						_error;

	Ogre::Real					_height;
	Ogre::Real					_foundation;

	std::vector<Ogre::Real>		_vertexData;
	std::vector<Ogre::uint16>	_indexData;

public:
	//WorldLot(const LotBoundary &footprint, const CellGenParams &gp);
	WorldLot(const LotBoundary &footprint, const CellParams &gp, const Ogre::Real ht, 
		const Ogre::Real uScale, const Ogre::Real vScale);

	bool hasError() { return _error; }

	static LotBoundary insetBoundary(const LotBoundary &b, const Ogre::Real &roadInset, 
		const Ogre::Real &standardInset);

	const std::vector<Ogre::Real>& getVertexData() { return _vertexData; }
	const std::vector<Ogre::uint16>& getIndexData() { return _indexData; }
	//void installVertexData(float*& vPtr);
	//void installIndexData(Ogre::uint16*& iPtr, size_t& voffset);

private:
	inline void appendVector3(std::vector<Ogre::Real> &vertexData, const Ogre::Vector3& v)
	{
		vertexData.push_back(v.x);
		vertexData.push_back(v.y);
		vertexData.push_back(v.z);
	}

	inline void appendVector3(std::vector<Ogre::Real> &vertexData, 
		const Ogre::Real &x, const Ogre::Real &y, const Ogre::Real &z)
	{
		vertexData.push_back(x);
		vertexData.push_back(y);
		vertexData.push_back(z);
	}

	inline void appendVector2(std::vector<Ogre::Real> &vertexData, 
		const Ogre::Real &x, const Ogre::Real &y)
	{
		vertexData.push_back(x);
		vertexData.push_back(y);
	}

	inline void appendPoly(std::vector<Ogre::uint16> &indexData, 
		const Ogre::uint16 &a, const Ogre::uint16 &b, const Ogre::uint16 &c)
	{
		indexData.push_back(a);
		indexData.push_back(b);
		indexData.push_back(c);
	}

};

#endif
