#include "stdafx.h"
#include "WorldCell.h"
#include "NodeInterface.h"
#include "RoadInterface.h"
#include "Geometry.h"
#include "WorldNode.h"
#include "WorldRoad.h"
#include "WorldFrame.h"
#include "SimpleNode.h"
#include "SimpleRoad.h"
#include "Triangulate.h"
#include "PerformanceTimer.h"
#include "WorldBlock.h"
#include "MeshBuilder.h"

#include <OgreEntity.h>
#include <OgreManualObject.h>
#include <OgreMaterialManager.h>
#include <OgreMeshManager.h>
#include <OgreStringConverter.h>
#include <tinyxml.h>

using namespace Ogre;
using namespace std;
int WorldCell::_instanceCount = 0;

const CellParams CellParams::MANHATTAN(
	0,		// type
	1,		// seed
	100,	// segmentSize
	0.2,	// segmentDeviance
	4,		// degree
	0.02,	// degreeDeviance
	30.0,	// snapSize
	0.1,	// snapDeviance
	18,		// buildingHeight
	0.6,	// buildingDeviance
	4.6,	// roadWidth
	0,		// roadLimit
	1.0,	// connectivity
	3.2,	// footpathWidth;
	0.28,	// footpathHeight;
	10.5,	// lotWidth
	18.0,	// lotDepth
	0.5,	// lotDeviance
	false	// debug
);

const CellParams CellParams::INDUSTRIAL(
	1,		// type
	1,		// seed
	84,		// segmentSize
	0.2,	// segmentDeviance
	4,		// degree
	0.05,	// degreeDeviance
	75,		// snapSize
	0.1,	// snapDeviance
	6,	// buildingHeight
	0.3,	// buildingDeviance
	3.5,	// roadWidth
	0,		// roadLimit
	0.15,	// connectivity
	2,	// footpathWidth;
	0.28,	// footpathHeight;
	32.0,	// lotWidth
	28.0,	// lotDepth
	0.6,	// lotDeviance
	false	// debug
);

const CellParams CellParams::SUBURBIA(
	2,		// type
	1,		// seed
	46,		// segmentSize
	0.6,	// segmentDeviance
	9,		// degree
	0.6,	// degreeDeviance
	40,		// snapSize
	0.1,	// snapDeviance
	4,		// buildingHeight
	0.1,	// buildingDeviance
	3.0,	// roadWidth
	0,		// roadLimit
	0.0,	// connectivity
	1.5,	// footpathWidth;
	0.28,	// footpathHeight;
	7.0,	// lotWidth
	12.0,	// lotDepth
	0.2,	// lotDeviance
	false	// debug
);

CellParams WorldCell::_defaultGenParams = CellParams::MANHATTAN;

#define USENORMALS 1


WorldCell::WorldCell(const RoadGraph &p, const RoadGraph &s,
		vector<NodeInterface*> &n, const Display mode) :
	_parentRoadGraph(p), _simpleRoadGraph(s)
{
	_displayMode = mode;
	_busy = false;

	// set up some default growth gen params
	_genParams = _defaultGenParams;

	_roadsEnt = 0;
	_buildingsEnt = 0;
	_debugMO = 0;
	_mbBuildings = 0;

	_name = "cell"+StringConverter::toString(_instanceCount++);
	_sceneNode = WorldFrame::getSingleton().getSceneManager()->getRootSceneNode()->createChildSceneNode(_name);

	setBoundary(n);
}

WorldCell::~WorldCell()
{
	clearBoundary();
	clearFilaments();
	clearRoadGraph();

	destroySceneObject();
	_sceneNode->detachAllObjects();
	_sceneNode->getCreator()->destroySceneNode(_sceneNode->getName());
}

void WorldCell::clear()
{
	clearBoundary();
	clearFilaments();
	clearRoadGraph();
	destroySceneObject();
}

void WorldCell::destroySceneObject()
{
	_sceneNode->detachAllObjects();

	if (_debugMO)
	{
		_sceneNode->getCreator()->destroyManualObject(_debugMO);
		delete _debugMO;
		_debugMO = 0;
	}

	if (_roadsEnt)
	{
		_sceneNode->getCreator()->destroyEntity(_roadsEnt);
		MeshManager::getSingleton().remove(_name+"Roads");
		_roadsEnt = 0;
	}

	if (_buildingsEnt)
	{
		_sceneNode->getCreator()->destroyEntity(_buildingsEnt);
		MeshManager::getSingleton().remove(_name+"Buildings");
		_buildingsEnt = 0;
	}
	// TODO ?
	MeshManager::getSingleton().remove(_name+"b0Mesh");
	MeshManager::getSingleton().remove(_name+"b1Mesh");
}

void WorldCell::update()
{
}

