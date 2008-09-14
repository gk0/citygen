#ifndef WORLDBLOCK_H
#define WORLDBLOCK_H

#include "stdafx.h"
#include "CellParams.h"
#include <OgreVector3.h>
#include <vector>
#include <list>
#include "Region.h"
#include "WorldLot.h"

//class Region;
class WorldLot;
class MeshBuilder;

struct LotBoundaryPoint
{
	bool			_roadAccess;
	Ogre::Vector3	_pos;

	LotBoundaryPoint(const bool ra, const Ogre::Vector3 &p)
		: _roadAccess(ra), _pos(p) {}
};
typedef std::vector<LotBoundaryPoint> LotBoundary;


class WorldBlock
{
private:

	std::vector<WorldLot*>						_lots;
	std::vector< std::vector<Ogre::Vector3> >	_debugLots;
	std::vector<Ogre::uint16>					_pavementPolys;
	std::vector<Ogre::Real>						_pavementVertexData;

public:
	bool	_error;

	WorldBlock(const std::vector<Ogre::Vector3> &boundary, const CellParams &gp, rando rg,
		MeshBuilder* mb, bool debug=false);
	virtual ~WorldBlock();

   void drawDebug(Ogre::ManualObject* debugMO);

private:
	std::list<citygen::Region*> subdivide(citygen::Region* region, const CellParams &params, rando rg);
	void addDebugLot(citygen::Region* r);

	bool slowSplitBoundary(const size_t &index, const Ogre::Real &deviance, rando rg, const LotBoundary &input, std::vector<LotBoundary> &output);
	
	std::vector< LotBoundary > slowSubdivide(std::vector<Ogre::Vector3> innerBoundary, const CellParams &params, rando rg);

	bool getLongestBoundarySideIndex(const LotBoundary &b, const Ogre::Real limitSq, size_t &index);
	bool getLongestNonBoundarySideIndex(const LotBoundary &b, const Ogre::Real limitSq, size_t &index);
};

#endif
