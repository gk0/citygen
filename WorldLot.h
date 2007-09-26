#ifndef WORLDLOT_H
#define WORLDLOT_H

#include "stdafx.h"
#include "CellGenParams.h"
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
	std::vector<Ogre::Vector3>	_vertices;
	std::vector<Ogre::Vector2>	_texCoords;
	std::vector<size_t>			_sidePolys;		
	std::vector<size_t>			_roofPolys;

	Ogre::Real					_height;
	Ogre::Real					_foundation;
	std::vector<Ogre::Vector3>	_footprint;

public:
	//WorldLot(const LotBoundary &footprint, const CellGenParams &gp);
	WorldLot(const LotBoundary &footprint, const CellGenParams &gp, const Ogre::Real ht);

	bool hasError() { return _error; }

	static bool build(const LotBoundary &footprint, const CellGenParams &gp,
		const Ogre::Real fnd, const Ogre::Real ht, Ogre::ManualObject* m);

	bool build(Ogre::ManualObject* m);
	void build2(Ogre::ManualObject* m);

	static bool buildHousey(const std::vector<Ogre::Vector2> &footprint, 
		const Ogre::Real foundation, const Ogre::Real height, Ogre::ManualObject* m);

	static bool buildCube(const std::vector<Ogre::Vector2> &footprint, 
		const Ogre::Real foundation, const Ogre::Real height, Ogre::ManualObject* m);

	static LotBoundary insetBoundary(const LotBoundary &b, const Ogre::Real &roadInset, 
		const Ogre::Real &standardInset);

	void addVertexData(Ogre::ManualObject* m);

	Ogre::uint16 addIndexData(Ogre::ManualObject* m, Ogre::uint16 offset);

};

#endif