void WorldCell::clearRoadGraph()
{
	// delete roads
	RoadIterator rIt, rEnd;
	for ( tie(rIt, rEnd) = _roadGraph.getRoads(); rIt != rEnd; rIt++)
	{
		RoadInterface* ri = _roadGraph.getRoad((*rIt));
		if (typeid(*ri) == typeid(SimpleRoad))
			delete ri;
	}

	// delete nodes
	NodeIterator nIt, nEnd;
	for ( boost::tie(nIt, nEnd) = _roadGraph.getNodes(); nIt != nEnd; nIt++)
		delete _roadGraph.getNode((*nIt));

	_roadGraph.clear();
}

struct CMP1231
{
	bool operator()(const pair<size_t, WorldRoad*> l, const pair<size_t, WorldRoad*> r)
	{
		return l.second->getLength() > r.second->getLength();
	}
};
bool cmpRd(const WorldRoad* l, const WorldRoad* r)
{
	return l->getLength() < r->getLength();
}

void WorldCell::generateRoadNetwork(rando genRandom)
{
	queue< pair<NodeInterface*, Vector2> > q;
	//TODO tidy

	Ogre::Real snapSzSquared = _genParams._snapSize * _genParams._snapSize;

	// 
	bool centreSuccess = false;
	if (_genParams._connectivity >= 1.0f)
	{
		// 2. Get Center as a start point !!return if its node inside the cell	
		vector<Vector2> pointList;
		vector<NodeInterface*>::const_iterator nIt, nEnd;
		for (nIt = _boundaryCycle.begin(), nEnd = _boundaryCycle.end(); nIt
				!= nEnd; nIt++)
			pointList.push_back((*nIt)->getPosition2D());

		_centre = Geometry::centerOfMass(pointList);

		if (isInside(_centre))
		{
			NodeInterface* startNode = createNode(_centre);

			// 3. Get initial direction vector from longest boundary road
			RoadInterface *longest = getLongestBoundaryRoad();
			Vector2 direction = longest->getSrcNode()->getPosition2D() - longest->getDstNode()->getPosition2D();
			direction.normalise();
			direction *= _genParams._segmentSize;

			q.push(make_pair<NodeInterface*, Vector2>(startNode, direction));
			centreSuccess = true;
		}
	}

	if (!centreSuccess)
	{
		// get a list of boundary roads
		size_t boundaryRoadsN = _boundaryCycle.size();
		vector< pair<size_t, WorldRoad*> > boundaryRoads;
		boundaryRoads.reserve(boundaryRoadsN);
		std::vector<Real> lengths;

		for (size_t j, i=0; i<_boundaryCycle.size(); i++)
		{
			j = (i+1)%boundaryRoadsN;
			WorldRoad* wr = getWorldRoad(
					static_cast<WorldNode*>(_boundaryCycle[i]),
					static_cast<WorldNode*>(_boundaryCycle[j]));
			lengths.push_back(wr->getLength());
			boundaryRoads.push_back(make_pair<size_t, WorldRoad*>(i, wr));
		}

		// sort roads by length
		std::sort(boundaryRoads.begin(), boundaryRoads.end(), CMP1231());

		// for half of the roads, the largest ones
		for (size_t i=0; i<(boundaryRoadsN/2); i++)
		{
			// first node
			Vector3 pos1_3D(boundaryRoads[i].second->getMidPoint());
			Vector2 pos1(pos1_3D.x, pos1_3D.z);

			// get the road direction
			size_t j = boundaryRoads[i].first;
			size_t k = (j+1)%boundaryRoadsN;
			Vector2 roadDir(_boundaryCycle[k]->getPosition2D()
					- _boundaryCycle[j]->getPosition2D());
			roadDir = roadDir.perpendicular();
			roadDir.normalise();
			Vector2 pos2 = pos1 + (roadDir * _genParams._segmentSize);
			pos1 -= (roadDir * _genParams._segmentSize);

			NodeInterface* currentNode = 0;
			if (!isInside(pos2))
				continue;
			currentNode = createNode(pos2);

			// alter adjoining boundary road
			RoadId rd;
			NodeId nd;
			Vector3 newPoint;

			// 6600 vs. 8600 debug
			// 94 vs. 109 release
			//switch(_roadGraph.findClosestSnappedIntersection(currentNode->getPosition2D(), cursor, snapSzSquared, nd, rd, newPoint))
			switch (_roadGraph.snapInfo(currentNode->_nodeId, pos1,
					_genParams._snapSize, newPoint, nd, rd))
			{
			case 0:
				break;
			case 1:
			{
				// road intersection
				//NodeInterface *cursorNode = createNode(newPoint);
				NodeInterface *cursorNode = new SimpleNode(_roadGraph);
				NodeId cursorNodeId = _roadGraph.addNode(cursorNode);
				cursorNode->_nodeId = cursorNodeId;
				createRoad(currentNode, cursorNode);

				// get intersected source src and dst
				RoadInterface *ri = _roadGraph.getRoad(rd);
				NodeId srcNodeId = _roadGraph.getSrc(rd);
				NodeId dstNodeId = _roadGraph.getDst(rd);

				//if road is a boundary road
				if (typeid(*ri) == typeid(WorldRoad))
				{
					cursorNode->setPosition3D(newPoint.x, newPoint.y, newPoint.z);
					// remove road segment from graph
					_roadGraph.removeRoad(srcNodeId, dstNodeId);

					// create replacement segments
					RoadId rd;
					if (_roadGraph.addRoad(srcNodeId, cursorNodeId, rd))
						_roadGraph.setRoad(rd, ri);
					if (_roadGraph.addRoad(cursorNodeId, dstNodeId, rd))
						_roadGraph.setRoad(rd, ri);
				}
				else
				{
					cursorNode->setPosition2D(newPoint.x, newPoint.z);
					// delete the road rd
					deleteRoad(ri);

					// reconstruct road in the form of two segments
					createRoad(_roadGraph.getNode(srcNodeId), cursorNode);
					createRoad(cursorNode, _roadGraph.getNode(dstNodeId));
				}
				q.push(make_pair<NodeInterface*, Vector2>(currentNode, roadDir));
			}
				break;
			case 2:
				// node snap
				// MMM: dont snap to your self ass monkey
				if (currentNode != _roadGraph.getNode(nd))
					createRoad(currentNode, _roadGraph.getNode(nd));

				q.push(make_pair<NodeInterface*, Vector2>(currentNode, roadDir));
				break;
			}
		}
	}

	Ogre::Real segDevSz = _genParams._segmentSize * _genParams._segmentDeviance;
	Ogre::Real segSzBase = _genParams._segmentSize - (segDevSz / 2);
	Real degDev = _genParams._degree * _genParams._degreeDeviance;
	Real degBase = _genParams._degree - (degDev / 2);

	srand(_genParams._seed);

	size_t roadCount = 0;

	while (!q.empty())
	{
		NodeInterface* currentNode;
		Vector2 currentDirection;
		boost::tie(currentNode, currentDirection) = q.front();
		q.pop();

		//Ogre::Radian theta(Math::TWO_PI / (degBase + (degDev * ((float)rand()/(float)RAND_MAX))));
		Ogre::Radian theta(Math::TWO_PI / (degBase + (degDev * genRandom())));

		// alter our direction vector 
		for (unsigned int i=0; i < _genParams._degree; i++)
		{
			if (_genParams._roadLimit != 0 && roadCount
					>= _genParams._roadLimit)
			//if(roadCount++ >= 29)
			{
				while (!q.empty())
					q.pop();
				break;
			}

			if (roadCount >= 39)
			{
				size_t z = 0;
			}

			// get a candidate
			Geometry::rotate(currentDirection, theta);
			// doesn't work grrrrrrrrrr

			currentDirection.normalise();
			//Real segSz = (segSzBase + (segDevSz *  ((float)rand()/(float)RAND_MAX)));
			Real segSz = (segSzBase + (segDevSz * genRandom()));
			currentDirection *= segSz;
			Vector2 cursor(currentDirection + currentNode->getPosition2D());

			RoadId rd;
			NodeId nd;
			Vector3 newPoint;

			// 6600 vs. 8600 debug
			// 94 vs. 109 release
			//switch(_roadGraph.findClosestSnappedIntersection(currentNode->getPosition2D(), cursor, snapSzSquared, nd, rd, newPoint))
			switch (_roadGraph.snapInfo(currentNode->_nodeId, cursor,
					_genParams._snapSize, newPoint, nd, rd))
			{
			case 0:
				// no intersection found
			{
				NodeInterface *cursorNode = createNode(cursor);
				createRoad(currentNode, cursorNode);
				roadCount++;
				q.push(make_pair<NodeInterface*, Vector2>(cursorNode,
						currentDirection)); // enqueue
			}
				break;
			case 1:
			{
				//if(((float)rand()/(float)RAND_MAX) > _genParams._connectivity)
				if (genRandom() > _genParams._connectivity)
					break;

				// road intersection
				//NodeInterface *cursorNode = createNode(newPoint);
				NodeInterface *cursorNode = new SimpleNode(_roadGraph);
				NodeId cursorNodeId = _roadGraph.addNode(cursorNode);
				cursorNode->_nodeId = cursorNodeId;
				createRoad(currentNode, cursorNode);
				roadCount++;

				// get intersected source src and dst
				RoadInterface *ri = _roadGraph.getRoad(rd);
				NodeId srcNodeId = _roadGraph.getSrc(rd);
				NodeId dstNodeId = _roadGraph.getDst(rd);

				//if road is a boundary road
				if (typeid(*ri) == typeid(WorldRoad))
				{
					cursorNode->setPosition3D(newPoint.x, newPoint.y, newPoint.z);
					// remove road segment from graph
					_roadGraph.removeRoad(srcNodeId, dstNodeId);

					// create replacement segments
					RoadId rd;
					if (_roadGraph.addRoad(srcNodeId, cursorNodeId, rd))
						_roadGraph.setRoad(rd, ri);
					if (_roadGraph.addRoad(cursorNodeId, dstNodeId, rd))
						_roadGraph.setRoad(rd, ri);
				}
				else
				{
					cursorNode->setPosition2D(newPoint.x, newPoint.z);
					// delete the road rd
					deleteRoad(ri);

					// reconstruct road in the form of two segments
					createRoad(_roadGraph.getNode(srcNodeId), cursorNode);
					createRoad(cursorNode, _roadGraph.getNode(dstNodeId));
				}
			}
				break;
			case 2:
				// node snap
				//if(((float)rand()/(float)RAND_MAX) > _genParams._connectivity)
				if (genRandom() > _genParams._connectivity)
					break;
				// MMM: dont snap to your self ass monkey
				if (currentNode != _roadGraph.getNode(nd))
				{
					createRoad(currentNode, _roadGraph.getNode(nd));
					roadCount++;
				}
				break;

			}
		}
	}
}

