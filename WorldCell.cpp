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
	70,		// segmentSize
	0.02,	// segmentDeviance
	4,		// degree
	0.0,	// degreeDeviance
	2.1,	// aspect
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
	40,		// segmentSize
	0.2,	// segmentDeviance
	4,		// degree
	0.05,	// degreeDeviance
	1.2,	// aspect
	35,		// snapSize
	0.1,	// snapDeviance
	6,		// buildingHeight
	0.3,	// buildingDeviance
	3.5,	// roadWidth
	0,		// roadLimit
	0.15,	// connectivity
	2,		// footpathWidth;
	0.28,	// footpathHeight;
	24.0,	// lotWidth
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
	1.0,	// aspect
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

NodeInterface* WorldCell::placeSegment(rando &genRandom, NodeInterface* currentNode, Vector2 &cursor, size_t &roadCount)
{
	RoadId rd;
	NodeId nd;
	Vector3 newPoint;

	// 6600 vs. 8600 debug
	// 94 vs. 109 release
	switch (_roadGraph.snapInfo(currentNode->_nodeId, cursor,
			_genParams._snapSize, newPoint, nd, rd))
	{
	case 0:
		// no intersection found
		{
			NodeInterface *cursorNode = createNode(cursor);
			createRoad(currentNode, cursorNode);
			roadCount++;
			return cursorNode;
			//q.push(make_pair<NodeInterface*, Vector2>(cursorNode,
			//		currentDirection)); // enqueue
		}
	case 1:
		{
			//if(((float)rand()/(float)RAND_MAX) > _genParams._connectivity)
			if (genRandom() > _genParams._connectivity)
				return 0;

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
			break;
		}	
	case 2:
		// node snap
		//if(((float)rand()/(float)RAND_MAX) > _genParams._connectivity)
		if (genRandom() > _genParams._connectivity)
			return 0;
		// MMM: dont snap to your self ass monkey
		if (currentNode != _roadGraph.getNode(nd))
		{
			createRoad(currentNode, _roadGraph.getNode(nd));
			roadCount++;
		}
		break;
	}
	return 0;
}

void WorldCell::generateRoadNetwork(rando genRandom)
{
	queue< pair<NodeInterface*, Vector2> > q;
	size_t roadCount = 0;
	//return;

	// TODO: tidy me up
	bool centreSuccess = false;
/*	if (_genParams._connectivity >= 1.0f)
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
			direction = direction.perpendicular() * _genParams._segmentSize;

			q.push(make_pair<NodeInterface*, Vector2>(startNode, direction));
			centreSuccess = true;
		}
	}*/

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

		//
		size_t numberOfRoadsToStartFrom = boundaryRoadsN/2;
		if(_genParams._connectivity >= 1.0f) numberOfRoadsToStartFrom = 1;

		// for half of the roads, the largest ones
		for (size_t i=0; i<numberOfRoadsToStartFrom; i++)
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
			Vector2 pos2 = pos1 + (roadDir * _genParams._segmentSize*2.1);
			pos1 -= (roadDir * _genParams._segmentSize);

			NodeInterface* currentNode = 0;
			if (!isInside(pos2))
				continue;
			currentNode = createNode(pos2);
/*
			NodeInterface* ni = placeSegment(genRandom, currentNode, pos1, roadCount);
			if(ni)
			{
				RoadId rd = _roadGraph.getRoadId(currentNode->_nodeId, ni->_nodeId);
				RoadInterface* ri = _roadGraph.getRoad(rd);
				delete ri;
				_roadGraph.removeRoad(rd);
				NodeId nd = ni->_nodeId;
				delete ni;
				_roadGraph.removeNode(nd);
			}
			else
				q.push(make_pair<NodeInterface*, Vector2>(currentNode, roadDir));
*/
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

	while (!q.empty())
	{
		NodeInterface* currentNode;
		Vector2 currentDirection;
		boost::tie(currentNode, currentDirection) = q.front();
		q.pop();

		//Ogre::Radian theta(Math::TWO_PI / (degBase + (degDev * ((float)rand()/(float)RAND_MAX))));
		Ogre::Radian theta(Math::TWO_PI / (degBase + (degDev * genRandom())));

		Vector2 originalDirection(currentDirection);

		// alter our direction vector 
		for (unsigned int i=0; i < _genParams._degree; i++)
		{
			if (!_genParams._mcbDebug && _genParams._roadLimit != 0 && 
				roadCount >= _genParams._roadLimit)
			//if(roadCount++ >= 29)
			{
				while (!q.empty())
					q.pop();
				break;
			}

			if (roadCount == 42)
			{
				size_t z = 0;
			}

			// get a candidate
			
			Geometry::rotate(currentDirection, theta);
			// doesn't work grrrrrrrrrr

			currentDirection.normalise();
			//Real segSz = (segSzBase + (segDevSz *  ((float)rand()/(float)RAND_MAX)));
			Real segSz= (segSzBase + (segDevSz * genRandom()));
			if(_genParams._degree == 4 && (i==1 || i==3)) segSz *= _genParams._aspect;
			currentDirection *= segSz;
			Vector2 cursor(currentDirection + currentNode->getPosition2D());

			if (_genParams._roadLimit != 0 && 
				roadCount >= (_genParams._roadLimit - 1))
			{
				RoadId rd;
				NodeId nd;
				Vector3 newPoint;
				int snapState = _roadGraph.snapInfo(currentNode->_nodeId, cursor,
					_genParams._snapSize, newPoint, nd, rd);
				LogManager::getSingleton().logMessage("I got: "+StringConverter::toString(snapState));
			}
			NodeInterface* ni = placeSegment(genRandom, currentNode, cursor, roadCount);
			if(ni) q.push(make_pair<NodeInterface*, Vector2>(ni, originalDirection));
		}
	}
}

