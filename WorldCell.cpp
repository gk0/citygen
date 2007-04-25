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

using namespace Ogre;
using namespace std;
int WorldCell::mInstanceCount = 0;

WorldCell::WorldCell(RoadGraph &p, RoadGraph &s)
 : 	mParentRoadGraph(p),
	mSimpleRoadGraph(s)
{
	init();
}
/*
WorldCell::WorldCell(RoadGraph &p, vector<RoadInterface*> &b)
 : 	mParentRoadGraph(p)
{
	init();
	setBoundary(b);
}*/


WorldCell::WorldCell(RoadGraph &p, RoadGraph &s, vector<NodeInterface*> &n, vector<RoadInterface*> &b)
 : 	mParentRoadGraph(p),
	mSimpleRoadGraph(s)
{
	init();
	setBoundary(n, b);
}

void WorldCell::init()
{
	// set up some default growth gen params
	mGrowthGenParams.seed = 0;
	mGrowthGenParams.segmentSize = 6;
	mGrowthGenParams.segmentDeviance = 0.4;
	mGrowthGenParams.degree = 4;
	mGrowthGenParams.degreeDeviance = 0.01;
	mGrowthGenParams.snapSize = 2.4;
	mGrowthGenParams.snapDeviance = 0.1;

	mManualObject = 0;
	mManualObject2 = 0;

	mName = "Cell"+StringConverter::toString(mInstanceCount++);
	mSceneNode = WorldFrame::getSingleton().getSceneManager()->getRootSceneNode()->createChildSceneNode(mName);
}