void WorldCell::prebuild()
{
	if(_displayMode >= view_cell) prebuildRoads();
	if(_displayMode >= view_box) prebuildBuildings();
}


void WorldCell::prebuildRoads()
{
	// Define a random number generator and init with a reproducible seed.
	base_generator_type generator(_genParams._seed);

	// Define a uniform random number distribution which produces "double"
	// values between 0 and 1 (0 inclusive, 1 exclusive).
	boost::uniform_real<> uni_dist(0, 1);
	rando rg(generator, uni_dist);

	clearRoadGraph();
	installGraph();

	//PerformanceTimer gpt("Generation_"+StringConverter::toString((int)this));
	generateRoadNetwork(rg);
	//gpt.stop();
	//LogManager::getSingleton().logMessage(gpt.toString());

	NodeIterator nIt, nEnd;
	for ( boost::tie(nIt, nEnd) = _roadGraph.getNodes(); nIt != nEnd; nIt++)
	{
		NodeInterface* ni = _roadGraph.getNode(*nIt);
		if (typeid(*ni) == typeid(SimpleNode))
		{
			static_cast<SimpleNode*>(ni)->prebuild();
			//static_cast<SimpleNode*>(ni)->build(roadBuilder, mat);
		}
	}

	RoadIterator rIt, rEnd;
	for ( boost::tie(rIt, rEnd) = _roadGraph.getRoads(); rIt != rEnd; rIt++)
	{
		RoadInterface* ri = _roadGraph.getRoad(*rIt);
		if (typeid(*ri) == typeid(SimpleRoad))
		{
			static_cast<SimpleRoad*>(ri)->prebuild();
			//static_cast<SimpleRoad*>(ri)->build(roadBuilder, mat2);
		}
	}
}

