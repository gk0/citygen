#ifndef NODEINTERFACE_H
#define NODEINTERFACE_H

#include "stdafx.h"
#include "RoadGraph.h"
#include <OgreVector2.h>
#include <OgreVector3.h>

class NodeInterface
{
protected:
	RoadGraph& _roadGraph;

public:
	NodeId _nodeId;
	Ogre::Real _r,_s;
	NodeInterface(RoadGraph& g) : _roadGraph(g)
	{
	}
	virtual ~NodeInterface() {}
	virtual Ogre::Vector2 getPosition2D() const = 0;
	virtual Ogre::Vector3 getPosition3D() const = 0;
	virtual bool setPosition2D(Ogre::Real x, Ogre::Real z) = 0;
	virtual bool setPosition2D(const Ogre::Vector2& pos) = 0;
//	virtual void setPosition3D(Ogre::Real x, Ogre::Real y, Ogre::Real z) = 0;

	virtual void onAddRoad() {}
	virtual void onRemoveRoad() {}
	
	virtual std::pair<Ogre::Vector3, Ogre::Vector3> getRoadJunction(RoadId rd) = 0;
	virtual size_t getDegree()
	{
		return _roadGraph.getDegree(_nodeId);
	}

	friend RoadInterface* getRoad(NodeInterface* n1, NodeInterface* n2)
	{
		RoadGraph& g(n1->_roadGraph);
		return g.getRoad(g.getRoadId(n1->_nodeId, n2->_nodeId));
	}

	friend std::ostream& operator<<(std::ostream& os, const NodeInterface& n)
	{
		os << n.getPosition2D();
		return os;
	}
};

//std::ostream& operator<<(std::ostream& os, const NodeInterface& n)
//{
//	os << n.getPosition2D();
//	return os;
//}

#endif
