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

	std::vector<RoadInterface*> mBoundaryRoads;
	std::vector<NodeInterface*> mBoundaryCycle;
	std::vector<RoadInterface*> mFilamentRoads;

public:
	WorldCell(RoadGraph &p);
	WorldCell(RoadGraph &p, std::vector<NodeInterface*> &n, std::vector<RoadInterface*> &b);
	WorldCell(RoadGraph &p, std::vector<RoadInterface*> &b);
	virtual ~WorldCell();

	GrowthGenParams getGrowthGenParams() const;
	void setGrowthGenParams(const GrowthGenParams &g);

	const std::vector<RoadInterface*>& getBoundary() const;
	void setBoundary(const std::vector<NodeInterface*> &nodeCycle, const std::vector<RoadInterface*> &b);
	void setBoundary(const std::vector<RoadInterface*> &b);

	const std::vector<RoadInterface*>& getFilaments() const;
	void addFilament(WorldRoad* f);
	void removeFilament(WorldRoad* f);
	
	void update();
	//void createCell();
	//void destroyCell();

	void build();
	bool isInside(const Ogre::Vector2 &loc) const;
	bool isOnBoundary(NodeInterface *ni);
	bool compareBoundary(const std::vector<RoadInterface*>& roadCycle) const;

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

	static bool extractFootprint(const std::vector<NodeInterface*> &nodeCycle, 
						 const std::vector<RoadInterface*> &roadCycle, 
						 std::vector<Ogre::Vector2> &footprint); 

	void createBuilding(Ogre::ManualObject* m, const std::vector<Ogre::Vector2> &footprint,
							   const Ogre::Real foundation, const Ogre::Real height);

	void buildSegment(const Ogre::Vector3 &a1, const Ogre::Vector3 &a2, const Ogre::Vector3 &aNorm,
			const Ogre::Vector3 &b1, const Ogre::Vector3 &b2, const Ogre::Vector3 &bNorm, 
			Ogre::Real uMin, Ogre::Real uMax);

	bool createJunction(const NodeId nd, Ogre::ManualObject *m);
	void createRoad(const RoadId rd, Ogre::ManualObject *m);
};

#endif
