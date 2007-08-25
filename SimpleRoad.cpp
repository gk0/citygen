#include "stdafx.h"
#include "SimpleRoad.h"
#include "Geometry.h"

using namespace Ogre;


SimpleRoad::SimpleRoad(NodeInterface *src, NodeInterface *dst)
 : _roadWidth(0.4)
{
	_srcNode = src;
	_dstNode = dst;
}

NodeInterface* SimpleRoad::getSrcNode() const
{
	return _srcNode;
}

NodeInterface* SimpleRoad::getDstNode() const
{
	return _dstNode;
}

bool SimpleRoad::isRoadCycle()
{
	return _isRoadCycle;
}

void SimpleRoad::setRoadCycle(bool cycle)
{
	_isRoadCycle = cycle;
}

bool SimpleRoad::rayCross(const Ogre::Vector2& loc)
{
	return Geometry::rayCross(loc, _srcNode->getPosition2D(), _dstNode->getPosition2D());
}

Ogre::Real SimpleRoad::getLengthSquared() const
{
	return (_srcNode->getPosition2D() - _dstNode->getPosition2D()).squaredLength();
}

Ogre::Real SimpleRoad::getWidth() const
{
	return _roadWidth;
}

void SimpleRoad::setWidth(const Ogre::Real& w)
{
	_roadWidth = w;
}
