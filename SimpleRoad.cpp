#include "stdafx.h"
#include "SimpleRoad.h"
#include "Geometry.h"
#include "MeshBuilder.h"

using namespace Ogre;


SimpleRoad::SimpleRoad(NodeInterface *src, NodeInterface *dst, RoadId rd)
 : _roadWidth(0.4), _roadId(rd)
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

void SimpleRoad::prebuild()
{
	_vertexData.clear();
	_indexData.clear();
	_vertexData.reserve(32);
	_indexData.reserve(6);

	Vector3 a1,a2, b1, b2;
	//RoadInterface* rd = getRoad(_srcNode, _dstNode);
	boost::tie(a1,a2) = _srcNode->getRoadJunction(_roadId);
	boost::tie(b2,b1) = _dstNode->getRoadJunction(_roadId);
	//buildSegment(a1, a2, Vector3::UNIT_Y, b1, b2, Vector3::UNIT_Y, 0, 4);
	Real uMin = 0, uMax = (((a1+a2) / 2) - ((b1+b2) / 2)).length();

	MeshBuilder::addVData3(_vertexData, a1);
	MeshBuilder::addVData3(_vertexData, Vector3::UNIT_Y);
	MeshBuilder::addVData2(_vertexData, uMin, 0);
	MeshBuilder::addVData3(_vertexData, a2);
	MeshBuilder::addVData3(_vertexData, Vector3::UNIT_Y);
	MeshBuilder::addVData2(_vertexData, uMin, 1);

	MeshBuilder::addVData3(_vertexData, b2);
	MeshBuilder::addVData3(_vertexData, Vector3::UNIT_Y);
	MeshBuilder::addVData2(_vertexData, uMax, 1);
	MeshBuilder::addVData3(_vertexData, b1);
	MeshBuilder::addVData3(_vertexData, Vector3::UNIT_Y);
	MeshBuilder::addVData2(_vertexData, uMax, 0);

	MeshBuilder::addIData3(_indexData, 3, 1, 0);
	MeshBuilder::addIData3(_indexData, 3, 2, 1);
}

void SimpleRoad::build(MeshBuilder& meshBuilder, Material* mat)
{
	meshBuilder.registerData(mat, _vertexData, _indexData);
}