void WorldCell::prebuild()
{
	if(_displayMode >= view_cell) prebuildRoads();
	if(_displayMode >= view_box) prebuildBuildings();
}

void WorldCell::prebuild1()
{
	if(_displayMode >= view_cell) prebuildRoads();
}

void WorldCell::prebuild2()
{
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
	materials[0] = static_cast<MaterialPtr>(MaterialManager::getSingleton().getByName("gk/Building1WRelief")).get();
	materials[1] = static_cast<MaterialPtr>(MaterialManager::getSingleton().getByName("gk/Building2WRelief")).get();
	materials[2] = static_cast<MaterialPtr>(MaterialManager::getSingleton().getByName("gk/Building3WRelief")).get();
	materials[3] = static_cast<MaterialPtr>(MaterialManager::getSingleton().getByName("gk/Building4WRelief")).get();
	materials[4] = static_cast<MaterialPtr>(MaterialManager::getSingleton().getByName("gk/Building5WNormalMap")).get();
	materials[5] = static_cast<MaterialPtr>(MaterialManager::getSingleton().getByName("gk/Paving")).get();

	vector< vector<NodeInterface*> > cycles;
	_roadGraph.extractEnclosedRegions(cycles,10000);

	size_t blockErrors = 0;
	_blocks.reserve(cycles.size());
	BOOST_FOREACH(vector<NodeInterface*> &cycle, cycles)
	{
		vector<Vector3> poly;
		if(extractPolygon(cycle, poly))
		{
			WorldBlock* b = new WorldBlock(poly, _genParams, rg, _mbBuildings, materials, _genParams._debug);
			if(b->_error) 
			{
				blockErrors++;
				delete b;
			}
			else _blocks.push_back(b);
		}
		else blockErrors++;
	}
	//if(blockErrors > 0) LogManager::getSingleton().logMessage(_name+"\tblock error count\t"+StringConverter::toString(blockErrors));
}