WorldCell::~WorldCell()
{
	clearBoundary();
	clearFilaments();
	clearRoadGraph();
	destroySceneObject();
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
	if(mManualObject) {
		mSceneNode->detachObject(mManualObject->getName());
		mSceneNode->getCreator()->destroyManualObject(mManualObject);
		delete mManualObject;
		mManualObject = 0;
	}
	if(mManualObject2) {
		mSceneNode->detachObject(mManualObject2->getName());
		mSceneNode->getCreator()->destroyManualObject(mManualObject2);
		delete mManualObject2;
		mManualObject2 = 0;
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
	// get time:
	wxDateTime t2, t1 = wxDateTime::UNow();

	//1. Clear Road Graph and destroy scene object
	destroySceneObject();
	clearRoadGraph();
	installGraph();

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
	//-direction;

	int seed = mGrowthGenParams.seed;

	Ogre::Real segDevSz = mGrowthGenParams.segmentSize * mGrowthGenParams.segmentDeviance;
	Ogre::Real segSzBase = mGrowthGenParams.segmentSize - segDevSz;

	//int degDev = mGrowthGenParams.degree * mGrowthGenParams.degreeDeviance;
	//int degBase = mGrowthGenParams.degree - degDev;
	Real degDev = mGrowthGenParams.degree * mGrowthGenParams.degreeDeviance;
	Real degBase = mGrowthGenParams.degree - (degDev / 2);

	queue<NodeInterface*> q;
	q.push(startNode);

	srand(seed++);

	while(!q.empty())
	{
		NodeInterface* currentNode = q.front();
		q.pop();

		Ogre::Radian theta(Math::TWO_PI / (degBase + (degDev * ((float)rand()/(float)RAND_MAX))));

		// alter our direction vector 
		for(unsigned int i=0; i < mGrowthGenParams.degree; i++)
		{
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
				NodeInterface *cursorNode = new SimpleNode(newPoint);
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
	// get time interval
	t2 = wxDateTime::UNow();

	// Create the Road Network
	mManualObject = new ManualObject(mName);
	mManualObject->begin("gk/Road", Ogre::RenderOperation::OT_TRIANGLE_LIST);

	// Road Segments
	RoadIterator rIt, rEnd;
	for(boost::tie(rIt, rEnd) = mRoadGraph.getRoads(); rIt != rEnd; rIt++)
		createRoad(*rIt, mManualObject);
	
	// Road Junctions
	int junctionFailCount = 0;
	NodeIterator nIt, nEnd;
	for(boost::tie(nIt, nEnd) = mRoadGraph.getNodes(); nIt != nEnd; nIt++)
		if(!createJunction(*nIt, mManualObject)) junctionFailCount++;

	mManualObject->end();
	mSceneNode->attachObject(mManualObject);


	//TODO: oh shit, it works for small cells but not bigns

	// build boxes
	// check the road graph to get a count of the number of cycles
	vector< vector<NodeInterface*> > nodeCycles;
	vector< vector<RoadInterface*> > roadCycles;
	vector<RoadInterface*> filaments;
	mRoadGraph.extractPrimitives(filaments, nodeCycles, roadCycles);


	//TODO: oh shit, it works for small cells but not bigns


	// declare the manual object
	mManualObject2 = new ManualObject(mName+"b");
	mManualObject2->begin("gk/Building", Ogre::RenderOperation::OT_TRIANGLE_LIST);

	// create lot of little boxes with our cycles
	vector< vector<NodeInterface*> >::const_iterator ncIt, ncEnd;
	vector< vector<RoadInterface*> >::const_iterator rcIt, rcEnd;
	for(ncIt = nodeCycles.begin(), ncEnd = nodeCycles.end(),
		rcIt = roadCycles.begin(), rcEnd = roadCycles.end(); 
		ncIt != ncEnd, rcIt != rcEnd; ncIt++, rcIt++)
	{
		// get the foot print
		vector<Vector2> footprint, result;
		if(!extractFootprint(*ncIt, *rcIt, footprint))
			continue;

		// get base foundation height
		Real foundation = (*(ncIt->begin()))->getPosition3D().y - 1;
		Real height = foundation + 1.2 + (((float)rand()/(float)RAND_MAX) * 4);

		//
		createBuilding(mManualObject2, footprint, foundation, height);

	}
	mManualObject2->end();
	mSceneNode->attachObject(mManualObject2);

	wxLongLong lTimeGen = (t2 - t1).GetMilliseconds();
	wxLongLong lTimeTotal = (wxDateTime::UNow() - t1).GetMilliseconds();
	String strTimeGen = static_cast<const char*>(lTimeGen.ToString().mb_str());
	String strTimeTotal = static_cast<const char*>(lTimeTotal.ToString().mb_str());
	LogManager::getSingleton().logMessage("Cell build time: "+strTimeTotal+"ms. ("+strTimeGen+"ms)", LML_CRITICAL);
	LogManager::getSingleton().logMessage(" junction fail count: "+StringConverter::toString(junctionFailCount), LML_CRITICAL);
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
/*
Vector2 WorldCell::getCentre() const
{
	if(!isValid())
	{
		// find the centre point of cell using a simple averaging technique
/	Vector2 centre(0,0);
		set<RoadInterface*>::const_iterator rIt, rEnd;
		for(rIt = mBoundaryRoads.begin(), rEnd = mBoundaryRoads.end(); rIt != rEnd; rIt++)
		{
			centre += (*rIt)->getSrcNode()->getPosition2D();
			centre += (*rIt)->getDstNode()->getPosition2D();
		}
		if(centre.x != 0) centre.x /= (mBoundaryRoads.size()*2);
		if(centre.y != 0) centre.y /= (mBoundaryRoads.size()*2);
		return centre;
	}
	else
		return mCentre;
}*/

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

void WorldCell::setBoundary(const vector<NodeInterface*> &nodeCycle, const vector<RoadInterface*> &b)
{
	clearRoadGraph();
	clearBoundary();
	mBoundaryRoads = b;
	mBoundaryCycle = nodeCycle;

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
/*
void WorldCell::setBoundary(const vector<RoadInterface*> &b)
{
	if(b != mBoundaryRoads)
	{
		clearRoadGraph();
		clearBoundary();
		mBoundaryRoads = b;
		mBoundaryCycle = getBoundaryCycle();

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
}*/

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
		}
	}
	invalidate();
}

void WorldCell::clearBoundary()
{
	vector<RoadInterface*>::iterator rIt, rEnd;
	for(rIt = mBoundaryRoads.begin(), rEnd = mBoundaryRoads.end(); rIt != rEnd; rIt++)
	{
		if(typeid(*(*rIt)) == typeid(WorldRoad))
			static_cast<WorldRoad*>(*rIt)->detach(this);
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

GrowthGenParams WorldCell::getGrowthGenParams() const
{
	return mGrowthGenParams;
}
void WorldCell::setGrowthGenParams(const GrowthGenParams &g)
{
	mGrowthGenParams = g;
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
	SimpleNode *sn = new SimpleNode(pos);
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
/*
// there is a decision here about whether or not i should have just calculate this when 
// working out the cycle but I've decided that since this data is need so less often 
// than just the boundary set it is an acceptable overhead
// there isn't that much work in it!
vector<NodeInterface*> WorldCell::getBoundaryCycle()
{
	// the direction is dictated by the order found in the first road seg, so random!!
	vector<NodeInterface*> cycle;


	vector<RoadInterface*> boundaries(mBoundaryRoads);
	vector<RoadInterface*>::iterator rIt = boundaries.begin(), rEnd; 
	if(rIt != boundaries.end())
	{
		cycle.push_back((*rIt)->getSrcNode());
		cycle.push_back((*rIt)->getDstNode());
		boundaries.erase(rIt);
		rIt = boundaries.begin();
	}
	while(rIt != boundaries.end())
	{
		for(rEnd = boundaries.end(); rIt != rEnd; rIt++)
		{
			size_t lastCycleIndex = cycle.size() - 1;
			if(cycle[lastCycleIndex] == (*rIt)->getSrcNode())
			{
				cycle.push_back((*rIt)->getDstNode());
				boundaries.erase(rIt);
				rIt = boundaries.begin();
				break;
			}
			else if(cycle[lastCycleIndex] == (*rIt)->getDstNode())
			{
				cycle.push_back((*rIt)->getSrcNode());
				boundaries.erase(rIt);
				rIt = boundaries.begin();
				break;
			}
		}
	}
	return cycle;
}
*/

bool WorldCell::isOnBoundary(NodeInterface *ni)
{
	vector<NodeInterface*>::const_iterator nIt, nEnd;
	for(nIt = mBoundaryCycle.begin(), nEnd = mBoundaryCycle.end(); nIt != nEnd; nIt++)
	{
		if((*nIt) == ni) return true;
	}
	return false;
}

bool WorldCell::compareBoundary(const vector<RoadInterface*>& roadCycle) const
{
	// compare size
	if(roadCycle.size() != mBoundaryRoads.size()) 
		return false;

	// find match
	size_t i, j, offset, N = roadCycle.size();
	for(i = 0; i < N; i++)
	{
		if(roadCycle[0] == mBoundaryRoads[i]) 
			break;
	}
	if(i >= N)
		return false;
	else
		offset = i;

	// try forwards
	for(i = 1; i < N; i++)
	{
		if(roadCycle[i] != mBoundaryRoads[(i + offset) % N])
			break;
	}
	if(i >= N)
		return true;

	// try backwards
	for(i = 1, j = offset-1; i < N; i++, j--)
	{
		if(roadCycle[i] != mBoundaryRoads[j % N])
			return false;
	}
	return true;
}

bool WorldCell::extractFootprint(const vector<NodeInterface*> &nodeCycle, 
								const vector<RoadInterface*> &roadCycle,
								vector<Vector2> &footprint)
{
	// get size
	size_t i, j, N = nodeCycle.size();

	// check cycle
	if(N < 3 || N != roadCycle.size())
		throw Exception(Exception::ERR_INVALIDPARAMS, 
		"Invalid number of nodes/roads in cycle", "WorldCell::extractFootprint");

	// prepare footprint data structures
	vector<Vector2> originalFootprint, newFootprint(N);
	originalFootprint.reserve(N);
	for(i = 0; i < N; i++) originalFootprint.push_back(nodeCycle[i]->getPosition2D()); 

	// create footprint edge structure
	vector< pair<Vector2, Vector2> > edges;
	edges.reserve(N);

	// get footprint edge vectors
	Vector2 prevPoint = originalFootprint[(N-1)];
	for(i = 0; i < N; i++)
	{
		Vector2 dir(originalFootprint[i] - prevPoint);
		dir = dir.perpendicular();
		dir.normalise();
		dir *= roadCycle[i]->getWidth();
		edges.push_back(make_pair(prevPoint + dir, originalFootprint[i] + dir));
		prevPoint = originalFootprint[i];
	}
	//for(i = 0; i < N; i++)
	//{
	//	j = (i + 1) % N;
	//	Vector2 dir(originalFootprint[j] - originalFootprint[i]);
	//	dir = dir.perpendicular();
	//	dir.normalise();
	//	dir *= roadCycle[j]->getWidth();
	//	edges.push_back(make_pair(originalFootprint[i] + dir, originalFootprint[j] + dir));
	//}

	// calculate footprint points from edges
	for(i = 0; i < N; i++)
	{
		j = (i + 1) % N;

		// get edge intersection point
		if(!Geometry::lineIntersect(edges[i].first, edges[i].second, edges[j].first, 
			edges[j].second, newFootprint[i]))
			return false;

		// require point to be inside original footprint
		if(!Geometry::isInside(newFootprint[i], originalFootprint))
			return false;
	}
	footprint = newFootprint;
	return true;
}

void WorldCell::createBuilding(ManualObject* m, const vector<Vector2> &footprint,
							   const Real foundation, const Real height)
{
	vector<Vector2> result;

	// roof
	if(Triangulate::Process(footprint, result))
	{
		// sides
		size_t i, j, N = footprint.size();
		for(i = 0; i < N; i++)
		{
			j = (i + 1) % N;
			Vector2 perp((footprint[i] - footprint[j]).perpendicular());
			Vector3 normal(perp.x, 0, perp.y);
			normal.normalise();
			m->position(Vector3(footprint[i].x, height, footprint[i].y));
			m->normal(normal);
			m->position(Vector3(footprint[j].x, foundation, footprint[j].y));
			m->normal(normal);
			m->position(Vector3(footprint[i].x, foundation, footprint[i].y));
			m->normal(normal);


			m->position(Vector3(footprint[i].x, height, footprint[i].y));
			m->normal(normal);
			m->position(Vector3(footprint[j].x, height, footprint[j].y));
			m->normal(normal);
			m->position(Vector3(footprint[j].x, foundation, footprint[j].y));
			m->normal(normal);
		}

		for(size_t i=0; i<result.size(); i+=3)
		{
			m->position(Vector3(result[i+2].x, height, result[i+2].y));
			m->normal(Vector3::UNIT_Y);
			m->position(Vector3(result[i+1].x, height, result[i+1].y));
			m->normal(Vector3::UNIT_Y);
			m->position(Vector3(result[i].x, height, result[i].y));
			m->normal(Vector3::UNIT_Y);
		}
	}
}

void WorldCell::buildSegment(const Vector3 &a1, const Vector3 &a2, const Vector3 &aNorm,
			const Vector3 &b1, const Vector3 &b2, const Vector3 &bNorm, Real uMin, Real uMax)
{
	// create road segment
	mManualObject->position(a1);
	mManualObject->normal(aNorm);
	mManualObject->textureCoord(uMin, 0);
	mManualObject->position(a2);
	mManualObject->normal(aNorm);
	mManualObject->textureCoord(uMin, 1);
	mManualObject->position(b1);
	mManualObject->normal(bNorm);
	mManualObject->textureCoord(uMax, 0);

	mManualObject->position(a2);
	mManualObject->normal(aNorm);
	mManualObject->textureCoord(uMin, 1);
	mManualObject->position(b2);
	mManualObject->normal(bNorm);
	mManualObject->textureCoord(uMax, 1);
	mManualObject->position(b1);
	mManualObject->normal(bNorm);
	mManualObject->textureCoord(uMax, 0);
}

void WorldCell::createRoad(const RoadId rd, ManualObject *m)
{
	RoadInterface* r = mRoadGraph.getRoad(rd);
	if(typeid(*r) != typeid(WorldRoad))
	{
		//const Vector3 a3, b3;
		//a3 = mRoadGraph.getSrcNode(*rIt)->getPosition3D();
		//b3 = mRoadGraph.getDstNode(*rIt)->getPosition3D();
		Real roadWidth = r->getWidth();

		Vector2 a, b, dir, offset;
		a = mRoadGraph.getSrcNode(rd)->getPosition2D();
		b = mRoadGraph.getDstNode(rd)->getPosition2D();
		dir = b - a;
		dir.normalise(); // for radius crap
		offset = dir.perpendicular();
		offset.normalise();
		offset *= roadWidth;

		// give a little room for junctions
		// TODO: the problem is MUCH more complicated than this
		//dir *= (roadWidth);
		//a += dir;
		//b -= dir;

		//mManualObject->position(mRoadGraph.getSrcNode(*rIt)->getPosition3D());
		//mManualObject->position(mRoadGraph.getDstNode(*rIt)->getPosition3D());

		Real ay = mRoadGraph.getSrcNode(rd)->getPosition3D().y - 0.002;
		Real by = mRoadGraph.getDstNode(rd)->getPosition3D().y - 0.001;
		Vector3 a1(a.x - offset.x, ay, a.y - offset.y);
		Vector3 a2(a.x + offset.x, ay, a.y + offset.y);
		Vector3 b1(b.x - offset.x, by, b.y - offset.y);
		Vector3 b2(b.x + offset.x, by, b.y + offset.y);

		buildSegment(a1, a2, Vector3::UNIT_Y, b1, b2, Vector3::UNIT_Y, 0, 4);
	}
}


bool WorldCell::createJunction(const NodeId nd, ManualObject *m)
{
	// how many roads connect here
	size_t degree = mRoadGraph.getDegree(nd);

	switch(degree)
	{
	case 0:
	case 1:
	case 2:
		// nothing to do here
		return true;
	case 3:
		//createTJunction(nd, m);
		return true;
	case 4:
	case 5:
	case 6:
	case 7:
		//createTJunction(nd, m);
		break;
	default:
		// can't do this aggh
		return false;
	}

	NodeInterface *node = mRoadGraph.getNode(nd);
	Vector2 nodePos2D = node->getPosition2D();
	Real height = node->getPosition3D().y + 0.1;

	//THINK!!
	//
	RoadId firstRoad, previousRoad, currentRoad;
	NodeId currentNode;

	vector<Vector2> pointlist;
	RoadIterator2 rIt, rEnd;
	boost::tie(rIt, rEnd) = mRoadGraph.getRoadsFromNode(nd); 
	firstRoad = previousRoad = *rIt;

	// start with the second road using the first as prev
	for(size_t i = 1; i < degree; i++)
	{
		mRoadGraph.getCounterClockwiseMostFromPrev(mRoadGraph.getDst(previousRoad), nd, currentNode);
		currentRoad = mRoadGraph.getRoad(nd, currentNode);
		Vector2 pos;
		if(getRoadBounaryIntersection(previousRoad, currentRoad, pos))
			pointlist.push_back(pos);
		else
			return false;
		previousRoad = currentRoad;
	}
	Vector2 pos;
	if(getRoadBounaryIntersection(previousRoad, firstRoad, pos))
		pointlist.push_back(pos);
	else
		return false;

	//
	//Triangulate it 
	vector<Vector2> result;
	if(Triangulate::Process(pointlist, result))
	{
		for(size_t i=0; i<result.size(); i+=3)
		{
			m->position(Vector3(result[i+2].x, height, result[i+2].y));
			m->normal(Vector3::UNIT_Y);
			m->position(Vector3(result[i+1].x, height, result[i+1].y));
			m->normal(Vector3::UNIT_Y);
			m->position(Vector3(result[i].x, height, result[i].y));
			m->normal(Vector3::UNIT_Y);
		}
		return true;
	}
	else return false;
}

bool WorldCell::getRoadBounaryIntersection(const RoadId leftR, const RoadId rightR, Vector2 &pos)
{
	Real lWidth, rWidth;
	Vector2 l1, l2, r1, r2, lOffset, rOffset;

	l1 = mRoadGraph.getSrcNode(leftR)->getPosition2D();
	l2 = mRoadGraph.getDstNode(leftR)->getPosition2D();
	r1 = mRoadGraph.getSrcNode(rightR)->getPosition2D();
	r2 = mRoadGraph.getDstNode(rightR)->getPosition2D();

	lWidth = mRoadGraph.getRoad(leftR)->getWidth();
	rWidth = mRoadGraph.getRoad(rightR)->getWidth();

	lOffset = (l2 - l1).perpendicular();
	lOffset.normalise();
	lOffset *= lWidth;

	rOffset = (r2 - r1).perpendicular();
	rOffset.normalise();
	rOffset *= rWidth;

	l1 -= lOffset;
	l2 -= lOffset;
	r1 += rOffset;
	r2 += rOffset;

	return Geometry::lineIntersect(l1, l2, r1, r2, pos);
}
