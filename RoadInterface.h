#ifndef ROADINTERFACE_H
#define ROADINTERFACE_H

#include "stdafx.h"
#include "NodeInterface.h"

class RoadInterface
{

public:
	virtual ~RoadInterface() {}
	virtual NodeInterface* getSrcNode() = 0;
	virtual NodeInterface* getDstNode() = 0;
	virtual bool isRoadCycle() = 0;
	virtual void setRoadCycle(bool cycle) = 0;
	virtual bool rayCross(const Ogre::Vector2& loc) = 0;
	virtual Ogre::Real getLengthSquared() const = 0;

}; 

#endif