void WorldCell::prebuildBuildings()
{
	// Define a random number generator and init with a reproducible seed.
	base_generator_type generator(_genParams._seed);

	// Define a uniform random number distribution which produces "double"
	// values between 0 and 1 (0 inclusive, 1 exclusive).
	boost::uniform_real<> uni_dist(0, 1);
	rando rg(generator, uni_dist);

	// blocks
	_mbBuildings = new MeshBuilder(_name+"Buildings", "custom", this);

	// set up materials
	vector<Material*> materials(6);
	materials[0] = static_cast<MaterialPtr>(MaterialManager::getSingleton().getByName("gk/Building1WNormalMap")).get();
	materials[1] = static_cast<MaterialPtr>(MaterialManager::getSingleton().getByName("gk/Building2WNormalMap")).get();
	materials[2] = static_cast<MaterialPtr>(MaterialManager::getSingleton().getByName("gk/Building3WNormalMap")).get();
	materials[3] = static_cast<MaterialPtr>(MaterialManager::getSingleton().getByName("gk/Building4WNormalMap")).get();
	materials[4] = static_cast<MaterialPtr>(MaterialManager::getSingleton().getByName("gk/Building5WNormalMap")).get();
	materials[5] = static_cast<MaterialPtr>(MaterialManager::getSingleton().getByName("gk/Paving")).get();

	vector< vector<NodeInterface*> > cycles;
	_roadGraph.extractFootprints(cycles, _genParams._lotWidth);

	BOOST_FOREACH(vector<NodeInterface*> &cycle, cycles)
	{
		vector<Vector3> poly;
		extractPolygon(cycle, poly);

		if(_genParams._debug)
			_blocks.push_back(new WorldBlock(poly, _genParams, rg, _mbBuildings, materials, true));
		else
			_blocks.push_back(new WorldBlock(poly, _genParams, rg, _mbBuildings, materials));
	}
}

