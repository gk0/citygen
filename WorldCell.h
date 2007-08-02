#ifndef WORLDCELL_H
#define WORLDCELL_H

#include "stdafx.h"
#include "WorldObject.h"
#include "RoadGraph.h"

class WorldRoad;
class NodeInterface;
class RoadInterface;

typedef struct {
	unsigned int type;
	int seed;
	Ogre::Real segmentSize;
	Ogre::Real segmentDeviance;
	unsigned int degree;
	Ogre::Real degreeDeviance;
	Ogre::Real snapSize;
	Ogre::Real snapDeviance;
	Ogre::Real buildingHeight;
	Ogre::Real buildingDeviance;
	Ogre::Real roadWidth;
	size_t roadLimit;
	Ogre::Real connectivity;
	Ogre::Real lotSize;
	Ogre::Real lotDeviance;
} GrowthGenParams;

class WorldCell : public WorldObject
{

private:
	static int mInstanceCount;

	Ogre::String mName;
	Ogre::Vector2 mCentre;
	Ogre::ManualObject* mRoadNetwork;
	Ogre::ManualObject* mRoadJunctions;
	Ogre::ManualObject* mBuildings;
	Ogre::ManualObject* mDebug;

	RoadGraph mRoadGraph;
	RoadGraph &mParentRoadGraph;
	RoadGraph &mSimpleRoadGraph;
	GrowthGenParams mGrowthGenParams;

	std::vector<RoadInterface*> mBoundaryRoads;
	std::vector<NodeInterface*> mBoundaryCycle;
	std::vector<RoadInterface*> mFilamentRoads;

	bool mShowRoads, mShowBuildings;
	size_t mRoadLimit;

	// gcd crap
	RoadGraph mGCDRoadGraph;
	Ogre::ManualObject* mOverlay;
	Ogre::ManualObject* mOverlay2;
	std::list<NodeId> mGCDHeap;
	std::vector< std::vector<NodeInterface*> > mGCDFilaments;
	std::vector< std::vector<NodeInterface*> > mGCDNodeCycles;
	std::vector< std::vector<RoadInterface*> > mGCDRoadCycles;

public:
	WorldCell(RoadGraph &p, RoadGraph &s);
	WorldCell(RoadGraph &p, RoadGraph &s, std::vector<NodeInterface*> &n);
	virtual ~WorldCell();

	GrowthGenParams getGenParams() const;
	void setGenParams(const GrowthGenParams &g);

	const std::vector<RoadInterface*>& getBoundaryRoads() const;
	const std::vector<NodeInterface*>& getBoundaryCycle() const;
	void setBoundary(const std::vector<NodeInterface*> &nodeCycle);

	const std::vector<RoadInterface*>& getFilaments() const;
	void addFilament(WorldRoad* f);
	void removeFilament(WorldRoad* f);
	
	void update();
	//void createCell();
	//void destroyCell();

	void build();
	bool isInside(const Ogre::Vector2 &loc) const;
	bool isBoundaryNode(const NodeInterface *ni) const;
	bool compareBoundary(const std::vector<NodeInterface*>& nodeCycle) const;
	void clearBoundary();
	void clear();

	void showSelected(bool show);
	void showRoads(bool show);
	void showBuildings(bool show);

	bool loadXML(const TiXmlHandle& worldRoot);
	TiXmlElement* saveXML();

	// for graphical DEBUG
	void beginGraphicalCellDecomposition();
	void stepGraphicalCellDecomposition();
	void drawGraphicalCellDecomposition();

	std::vector<Ogre::Vector3> getBoundaryPoints3D();
	std::vector<Ogre::Vector2> getBoundaryPoints2D();
	Ogre::Real calcArea2D();

private:
	void clearRoadGraph();
	void init();

	void clearFilaments();
	void destroySceneObject();

	NodeInterface* createNode(const Ogre::Vector2 &pos);
	RoadInterface* createRoad(NodeInterface *n1, NodeInterface *n2);
	void deleteRoad(RoadInterface *ri);
	void installGraph();
	void installRoad(RoadInterface* r, std::map<NodeInterface*, NodeInterface*> &nodeMap);
	RoadInterface* getLongestBoundaryRoad() const;

	bool extractFootprint(const std::vector<NodeInterface*> &nodeCycle,  
						 std::vector<Ogre::Vector2> &footprint); 

	bool createBuilding(Ogre::ManualObject* m, const std::vector<Ogre::Vector2> &footprint,
							   const Ogre::Real foundation, const Ogre::Real height);

	void buildSegment(const Ogre::Vector3 &a1, const Ogre::Vector3 &a2, const Ogre::Vector3 &aNorm,
			const Ogre::Vector3 &b1, const Ogre::Vector3 &b2, const Ogre::Vector3 &bNorm, 
			Ogre::Real uMin, Ogre::Real uMax);

	bool createJunction(const NodeId nd, Ogre::ManualObject *m);
	void createRoad(const RoadId rd, Ogre::ManualObject *m);
	bool getRoadBounaryIntersection(const RoadId leftR, const RoadId rightR, Ogre::Vector2 &pos);

	//hack
	RoadInterface* getRoad(NodeInterface* n1, NodeInterface* n2);

	void generateRoadNetwork();
	void buildRoadNetwork();
};

#endif
