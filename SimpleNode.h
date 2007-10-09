#ifndef BASICNODE_H
#define BASICNODE_H

#include "stdafx.h"
#include "NodeInterface.h"

class SimpleNode : public NodeInterface
{
private:
	Ogre::Vector3 _position;
	std::map<RoadId, std::pair<Ogre::Vector3, Ogre::Vector3>, road_less_than > _roadJunction;

	std::vector<RoadId> getClockwiseVecOfRoads();
	std::vector<NodeId> getClockwiseVecOfNodes(const std::vector<RoadId>& roads);

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

	void createJunction(Ogre::ManualObject* junctionPlate);
	
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

	inline static Ogre::Vector2 madnessCheck(const Ogre::Vector2& nodePos, const Ogre::Vector2& pos, 
		const Ogre::Real limitSq, const Ogre::Real limit)
	{
		Ogre::Vector2 dir = pos - nodePos;
		Ogre::Real len = dir.squaredLength();
		if(len < limitSq) 
		{	
			return pos;
		}
		else
		{
			dir.normalise();
			return (nodePos + (dir * limit));
		}
	}

}; 

#endif
