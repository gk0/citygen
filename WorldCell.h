#ifndef WORLDCELL_H
#define WORLDCELL_H

#include "stdafx.h"
#include "WorldObject.h"
#include "RoadGraph.h"

class WorldRoad;
class NodeInterface;
class RoadInterface;

typedef struct {
	int seed;
	Ogre::Real segmentSize;
	Ogre::Real segmentDeviance;
	unsigned int degree;
	Ogre::Real degreeDeviance;
	Ogre::Real snapSize;
	Ogre::Real snapDeviance;
} GrowthGenParams;

class WorldCell : public WorldObject
{

private:
	static int mInstanceCount;

	Ogre::String mName;
	Ogre::Vector2 mCentre;
	Ogre::ManualObject* mManualObject;
	Ogre::ManualObject* mManualObject2;

	RoadGraph mRoadGraph;
	RoadGraph &mParentRoadGraph;
	GrowthGenParams mGrowthGenParams;

	std::set<RoadInterface*> mBoundaryRoads;
	std::vector<NodeInterface*> mBoundaryCycle;
	std::vector<RoadInterface*> mFilamentRoads;

public:
	WorldCell(RoadGraph &p);
	WorldCell(RoadGraph &p, std::vector<NodeInterface*> &n, std::set<RoadInterface*> &b);
	WorldCell(RoadGraph &p, std::set<RoadInterface*> &b);
	virtual ~WorldCell();

	GrowthGenParams getGrowthGenParams() const;
	void setGrowthGenParams(const GrowthGenParams &g);

	const std::set<RoadInterface*>& getBoundary() const;
	void setBoundary(const std::vector<NodeInterface*> &nodeCycle, const std::set<RoadInterface*> &b);
	void setBoundary(const std::set<RoadInterface*> &b);

	const std::vector<RoadInterface*>& getFilaments() const;
	void addFilament(WorldRoad* f);
	void removeFilament(WorldRoad* f);
	
	void update();
	//void createCell();
	//void destroyCell();

	void build();
	bool isInside(const Ogre::Vector2 &loc) const;
	bool isOnBoundary(NodeInterface *ni);

private:
	void clearRoadGraph();
	void init();
	void clearBoundary();
	void clearFilaments();
	void destroySceneObject();

	NodeInterface* createNode(const Ogre::Vector2 &pos);
	RoadInterface* createRoad(NodeInterface *n1, NodeInterface *n2);
	void deleteRoad(RoadInterface *ri);
	std::vector<NodeInterface*> getBoundaryCycle();
	void installGraph();
	void installRoad(RoadInterface* r, std::map<NodeInterface*, NodeInterface*> &nodeMap);
	RoadInterface* getLongestBoundaryRoad() const;
};

#endif
