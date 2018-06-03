#ifndef WORLDBLOCK_H
#define WORLDBLOCK_H

#include "stdafx.h"
#include "CellParams.h"
#include <OgreVector3.h>
#include <vector>
#include <list>
#include "Region.h"

//class Region;
class WorldLot;
class MeshBuilder;

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
};

#endif
