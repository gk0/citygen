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

WorldCell::WorldCell(RoadGraph &p, RoadGraph &s)
 : 	mParentRoadGraph(p),
	mSimpleRoadGraph(s)
{
	init();
}

WorldCell::WorldCell(RoadGraph &p, RoadGraph &s, vector<NodeInterface*> &n)
 : 	mParentRoadGraph(p),
	mSimpleRoadGraph(s)
{
	init();
	setBoundary(n);
}

void WorldCell::init()
{
	mShowRoads = true;
	mShowBuildings = true;

	// set up some default growth gen params
	mGrowthGenParams.seed = 0;
	mGrowthGenParams.segmentSize = 6;
	mGrowthGenParams.segmentDeviance = 0.4;
	mGrowthGenParams.degree = 4;
	mGrowthGenParams.degreeDeviance = 0.01;
	mGrowthGenParams.snapSize = 2.4;
	mGrowthGenParams.snapDeviance = 0.1;
	mGrowthGenParams.buildingHeight = 2.4;
	mGrowthGenParams.buildingDeviance = 0.1;
	mGrowthGenParams.roadWidth = 0.4;
	mGrowthGenParams.roadLimit = 0;
	mGrowthGenParams.lotSize = 2;
	mGrowthGenParams.lotDeviance = 0.2;

	mRoadNetwork = 0;
	mRoadJunctions = 0;
	mBuildings = 0;
	mOverlay = 0;
	mOverlay2 = 0;

	mName = "cell"+StringConverter::toString(mInstanceCount++);
	mSceneNode = WorldFrame::getSingleton().getSceneManager()->getRootSceneNode()->createChildSceneNode(mName);
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
	if(mRoadNetwork) {
		mSceneNode->detachObject(mRoadNetwork->getName());
		mSceneNode->getCreator()->destroyManualObject(mRoadNetwork);
		delete mRoadNetwork;
		mRoadNetwork = 0;
	}
	if(mRoadJunctions) {
		mSceneNode->detachObject(mRoadJunctions->getName());
		mSceneNode->getCreator()->destroyManualObject(mRoadJunctions);
		delete mRoadJunctions;
		mRoadJunctions = 0;
	}

	if(mBuildings) {
		mSceneNode->detachObject(mBuildings->getName());
		mSceneNode->getCreator()->destroyManualObject(mBuildings);
		delete mBuildings;
		mBuildings = 0;
	}
}

void WorldCell::update()
{
}

void WorldCell::clearRoadGraph()
{
	// delete roads
	RoadIterator rIt, rEnd;
	for(tie(rIt, rEnd) = mRoadGraph.getRoads(); rIt != rEnd; rIt++) 
	{
		RoadInterface* ri = mRoadGraph.getRoad((*rIt));
		if(typeid(*ri) == typeid(SimpleRoad)) delete ri;
	}

	// delete nodes
	NodeIterator nIt, nEnd;
	for(boost::tie(nIt, nEnd) = mRoadGraph.getNodes(); nIt != nEnd; nIt++) 
		delete mRoadGraph.getNode((*nIt));

	mRoadGraph.clear();
}

