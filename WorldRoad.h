#ifndef WORLDROAD_H
#define WORLDROAD_H

#include "stdafx.h"
#include "WorldObject.h"
#include "RoadInterface.h"
#include "RoadGraph.h"
#include "MovableText.h"

class WorldNode;
class WorldCanvas;

class WorldRoad : public WorldObject, public RoadInterface
{
public:
	enum RoadIntersectionState {
		none,
		road,
		simple_node,
		world_node
	};

private:
	static int mRoadCount;

	RoadId mSimpleRoadId;

	Ogre::ManualObject *mManualObject;

	//SceneNode* mMeshNode;
	Ogre::MovableText *mLabel;
	Ogre::String mName;

	bool mIsRoadCycle;
//	WorldNode *mSrcNode, *mDstNode;

	RoadGraph &mRoadGraph;
	RoadGraph &mSimpleRoadGraph;

	Ogre::Real mRoadSegSz;
	Ogre::Real mRoadDeviance;
	Ogre::Real mRoadWidth;

	std::vector<RoadId> mRoadSegmentList;
	std::vector<Ogre::Vector3> mPlotList;
	bool bindToGraph;
	void buildSegment(const Ogre::Vector3 &a1, const Ogre::Vector3 &a2, const Ogre::Vector3 &aNorm,
		const Ogre::Vector3 &b1, const Ogre::Vector3 &b2, const Ogre::Vector3 &bNorm, Ogre::Real uMin, Ogre::Real uMax);


public:
	WorldRoad(WorldNode* src, WorldNode* dst, RoadGraph& g, 
					 RoadGraph& s, Ogre::SceneManager *creator, bool bind = true);
	virtual ~WorldRoad();

	NodeInterface* getSrcNode() const;
	NodeInterface* getDstNode() const;

	RoadIntersectionState snap(const Ogre::Real& snapSzSquared, NodeId& nd, RoadId& rd, Ogre::Vector2& intersection);

	bool hasIntersection();

	bool isRoadCycle();
	void setRoadCycle(bool cycle);

	bool rayCross(const Ogre::Vector2& loc);
	Ogre::Real getLengthSquared() const;
	const std::vector<RoadId>& getRoadSegmentList();
	Ogre::Real getWidth() const;
	
	void onMoveNode();

private:
	void build();
	void destroyRoadObject();

	void createRoadGraph();
	void destroyRoadGraph();
	void plotRoad();


	Ogre::Vector3 findNextPoint(const Ogre::Vector2& cursor, const Ogre::Vector2& direction, const Ogre::Degree& dev) const;

	bool getClosestIntersection(RoadId& rd, Ogre::Vector2& pos) const;
};

#endif
