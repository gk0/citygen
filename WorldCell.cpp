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

using namespace Ogre;
using namespace std;
int WorldCell::mInstanceCount = 0;

WorldCell::WorldCell(const RoadGraph &p, const RoadGraph &s)
 : 	_parentRoadGraph(p),
	_simpleRoadGraph(s)
{
	init();
}

WorldCell::WorldCell(const RoadGraph &p, const RoadGraph &s, vector<NodeInterface*> &n)
 : 	_parentRoadGraph(p),
	_simpleRoadGraph(s)
{
	init();
	setBoundary(n);
}

void WorldCell::init()
{
_busy = false;


	_showRoads = true;
	_showBuildings = true;

	// set up some default growth gen params
	_growthGenParams.seed = 0;
	_growthGenParams.segmentSize = 6;
	_growthGenParams.segmentDeviance = 0.4;
	_growthGenParams.degree = 4;
	_growthGenParams.degreeDeviance = 0.01;
	_growthGenParams.snapSize = 2.4;
	_growthGenParams.snapDeviance = 0.1;
	_growthGenParams.buildingHeight = 2.4;
	_growthGenParams.buildingDeviance = 0.1;
	_growthGenParams.roadWidth = 0.4;
	_growthGenParams.roadLimit = 0;
	_growthGenParams.connectivity = 1;
	_growthGenParams.lotSize = 2;
	_growthGenParams.lotDeviance = 0.2;

	_roadNetworkMO = 0;
	_roadJunctionsMO = 0;
	_buildingsMO = 0;
	_debugMO = 0;
	mOverlay = 0;
	mOverlay2 = 0;

	_name = "cell"+StringConverter::toString(mInstanceCount++);
	mSceneNode = WorldFrame::getSingleton().getSceneManager()->getRootSceneNode()->createChildSceneNode(_name);
}

WorldCell::~WorldCell()
{
	clearBoundary();
	clearFilaments();
	clearRoadGraph();
	if(mOverlay)
		delete mOverlay;

	destroySceneObject();
	mSceneNode->detachAllObjects();
	mSceneNode->getCreator()->destroySceneNode(mSceneNode->getName());
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
	if(_roadNetworkMO) {
		mSceneNode->detachObject(_roadNetworkMO->getName());
		mSceneNode->getCreator()->destroyManualObject(_roadNetworkMO);
		delete _roadNetworkMO;
		_roadNetworkMO = 0;
	}
	if(_roadJunctionsMO) {
		mSceneNode->detachObject(_roadJunctionsMO->getName());
		mSceneNode->getCreator()->destroyManualObject(_roadJunctionsMO);
		delete _roadJunctionsMO;
		_roadJunctionsMO = 0;
	}

	if(_buildingsMO) {
		mSceneNode->detachObject(_buildingsMO->getName());
		mSceneNode->getCreator()->destroyManualObject(_buildingsMO);
		delete _buildingsMO;
		_buildingsMO = 0;
	}

	if(_debugMO) {
		mSceneNode->detachObject(_debugMO->getName());
		mSceneNode->getCreator()->destroyManualObject(_debugMO);
		delete _debugMO;
		_debugMO = 0;
	}
}

void WorldCell::update()
{
}

void WorldCell::clearRoadGraph()
{
	// delete roads
	RoadIterator rIt, rEnd;
	for(tie(rIt, rEnd) = _roadGraph.getRoads(); rIt != rEnd; rIt++) 
	{
		RoadInterface* ri = _roadGraph.getRoad((*rIt));
		if(typeid(*ri) == typeid(SimpleRoad)) delete ri;
	}

	// delete nodes
	NodeIterator nIt, nEnd;
	for(boost::tie(nIt, nEnd) = _roadGraph.getNodes(); nIt != nEnd; nIt++) 
		delete _roadGraph.getNode((*nIt));

	_roadGraph.clear();
}


struct CMP1231{
  bool operator()(const pair<size_t, WorldRoad*> l, const pair<size_t, WorldRoad*> r)
	{
	return l.second->getLength() > r.second->getLength();
	}
};
bool cmpRd(const WorldRoad* l, const WorldRoad* r)
{
	return l->getLength() < r->getLength();
}