void WorldCell::generateRoadNetwork()
{
	// NOTE:
	// currently this algorithm start from the centre in the direction of the longest edge
	//  another possibility would be to start from the midpoint of the longest edge in 
	//  the direction of the centre point

	// 2. Get Center as a start point !!return if its node inside the cell	
	{
		vector<Vector2> pointList;
		vector<NodeInterface*>::const_iterator nIt, nEnd;
		for(nIt = mBoundaryCycle.begin(), nEnd = mBoundaryCycle.end(); nIt != nEnd; nIt++)
			pointList.push_back((*nIt)->getPosition2D());

		mCentre = Geometry::centerOfMass(pointList);
	}

	if(!isInside(mCentre)) 
		return;

	NodeInterface* startNode = createNode(mCentre);

	// 3. Get initial direction vector from longest boundary road
	RoadInterface *longest = getLongestBoundaryRoad();
	Vector2 direction = longest->getSrcNode()->getPosition2D() - longest->getDstNode()->getPosition2D();
	direction.normalise();   
	direction *= mGrowthGenParams.segmentSize;

	// work out how many times to rotate 
	//unsigned int rotateCount = (Math::TWO_PI / theta);
	//unsigned int degree = 4;
	Ogre::Real snapSzSquared = mGrowthGenParams.snapSize * mGrowthGenParams.snapSize;

	int seed = mGrowthGenParams.seed;

	Ogre::Real segDevSz = mGrowthGenParams.segmentSize * mGrowthGenParams.segmentDeviance;
	Ogre::Real segSzBase = mGrowthGenParams.segmentSize - (segDevSz / 2);

	//int degDev = mGrowthGenParams.degree * mGrowthGenParams.degreeDeviance;
	//int degBase = mGrowthGenParams.degree - degDev;
	Real degDev = mGrowthGenParams.degree * mGrowthGenParams.degreeDeviance;
	Real degBase = mGrowthGenParams.degree - (degDev / 2);

	queue<NodeInterface*> q;
	q.push(startNode);

	srand(seed++);

	size_t roadCount = 0;

	while(!q.empty())
	{
		NodeInterface* currentNode = q.front();
		q.pop();

		Ogre::Radian theta(Math::TWO_PI / (degBase + (degDev * ((float)rand()/(float)RAND_MAX))));

		// alter our direction vector 
		for(unsigned int i=0; i < mGrowthGenParams.degree; i++)
		{
			if(mGrowthGenParams.roadLimit != 0 && roadCount++ >= mGrowthGenParams.roadLimit) return;

			if(roadCount > 57)
			{
				size_t z = 0;
			}

			// get a candidate
			Geometry::rotate(direction, theta);
			// doesn't work grrrrrrrrrr

			direction.normalise();
			Real segSz = (segSzBase + (segDevSz *  ((float)rand()/(float)RAND_MAX)));
			direction *= segSz;
			Vector2 cursor(direction + currentNode->getPosition2D());

			RoadId rd;
			NodeId nd;
			Vector2 newPoint;

			// 6600 vs. 8600 debug
			// 94 vs. 109 release
			//switch(mRoadGraph.findClosestSnappedIntersection(currentNode->getPosition2D(), cursor, snapSzSquared, nd, rd, newPoint))
			switch(mRoadGraph.snapInfo(currentNode->mNodeId, cursor, snapSzSquared, nd, rd, newPoint))
			{
			case 0: 
				{
				// no intersection found
				NodeInterface *cursorNode = createNode(cursor);
				createRoad(currentNode, cursorNode);
				q.push(cursorNode); // enqueue
				}
				break;
			case 1: 
				{
				// road intersection
				//NodeInterface *cursorNode = createNode(newPoint);
				NodeInterface *cursorNode = new SimpleNode(mRoadGraph, newPoint);
				NodeId cursorNodeId = mRoadGraph.addNode(cursorNode);
				cursorNode->mNodeId = cursorNodeId;
				createRoad(currentNode, cursorNode);

				// get intersected source src and dst
				RoadInterface *ri = mRoadGraph.getRoad(rd);
				NodeId srcNodeId = mRoadGraph.getSrc(rd);
				NodeId dstNodeId = mRoadGraph.getDst(rd);

				//if road is a boundary road
				if(typeid(*ri) == typeid(WorldRoad))
				{
					// remove road segment from graph
					mRoadGraph.removeRoad(srcNodeId, dstNodeId);

					// create replacement segments
					RoadId rd;
					if(mRoadGraph.addRoad(srcNodeId, cursorNodeId, rd))
						mRoadGraph.setRoad(rd, ri);
					if(mRoadGraph.addRoad(cursorNodeId, dstNodeId, rd))
						mRoadGraph.setRoad(rd, ri);
				}
				else
				{
					// delete the road rd
					deleteRoad(ri);

					// reconstruct road in the form of two segments
					createRoad(mRoadGraph.getNode(srcNodeId), cursorNode);
					createRoad(cursorNode, mRoadGraph.getNode(dstNodeId));
				}
				}
				break;
			case 2:
				// node snap
				// MMM: dont snap to your self ass monkey
				if(currentNode != mRoadGraph.getNode(nd))
					createRoad(currentNode, mRoadGraph.getNode(nd));
				break;
				
			}
		}
	}
}

