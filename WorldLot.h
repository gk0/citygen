//#ifdef WORLDLOT_H
//#define WORLDLOT_H

#include "stdafx.h"
#include "WorldCell.h"

struct LotBoundaryPoint
{
	bool _roadAccess;
	Ogre::Vector2 _pos;
	LotBoundaryPoint(const bool ra, const Ogre::Vector2 &p)
		: _roadAccess(ra), _pos(p) {}
};
typedef std::vector<LotBoundaryPoint> LotBoundary;

class WorldLot
{
private:

public:
	static bool build(const LotBoundary &footprint, const GrowthGenParams &gp,
		const Ogre::Real fnd, const Ogre::Real ht, Ogre::ManualObject* m);

	static bool buildHousey(const std::vector<Ogre::Vector2> &footprint, 
		const Ogre::Real foundation, const Ogre::Real height, Ogre::ManualObject* m);

	static bool buildCube(const std::vector<Ogre::Vector2> &footprint, 
		const Ogre::Real foundation, const Ogre::Real height, Ogre::ManualObject* m);

	static LotBoundary insetBoundary(const LotBoundary &b, const Ogre::Real &roadInset, 
		const Ogre::Real &standardInset);
};

//#endif
