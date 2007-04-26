#ifndef SIMPLEROAD_H
#define SIMPLEROAD_H

#include "stdafx.h"
#include "RoadInterface.h"

class SimpleRoad : public RoadInterface
{
private:
	bool mIsRoadCycle;
	NodeInterface *mSrcNode, *mDstNode;
	Ogre::Real mRoadWidth;

public:
	SimpleRoad(NodeInterface *src, NodeInterface *dst);

	NodeInterface* getSrcNode() const;
	NodeInterface* getDstNode() const;
	bool isRoadCycle();
	void setRoadCycle(bool cycle);
	bool rayCross(const Ogre::Vector2& loc);
	Ogre::Real getLengthSquared() const;
	Ogre::Real getWidth() const;
	void setWidth(const Ogre::Real& w);

};

#endif