void WorldCell::build()
{
	destroySceneObject();
	if(_displayMode >= view_cell) buildRoads();
	if(_displayMode >= view_box) buildBuildings();
}

void WorldCell::buildRoads()
{
	//1. destroy scene objects

	// build road junctions
	Material* mat = static_cast<MaterialPtr>(MaterialManager::getSingleton().getByName("gk/RoadJunction")).get();
	Material* mat2 = static_cast<MaterialPtr>(MaterialManager::getSingleton().getByName("gk/Road")).get();
	MeshBuilder roadBuilder(_name+"Roads", "custom", this);
	NodeIterator nIt, nEnd;
	for ( boost::tie(nIt, nEnd) = _roadGraph.getNodes(); nIt != nEnd; nIt++)
	{
		NodeInterface* ni = _roadGraph.getNode(*nIt);
		if (typeid(*ni) == typeid(SimpleNode))
			static_cast<SimpleNode*>(ni)->build(roadBuilder, mat);
	}

	// build road segments
	RoadIterator rIt, rEnd;
	for ( boost::tie(rIt, rEnd) = _roadGraph.getRoads(); rIt != rEnd; rIt++)
	{
		RoadInterface* ri = _roadGraph.getRoad(*rIt);
		if (typeid(*ri) == typeid(SimpleRoad))
			static_cast<SimpleRoad*>(ri)->build(roadBuilder, mat2);
	}

	// create entity for road network
	roadBuilder.build();
	_roadsEnt = _sceneNode->getCreator()->createEntity(_name+"RoadsEnt", roadBuilder.getName());
	_sceneNode->attachObject(_roadsEnt);

}

void WorldCell::buildBuildings()
{
	// create ogre entity using mesh builder
	_mbBuildings->build();
	_buildingsEnt = _sceneNode->getCreator()->createEntity(_name+"BuildsEnt", _mbBuildings->getName());
	_sceneNode->attachObject(_buildingsEnt);

	delete _mbBuildings;
	_mbBuildings = 0;
	
	// am done with blocks now.
	BOOST_FOREACH(WorldBlock* b, _blocks) delete b;
	_blocks.clear();
}

bool WorldCell::isInside(const Ogre::Vector2 &loc) const
{
	bool rayCross = false;
	BOOST_FOREACH(RoadInterface* ri, _boundaryRoads)
if	(ri->rayCross(loc)) rayCross = !rayCross;
	return rayCross;
}

RoadInterface* WorldCell::getLongestBoundaryRoad() const
{
	RoadInterface* longest = 0;
	Ogre::Real length = 0;
	BOOST_FOREACH(RoadInterface* ri, _boundaryRoads)
{	if(longest == 0 || ri->getLengthSquared() > length)
	{
		longest = ri;
		length = longest->getLengthSquared();
	}
}
return longest;
}

void WorldCell::installGraph()
{
	// node map used to only insert nodes once
	map<NodeInterface*, NodeInterface*> nodeMap;

	// install nodes, road and filaments to map
	BOOST_FOREACH(NodeInterface* ni, _boundaryCycle)
		nodeMap[ni] = createNode(ni->getPosition3D());
	BOOST_FOREACH(RoadInterface* ri, _boundaryRoads)
		installRoad(ri, nodeMap);
	BOOST_FOREACH(RoadInterface* ri, _filamentRoads)
		installRoad(ri, nodeMap);
}

void WorldCell::installRoad(RoadInterface* ri,
		map<NodeInterface*, NodeInterface*> &nodeMap)
{
	//create the road node
	assert(typeid(*ri) == typeid(WorldRoad));
	BOOST_FOREACH(RoadId rSegId, static_cast<WorldRoad*>(ri)->getRoadSegmentList())
	{
		// check we have the nodes, if not add them
		NodeInterface *srcNd = _parentRoadGraph.getSrcNode(rSegId);
		NodeInterface *dstNd = _parentRoadGraph.getDstNode(rSegId);
		if(nodeMap.find(srcNd) == nodeMap.end())
			nodeMap[srcNd] = createNode(srcNd->getPosition3D());
		if(nodeMap.find(dstNd) == nodeMap.end())
			nodeMap[dstNd] = createNode(dstNd->getPosition3D());

		// create the road seg
		//createRoad(nodeMap[srcNd], nodeMap[dstNd]);
		RoadId rd;
		if(_roadGraph.addRoad(nodeMap[srcNd]->_nodeId, nodeMap[dstNd]->_nodeId, rd))
		{
			_roadGraph.setRoad(rd, ri);
		}
		else
			throw Exception(Exception::ERR_ITEM_NOT_FOUND, "Road not Installed", "WorldCell::installRoad");
	}
}

