#ifndef NODEINTERFACE_H
#define NODEINTERFACE_H

#include "stdafx.h"
#include "RoadGraph.h"

class NodeInterface
{

public:
	NodeId mNodeId;
	virtual ~NodeInterface() {}
	virtual Ogre::Vector2 getPosition2D() const = 0;
	virtual Ogre::Vector3 getPosition3D() const = 0;
	virtual bool setPosition2D(Ogre::Real x, Ogre::Real z) = 0;

	virtual void setPosition3D(Ogre::Real x, Ogre::Real y, Ogre::Real z) = 0;

};     

#endif