void WorldCell::generateRoadNetwork()
{
	queue< pair<NodeInterface*, Vector2> > q;
	//TODO tidy

	Ogre::Real snapSzSquared = _growthGenParams.snapSize * _growthGenParams.snapSize;


	// 
	bool centreSuccess = false;
	if(_growthGenParams.connectivity >= 1.0f)
	{
		// 2. Get Center as a start point !!return if its node inside the cell	
		vector<Vector2> pointList;
		vector<NodeInterface*>::const_iterator nIt, nEnd;
		for(nIt = _boundaryCycle.begin(), nEnd = _boundaryCycle.end(); nIt != nEnd; nIt++)
			pointList.push_back((*nIt)->getPosition2D());

		_centre = Geometry::centerOfMass(pointList);

		if(isInside(_centre)) 
		{
			NodeInterface* startNode = createNode(_centre);

			// 3. Get initial direction vector from longest boundary road
			RoadInterface *longest = getLongestBoundaryRoad();
			Vector2 direction = longest->getSrcNode()->getPosition2D() - longest->getDstNode()->getPosition2D();
			direction.normalise();   
			direction *= _growthGenParams.segmentSize;

			q.push(make_pair<NodeInterface*, Vector2>(startNode, direction));
			centreSuccess = true;
		}
	}

	if(!centreSuccess)
	{
		// get a list of boundary roads
		size_t boundaryRoadsN = _boundaryCycle.size();
		vector< pair<size_t, WorldRoad*> > boundaryRoads;
		boundaryRoads.reserve(boundaryRoadsN);
		std::vector<Real> lengths;

		for(size_t j,i=0; i<_boundaryCycle.size(); i++)
		{	
			j = (i+1)%boundaryRoadsN;
			WorldRoad* wr = getWorldRoad(static_cast<WorldNode*>(_boundaryCycle[i]),
										static_cast<WorldNode*>(_boundaryCycle[j]));
			lengths.push_back(wr->getLength());
			boundaryRoads.push_back(make_pair<size_t, WorldRoad*>(i, wr));
		}

		// sort roads by length
		std::sort(boundaryRoads.begin(), boundaryRoads.end(), CMP1231());

		// for half of the roads, the largest ones
		for(size_t i=0; i<(boundaryRoadsN/2); i++)
		{
			// first node
			Vector3 pos1_3D(boundaryRoads[i].second->getMidPoint());
			Vector2 pos1(pos1_3D.x, pos1_3D.z);

			// get the road direction
			size_t j = boundaryRoads[i].first;
			size_t k = (j+1)%boundaryRoadsN;
			Vector2 roadDir(_boundaryCycle[k]->getPosition2D() - _boundaryCycle[j]->getPosition2D());
			roadDir = roadDir.perpendicular();
			roadDir.normalise();
			Vector2 pos2 = pos1 + (roadDir * _growthGenParams.segmentSize);
			pos1 -= (roadDir * _growthGenParams.segmentSize);
			
			NodeInterface* currentNode = 0;
			if(!isInside(pos2)) 
				continue;
			currentNode = createNode(pos2);

			// alter adjoining boundary road
			RoadId rd;
			NodeId nd;
			Vector2 newPoint;

			// 6600 vs. 8600 debug
			// 94 vs. 109 release
			//switch(_roadGraph.findClosestSnappedIntersection(currentNode->getPosition2D(), cursor, snapSzSquared, nd, rd, newPoint))
			switch(_roadGraph.snapInfo(currentNode->mNodeId, pos1, snapSzSquared, nd, rd, newPoint))
			{
			case 0: 
				break;
			case 1: 
				{
					// road intersection
					//NodeInterface *cursorNode = createNode(newPoint);
					NodeInterface *cursorNode = new SimpleNode(_roadGraph, newPoint);
					NodeId cursorNodeId = _roadGraph.addNode(cursorNode);
					cursorNode->mNodeId = cursorNodeId;
					createRoad(currentNode, cursorNode);

					// get intersected source src and dst
					RoadInterface *ri = _roadGraph.getRoad(rd);
					NodeId srcNodeId = _roadGraph.getSrc(rd);
					NodeId dstNodeId = _roadGraph.getDst(rd);

					//if road is a boundary road
					if(typeid(*ri) == typeid(WorldRoad))
					{
						// remove road segment from graph
						_roadGraph.removeRoad(srcNodeId, dstNodeId);

						// create replacement segments
						RoadId rd;
						if(_roadGraph.addRoad(srcNodeId, cursorNodeId, rd))
							_roadGraph.setRoad(rd, ri);
						if(_roadGraph.addRoad(cursorNodeId, dstNodeId, rd))
							_roadGraph.setRoad(rd, ri);
					}
					else
					{
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
				if(currentNode != _roadGraph.getNode(nd))
					createRoad(currentNode, _roadGraph.getNode(nd));
					
				q.push(make_pair<NodeInterface*, Vector2>(currentNode, roadDir));
				break;
			}
		}
	}

	int seed = _growthGenParams.seed;
	Ogre::Real segDevSz = _growthGenParams.segmentSize * _growthGenParams.segmentDeviance;
	Ogre::Real segSzBase = _growthGenParams.segmentSize - (segDevSz / 2);
	Real degDev = _growthGenParams.degree * _growthGenParams.degreeDeviance;
	Real degBase = _growthGenParams.degree - (degDev / 2);

	srand(seed++);

	size_t roadCount = 0;

	while(!q.empty())
	{
		NodeInterface* currentNode;
		Vector2 currentDirection;
		boost::tie(currentNode, currentDirection) = q.front();
		q.pop();

		Ogre::Radian theta(Math::TWO_PI / (degBase + (degDev * ((float)rand()/(float)RAND_MAX))));

		// alter our direction vector 
		for(unsigned int i=0; i < _growthGenParams.degree; i++)
		{
			if(_growthGenParams.roadLimit != 0 && roadCount++ >= _growthGenParams.roadLimit) return;

			if(roadCount > 57)
			{
				size_t z = 0;
			}

			// get a candidate
			Geometry::rotate(currentDirection, theta);
			// doesn't work grrrrrrrrrr

			currentDirection.normalise();
			Real segSz = (segSzBase + (segDevSz *  ((float)rand()/(float)RAND_MAX)));
			currentDirection *= segSz;
			Vector2 cursor(currentDirection + currentNode->getPosition2D());

			RoadId rd;
			NodeId nd;
			Vector2 newPoint;

			// 6600 vs. 8600 debug
			// 94 vs. 109 release
			//switch(_roadGraph.findClosestSnappedIntersection(currentNode->getPosition2D(), cursor, snapSzSquared, nd, rd, newPoint))
			switch(_roadGraph.snapInfo(currentNode->mNodeId, cursor, snapSzSquared, nd, rd, newPoint))
			{
			case 0: 
				// no intersection found
				{
					NodeInterface *cursorNode = createNode(cursor);
					createRoad(currentNode, cursorNode);
					q.push(make_pair<NodeInterface*, Vector2>(cursorNode, currentDirection)); // enqueue
				}
				break;
			case 1: 
				{
				if(((float)rand()/(float)RAND_MAX) > _growthGenParams.connectivity)
					break;

				// road intersection
				//NodeInterface *cursorNode = createNode(newPoint);
				NodeInterface *cursorNode = new SimpleNode(_roadGraph, newPoint);
				NodeId cursorNodeId = _roadGraph.addNode(cursorNode);
				cursorNode->mNodeId = cursorNodeId;
				createRoad(currentNode, cursorNode);

				// get intersected source src and dst
				RoadInterface *ri = _roadGraph.getRoad(rd);
				NodeId srcNodeId = _roadGraph.getSrc(rd);
				NodeId dstNodeId = _roadGraph.getDst(rd);

				//if road is a boundary road
				if(typeid(*ri) == typeid(WorldRoad))
				{
					// remove road segment from graph
					_roadGraph.removeRoad(srcNodeId, dstNodeId);

					// create replacement segments
					RoadId rd;
					if(_roadGraph.addRoad(srcNodeId, cursorNodeId, rd))
						_roadGraph.setRoad(rd, ri);
					if(_roadGraph.addRoad(cursorNodeId, dstNodeId, rd))
						_roadGraph.setRoad(rd, ri);
				}
				else
				{
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
				if(((float)rand()/(float)RAND_MAX) > _growthGenParams.connectivity)
					break;
				// MMM: dont snap to your self ass monkey
				if(currentNode != _roadGraph.getNode(nd))
					createRoad(currentNode, _roadGraph.getNode(nd));
				break;
				
			}
		}
	}
	
}

void WorldCell::buildRoadNetwork()
{
	
	// Create the Road Junctions
	_roadJunctionsMO = new ManualObject(_name+"Junc");
	_roadJunctionsMO->begin("gk/RoadJunction", Ogre::RenderOperation::OT_TRIANGLE_LIST);

	NodeIterator nIt, nEnd;
	for(boost::tie(nIt, nEnd) = _roadGraph.getNodes(); nIt != nEnd; nIt++)
	{
		NodeInterface* ni = _roadGraph.getNode(*nIt);
		if(typeid(*ni) == typeid(SimpleNode))
		{
			static_cast<SimpleNode*>(ni)->createJunction(_roadJunctionsMO);
		}
	}

	_roadJunctionsMO->end();
	mSceneNode->attachObject(_roadJunctionsMO);

	// Create the Road Network
	_roadNetworkMO = new ManualObject(_name+"Road");
	_roadNetworkMO->begin("gk/Road", Ogre::RenderOperation::OT_TRIANGLE_LIST);

	// Road Segments
	RoadIterator rIt, rEnd;
	for(boost::tie(rIt, rEnd) = _roadGraph.getRoads(); rIt != rEnd; rIt++)
		createRoad(*rIt, _roadNetworkMO);

	_roadNetworkMO->end();
	_roadNetworkMO->setVisible(_showRoads);
	mSceneNode->attachObject(_roadNetworkMO);
}


void WorldCell::prebuild()
{
	clearRoadGraph();
	installGraph();
	generateRoadNetwork();
	vector< vector<Vector2> > footprints;
	_roadGraph.extractFootprints(footprints, _growthGenParams.lotSize);
	BOOST_FOREACH(vector<Vector2> &footprint, footprints)
	{
		_blocks.push_back(WorldBlock(footprint, _growthGenParams));
	}
}

void WorldCell::build()
{
	PerformanceTimer buildPT("Cell build"), roadPT("Road build");

	//1. Clear Road Graph and destroy scene object
	destroySceneObject();

	//std::vector<Ogre::Vector2> bd;
	//bd.push_back(Vector2(1000, 1000));
	//bd.push_back(Vector2(1000, 975));
	//bd.push_back(Vector2(975, 975));
	//bd.push_back(Vector2(975, 1000));


	//// declare the manual object
	_buildingsMO = new ManualObject(_name+"b");
	_buildingsMO->begin("gk/Building", Ogre::RenderOperation::OT_TRIANGLE_LIST);

	BOOST_FOREACH(WorldBlock &block, _blocks)
		block.build(_buildingsMO);


	_buildingsMO->end();
	_buildingsMO->setVisible(_showBuildings);
	mSceneNode->attachObject(_buildingsMO);

	// am done with blocks now.
	_blocks.clear();


	//generateRoadNetwork();
	buildRoadNetwork();
	//roadPT.stop();



	//PerformanceTimer buildingPT("Buildings");

	//buildingPT.stop();
	//buildPT.stop();

	// Log Debug Data
	//LogManager::getSingleton().logMessage(roadPT.toString()+" "+buildingPT.toString()+" "+buildPT.toString());
	//LogManager::getSingleton().logMessage("Junction fail count: "+StringConverter::toString(junctionFailCount));

	//DEBUG: count short roads
	//size_t i=0;
	//RoadIterator rIt, rEnd;
	//for(boost::tie(rIt, rEnd) = _roadGraph.getRoads(); rIt != rEnd; rIt++)
	//{
	//	Vector2 road(_roadGraph.getSrcNode(*rIt)->getPosition2D() - _roadGraph.getDstNode(*rIt)->getPosition2D());
	//	if(road.length() < _growthGenParams.snapSize) i++;
	//}
	//LogManager::getSingleton().logMessage("Small road count: "+StringConverter::toString(i));
	//LogManager::getSingleton().logMessage("Build fail count: "+StringConverter::toString(buildFail));

	return;
}

bool WorldCell::isInside(const Ogre::Vector2 &loc) const
{
	bool rayCross = false;
	BOOST_FOREACH(RoadInterface* ri, _boundaryRoads)
		if(ri->rayCross(loc)) rayCross = !rayCross;
	return rayCross;
}

RoadInterface* WorldCell::getLongestBoundaryRoad() const
{
	RoadInterface* longest = 0;
	Ogre::Real length = 0;
	BOOST_FOREACH(RoadInterface* ri, _boundaryRoads)
	{
		if(longest == 0 || ri->getLengthSquared() > length)
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
	BOOST_FOREACH(NodeInterface* ni, _boundaryCycle) nodeMap[ni] = createNode(ni->getPosition2D());
	BOOST_FOREACH(RoadInterface* ri, _boundaryRoads) installRoad(ri, nodeMap);
	BOOST_FOREACH(RoadInterface* ri, _filamentRoads) installRoad(ri, nodeMap);
}

void WorldCell::installRoad(RoadInterface* ri, map<NodeInterface*, NodeInterface*> &nodeMap)
{
	//create the road node
	assert(typeid(*ri) == typeid(WorldRoad));
	BOOST_FOREACH(RoadId rSegId, static_cast<WorldRoad*>(ri)->getRoadSegmentList())
	{
		// check we have the nodes, if not add them
		NodeInterface *srcNd = _parentRoadGraph.getSrcNode(rSegId);
		NodeInterface *dstNd = _parentRoadGraph.getDstNode(rSegId);
		if(nodeMap.find(srcNd) == nodeMap.end()) 
			nodeMap[srcNd] = createNode(srcNd->getPosition2D());
		if(nodeMap.find(dstNd) == nodeMap.end()) 
			nodeMap[dstNd] = createNode(dstNd->getPosition2D());

		// create the road seg
		//createRoad(nodeMap[srcNd], nodeMap[dstNd]);
		RoadId rd;
		if(_roadGraph.addRoad(nodeMap[srcNd]->mNodeId, nodeMap[dstNd]->mNodeId, rd))
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
	for(i=0; i<N; i++)
	{
		j = (i+1) % N;

		// This is messy, but fuck it i don't have the time
		WorldNode* wn1 = static_cast<WorldNode*>(_boundaryCycle[i]);
		WorldNode* wn2 = static_cast<WorldNode*>(_boundaryCycle[j]);

		RoadId rd = _simpleRoadGraph.getRoad(wn1->mSimpleNodeId, wn2->mSimpleNodeId);
		_boundaryRoads.push_back(_simpleRoadGraph.getRoad(rd));
	}

	// set up listeners to receive invalidate events from roads
	vector<RoadInterface*>::iterator rIt, rEnd;
	for(rIt = _boundaryRoads.begin(), rEnd = _boundaryRoads.end(); rIt != rEnd; rIt++)
	{
		if(typeid(*(*rIt)) == typeid(WorldRoad))
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
	for(rIt = _filamentRoads.begin(), rEnd = _filamentRoads.end(); rIt != rEnd; rIt++)
	{	
		assert(typeid(*(*rIt)) == typeid(WorldRoad));
		WorldRoad* wr = static_cast<WorldRoad*>(*rIt);
		if(wr == f)
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
	for(rIt = _filamentRoads.begin(), rEnd = _filamentRoads.end(); rIt != rEnd; rIt++)
	{
		if(typeid(*(*rIt)) == typeid(WorldRoad))
			static_cast<WorldRoad*>(*rIt)->detach(this);
	}
	_filamentRoads.clear();
	invalidate();
}

CellGenParams WorldCell::getGenParams() const
{
	return _growthGenParams;
}
void WorldCell::setGenParams(const CellGenParams &g)
{
	_growthGenParams = g;
	invalidate();
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
	sn->mNodeId = nd;
	return sn;
}

RoadInterface* WorldCell::createRoad(NodeInterface *n1, NodeInterface *n2)
{
	RoadId rd;
	if(_roadGraph.addRoad(n1->mNodeId, n2->mNodeId, rd))
	{
		SimpleRoad *sr = new SimpleRoad(n1, n2);
		sr->setWidth(_growthGenParams.roadWidth);
		_roadGraph.setRoad(rd, sr);
		return sr;
	}
	else
		return 0;
}

void WorldCell::deleteRoad(RoadInterface *ri)
{
	// remove it from the graph
	_roadGraph.removeRoad(ri->getSrcNode()->mNodeId, ri->getDstNode()->mNodeId);
	// delete it
	delete ri;
}

bool WorldCell::isBoundaryNode(const NodeInterface *ni) const
{
	BOOST_FOREACH(NodeInterface* n, _boundaryCycle) if(n==ni) return true;
	return false;
}


bool WorldCell::compareBoundary(const vector<NodeInterface*>& nodeCycle) const
{
	// compare size
	if(nodeCycle.size() != _boundaryCycle.size()) 
		return false;

	// find match
	size_t i, j, offset, N = nodeCycle.size();
	for(i = 0; i < N; i++)
	{
		if(nodeCycle[0] == _boundaryCycle[i]) 
			break;
	}
	if(i >= N)
		return false;
	else
		offset = i;

	// try forwards
	for(i = 1; i < N; i++)
	{
		if(nodeCycle[i] != _boundaryCycle[(i + offset) % N])
			break;
	}
	if(i >= N) 
		return true;

	// try backwards
	for(i = 1, j = offset-1; i < N; i++, j--)
	{
		if(nodeCycle[i] != _boundaryCycle[j % N])
			return false;
	}
	return true;
}

RoadInterface* WorldCell::getRoad(NodeInterface* n1, NodeInterface* n2)
{
	return _roadGraph.getRoad(_roadGraph.getRoad(n1->mNodeId, n2->mNodeId));
}

bool WorldCell::extractFootprint(const vector<NodeInterface*> &nodeCycle, vector<Vector2> &footprint)
{
	// get size
	size_t i, j, N = nodeCycle.size();

	// check cycle
	if(N < 3)
		throw Exception(Exception::ERR_INVALIDPARAMS, 
		"Invalid number of nodes in cycle", "WorldCell::extractFootprint");

	// prepare footprint data structures
	vector<Vector2> originalFootprint;
	vector<Real> roadWidths;
	originalFootprint.reserve(N);
	for(i=0; i<N; i++)
	{
		originalFootprint.push_back(nodeCycle[i]->getPosition2D());
		roadWidths.push_back(getRoad(nodeCycle[i], nodeCycle[(i+1)%N])->getWidth());
	}

/*	//Real width = roadCycle[0]->getWidth();
	Real width = roadWidths[0];
	for(i=1; i<N && roadWidths[i] == width; i++);
	if(i==N)
	{
		if(Geometry::polygonInset(width, originalFootprint))
		{
			footprint = originalFootprint;
			return true;
		}
		else return false;
	}
	else*/
	{
/*
		// I'm affraid its a little more complicated for this case
		// The sk code provides a skeleton for uniform insetting but 
		// cells primary road network join onto larger roads that require 
		// a non uniform inset, ideally a weighted straight skeleton should
		// but for the meantime, a manual inset for the larger roads is used

		// calculate the minimum road width
		for(i=1; i<N; i++) 
			if(roadCycle[i]->getWidth() < width) 
				width = roadCycle[i]->getWidth();

		// store a list of larger road sections
		vector< vector<Vector2> > roadSections;
		vector< Real > roadSectionWidths;

		vector<RoadInterface*> rds;
		for(i=0; i<N; i++) 
		{
			NodeId n1 = nodeCycle[i]->mNodeId;
			NodeId n2 = nodeCycle[(i+1)%N]->mNodeId;
			RoadId rd = _roadGraph.getRoad(n1, n2);
			RoadInterface* ri = _roadGraph.getRoad(rd);
			assert(roadCycle[(i+1)%N] == ri);
			rds.push_back(ri);
		}

		for(i=0; i<N; i++) 
		{
			if(roadCycle[i]->getWidth() != width)
			{
				// store width
				Real roadSectionWidth = roadCycle[i]->getWidth();

				NodeId n1 = nodeCycle[i]->mNodeId;
				NodeId n2 = nodeCycle[(i+1)%N]->mNodeId;
				RoadId rd = _roadGraph.getRoad(n1, n2);
				RoadInterface* ri = _roadGraph.getRoad(rd);
				//assert(roadCycle[i] == ri);

				// store points in section
				vector<Vector2> roadSection;
				roadSection.push_back(originalFootprint[(i-1)%N]);
				roadSection.push_back(originalFootprint[i++]);
				for(; i<N && roadCycle[i]->getWidth() == roadSectionWidth; i++)
					roadSection.push_back(originalFootprint[i]);

				i--;  // don't advance too far

				// add to list
				roadSectionWidths.push_back(roadSectionWidth);
				roadSections.push_back(roadSection);
			}
		}

		// get a minimal inset footprint
		if(!Geometry::polygonInset(width, originalFootprint))
			return false;

		size_t numOfSections = roadSections.size();
		for(i=0; i<numOfSections; i++)
		{
			// inset road section
			if(!Geometry::lineInset(roadSectionWidths[i], roadSections[i]))
				return false;

			// add to footprint
			//for(size_t j=0; j<roadSections[i].size(); j++) newFootprint.push_back(roadSections[i][j]);
			if(!Geometry::unionPolyAndLine(originalFootprint, roadSections[i]))
				return false;
		}
		footprint = originalFootprint;
		return true;
*/
		// try it old school see if it works any better

		// create footprint edge structure
		vector< pair<Vector2, Vector2> > edges;
		edges.reserve(N);
		vector<Vector2> newFootprint;
		newFootprint.reserve(N);

		// get footprint edge vectors
		for(i=0; i<N; i++)
		{
			j = (i+1)%N;
			Vector2 dir(originalFootprint[i] - originalFootprint[j]);
			dir = dir.perpendicular();
			dir.normalise();
			dir *= -roadWidths[i];
			edges.push_back(make_pair(originalFootprint[i] + dir, originalFootprint[j] + dir));
		}

		// calculate footprint points from edges
		for(i=0; i<N; i++)
		{
			j = (i+1)%N;
			// get edge intersection point
			Ogre::Real r,s;
			Vector2 intscn;
			if(Geometry::lineIntersect(edges[i].first, edges[i].second, 
				edges[j].first, edges[j].second, intscn, r, s) && r >= 0 && s <= 1)
			{
				newFootprint.push_back(intscn);
			}
			else
			{
				// no intersection, couldbe parallel could be mad lets average the pair
				newFootprint.push_back((edges[i].second + edges[j].first)/2);
			}
		}
		footprint.clear();
		footprint.insert(footprint.begin(), newFootprint.rbegin(), newFootprint.rend());
		return true;
	}
}

void WorldCell::buildSegment(const Vector3 &a1, const Vector3 &a2, const Vector3 &aNorm,
			const Vector3 &b1, const Vector3 &b2, const Vector3 &bNorm, Real uMin, Real uMax)
{
	// create road segment
	_roadNetworkMO->position(a1);
	_roadNetworkMO->normal(aNorm);
	_roadNetworkMO->textureCoord(uMin, 0);
	_roadNetworkMO->position(a2);
	_roadNetworkMO->normal(aNorm);
	_roadNetworkMO->textureCoord(uMin, 1);
	_roadNetworkMO->position(b1);
	_roadNetworkMO->normal(bNorm);
	_roadNetworkMO->textureCoord(uMax, 0);

	_roadNetworkMO->position(a2);
	_roadNetworkMO->normal(aNorm);
	_roadNetworkMO->textureCoord(uMin, 1);
	_roadNetworkMO->position(b2);
	_roadNetworkMO->normal(bNorm);
	_roadNetworkMO->textureCoord(uMax, 1);
	_roadNetworkMO->position(b1);
	_roadNetworkMO->normal(bNorm);
	_roadNetworkMO->textureCoord(uMax, 0);
}

void WorldCell::createRoad(const RoadId rd, ManualObject *m)
{
	RoadInterface* r = _roadGraph.getRoad(rd);
	if(typeid(*r) != typeid(WorldRoad))
	{
		Vector3 a1,a2, b1, b2;
		boost::tie(a1,a2) = _roadGraph.getSrcNode(rd)->getRoadJunction(rd);
		boost::tie(b2,b1) = _roadGraph.getDstNode(rd)->getRoadJunction(rd);
		buildSegment(a1, a2, Vector3::UNIT_Y, b1, b2, Vector3::UNIT_Y, 0, 4);
	}
}

void WorldCell::showSelected(bool show)
{
	std::vector<RoadInterface*>::iterator bIt, bEnd;
	for(bIt = _boundaryRoads.begin(), bEnd = _boundaryRoads.end(); bIt != bEnd; bIt++)
	{
		if(typeid(*(*bIt)) == typeid(WorldRoad))
			static_cast<WorldRoad*>(*bIt)->showSelected(show);
	}
}

bool WorldCell::loadXML(const TiXmlHandle& cellRoot)
{
	// Load GenParams, not Smart Enough to do our own boundary or filaments, see parent
	{
		TiXmlElement *element = cellRoot.FirstChild("genparams").FirstChild().Element();

		for(; element; element=element->NextSiblingElement())
		{
			string key = element->Value();
			
			if(key == "seed")
				element->QueryIntAttribute("value", &_growthGenParams.seed);
			else if(key == "type") 
			{
				int t;
				element->QueryIntAttribute("value", &t);
				_growthGenParams.type = static_cast<unsigned int>(t);
			}
			else if(key == "segmentSize")
				element->QueryFloatAttribute("value", &_growthGenParams.segmentSize);
			else if(key == "segmentDeviance")
				element->QueryFloatAttribute("value", &_growthGenParams.segmentDeviance);
			else if(key == "degree") 
			{
				int deg;
				element->QueryIntAttribute("value", &deg);
				_growthGenParams.degree = static_cast<unsigned int>(deg);
			}else if(key == "degreeDeviance")
				element->QueryFloatAttribute("value", &_growthGenParams.degreeDeviance);
			else if(key == "snapSize")
				element->QueryFloatAttribute("value", &_growthGenParams.snapSize);
			else if(key == "snapDeviance")
				element->QueryFloatAttribute("value", &_growthGenParams.snapDeviance);
			else if(key == "roadWidth")
				element->QueryFloatAttribute("value", &_growthGenParams.roadWidth);
			else if(key == "buildingHeight")
				element->QueryFloatAttribute("value", &_growthGenParams.buildingHeight);
			else if(key == "buildingDeviance")
				element->QueryFloatAttribute("value", &_growthGenParams.buildingDeviance);
			else if(key == "connectivity")
				element->QueryFloatAttribute("value", &_growthGenParams.connectivity);
			else if(key == "lotSize")
				element->QueryFloatAttribute("value", &_growthGenParams.lotSize);
			else if(key == "lotDeviance")
				element->QueryFloatAttribute("value", &_growthGenParams.lotDeviance);
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
	addNewElement(gp, "seed")->SetAttribute("value", _growthGenParams.seed);
	addNewElement(gp, "type")->SetAttribute("value", _growthGenParams.type);
	addNewElement(gp, "segmentSize")->SetDoubleAttribute("value", _growthGenParams.segmentSize);
	addNewElement(gp, "segmentDeviance")->SetDoubleAttribute("value", _growthGenParams.segmentDeviance);
	addNewElement(gp, "degree")->SetAttribute("value", _growthGenParams.degree);
	addNewElement(gp, "degreeDeviance")->SetDoubleAttribute("value", _growthGenParams.degreeDeviance);
	addNewElement(gp, "snapSize")->SetDoubleAttribute("value", _growthGenParams.snapSize);
	addNewElement(gp, "snapDeviance")->SetDoubleAttribute("value", _growthGenParams.snapDeviance);
	addNewElement(gp, "roadWidth")->SetDoubleAttribute("value", _growthGenParams.roadWidth);
	addNewElement(gp, "buildingHeight")->SetDoubleAttribute("value", _growthGenParams.buildingHeight);
	addNewElement(gp, "buildingDeviance")->SetDoubleAttribute("value", _growthGenParams.buildingDeviance);
	addNewElement(gp, "connectivity")->SetDoubleAttribute("value", _growthGenParams.connectivity);
	addNewElement(gp, "lotSize")->SetDoubleAttribute("value", _growthGenParams.lotSize);
	addNewElement(gp, "lotDeviance")->SetDoubleAttribute("value", _growthGenParams.lotDeviance);

	return root;
}

void WorldCell::showRoads(bool show)
{
	_showRoads = show;
	if(_roadNetworkMO) _roadNetworkMO->setVisible(_showRoads);
	if(_roadJunctionsMO) _roadJunctionsMO->setVisible(_showRoads);
}

void WorldCell::showBuildings(bool show)
{
	_showBuildings = show;
	if(_buildingsMO) _buildingsMO->setVisible(_showBuildings);
}


void WorldCell::beginGraphicalCellDecomposition()
{
	// take a copy of the graph
	mGCDRoadGraph = _roadGraph;

	//init roads
	RoadIterator rIt, rEnd;
	for(tie(rIt, rEnd) = mGCDRoadGraph.getRoads(); rIt != rEnd; rIt++)
		mGCDRoadGraph.getRoad(*rIt)->setRoadCycle(false);

	//set<NodeId> mGCDHeap = vertices; 
	//gk: at the moment I'm using a vertex to store vertices so they are already sorted
	NodeIterator nIt, nEnd;
	boost::tie(nIt, nEnd) = mGCDRoadGraph.getNodes();
	//set<NodeId> mGCDHeap(i, end);
	
	//SET needs sort
	//sort(mGCDHeap.begin(), mGCDHeap.end(), comp());
	//it would be neater o use a comparator but our comparision isn't standalone


	//insert one
	mGCDHeap.clear();
	mGCDHeap.push_back(*nIt++);

	for(; nIt != nEnd; ++nIt)
	{
		for(std::list<NodeId>::iterator tit = mGCDHeap.begin(); true; tit++)
		{
			if(tit == mGCDHeap.end()) {
				mGCDHeap.push_back(*nIt);
				break;
			}
			else if(mGCDRoadGraph.getNode(*nIt)->getPosition2D().x < mGCDRoadGraph.getNode(*tit)->getPosition2D().x)
			{
				mGCDHeap.insert(tit, *nIt);
				break;
			}
		}
	}

	drawGraphicalCellDecomposition();
}

void WorldCell::drawGraphicalCellDecomposition()
{	
	// 
	if(mOverlay)
	{
		mSceneNode->detachObject(mOverlay);
		delete mOverlay;
	}

	if(mOverlay2)
	{
		mSceneNode->detachObject(mOverlay2);
		delete mOverlay2;
	}

	// Specify clearance
	Vector3 extraClearance(0, 2, 0);

	// begin manual object
	mOverlay = new ManualObject(_name+"o");
	mOverlay->begin("gk/Hilite/Red2", Ogre::RenderOperation::OT_LINE_LIST);

	// for each edge
	RoadIterator rIt, rEnd;
	for(tie(rIt, rEnd) = mGCDRoadGraph.getRoads(); rIt != rEnd; rIt++) 
	{
		NodeInterface* srcNode = mGCDRoadGraph.getSrcNode((*rIt));
		NodeInterface* dstNode = mGCDRoadGraph.getDstNode((*rIt));
		mOverlay->position(srcNode->getPosition3D() + extraClearance);
		mOverlay->position(dstNode->getPosition3D() + extraClearance);
	}

	// end manual object
	mOverlay->end();
	mSceneNode->attachObject(mOverlay);


	// begin manual object
	mOverlay2 = new ManualObject(_name+"o2");
	mOverlay2->begin("gk/Hilite/Yellow", Ogre::RenderOperation::OT_LINE_LIST);

	// for each cell
	for(size_t i=0; i<mGCDNodeCycles.size(); i++)
	{
		std::vector<Vector2> polypoints;
		polypoints.reserve(mGCDNodeCycles[i].size());

		for(size_t j=0; j<mGCDNodeCycles[i].size(); j++)
		{
			polypoints.push_back(mGCDNodeCycles[i][j]->getPosition2D());
		}
		Geometry::polygonInset(0.2f, polypoints);

		for(size_t j=0; j<(mGCDNodeCycles[i].size() - 1); j++)
		{
			Vector3 src(polypoints[j].x, mGCDNodeCycles[i][j]->getPosition3D().y + 2, polypoints[j].y);
			mOverlay2->position(src);

			Vector3 dst(polypoints[j+1].x, mGCDNodeCycles[i][j+1]->getPosition3D().y + 2, polypoints[j+1].y);
			mOverlay2->position(dst);
		}

		Vector3 src(polypoints[mGCDNodeCycles[i].size() - 1].x, 
			mGCDNodeCycles[i][mGCDNodeCycles[i].size() - 1]->getPosition3D().y + 2, 
			polypoints[mGCDNodeCycles[i].size() - 1].y);
		mOverlay2->position(src);

		Vector3 dst(polypoints[0].x, 
			mGCDNodeCycles[i][0]->getPosition3D().y + 2, polypoints[0].y);
		mOverlay2->position(dst);

	}

	// end manual object
	mOverlay2->end();
	mSceneNode->attachObject(mOverlay2);


}

void WorldCell::stepGraphicalCellDecomposition()
{	
	
	try {
	
	//while (mGCDHeap is not empty) do
	if(mGCDHeap.size() != 0)
	{
		//Vertex v0 = mGCDHeap.GetMin();
		NodeId v0 = *(mGCDHeap.begin());

		switch(out_degree(v0, mGCDRoadGraph.mGraph))
		{
		case 0:
			remove_vertex(v0, mGCDRoadGraph.mGraph);
			RoadGraph::removeFromHeap(v0, mGCDHeap);
			break;
		case 1:
			RoadGraph::extractFilament(v0, RoadGraph::getFirstAdjacent(v0, mGCDRoadGraph.mGraph), mGCDRoadGraph.mGraph, mGCDHeap, mGCDFilaments);
			//oss<<"Filament: "<<graph[v0].getName()<<endl;

			//DEBUG
			//mGCDHeap.erase(v0);
			break;
		default:
			RoadGraph::extractPrimitive(v0, mGCDRoadGraph.mGraph, mGCDHeap, mGCDFilaments, mGCDNodeCycles); // filament or minimal cycle
			//oss<<"Cycle or Filament: "<<mGraph[v0].getName()<<endl;

			//DEBUG
			//mGCDHeap.erase(v0);
			break;
		}
		//mGCDHeap.erase(v0);
	}

	}
	catch(Exception e)
	{
		LogManager::getSingleton().logMessage(e.getDescription());

		//DANGER - setting partially decomposed graph to real graph
		//mGraph = g;
	}
	drawGraphicalCellDecomposition();
}

vector<Vector3> WorldCell::getBoundaryPoints3D()
{
	vector<Vector3> pointList;
	pointList.reserve(_boundaryCycle.size());
	BOOST_FOREACH(NodeInterface* ni, _boundaryCycle)
		pointList.push_back(ni->getPosition3D());
	return pointList;
}

vector<Vector2> WorldCell::getBoundaryPoints2D()
{
	vector<Vector2> pointList;
	pointList.reserve(_boundaryCycle.size());
	BOOST_FOREACH(NodeInterface* ni, _boundaryCycle)
		pointList.push_back(ni->getPosition2D());
	return pointList;
}

Real WorldCell::calcArea2D()
{
	return Geometry::polygonArea(getBoundaryPoints2D());
}