void WorldCell::setBoundary(const vector<NodeInterface*> &nodeCycle)
{
	clearRoadGraph();
	clearBoundary();
	_boundaryCycle = nodeCycle;

	size_t i, j, N = _boundaryCycle.size();
	for (i=0; i<N; i++)
	{
		j = (i+1) % N;

		// This is messy, but fuck it i don't have the time
		WorldNode* wn1 = static_cast<WorldNode*>(_boundaryCycle[i]);
		WorldNode* wn2 = static_cast<WorldNode*>(_boundaryCycle[j]);

		RoadId rd = _simpleRoadGraph.getRoadId(wn1->mSimpleNodeId,
				wn2->mSimpleNodeId);
		_boundaryRoads.push_back(_simpleRoadGraph.getRoad(rd));
	}

	// set up listeners to receive invalidate events from roads
	vector<RoadInterface*>::iterator rIt, rEnd;
	for (rIt = _boundaryRoads.begin(), rEnd = _boundaryRoads.end(); rIt != rEnd; rIt++)
	{
		if (typeid(*(*rIt)) == typeid(WorldRoad))
		{
			static_cast<WorldRoad*>(*rIt)->attach(this);
		}
	}
	invalidate();
}

void WorldCell::addFilament(WorldRoad* f)
{
	f->attach(this);
	_filamentRoads.push_back(f);
	invalidate();
}

void WorldCell::removeFilament(WorldRoad* f)
{
	vector<RoadInterface*>::iterator rIt, rEnd;
	for (rIt = _filamentRoads.begin(), rEnd = _filamentRoads.end(); rIt != rEnd; rIt++)
	{
		assert(typeid(*(*rIt)) == typeid(WorldRoad));
		WorldRoad* wr = static_cast<WorldRoad*>(*rIt);
		if (wr == f)
		{
			wr->detach(this);
			_filamentRoads.erase(rIt);
			break;
		}
	}
	invalidate();
}

// TODO: I really need to get rid of this boundary road crap
void WorldCell::clearBoundary()
{
	BOOST_FOREACH(RoadInterface* ri, _boundaryRoads)
{
		if(typeid(*ri) == typeid(WorldRoad))
			static_cast<WorldRoad*>(ri)->detach(this);
	}
		_boundaryRoads.clear();
	invalidate();
}

void WorldCell::clearFilaments()
{
	vector<RoadInterface*>::iterator rIt, rEnd;
	for (rIt = _filamentRoads.begin(), rEnd = _filamentRoads.end(); rIt != rEnd; rIt++)
	{
		if (typeid(*(*rIt)) == typeid(WorldRoad))
			static_cast<WorldRoad*>(*rIt)->detach(this);
	}
	_filamentRoads.clear();
	invalidate();
}

CellParams WorldCell::getGenParams() const
{
	return _genParams;
}

CellParams WorldCell::getDefaultGenParams()
{
	return _defaultGenParams;
}

void WorldCell::setGenParams(const CellParams &g)
{
	_genParams = g;
	invalidate();
}

void WorldCell::setDefaultGenParams(const CellParams &g)
{
	_defaultGenParams = g;
}

const vector<RoadInterface*>& WorldCell::getBoundaryRoads() const
{
	return _boundaryRoads;
}

const vector<NodeInterface*>& WorldCell::getBoundaryCycle() const
{
	return _boundaryCycle;
}

const vector<RoadInterface*>& WorldCell::getFilaments() const
{
	return _filamentRoads;
}

NodeInterface* WorldCell::createNode(const Vector2 &pos)
{
	SimpleNode *sn = new SimpleNode(_roadGraph, pos);
	NodeId nd = _roadGraph.addNode(sn);
	sn->_nodeId = nd;
	return sn;
}

NodeInterface* WorldCell::createNode(const Vector3 &pos)
{
	SimpleNode *sn = new SimpleNode(_roadGraph, pos);
	NodeId nd = _roadGraph.addNode(sn);
	sn->_nodeId = nd;
	return sn;
}

RoadInterface* WorldCell::createRoad(NodeInterface *n1, NodeInterface *n2)
{
	RoadId rd;
	if (_roadGraph.addRoad(n1->_nodeId, n2->_nodeId, rd))
	{
		SimpleRoad *sr = new SimpleRoad(n1, n2, rd);
		sr->setWidth(_genParams._roadWidth);
		_roadGraph.setRoad(rd, sr);
		return sr;
	}
	else
		return 0;
}

void WorldCell::deleteRoad(RoadInterface *ri)
{
	// remove it from the graph
	_roadGraph.removeRoad(ri->getSrcNode()->_nodeId, ri->getDstNode()->_nodeId);
	// delete it
	delete ri;
}

