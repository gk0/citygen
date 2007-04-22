#ifndef BASICNODE_H
#define BASICNODE_H

#include "stdafx.h"
#include "RoadGraph.h"
#include "NodeInterface.h"
#include "WorldFrame.h"

class SimpleNode : public NodeInterface
{
private:
	Ogre::Vector3 mPosition;

public:
	SimpleNode() { }
	SimpleNode(Ogre::Real x, Ogre::Real z)
	{
		setPosition2D(x, z);
	}
	SimpleNode(const Ogre::Vector2 &pos)
	{
		setPosition2D(pos.x, pos.y);
	}
	SimpleNode(const Ogre::Vector3 &pos)
	{
		setPosition3D(pos.x, pos.y, pos.z);
	}

	inline Ogre::Vector2 getPosition2D() const
	{
		return Ogre::Vector2(mPosition.x, mPosition.z);
	}


	Ogre::Vector3 getPosition3D() const
	{
		return mPosition;
	}


private:
	
	bool setPosition2D(Ogre::Real x, Ogre::Real z)
	{
		Ogre::Real y;
		if(WorldFrame::getSingleton().plotPointOnTerrain(x, y, z))
		{
			setPosition3D(x,y+0.199,z);
			return true;
		}
		return false;
	}

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


}; 

#endif
