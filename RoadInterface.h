#ifndef ROADINTERFACE_H
#define ROADINTERFACE_H

#include "stdafx.h"
#include "NodeInterface.h"

class RoadInterface
{

public:
	virtual ~RoadInterface() {}
	virtual NodeInterface* getSrcNode() const = 0;
	virtual NodeInterface* getDstNode() const = 0;
	virtual bool isRoadCycle() = 0;
	virtual void setRoadCycle(bool cycle) = 0;
	virtual bool rayCross(const Ogre::Vector2& loc) = 0;
	virtual Ogre::Real getLengthSquared() const = 0;
	virtual Ogre::Real getWidth() const = 0;

	virtual void onMoveNode() {};
	virtual void setEdgeBoundary(const NodeId nd, const std::pair<Ogre::Vector3, Ogre::Vector3> &pos) {};
}; 

#endif
