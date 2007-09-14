#ifndef WORLDCELL_H
#define WORLDCELL_H

#include "stdafx.h"
#include "WorldObject.h"
#include "WorldBlock.h"
#include "RoadGraph.h"
#include "CellGenParams.h"

class WorldRoad;
class NodeInterface;
class RoadInterface;


class WorldCell : public WorldObject
{

private:
	bool				_busy;

	static int			_instanceCount;

	Ogre::String		_name;
	Ogre::Vector2		_centre;
	Ogre::ManualObject* _roadNetworkMO;
	Ogre::ManualObject* _roadJunctionsMO;
	Ogre::ManualObject* _buildingsMO;
	Ogre::ManualObject* _buildingsMO1;
	Ogre::ManualObject* _buildingsMO2;
	Ogre::ManualObject* _debugMO;
	bool				_showRoads;			
	bool				_showBuildings;
	size_t				_roadLimit;

	RoadGraph			_roadGraph;
	const RoadGraph&	_parentRoadGraph;
	const RoadGraph&	_simpleRoadGraph;
	CellGenParams		_genParams;

	std::vector<RoadInterface*> _boundaryRoads;
	std::vector<NodeInterface*> _boundaryCycle;
	std::vector<RoadInterface*> _filamentRoads;

	std::vector<WorldBlock>		_blocks;


public:
	WorldCell(const RoadGraph &p, const RoadGraph &s);
	WorldCell(const RoadGraph &p, const RoadGraph &s, std::vector<NodeInterface*> &n);
	virtual ~WorldCell();

	CellGenParams getGenParams() const;
	void setGenParams(const CellGenParams &g);

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
	void prebuild();
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

	std::vector<Ogre::Vector3> getBoundaryPoints3D();
	std::vector<Ogre::Vector2> getBoundaryPoints2D();
	Ogre::Real calcArea2D();
	void extractPolygon(std::vector<NodeInterface*> &cycle, std::vector<Ogre::Vector3> &poly);


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

	void generateRoadNetwork(rando genRandom);
	void buildRoadNetwork();
};

#endif
