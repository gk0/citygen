#ifndef WORLDCELL_H
#define WORLDCELL_H

#include "stdafx.h"
#include "WorldObject.h"
#include "WorldBlock.h"
#include "RoadGraph.h"
#include "CellParams.h"
#include "ExportDoc.h"

class WorldRoad;
class NodeInterface;
class RoadInterface;

class TiXmlHandle;
class TiXmlElement;

class WorldCell : public WorldObject, public Ogre::ManualResourceLoader
{
public:
	enum Display {
		view_primary,
		view_cell,
		view_box,
		view_building
	};

private:

	Display				_displayMode;
	bool				_busy;

	static int			_instanceCount;
	static CellParams _defaultGenParams;

	Ogre::String		_name;
	Ogre::Vector2		_centre;
	Ogre::ManualObject* _debugMO;

	Ogre::Entity*		_buildingsEnt;
	Ogre::Entity*		_roadsEnt;
	size_t				_roadLimit;

	RoadGraph			_roadGraph;
	const RoadGraph&	_parentRoadGraph;
	const RoadGraph&	_simpleRoadGraph;
	CellParams			_genParams;

	std::vector<RoadInterface*> _boundaryRoads;
	std::vector<NodeInterface*> _boundaryCycle;
	std::vector<RoadInterface*> _filamentRoads;

	std::vector<WorldBlock*>	_blocks;

	MeshBuilder*		_mbBuildings;


public:
	WorldCell(const RoadGraph &p, const RoadGraph &s, std::vector<NodeInterface*> &n, const Display mode);
	virtual ~WorldCell();

	CellParams getGenParams() const;
	static CellParams getDefaultGenParams();
	void setGenParams(const CellParams &g);
	static void setDefaultGenParams(const CellParams &g);

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

	void setSelected(bool show);
	void setHighlighted(bool show);
	void setDisplayMode(Display mode);

	bool loadXML(const TiXmlHandle& worldRoot);
	TiXmlElement* saveXML();

	std::vector<Ogre::Vector3> getBoundaryPoints3D();
	std::vector<Ogre::Vector2> getBoundaryPoints2D();
	Ogre::Real calcArea2D();
	void extractPolygon(std::vector<NodeInterface*> &cycle, std::vector<Ogre::Vector3> &poly);

	void loadResource(Ogre::Resource* r)
	{
		//prebuild();
		//build();
	}

	static void resetInstanceCount() { _instanceCount = 0; }

	void exportObject(ExportDoc &doc);

private:
	void clearRoadGraph();

	void clearFilaments();
	void destroySceneObject();

	NodeInterface* createNode(const Ogre::Vector2 &pos);
	NodeInterface* createNode(const Ogre::Vector3 &pos);
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
	void buildRoads();
	void buildBuildings();
	void prebuildRoads();
	void prebuildBuildings();

};

#endif
