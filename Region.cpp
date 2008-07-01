#include "stdafx.h"
#include "Region.h"
#include <OgreLogManager.h>
#include <OgreStringConverter.h>
#include <vector>

using namespace Ogre;
using namespace citygen;


Region::Region(DirectedEdge* anchor, size_t numOfEdges)
{
	_anchor = anchor;
	_numOfEdges = numOfEdges;
	assert(numOfEdges >= 3);
}
/*
Region::Region(std::list<Ogre::Vector3> points, bool exterior)
{
	// get iterator
	std::list<Ogre::Vector3>::const_iterator pIt = points.begin(), pEnd = points.end();

	// assign first
	DirectedEdge* last = _anchor = new DirectedEdge(*pIt);
	last->_exterior = exterior;
	pIt++;
	for(; pIt != pEnd; pIt++)
	{
		last = new DirectedEdge(*pIt, last);
		last->_exterior = exterior;
	}
	DirectedEdge::closeCycle(last, _anchor);
	_numOfEdges = points.size();
}
*/
Region::Region(std::vector<Ogre::Vector3> points, bool exterior)
{
	//void* memPtr = malloc(points.size()* sizeof(DirectedEdge));
	//DirectedEdge* edges = static_cast<DirectedEdge*>(memPtr);

	// get iterator
	std::vector<Ogre::Vector3>::const_iterator pIt = points.begin(), pEnd = points.end();

	// assign first
	DirectedEdge* last = (_anchor = new DirectedEdge(*pIt));
	last->_exterior = exterior;
	pIt++;
	for(; pIt != pEnd; pIt++)
	{
		last = new DirectedEdge(*pIt, last);
		last->_exterior = exterior;
	}
	DirectedEdge::closeCycle(last, _anchor);
	_numOfEdges = points.size();
}

Region::~Region(void)
{
	if(_anchor != 0)
	{
		DirectedEdge* curr = _anchor->next();
		do
		{
			// !! error
			curr = curr->next();
			delete curr->prev();
		}
		while (curr != _anchor);
	}
}

void Region::unlink()
{
	_anchor = 0;
}

DirectedEdge* Region::insert(DirectedEdge* edge, Ogre::Vector3 p)
{
	_numOfEdges++;
	return edge->insert(p);
}

void Region::remove(DirectedEdge* edge)
{
	DirectedEdge* prev = edge->prev();
	DirectedEdge* next = edge->next();
	if (edge == _anchor) _anchor = next;
	delete edge;
	DirectedEdge::closeCycle(prev, next);
	_numOfEdges--;
}


bool sortDirectedEdgeByS(DirectedEdge* de1, DirectedEdge* de2)
{
	return (de1->_s < de2->_s);
}


// split fast doesn't handle coincident splits or no split
// that's ok for subdivision since this never seems! to occur
// probably FP accuracy makes it difficult
//#define SPLIT_FAST 1

std::list<Region*> Region::SplitRegion(Region *region, Vector3 &a, Vector3 &b)
{
	Vector3 ab = b - a;
	double Lsq = ab.squaredLength();

	DirectedEdge* curr = region->first();
	do
	{
		//LogManager::getSingleton().logMessage("Region Point " + StringConverter::toString(curr->srcPos()));

		Vector3 ac = curr->srcPos() - a;

		//s, indicates the pos of point c relative to line ab
		// - s<0      c is left of ab
		// - s>0      c is right of ab
		// - s=0      c is on ab
		// s = ((Ay-Cy)(Bx-Ax)-(Ax-Cx)(By-Ay)) / L^2
		curr->_s = (-ac.z * ab.x + ac.x * ab.z) / Lsq;

		// next
		curr = curr->next();
	}
	while (curr != region->first());

	std::vector<DirectedEdge*> created;
	curr = region->first();
	do
	{
		if((curr->_s > 0 && curr->next()->_s <= 0)
			|| (curr->_s <= 0 && curr->next()->_s > 0))
		{
			Vector3 cd = curr->dstPos() - curr->srcPos();
			double denom = (ab.x * cd.z) - (ab.z * cd.x);
			if(denom == 0)
				continue;

			Vector3 ca = a - curr->srcPos();
			double r = ((ca.z * cd.x) - (ca.x * cd.z)) / denom;    // pos on ab
			double s = ((ca.z * ab.x) - (ca.x * ab.z)) / denom;    // pos on cd

			DirectedEdge* d;

#ifndef SPLIT_FAST
			if(curr->_s == 0)
			{
				d = curr;
			}
			else if(curr->next()->_s == 0)
			{
				d = curr->next();
			}
			else
#endif
			d = region->insert(curr, Vector3(
					curr->srcPos().x + s * (cd.x),
					curr->srcPos().y + s * (cd.y),
					curr->srcPos().z + s * (cd.z)
					));
			d->_s = r;

			// add it to the created list
			//created.insert(created.end(), d);
			created.push_back(d);

			curr = d;
			// for height best use cd instead ab is always flat
		}
		curr = curr->next();
	}
	while (curr != region->first());


	//LogManager::getSingleton().logMessage("Created Points " + StringConverter::toString(created.size()));


#ifndef SPLIT_FAST
	if (created.size() == 0)
	{
		std::list<Region*> regions;
		regions.insert(regions.end(), region);
		return regions;
	}
#endif

	// NOTE!!
	//created.sort(sortDirectedEdgeByS); std::list adds a whopping 2 seconds - that is just nuts

	// sort the created list by location on ab
	std::sort(created.begin(), created.end(), sortDirectedEdgeByS);

	// reset the s value to 0
	curr = region->first();
	do
	{
		curr->_s = 0;
		curr = curr->next();
	}
	while (curr != region->first());


	// bridge the intersection in, out, in, out ...
	std::vector<DirectedEdge*>::const_iterator edgeIt2, edgeIt = created.begin(), edgeEnd = created.end();
	for(; edgeIt != edgeEnd; edgeIt++)
	{
		edgeIt2 = edgeIt;
		edgeIt2++;
		DirectedEdge::bridge(*edgeIt, *edgeIt2);
		//LogManager::getSingleton().logMessage("Bridged");
		edgeIt++;
	}

	// finally extract the new regions
	std::list<Region*> regions;
	BOOST_FOREACH(DirectedEdge* createdEdge, created)
	{
		curr = createdEdge;
		bool addRegion = true;
		size_t edgeCount = 0;
		do
		{
			if (curr->_s > 0)
			{
				addRegion = false;
				//LogManager::getSingleton().logMessage("Found an already visited vertex");
				break;
			}
			curr->_s = 1.0;
			edgeCount++;
			curr = curr->next();
		}
		while (curr != createdEdge);

		if (addRegion) {
			regions.insert(regions.end(), new Region(createdEdge, edgeCount));
			//LogManager::getSingleton().logMessage("Added a region");
		}
	}
	region->unlink();
	delete region;
	return regions;
}

DirectedEdge* Region::getLongestEdge(const bool exterior, Real &lengthSq)
{
	// declare vars and set initial longest to first segment
	Ogre::Real currLSq;
	lengthSq = 0;

	DirectedEdge *longest = 0, *curr = first();
	do
	{
		if(curr->_exterior == exterior)
		{
			currLSq = curr->getLengthSquared2D();
			if(currLSq > lengthSq) {
				lengthSq = currLSq;
				longest = curr;
			}
		}
		curr = curr->next();
	}
	while (curr != first());

	return longest;
}

bool Region::hasExterior()
{
	DirectedEdge* curr = first();
	do
	{
		if(curr->_exterior) return true;
		curr = curr->next();
	}
	while (curr != first());
	return false;
}
