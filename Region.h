#pragma once

#include "DirectedEdge.h"
#include <list>
#include <vector>

namespace citygen 
{

class Region
{
private:
	DirectedEdge* _anchor;
	size_t _numOfEdges;

public:
	Region(DirectedEdge* anchor, size_t numOfEdges);
	//Region(std::list<Ogre::Vector3> points, bool exterior = true);
	Region(std::vector<Ogre::Vector3> points, bool exterior = true);
	~Region(void);

	DirectedEdge* first() 
	{ 
		return _anchor; 
	}

	size_t size()
	{
		return _numOfEdges;
	}

	void unlink();

	DirectedEdge* insert(DirectedEdge* edge, Ogre::Vector3 p);
	void remove(DirectedEdge* edge);

	static std::list<Region*> SplitRegion(Region *region, Ogre::Vector3 &a, Ogre::Vector3 &b);

	DirectedEdge* getLongestEdge(const bool exterior, Ogre::Real &lengthSq);

	bool hasExterior();
};

};