void WorldCell::buildRoadNetwork()
{
	
	// Create the Road Junctions
	mRoadJunctions = new ManualObject(mName+"Junc");
	mRoadJunctions->begin("gk/RoadJunction", Ogre::RenderOperation::OT_TRIANGLE_LIST);
	int junctionFailCount = 0;

	NodeIterator nIt, nEnd;
	for(boost::tie(nIt, nEnd) = mRoadGraph.getNodes(); nIt != nEnd; nIt++)
	{
		NodeInterface* ni = mRoadGraph.getNode(*nIt);
		if(typeid(*ni) == typeid(SimpleNode))
		{
			static_cast<SimpleNode*>(ni)->createJunction(mRoadJunctions);
		}
	}

	mRoadJunctions->end();
	mSceneNode->attachObject(mRoadJunctions);

	// Create the Road Network
	mRoadNetwork = new ManualObject(mName+"Road");
	mRoadNetwork->begin("gk/Road", Ogre::RenderOperation::OT_TRIANGLE_LIST);

	// Road Segments
	RoadIterator rIt, rEnd;
	for(boost::tie(rIt, rEnd) = mRoadGraph.getRoads(); rIt != rEnd; rIt++)
		createRoad(*rIt, mRoadNetwork);

	mRoadNetwork->end();
	mRoadNetwork->setVisible(mShowRoads);
	mSceneNode->attachObject(mRoadNetwork);
}

void WorldCell::build()
{
	//return;
	// possibly(most likely) dangerous and stupid, but make any other thread accessing this method sleep until done
	static bool busy = false;
	if(busy)
	{
		//while(busy) Sleep(100);
		// am doing it, sheesh!
		return;
	}
	busy = true;




	PerformanceTimer buildPT("Cell build"), roadPT("Road build");

	//1. Clear Road Graph and destroy scene object
	destroySceneObject();
	clearRoadGraph();
	installGraph();

	//std::vector<Ogre::Vector2> bd;
	//bd.push_back(Vector2(1000, 1000));
	//bd.push_back(Vector2(1000, 975));
	//bd.push_back(Vector2(975, 975));
	//bd.push_back(Vector2(975, 1000));


	//// declare the manual object
	//mBuildings = new ManualObject(mName+"b");
	//mBuildings->begin("gk/Building", Ogre::RenderOperation::OT_TRIANGLE_LIST);
	//WorldBlock::build(bd, 0, 10, mBuildings);

	//mBuildings->end();
	//mSceneNode->attachObject(mBuildings);
	//busy = false;
	//return;

	generateRoadNetwork();
	buildRoadNetwork();
	


	//roadPT.stop();
	PerformanceTimer buildingPT("Buildings");

	// build boxes
	// check the road graph to get a count of the number of cycles
	vector< vector<NodeInterface*> > nodeCycles;
	vector<RoadInterface*> filaments;
	mRoadGraph.extractPrimitives(filaments, nodeCycles);

	// declare the manual object
	mBuildings = new ManualObject(mName+"b");
	mBuildings->begin("gk/Building", Ogre::RenderOperation::OT_TRIANGLE_LIST);

	Real buildingDeviance = mGrowthGenParams.buildingHeight * mGrowthGenParams.buildingDeviance;
	Real buildingHeight = mGrowthGenParams.buildingHeight - (buildingDeviance / 2);

	// create lot of little boxes with our cycles
	size_t buildFail=0;
	BOOST_FOREACH(vector<NodeInterface*>& nodeCycle, nodeCycles)
	{
		// get the foot print
		vector<Vector2> footprint, result;
		if(!extractFootprint(nodeCycle, footprint))
			continue;

		// get base foundation height
		Real foundation = (*(nodeCycle.begin()))->getPosition3D().y - 1;
		//Real height = foundation + buildingHeight + (buildingDeviance * ((float)rand()/(float)RAND_MAX));

		//
		//if(!createBuilding(mBuildings, footprint, foundation, height)) buildFail++;
		WorldBlock::build(footprint, foundation, mGrowthGenParams, mBuildings);
	}
	mBuildings->end();
	mSceneNode->attachObject(mBuildings);

	
	mBuildings->setVisible(mShowBuildings);

	buildingPT.stop();
	buildPT.stop();


	// Log Debug Data
	LogManager::getSingleton().logMessage(roadPT.toString()+" "+buildingPT.toString()+" "+buildPT.toString());
	//LogManager::getSingleton().logMessage("Junction fail count: "+StringConverter::toString(junctionFailCount));

	//RoadIterator rIt, rEnd;
	size_t i=0;
	RoadIterator rIt, rEnd;
	for(boost::tie(rIt, rEnd) = mRoadGraph.getRoads(); rIt != rEnd; rIt++)
	{
		Vector2 road(mRoadGraph.getSrcNode(*rIt)->getPosition2D() - mRoadGraph.getDstNode(*rIt)->getPosition2D());
		if(road.length() < mGrowthGenParams.snapSize) i++;
	}
	//LogManager::getSingleton().logMessage("Small road count: "+StringConverter::toString(i));
	//LogManager::getSingleton().logMessage("Build fail count: "+StringConverter::toString(buildFail));

	
	busy = false;
	return;
}

