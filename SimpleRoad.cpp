#include "stdafx.h"
#include "SimpleRoad.h"
#include "Geometry.h"


SimpleRoad::SimpleRoad(NodeInterface *src, NodeInterface *dst)
 : mRoadWidth(0.4)
{
	mSrcNode = src;
	mDstNode = dst;
}

NodeInterface* SimpleRoad::getSrcNode() 
{
	return mSrcNode;
}

NodeInterface* SimpleRoad::getDstNode() 
{
	return mDstNode;
}

bool SimpleRoad::isRoadCycle()
{
	return mIsRoadCycle;
}

void SimpleRoad::setRoadCycle(bool cycle)
{
	mIsRoadCycle = cycle;
}

bool SimpleRoad::rayCross(const Ogre::Vector2& loc)
{
	return Geometry::rayCross(loc, mSrcNode->getPosition2D(), mDstNode->getPosition2D());
}

Ogre::Real SimpleRoad::getLengthSquared() const
{
	return (mSrcNode->getPosition2D() - mDstNode->getPosition2D()).squaredLength();
}

Ogre::Real SimpleRoad::getWidth() const
{
	return mRoadWidth;
}