bool WorldCell::isBoundaryNode(const NodeInterface *ni) const
{
	BOOST_FOREACH(NodeInterface* n, _boundaryCycle)
		if	(n==ni) return true;
	return false;
}

bool WorldCell::compareBoundary(const vector<NodeInterface*>& nodeCycle) const
{
	// compare size
	if (nodeCycle.size() != _boundaryCycle.size())
		return false;

	// find match
	size_t i, j, offset, N = nodeCycle.size();
	for (i = 0; i < N; i++)
	{
		if (nodeCycle[0] == _boundaryCycle[i])
			break;
	}
	if (i >= N)
		return false;
	else
		offset = i;

	// try forwards
	for (i = 1; i < N; i++)
	{
		if (nodeCycle[i] != _boundaryCycle[(i + offset) % N])
			break;
	}
	if (i >= N)
		return true;

	// try backwards
	for (i = 1, j = offset-1; i < N; i++, j--)
	{
		if (nodeCycle[i] != _boundaryCycle[j % N])
			return false;
	}
	return true;
}

RoadInterface* WorldCell::getRoad(NodeInterface* n1, NodeInterface* n2)
{
	return _roadGraph.getRoad(_roadGraph.getRoadId(n1->_nodeId, n2->_nodeId));
}
void WorldCell::setSelected(bool show)
{
	std::vector<RoadInterface*>::iterator bIt, bEnd;
	for (bIt = _boundaryRoads.begin(), bEnd = _boundaryRoads.end(); bIt != bEnd; bIt++)
	{
		if (typeid(*(*bIt)) == typeid(WorldRoad))
			static_cast<WorldRoad*>(*bIt)->setSelected(show);
	}
}

void WorldCell::setHighlighted(bool show)
{
	std::vector<RoadInterface*>::iterator bIt, bEnd;
	for (bIt = _boundaryRoads.begin(), bEnd = _boundaryRoads.end(); bIt != bEnd; bIt++)
	{
		if (typeid(*(*bIt)) == typeid(WorldRoad))
			static_cast<WorldRoad*>(*bIt)->setHighlighted(show);
	}
}

bool WorldCell::loadXML(const TiXmlHandle& cellRoot)
{
	// Load GenParams, not Smart Enough to do our own boundary or filaments, see parent
	{
		TiXmlElement *element = cellRoot.FirstChild("genparams").FirstChild().Element();

		for (; element; element=element->NextSiblingElement())
		{
			string key = element->Value();

			if (key == "seed")
				element->QueryIntAttribute("value", &_genParams._seed);
			else if (key == "type")
			{
				int t;
				element->QueryIntAttribute("value", &t);
				_genParams._type = static_cast<unsigned int>(t);
			}
			else if (key == "segmentSize")
				element->QueryFloatAttribute("value", &_genParams._segmentSize);
			else if (key == "segmentDeviance")
				element->QueryFloatAttribute("value",
						&_genParams._segmentDeviance);
			else if (key == "degree")
			{
				int deg;
				element->QueryIntAttribute("value", &deg);
				_genParams._degree = static_cast<unsigned int>(deg);
			}
			else if (key == "degreeDeviance")
				element->QueryFloatAttribute("value",
						&_genParams._degreeDeviance);
			else if (key == "snapSize")
				element->QueryFloatAttribute("value", &_genParams._snapSize);
			else if (key == "snapDeviance")
				element->QueryFloatAttribute("value", &_genParams._snapDeviance);
			else if (key == "roadWidth")
				element->QueryFloatAttribute("value", &_genParams._roadWidth);
			else if (key == "buildingHeight")
				element->QueryFloatAttribute("value",
						&_genParams._buildingHeight);
			else if (key == "buildingDeviance")
				element->QueryFloatAttribute("value",
						&_genParams._buildingDeviance);
			else if (key == "connectivity")
				element->QueryFloatAttribute("value", &_genParams._connectivity);
			else if (key == "lotWidth")
				element->QueryFloatAttribute("value", &_genParams._lotWidth);
			else if (key == "lotDepth")
				element->QueryFloatAttribute("value", &_genParams._lotDepth);
			else if (key == "lotDeviance")
				element->QueryFloatAttribute("value", &_genParams._lotDeviance);
		}
	}
	return true;
}

//TODO: put this somewhere relevant
TiXmlElement* addNewElement(TiXmlElement *root, const char *s)
{
	TiXmlElement *elem = new TiXmlElement(s);
	root->LinkEndChild(elem);
	return elem;
}

