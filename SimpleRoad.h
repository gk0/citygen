#ifndef SIMPLEROAD_H
#define SIMPLEROAD_H

#include "stdafx.h"
#include "RoadInterface.h"

class MeshBuilder;

class SimpleRoad : public RoadInterface
{
private:
	bool			_isRoadCycle;
	NodeInterface	*_srcNode, *_dstNode;
	Ogre::Real		_roadWidth;
	std::vector<Ogre::Real> _vertexData;
	std::vector<Ogre::uint16> _indexData;
	RoadId _roadId;

public:
	SimpleRoad(NodeInterface *src, NodeInterface *dst, RoadId rd);

	NodeInterface* getSrcNode() const;
	NodeInterface* getDstNode() const;
	bool isRoadCycle();
	void setRoadCycle(bool cycle);
	bool rayCross(const Ogre::Vector2& loc);
	Ogre::Real getLengthSquared() const;
	Ogre::Real getWidth() const;
	void setWidth(const Ogre::Real& w);

	void prebuild();
	void build(MeshBuilder &meshBuilder, Ogre::Material* mat);

};

#endif
