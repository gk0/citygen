#ifndef CITYCELLMANAGER_H
#define CITYCELLMANAGER_H

#include "stdafx.h"
#include "RoadGraph.h"
#include "CityCell.h"

typedef std::vector< CityCell >::iterator CityCellIterator;

class CityCellManager {

private:
	std::vector< CityCell > mCityCells;

public:
	CityCellManager();
	void setCells(RoadGraph* parent, const PrimitiveVec& primitives);

	std::pair<const CityCellIterator, const CityCellIterator> getCells();
	//bool pickCell(const Ogre::Vector2& loc, Primitive& p);
	//bool pointInPolygon(const Ogre::Vector2& loc, const Primitive& p);
};

#endif