bool WorldCell::isInside(const Ogre::Vector2 &loc) const
{
	bool rayCross = false;
	vector<RoadInterface*>::const_iterator rIt, rEnd;
	for(rIt = mBoundaryRoads.begin(), rEnd = mBoundaryRoads.end(); rIt != rEnd; rIt++)
	{
		if((*rIt)->rayCross(loc)) rayCross = !rayCross;
	}
	return rayCross;
}

RoadInterface* WorldCell::getLongestBoundaryRoad() const
{
	RoadInterface* longest = 0;
	Ogre::Real length;
	vector<RoadInterface*>::const_iterator rIt, rEnd;
	for(rIt = mBoundaryRoads.begin(), rEnd = mBoundaryRoads.end(); rIt != rEnd; rIt++)
	{
		if(longest == 0 || (*rIt)->getLengthSquared() > length)
		{
			longest = (*rIt);
			length = longest->getLengthSquared();
		}
	}
	return longest;
}

void WorldCell::installGraph()
{
	map<NodeInterface*, NodeInterface*> nodeMap;
	vector<NodeInterface*>::iterator nIt, nEnd;
	for(nIt = mBoundaryCycle.begin(), nEnd = mBoundaryCycle.end(); nIt != nEnd; nIt++)
	{
		nodeMap[*nIt] = createNode((*nIt)->getPosition2D());
	}
	vector<RoadInterface*>::iterator bIt, bEnd;
	for(bIt = mBoundaryRoads.begin(), bEnd = mBoundaryRoads.end(); bIt != bEnd; bIt++)
	{
		installRoad(*bIt, nodeMap);
	}
	vector<RoadInterface*>::iterator fIt, fEnd;
	for(fIt = mFilamentRoads.begin(), fEnd = mFilamentRoads.end(); fIt != fEnd; fIt++)
	{
		installRoad(*fIt, nodeMap);
	}
}

void WorldCell::installRoad(RoadInterface* r, map<NodeInterface*, NodeInterface*> &nodeMap)
{
	//create the road node
	assert(typeid(*r) == typeid(WorldRoad));
	WorldRoad *wr = static_cast<WorldRoad*>(r);
	const vector<RoadId> &roadSegs(wr->getRoadSegmentList());
	vector<RoadId>::const_iterator rIdIt, rIdEnd;
	for(rIdIt = roadSegs.begin(), rIdEnd = roadSegs.end(); rIdIt != rIdEnd; rIdIt++)
	{
		// check we have the nodes, if not add them
		NodeInterface *srcNd = mParentRoadGraph.getSrcNode(*rIdIt);
		NodeInterface *dstNd = mParentRoadGraph.getDstNode(*rIdIt);
		if(nodeMap.find(srcNd) == nodeMap.end()) 
			nodeMap[srcNd] = createNode(srcNd->getPosition2D());
		if(nodeMap.find(dstNd) == nodeMap.end()) 
			nodeMap[dstNd] = createNode(dstNd->getPosition2D());

		// create the road seg
		//createRoad(nodeMap[srcNd], nodeMap[dstNd]);
		RoadId rd;
		if(mRoadGraph.addRoad(nodeMap[srcNd]->mNodeId, nodeMap[dstNd]->mNodeId, rd))
		{
			mRoadGraph.setRoad(rd, r);
		}
		else 
			throw Exception(Exception::ERR_ITEM_NOT_FOUND, "Road not Installed", "WorldCell::installRoad");
	}
}