TiXmlElement* WorldCell::saveXML()
{
	TiXmlElement *root = new TiXmlElement("cell");

	// Save Cycle
	TiXmlElement *cycle = addNewElement(root, "cycle");
	BOOST_FOREACH(NodeInterface* ni, _boundaryCycle)
addNewElement(cycle, "node")->SetAttribute("id", (int)ni);

	// Save Filaments
		TiXmlElement *filaments = addNewElement(root, "filaments");
	BOOST_FOREACH(RoadInterface* ri, _filamentRoads)
{
		TiXmlElement *filament = addNewElement(filaments, "filament");
		addNewElement(filament, "node")->SetAttribute("id", (int)ri->getSrcNode());
		addNewElement(filament, "node")->SetAttribute("id", (int)ri->getDstNode());
	}

	// Save Params
		TiXmlElement *gp = addNewElement(root, "genparams");
	addNewElement(gp, "seed")->SetAttribute("value", _genParams._seed);
	addNewElement(gp, "type")->SetAttribute("value", _genParams._type);
	addNewElement(gp, "segmentSize")->SetDoubleAttribute("value",
			_genParams._segmentSize);
	addNewElement(gp, "segmentDeviance")->SetDoubleAttribute("value",
			_genParams._segmentDeviance);
	addNewElement(gp, "degree")->SetAttribute("value", _genParams._degree);
	addNewElement(gp, "degreeDeviance")->SetDoubleAttribute("value",
			_genParams._degreeDeviance);
	addNewElement(gp, "snapSize")->SetDoubleAttribute("value", _genParams._snapSize);
	addNewElement(gp, "snapDeviance")->SetDoubleAttribute("value",
			_genParams._snapDeviance);
	addNewElement(gp, "roadWidth")->SetDoubleAttribute("value", _genParams._roadWidth);
	addNewElement(gp, "buildingHeight")->SetDoubleAttribute("value",
			_genParams._buildingHeight);
	addNewElement(gp, "buildingDeviance")->SetDoubleAttribute("value",
			_genParams._buildingDeviance);
	addNewElement(gp, "connectivity")->SetDoubleAttribute("value",
			_genParams._connectivity);
	addNewElement(gp, "lotWidth")->SetDoubleAttribute("value", _genParams._lotWidth);
	addNewElement(gp, "lotDepth")->SetDoubleAttribute("value", _genParams._lotDepth);
	addNewElement(gp, "lotDeviance")->SetDoubleAttribute("value",
			_genParams._lotDeviance);

	return root;
}

vector<Vector3> WorldCell::getBoundaryPoints3D()
{
	vector<Vector3> pointList;
	pointList.reserve(_boundaryCycle.size());
	BOOST_FOREACH(NodeInterface* ni, _boundaryCycle)
pointList	.push_back(ni->getPosition3D());
	return pointList;
}

vector<Vector2> WorldCell::getBoundaryPoints2D()
{
	vector<Vector2> pointList;
	pointList.reserve(_boundaryCycle.size());
	BOOST_FOREACH(NodeInterface* ni, _boundaryCycle)
pointList	.push_back(ni->getPosition2D());
	return pointList;
}

Real WorldCell::calcArea2D()
{
	return Geometry::polygonArea(getBoundaryPoints2D());
}

void WorldCell::extractPolygon(vector<NodeInterface*> &cycle,
		vector<Vector3> &poly)
{
	size_t i, j, N = cycle.size();
	vector<Real> insets;
	poly.reserve(cycle.size());
	insets.reserve(cycle.size());

	for (i=0; i<N; i++)
	{
		j = (i+1)%N;
		poly.push_back(cycle[i]->getPosition3D());
		insets.push_back(_roadGraph.getRoad(_roadGraph.getRoadId(cycle[i]->_nodeId, cycle[j]->_nodeId))->getWidth());
	}
	Geometry::polygonInset(insets, poly);
}

void WorldCell::exportObject(ExportDoc &doc)
{
	doc.addMesh(_buildingsEnt->getMesh());
	doc.addMesh(_roadsEnt->getMesh());
}

void WorldCell::setDisplayMode(Display mode)
{
	// build and show what is necessary, hide what is not
	switch(mode)
	{
	case view_primary:
		if(_roadsEnt) _roadsEnt->setVisible(false);
		if(_buildingsEnt) _buildingsEnt->setVisible(false);
		break;
	case view_cell:
		if(_roadsEnt == 0)
		{
			prebuildRoads();
			buildRoads();
		}
		else _roadsEnt->setVisible(true);
		if(_buildingsEnt) _buildingsEnt->setVisible(false);
		break;
	case view_box:
	case view_building:
		if(_roadsEnt == 0)
		{
			prebuildRoads();
			buildRoads();
		}
		else _roadsEnt->setVisible(true);
		if(_buildingsEnt == 0)
		{
			prebuildBuildings();
			buildBuildings();
		}
		else _buildingsEnt->setVisible(true);
		break;
	}
	_displayMode = mode;
}