void WorldCell::build()
{
	destroySceneObject();
	if(_displayMode >= view_cell) buildRoads();
	if(_displayMode >= view_box) buildBuildings();
	if(_genParams._debug) buildDebugOverlay();
	//if(_genParams._mcbDebug && _debugMO) _sceneNode->attachObject(_debugMO);
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

	// install boundary roads and filaments to map
	BOOST_FOREACH(RoadInterface* ri, _boundaryRoads) installRoad(ri, nodeMap);
	BOOST_FOREACH(RoadInterface* ri, _filamentRoads) installRoad(ri, nodeMap);
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
		RoadId rd;
		if(_roadGraph.addRoad(nodeMap[srcNd]->_nodeId, nodeMap[dstNd]->_nodeId, rd))
			_roadGraph.setRoad(rd, ri);
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
			else if (key == "aspect")
				element->QueryFloatAttribute("value", &_genParams._aspect);
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
	addNewElement(gp, "aspect")->SetDoubleAttribute("value",
		_genParams._aspect);
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

void WorldCell::constructSLAV(const vector<NodeInterface*> &cycle, SLAV &sv)
{
	size_t i,j,N = cycle.size();
	assert(N >= 3);
	Real lastInset = _roadGraph.getRoad(_roadGraph.getRoadId(cycle[N-1]->_nodeId, cycle[0]->_nodeId))->getWidth();
	size_t lastI = N-1;
	for(i=0; i<N; i++)
	{
		j = (i+1) % N;
		InsetVertex *iv = new InsetVertex();
		iv->_intersectionTested = false;
		iv->_inset = _roadGraph.getRoad(_roadGraph.getRoadId(cycle[i]->_nodeId, cycle[j]->_nodeId))->getWidth();

		// a special case is required for the end node of filaments
		if(cycle[i]->getDegree()==1)
		{	
			// create two vertices in place of one point to create a cap for the segment end
			Vector2 pos2D(cycle[i]->getPosition2D());
			Vector3 pos3D(cycle[i]->getPosition3D());
			Vector2 segmentVec = pos2D -  cycle[lastI]->getPosition2D();
			segmentVec.normalise();
			Vector2 segmentPerp = segmentVec.perpendicular();
			
			// Note: Two different approaches are possible to creating the inset vectors
			// a: create both points with the target = position and do not test
			// b: move pos slightly so that intersection is not detected
			// -- i've chosen b, which is slighty more expensive but less problematic later
			segmentVec *= iv->_inset;
			segmentPerp *= iv->_inset;
			iv->_insetTarget = pos2D + segmentVec + segmentPerp;
			iv->_pos.x = pos3D.x + (0.001) * segmentPerp.x;
			iv->_pos.y = pos3D.y;
			iv->_pos.z = pos3D.z + (0.001) * segmentPerp.y;
			sv.add(iv);
			
			InsetVertex *iv2 = new InsetVertex();
			iv2->_intersectionTested = false;
			iv2->_inset = iv->_inset;
			iv2->_insetTarget = pos2D + segmentVec - segmentPerp;
			iv2->_pos.x = pos3D.x - (0.001) * segmentPerp.x;
			iv2->_pos.y = pos3D.y;
			iv2->_pos.z = pos3D.z - (0.001) * segmentPerp.y;
			sv.add(iv2);
		}
		else
		{
			iv->_insetTarget = Geometry::calcInsetTarget(cycle[lastI]->getPosition3D(), cycle[i]->getPosition3D(), 
				cycle[j]->getPosition3D(), lastInset, iv->_inset);
			iv->_pos = cycle[i]->getPosition3D();
			sv.add(iv);
		}
		lastI = i;
		lastInset = iv->_inset;
	}
}

bool WorldCell::extractPolygon(vector<NodeInterface*> &cycle,
		vector<Vector3> &poly)
{
	SLAV sv;
	constructSLAV(cycle, sv);
	return Skeletor::processInset(sv, poly);
}

void WorldCell::buildDebugOverlay()
{

	// vis
	queue<vector<pair<Vector3, Vector3> > > insetPairsSet;

	if(_debugMO)
	{
		_sceneNode->detachObject(_debugMO);
		delete _debugMO;
	}
	_debugMO = new ManualObject(_name+"Debug");

	// extract cycles
	vector< vector<NodeInterface*> > cycles;
	_roadGraph.extractEnclosedRegions(cycles, 10000);

	BOOST_FOREACH(vector<NodeInterface*> &cycle, cycles)
	{
		// inset data
		vector< vector<Vector3> > polysOut;
		SLAV sv;
		constructSLAV(cycle, sv);

		while(true)
		{
			///////////////////////////////////////////////////////////////////
			// 1. Intersection test for all inset vectors
			///////////////////////////////////////////////////////////////////
			
			// find the earliest intersection
			Real intersectionLocation = 1;
			Vector2 intersection(Vector2::ZERO);
			InsetVertex *iv,*firstOffender=0;

			iv = sv.getRoot();
			do
			{
				if(!iv->_intersectionTested)
				{
					Vector2 ivPos2D(iv->_pos.x, iv->_pos.z);
					Vector2 nextIvPos2D(iv->_right->_pos.x, iv->_right->_pos.z);

					// check if the pair intersect and store the lowest
					Real r;
					Vector2 tmpInscn;
					if(Geometry::lineIntersect(ivPos2D, iv->_insetTarget, 
							nextIvPos2D, iv->_right->_insetTarget, tmpInscn, r) &&
							r >= 0 && r <= 1)
					{
						// TODO: tolerance value could be used here
						if(r < intersectionLocation) 
						{
							intersectionLocation = r;
							firstOffender = iv;
							intersection = tmpInscn;
						}
					}
					else
						iv->_intersectionTested = true;
				}
				iv = iv->_right;
			}
			while(iv != sv.getRoot());
			
			///////////////////////////////////////////////////////////////////
			// 2. Process Bisector Intersection
			///////////////////////////////////////////////////////////////////
			
			// find the closest intersection
			if(intersectionLocation != 1.0)
			{
// DIRTY VIS BASTARD
				{
					vector< pair<Vector3, Vector3> > insetPairs;
					iv = sv.getRoot();
					do
					{
						pair<Vector3, Vector3> insetPair;
						insetPair.first = iv->_pos;
						insetPair.first.y += 0.1;
						insetPair.second.x = iv->_pos.x + intersectionLocation * (iv->_insetTarget.x - iv->_pos.x);
						insetPair.second.y = iv->_pos.y + 0.1;
						insetPair.second.z = iv->_pos.z + intersectionLocation * (iv->_insetTarget.y - iv->_pos.z);
						insetPairs.push_back(insetPair);
						iv = iv->_right;
					}
					while(iv != sv.getRoot());
					insetPairsSet.push(insetPairs);
				}
// end
				// remove the first offender
				InsetVertex *secondOffender = firstOffender->_right;
				sv.remove(firstOffender);

				// if there is a valid polygon remaining
				if(sv.getSize() >= 3)
				{
					// update the pos and inset of the remaining vertices
					iv = sv.getRoot();
					do
					{
						iv->_pos.x = iv->_pos.x + intersectionLocation * (iv->_insetTarget.x - iv->_pos.x);
						iv->_pos.z = iv->_pos.z + intersectionLocation * (iv->_insetTarget.y - iv->_pos.z);
						iv->_inset = iv->_inset * (1 - intersectionLocation);
						iv = iv->_right;
					}
					while(iv != sv.getRoot());
					
					// update the second offender
					
					//LogManager::getSingleton().logMessage("Int:"+StringConverter::toString(intersection));
					secondOffender->_pos.x = intersection.x;
					secondOffender->_pos.z = intersection.y;
					secondOffender->_insetTarget = Geometry::calcInsetTarget(secondOffender->_left->_pos, secondOffender->_pos, 
							secondOffender->_right->_pos, secondOffender->_left->_inset, secondOffender->_inset);

					secondOffender->_intersectionTested = false;
					secondOffender->_left->_intersectionTested = false;
				}
				else
				{
					//LogManager::getSingleton().logMessage("Less than 3 vertices after collapse.");
					continue;
				}
			}
			else
			{
// DIRTY VIS BASTARD
				{
					vector< pair<Vector3, Vector3> > insetPairs;
					iv = sv.getRoot();
					do
					{
						pair<Vector3, Vector3> insetPair;
						insetPair.first = iv->_pos;
						insetPair.second.x = iv->_pos.x + intersectionLocation * (iv->_insetTarget.x - iv->_pos.x);
						insetPair.second.y = iv->_pos.y;
						insetPair.second.z = iv->_pos.z + intersectionLocation * (iv->_insetTarget.y - iv->_pos.z);
						insetPairs.push_back(insetPair);
						iv = iv->_right;
					}
					while(iv != sv.getRoot());
					insetPairsSet.push(insetPairs);
				}
// end
				//LogManager::getSingleton().logMessage("Valid.");
				break;
			}
		}
			// vis
		size_t col = 0;
		while(!insetPairsSet.empty())
		{
			vector<pair<Vector3, Vector3> > insetPairs = insetPairsSet.front();
			insetPairsSet.pop();
			_debugMO->begin("gk/Hilite/Rainbow"+StringConverter::toString(col%6), Ogre::RenderOperation::OT_LINE_LIST);
			for(size_t i=0; i<insetPairs.size(); i++)
			{
				_debugMO->position(insetPairs[i].first);
				_debugMO->position(insetPairs[i].second);
			}
			_debugMO->end();
			col++;

			_debugMO->begin("gk/Default", Ogre::RenderOperation::OT_LINE_LIST);
			for(size_t i=0; i<insetPairs.size(); i++)
			{
				size_t j = (i+1)%insetPairs.size();
				_debugMO->position(insetPairs[i].first);
				_debugMO->position(insetPairs[j].first);
				_debugMO->position(insetPairs[i].second);
				_debugMO->position(insetPairs[j].second);
			}
			_debugMO->end();
		}
	}
	_sceneNode->attachObject(_debugMO);
}