void WorldCell::setBoundary(const vector<NodeInterface*> &nodeCycle)
{
	clearRoadGraph();
	clearBoundary();
	mBoundaryCycle = nodeCycle;

	size_t i, j, N = mBoundaryCycle.size();
	for(i=0; i<N; i++)
	{
		j = (i+1) % N;

		// This is messy, but fuck it i don't have the time
		WorldNode* wn1 = static_cast<WorldNode*>(mBoundaryCycle[i]);
		WorldNode* wn2 = static_cast<WorldNode*>(mBoundaryCycle[j]);

		RoadId rd = mSimpleRoadGraph.getRoad(wn1->mSimpleNodeId, wn2->mSimpleNodeId);
		mBoundaryRoads.push_back(mSimpleRoadGraph.getRoad(rd));
	}

	// set up listeners to receive invalidate events from roads
	vector<RoadInterface*>::iterator rIt, rEnd;
	for(rIt = mBoundaryRoads.begin(), rEnd = mBoundaryRoads.end(); rIt != rEnd; rIt++)
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
	mFilamentRoads.push_back(f);
	invalidate();
}

void WorldCell::removeFilament(WorldRoad* f)
{
	vector<RoadInterface*>::iterator rIt, rEnd;
	for(rIt = mFilamentRoads.begin(), rEnd = mFilamentRoads.end(); rIt != rEnd; rIt++)
	{	
		assert(typeid(*(*rIt)) == typeid(WorldRoad));
		WorldRoad* wr = static_cast<WorldRoad*>(*rIt);
		if(wr == f)
		{
			wr->detach(this);
			mFilamentRoads.erase(rIt);
			break;
		}
	}
	invalidate();
}

void WorldCell::clearBoundary()
{
	BOOST_FOREACH(RoadInterface* ri, mBoundaryRoads)
	{
		if(typeid(*ri) == typeid(WorldRoad))
			static_cast<WorldRoad*>(ri)->detach(this);
	}
	mBoundaryRoads.clear();
	invalidate();
}

void WorldCell::clearFilaments()
{
	vector<RoadInterface*>::iterator rIt, rEnd;
	for(rIt = mFilamentRoads.begin(), rEnd = mFilamentRoads.end(); rIt != rEnd; rIt++)
	{
		if(typeid(*(*rIt)) == typeid(WorldRoad))
			static_cast<WorldRoad*>(*rIt)->detach(this);
	}
	mFilamentRoads.clear();
	invalidate();
}

GrowthGenParams WorldCell::getGenParams() const
{
	return mGrowthGenParams;
}
void WorldCell::setGenParams(const GrowthGenParams &g)
{
	mGrowthGenParams = g;
	invalidate();
}

const vector<RoadInterface*>& WorldCell::getBoundaryRoads() const
{
	return mBoundaryRoads;
}

const vector<NodeInterface*>& WorldCell::getBoundaryCycle() const
{
	return mBoundaryCycle;
}

const vector<RoadInterface*>& WorldCell::getFilaments() const
{
	return mFilamentRoads;
}

NodeInterface* WorldCell::createNode(const Vector2 &pos)
{
	SimpleNode *sn = new SimpleNode(mRoadGraph, pos);
	NodeId nd = mRoadGraph.addNode(sn);
	sn->mNodeId = nd;
	return sn;
}

RoadInterface* WorldCell::createRoad(NodeInterface *n1, NodeInterface *n2)
{
	RoadId rd;
	if(mRoadGraph.addRoad(n1->mNodeId, n2->mNodeId, rd))
	{
		SimpleRoad *sr = new SimpleRoad(n1, n2);
		sr->setWidth(mGrowthGenParams.roadWidth);
		mRoadGraph.setRoad(rd, sr);
		return sr;
	}
	else
		return 0;
}

void WorldCell::deleteRoad(RoadInterface *ri)
{
	// remove it from the graph
	mRoadGraph.removeRoad(ri->getSrcNode()->mNodeId, ri->getDstNode()->mNodeId);
	// delete it
	delete ri;
}

bool WorldCell::isOnBoundary(NodeInterface *ni)
{
	vector<NodeInterface*>::const_iterator nIt, nEnd;
	for(nIt = mBoundaryCycle.begin(), nEnd = mBoundaryCycle.end(); nIt != nEnd; nIt++)
	{
		if((*nIt) == ni) return true;
	}
	return false;
}


bool WorldCell::compareBoundary(const vector<NodeInterface*>& nodeCycle) const
{
	// compare size
	if(nodeCycle.size() != mBoundaryCycle.size()) 
		return false;

	// find match
	size_t i, j, offset, N = nodeCycle.size();
	for(i = 0; i < N; i++)
	{
		if(nodeCycle[0] == mBoundaryCycle[i]) 
			break;
	}
	if(i >= N)
		return false;
	else
		offset = i;

	// try forwards
	for(i = 1; i < N; i++)
	{
		if(nodeCycle[i] != mBoundaryCycle[(i + offset) % N])
			break;
	}
	if(i >= N) 
		return true;

	// try backwards
	for(i = 1, j = offset-1; i < N; i++, j--)
	{
		if(nodeCycle[i] != mBoundaryCycle[j % N])
			return false;
	}
	return true;
}

