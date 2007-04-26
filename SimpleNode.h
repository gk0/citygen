#ifndef BASICNODE_H
#define BASICNODE_H

#include "stdafx.h"
#include "NodeInterface.h"

class SimpleNode : public NodeInterface
{
private:
	Ogre::Vector3 mPosition;
	RoadGraph& mRoadGraph;
	std::map<RoadId, std::pair<Ogre::Vector3, Ogre::Vector3>, road_less_than > mRoadJunction;


public:
	SimpleNode(RoadGraph &g);
	SimpleNode(RoadGraph &g, Ogre::Real x, Ogre::Real z);
	SimpleNode(RoadGraph &g, const Ogre::Vector2 &pos);
	SimpleNode(RoadGraph &g, const Ogre::Vector3 &pos);

	inline Ogre::Vector2 getPosition2D() const
	{
		return Ogre::Vector2(mPosition.x, mPosition.z);
	}


	Ogre::Vector3 getPosition3D() const
	{
		return mPosition;
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
		mPosition.x = x;
		mPosition.y = y;
		mPosition.z = z;
	}

	void createTerminus();
	bool createTJunction();


}; 

#endif
