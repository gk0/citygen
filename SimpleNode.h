#ifndef BASICNODE_H
#define BASICNODE_H

#include "stdafx.h"
#include "NodeInterface.h"

class MeshBuilder;

class SimpleNode : public NodeInterface
{
private:
	std::vector<Ogre::Real> _vertexData;
	std::vector<Ogre::uint16> _indexData;
	Ogre::Vector3 _position;
	std::map<RoadId, std::pair<Ogre::Vector3, Ogre::Vector3>, road_less_than > _roadJunction;

public:
	SimpleNode(RoadGraph &g);
	SimpleNode(RoadGraph &g, Ogre::Real x, Ogre::Real z);
	SimpleNode(RoadGraph &g, const Ogre::Vector2 &pos);
	SimpleNode(RoadGraph &g, const Ogre::Vector3 &pos);

	inline Ogre::Vector2 getPosition2D() const
	{
		return Ogre::Vector2(_position.x, _position.z);
	}

	Ogre::Vector3 getPosition3D() const
	{
		return _position;
	}

	bool setPosition2D(Ogre::Real x, Ogre::Real y);

	void prebuild();
	void build(MeshBuilder &meshBuilder, Ogre::Material* mat);
	
	std::pair<Ogre::Vector3, Ogre::Vector3> getRoadJunction(RoadId rd);



private:
	bool setPosition2D(const Ogre::Vector2& pos)
	{
		return setPosition2D(pos.x, pos.y);
	}

	void setPosition3D(Ogre::Real x, Ogre::Real y, Ogre::Real z)
	{
		_position.x = x;
		_position.y = y;
		_position.z = z;
	}

	void createTerminus();
	bool createTJunction();


}; 

#endif