RoadInterface* WorldCell::getRoad(NodeInterface* n1, NodeInterface* n2)
{
	return mRoadGraph.getRoad(mRoadGraph.getRoad(n1->mNodeId, n2->mNodeId));
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
			RoadId rd = mRoadGraph.getRoad(n1, n2);
			RoadInterface* ri = mRoadGraph.getRoad(rd);
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
				RoadId rd = mRoadGraph.getRoad(n1, n2);
				RoadInterface* ri = mRoadGraph.getRoad(rd);
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
	mRoadNetwork->position(a1);
	mRoadNetwork->normal(aNorm);
	mRoadNetwork->textureCoord(uMin, 0);
	mRoadNetwork->position(a2);
	mRoadNetwork->normal(aNorm);
	mRoadNetwork->textureCoord(uMin, 1);
	mRoadNetwork->position(b1);
	mRoadNetwork->normal(bNorm);
	mRoadNetwork->textureCoord(uMax, 0);

	mRoadNetwork->position(a2);
	mRoadNetwork->normal(aNorm);
	mRoadNetwork->textureCoord(uMin, 1);
	mRoadNetwork->position(b2);
	mRoadNetwork->normal(bNorm);
	mRoadNetwork->textureCoord(uMax, 1);
	mRoadNetwork->position(b1);
	mRoadNetwork->normal(bNorm);
	mRoadNetwork->textureCoord(uMax, 0);
}

void WorldCell::createRoad(const RoadId rd, ManualObject *m)
{
	RoadInterface* r = mRoadGraph.getRoad(rd);
	if(typeid(*r) != typeid(WorldRoad))
	{
		Vector3 a1,a2, b1, b2;
		boost::tie(a1,a2) = mRoadGraph.getSrcNode(rd)->getRoadJunction(rd);
		boost::tie(b2,b1) = mRoadGraph.getDstNode(rd)->getRoadJunction(rd);
		buildSegment(a1, a2, Vector3::UNIT_Y, b1, b2, Vector3::UNIT_Y, 0, 4);
	}
}

void WorldCell::showSelected(bool show)
{
	std::vector<RoadInterface*>::iterator bIt, bEnd;
	for(bIt = mBoundaryRoads.begin(), bEnd = mBoundaryRoads.end(); bIt != bEnd; bIt++)
	{
		if(typeid(*(*bIt)) == typeid(WorldRoad))
			static_cast<WorldRoad*>(*bIt)->showSelected(show);
	}
}

bool WorldCell::loadXML(const TiXmlHandle& cellRoot)
{
	// Load GenParams, not Smart Enough to do our own boundary, see parent
	{
		TiXmlElement *element = cellRoot.FirstChild("genparams").FirstChild().Element();

		for(; element; element=element->NextSiblingElement())
		{
			string key = element->Value();
			
			if(key == "seed")
				element->QueryIntAttribute("value", &mGrowthGenParams.seed);
			else if(key == "segmentSize")
				element->QueryFloatAttribute("value", &mGrowthGenParams.segmentSize);
			else if(key == "segmentDeviance")
				element->QueryFloatAttribute("value", &mGrowthGenParams.segmentDeviance);
			else if(key == "degree") 
			{
				int deg;
				element->QueryIntAttribute("value", &deg);
				mGrowthGenParams.degree = static_cast<unsigned int>(deg);
			}else if(key == "degreeDeviance")
				element->QueryFloatAttribute("value", &mGrowthGenParams.degreeDeviance);
			else if(key == "snapSize")
				element->QueryFloatAttribute("value", &mGrowthGenParams.snapSize);
			else if(key == "snapDeviance")
				element->QueryFloatAttribute("value", &mGrowthGenParams.snapDeviance);
			else if(key == "roadWidth")
				element->QueryFloatAttribute("value", &mGrowthGenParams.roadWidth);
			else if(key == "buildingHeight")
				element->QueryFloatAttribute("value", &mGrowthGenParams.buildingHeight);
			else if(key == "buildingDeviance")
				element->QueryFloatAttribute("value", &mGrowthGenParams.buildingDeviance);
		}
	}
	return true;
}

