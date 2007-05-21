#ifndef WORLDNODE_H
#define WORLDNODE_H

#include "stdafx.h"
#include "WorldObject.h"
#include "NodeInterface.h"
#include "MovableText.h"
#include "RoadGraph.h"

class WorldRoad;

class WorldNode : public WorldObject, public NodeInterface
{

private:
	size_t mDegree;

	RoadGraph& mRoadGraph;
	RoadGraph& mSimpleRoadGraph;
	static int mInstanceCount;
	Ogre::ManualObject* mJunctionPlate;
	Ogre::Entity* mMesh;
	Ogre::Entity* mHighlight;
	Ogre::Entity* mSelected;
	Ogre::MovableText* mLabel;
	Ogre::SceneManager* mCreator;
	Ogre::String mName;
	//std::vector<WorldRoad*> mRoads;
	std::map<RoadId, std::pair<Ogre::Vector3, Ogre::Vector3>, road_less_than > mRoadJunction;

	Ogre::Vector2 getRoadBounaryIntersection(const RoadId leftR, const RoadId rightR);
	void onMove();

public:
	NodeId mSimpleNodeId;
	
	WorldNode(RoadGraph& g, RoadGraph& s, Ogre::SceneManager *creator);
	~WorldNode();

	void setLabel(const Ogre::String& label);
	const Ogre::String& getLabel() const;
	void showHighlighted(bool highlighted);
	void showSelected(bool selected);
	Ogre::Vector3 getPosition3D() const;
	void setPosition(const Ogre::Vector3 &pos);
	bool setPosition2D(Ogre::Real x, Ogre::Real z);
	bool setPosition2D(const Ogre::Vector2& pos);
	void setPosition3D(Ogre::Real x, Ogre::Real y, Ogre::Real z);
	void setPosition3D(const Ogre::Vector3& pos);
	void setPosition(Ogre::Real x, Ogre::Real y, Ogre::Real z);

	Ogre::Vector2 getPosition2D() const;
	const Ogre::Vector3& getPosition() const { return WorldObject::getPosition(); }

	//void attach(WorldObject* wo);
	//void detach(WorldObject* wo);
	void invalidate();

	bool move(Ogre::Vector2 pos);

	void notify();
	
	bool loadXML(const TiXmlHandle& nodeRoot);
	bool hasRoadIntersection();

	void build();

	std::pair<Ogre::Vector3, Ogre::Vector3> getRoadJunction(RoadId rd);

	void createTerminus();

	void onAddRoad();
	void onRemoveRoad();

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