TiXmlElement* WorldCell::saveXML()
{
	TiXmlElement *root = new TiXmlElement("cell"); 

	TiXmlElement *cycle = new TiXmlElement("cycle"); 
	root->LinkEndChild(cycle);

	for(size_t i=0; i<mBoundaryCycle.size(); i++) 
	{	
		TiXmlElement * node;
		node = new TiXmlElement("node");  
		node->SetAttribute("id", (int) mBoundaryCycle[i]);
		cycle->LinkEndChild(node);
	}

	TiXmlElement *growthGenParams = new TiXmlElement("genparams"); 
	root->LinkEndChild(growthGenParams);

	TiXmlElement *seed = new TiXmlElement("seed");  
	seed->SetAttribute("value", mGrowthGenParams.seed);
	growthGenParams->LinkEndChild(seed);

	TiXmlElement *segmentSize = new TiXmlElement("segmentSize");  
	segmentSize->SetDoubleAttribute("value", mGrowthGenParams.segmentSize);
	growthGenParams->LinkEndChild(segmentSize);

	TiXmlElement *segmentDeviance = new TiXmlElement("segmentDeviance");  
	segmentDeviance->SetDoubleAttribute("value", mGrowthGenParams.segmentDeviance);
	growthGenParams->LinkEndChild(segmentDeviance);

	TiXmlElement *degree = new TiXmlElement("degree");  
	degree->SetAttribute("value", mGrowthGenParams.degree);
	growthGenParams->LinkEndChild(degree);

	TiXmlElement *degreeDeviance = new TiXmlElement("degreeDeviance");  
	degreeDeviance->SetDoubleAttribute("value", mGrowthGenParams.degreeDeviance);
	growthGenParams->LinkEndChild(degreeDeviance);

	TiXmlElement *snapSize = new TiXmlElement("snapSize");  
	snapSize->SetDoubleAttribute("value", mGrowthGenParams.snapSize);
	growthGenParams->LinkEndChild(snapSize);

	TiXmlElement *snapDeviance = new TiXmlElement("snapDeviance");  
	snapDeviance->SetDoubleAttribute("value", mGrowthGenParams.snapDeviance);
	growthGenParams->LinkEndChild(snapDeviance);

	TiXmlElement *roadWidth = new TiXmlElement("roadWidth");  
	roadWidth->SetDoubleAttribute("value", mGrowthGenParams.roadWidth);
	growthGenParams->LinkEndChild(roadWidth);

	TiXmlElement *buildingHeight = new TiXmlElement("buildingHeight");  
	buildingHeight->SetDoubleAttribute("value", mGrowthGenParams.buildingHeight);
	growthGenParams->LinkEndChild(buildingHeight);

	TiXmlElement *buildingDeviance = new TiXmlElement("buildingDeviance");  
	buildingDeviance->SetDoubleAttribute("value", mGrowthGenParams.buildingDeviance);
	growthGenParams->LinkEndChild(buildingDeviance);

	return root;
}

void WorldCell::showRoads(bool show)
{
	mShowRoads = show;
	if(mRoadNetwork) mRoadNetwork->setVisible(mShowRoads);
	if(mRoadJunctions) mRoadJunctions->setVisible(mShowRoads);
}

void WorldCell::showBuildings(bool show)
{
	mShowBuildings = show;
	if(mBuildings) mBuildings->setVisible(mShowBuildings);
}


void WorldCell::beginGraphicalCellDecomposition()
{
	// take a copy of the graph
	mGCDRoadGraph = mRoadGraph;

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
	mOverlay = new ManualObject(mName+"o");
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
	mOverlay2 = new ManualObject(mName+"o2");
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
	pointList.reserve(mBoundaryCycle.size());
	BOOST_FOREACH(NodeInterface* ni, mBoundaryCycle)
		pointList.push_back(ni->getPosition3D());
	return pointList;
}

vector<Vector2> WorldCell::getBoundaryPoints2D()
{
	vector<Vector2> pointList;
	pointList.reserve(mBoundaryCycle.size());
	BOOST_FOREACH(NodeInterface* ni, mBoundaryCycle)
		pointList.push_back(ni->getPosition2D());
	return pointList;
}

Real WorldCell::calcArea2D()
{
	return Geometry::polygonArea(getBoundaryPoints2D());
}
