#include "stdafx.h"
#include "WorldNode.h"
#include "WorldRoad.h"
#include "WorldFrame.h"
#include "SimpleNode.h"
#include "Geometry.h"

int WorldRoad::mRoadCount = 0;
using namespace Ogre;

WorldRoad::WorldRoad(WorldNode* src, WorldNode* dst, RoadGraph& g, 
					 RoadGraph& s, Ogre::SceneManager *creator, bool bind)
	: mRoadGraph(g),
	  mSimpleRoadGraph(s),
	  mRoadSegSz(10), //DEBUG: set to v. big 50 normal is 10
	  mRoadWidth(1.0),
	  mRoadDeviance(30),
	  mManualObject(0)
{
	mSimpleRoadGraph.addRoad(src->mSimpleNodeId, dst->mSimpleNodeId, mSimpleRoadId);
	mSimpleRoadGraph.setRoad(mSimpleRoadId, this);

	mName = "Road" + StringConverter::toString(mRoadCount++);
	// create our scene node
	mSceneNode = creator->getRootSceneNode()->createChildSceneNode(mName);

	//omg i should like draw a line from my old node to my new node
	bindToGraph = bind;

	//mSrcNode->attach(this);
	//mDstNode->attach(this);

	//mSrcNode->invalidate();
	//mDstNode->invalidate();

	plotRoad();
}

WorldRoad::~WorldRoad()
{
	destroyRoadObject();
	SceneManager* destroyer = mSceneNode->getCreator();
	destroyer->destroySceneNode(mSceneNode->getName());

	destroyRoadGraph();
	mSimpleRoadGraph.removeRoad(mSimpleRoadId);
}


//void WorldRoad::setSrcNode(WorldNode* src)
//{
//	if(src != mSrcNode)
//	{
//		mSrcNode->detach(this);
//		mSrcNode = src;
//		mSrcNode->attach(this);
//		mSrcNode->invalidate();
//	}
//}
//
//void WorldRoad::setDstNode(WorldNode* dst)
//{
//	if(dst != mDstNode)
//	{
//		mDstNode->detach(this);
//		mDstNode = dst;
//		mDstNode->attach(this);
//		mDstNode->invalidate();
//	}
//}

NodeInterface* WorldRoad::getSrcNode() const
{
	return mSimpleRoadGraph.getSrcNode(mSimpleRoadId);
}

NodeInterface* WorldRoad::getDstNode() const
{
	return mSimpleRoadGraph.getDstNode(mSimpleRoadId);
}
/*
//simple
void WorldRoad::createRoadObject()
{
	//omg i should like draw a line from my old node to my new node
	Vector3 offset(0,3,0);
	mManualObject = new ManualObject(mName); 
	mManualObject->begin("gk/Hilite/Red", Ogre::RenderOperation::OT_LINE_LIST);
	mManualObject->position(mSrcNode->getPosition()+offset); 
	mManualObject->position(mDstNode->getPosition()+offset); 
	mManualObject->end(); 

	mSceneNode->attachObject(mManualObject);
}
*/

void WorldRoad::plotRoad()
{
	// Carefull Now!
	// if its valid someone must've plotted the road
	if(isValid()) return;		

	// always destroy previous
	mPlotList.clear();

	Ogre::Real segmentSizeSq(mRoadSegSz * mRoadSegSz);
	Ogre::Vector2 direction;
	Ogre::Angle devAngle(mRoadDeviance);
	Vector3 groundClearance(0,0.3,0);
	Vector2 cursor2D(getSrcNode()->getPosition2D());
	Vector2 dstPoint2D(getDstNode()->getPosition2D());
	int segCount = 0;

	// set our start node
	mPlotList.push_back(getSrcNode()->getPosition3D() + groundClearance);

	std::stringstream oss;
	oss << "Point "<<(mPlotList.size() - 1)<<": " << mPlotList[mPlotList.size() - 1] << std::endl;

	while(true)
	{
		// if outside range of dest point
		if((cursor2D - dstPoint2D).squaredLength() > segmentSizeSq)
		{
			// get target direction
			direction = dstPoint2D - cursor2D;

			// get the next point
			Vector3 nextPoint3D = findNextPoint(cursor2D, direction, devAngle);
			nextPoint3D += groundClearance;

			// add our nodes to the road graph
			mPlotList.push_back(nextPoint3D);
			oss << "Point "<<(mPlotList.size() - 1)<<": dist("<< (cursor2D - dstPoint2D).length() <<") seg("
				<< (mPlotList[mPlotList.size() - 2] - mPlotList[mPlotList.size() - 1]).length() <<") "
				<< mPlotList[mPlotList.size() - 1] << std::endl;
			
			// advance cursor	
			cursor2D = Vector2(nextPoint3D.x, nextPoint3D.z);

			// increment road segment count
			segCount++;
		}
		else
			break;
	}
	

//	LogManager::getSingleton().logMessage("Plotted "+StringConverter::toString(mPlotList.size())+" segments.", LML_DEBUG);

	mPlotList.push_back(getDstNode()->getPosition3D() + groundClearance);

//	oss << "Point "<<(mPlotList.size() - 1)<<": " << mPlotList[mPlotList.size() - 1] << std::endl;
//	LogManager::getSingleton().logMessage(oss.str(), LML_DEBUG);

	if(bindToGraph)
	{
		createRoadGraph();
	}
}

void WorldRoad::createRoadGraph()
{
	// always delete the last graph
	destroyRoadGraph();

	if(mPlotList.size() == 2)
	{
		// create first road segment
		RoadId roadSegId;
		mRoadGraph.addRoad(getSrcNode()->mNodeId, getDstNode()->mNodeId, this, roadSegId);
		mRoadSegmentList.push_back(roadSegId);
	}
	else
	{
		// create first road segment
		SimpleNode* bn = new SimpleNode(mPlotList[1]);
		NodeId cursorNode = mRoadGraph.addNode(bn);
		RoadId roadSegId;
		mRoadGraph.addRoad(getSrcNode()->mNodeId, cursorNode, this, roadSegId);
		mRoadSegmentList.push_back(roadSegId);

		// create each road segment
		for(unsigned int i=2; i<(mPlotList.size()-1); i++)
		{
			// add our nodes to the road graph
			SimpleNode* bn = new SimpleNode(mPlotList[i]);
			NodeId nextNodeId = mRoadGraph.addNode(bn);
			mRoadGraph.addRoad(cursorNode, nextNodeId, this, roadSegId);
			mRoadSegmentList.push_back(roadSegId);

			// advance cursor
			cursorNode = nextNodeId;
		}

		// create last road segment
		mRoadGraph.addRoad(cursorNode, getDstNode()->mNodeId, this, roadSegId);
		mRoadSegmentList.push_back(roadSegId);
	}

	//mSrcNode->invalidate();
	//mDstNode->invalidate();
}

void WorldRoad::destroyRoadGraph()
{
	// clear previous road segs.
	if(mRoadSegmentList.size() > 0) 
		mRoadGraph.removeRoad(mRoadSegmentList[0]);
	for(unsigned int i=1; i<mRoadSegmentList.size(); i++)
	{
		NodeId nd = mRoadGraph.getSrc(mRoadSegmentList[i]);
		mRoadGraph.removeRoad(mRoadSegmentList[i]);
		delete mRoadGraph.getNode(nd);
		mRoadGraph.removeNode(nd);
	}

	// clear the list
	mRoadSegmentList.clear();
}

void WorldRoad::build()
{
	//return;

	// always destroy previous
	destroyRoadObject();

	// declare the manual object
	mManualObject = new ManualObject(mName); 
	mManualObject->begin("gk/Road", Ogre::RenderOperation::OT_TRIANGLE_LIST);

	// vars
	Vector3 currRoadSegNormal, nextRoadSegNormal;
	Vector3 currRoadSegVector, nextRoadSegVector;
	Vector2 currRoadSegVector2D, nextRoadSegVector2D;
	Vector2 currRoadSegOffset, nextRoadSegOffset;
	Real currRoadSegLength, nextRoadSegLength;
	Real uMin = 0, uMax = 0;
	Vector3 a, a1, a2, aNormal, b, b1, b2, bNormal, c;

	// init
	if(mPlotList.size() >= 2)
	{
		nextRoadSegVector = mPlotList[1] - mPlotList[0];
		nextRoadSegVector2D.x = nextRoadSegVector.x;
		nextRoadSegVector2D.y = nextRoadSegVector.z;

		// use the 2D road seg vector to calculate length for speed
		nextRoadSegLength = nextRoadSegVector2D.length();

		// calculate the offset of length roadWidth
		nextRoadSegOffset = nextRoadSegVector2D.perpendicular();
		nextRoadSegOffset.normalise();
		nextRoadSegOffset *= mRoadWidth;

		// calculate normal
		nextRoadSegNormal = Vector3(nextRoadSegOffset.x, 0, nextRoadSegOffset.y).crossProduct(nextRoadSegVector);
		nextRoadSegNormal.normalise();
		bNormal = (nextRoadSegNormal + Vector3::UNIT_Y) / 2;


//get the first b from the node
		b = mPlotList[0];
		if(bindToGraph) 
		{
			//mSrcNode->invalidate();
			//getSrcNode()->validate();
			if(mRoadGraph.getSrcNode(mRoadSegmentList[0])->getPosition3D() != mPlotList[0])
				boost::tie(b1, b2) = getSrcNode()->getRoadJunction(mRoadSegmentList[0]);
			else
				boost::tie(b2, b1) = getSrcNode()->getRoadJunction(mRoadSegmentList[0]);
		}
		else
		{
		// calculate vertex positions using the offset
		b1.x = b.x - nextRoadSegOffset.x;
		b1.y = b.y;
		b1.z = b.z - nextRoadSegOffset.y;
		b2.x = b.x + nextRoadSegOffset.x;
		b2.y = b.y;
		b2.z = b.z + nextRoadSegOffset.y;
		}

	}

	for(size_t i=0; i<(mPlotList.size()-2); i++)
	{
		// for a road segment pointA -> pointB
		a = b;
		b = mPlotList[i+1], c = mPlotList[i+2];

		// get current from last runs next vars
		currRoadSegVector = nextRoadSegVector;
		currRoadSegVector2D = nextRoadSegVector2D;
		currRoadSegOffset = nextRoadSegOffset;
		currRoadSegNormal = nextRoadSegNormal;
		currRoadSegLength = nextRoadSegLength;

		// advance A to previous B
		a1 = b1;
		a2 = b2;
		aNormal = bNormal;

		// calculate vertex positions using the offset
		b1.x = b.x - currRoadSegOffset.x;
		b1.y = b.y;
		b1.z = b.z - currRoadSegOffset.y;
		b2.x = b.x + currRoadSegOffset.x;
		b2.y = b.y;
		b2.z = b.z + currRoadSegOffset.y;

		// calculate next road seg vectors to get b normal
		nextRoadSegVector = c - b;
		nextRoadSegVector2D.x = nextRoadSegVector.x;
		nextRoadSegVector2D.y = nextRoadSegVector.z;

		// use the 2D road seg vector to calculate length for speed
		nextRoadSegLength = nextRoadSegVector2D.length();

		// calculate the offset of length roadWidth
		nextRoadSegOffset = nextRoadSegVector2D.perpendicular();
		nextRoadSegOffset.normalise();
		nextRoadSegOffset *= mRoadWidth;

		// calculate normal
		Vector3 nextRoadSegNormal = Vector3(nextRoadSegOffset.x, 0, nextRoadSegOffset.y).crossProduct(nextRoadSegVector);
		nextRoadSegNormal.normalise();

		// calculate b normal 
		bNormal = (currRoadSegNormal + nextRoadSegNormal) / 2;

		// create road segment
		uMax += currRoadSegLength;
		buildSegment(a1, a2, aNormal, b1, b2, bNormal, uMin, uMax);
		uMin += currRoadSegLength;
	}

	// finish end
	if(mPlotList.size() >= 2)
	{
		size_t i = mPlotList.size() - 2;

		// for a road segment pointA -> pointB
		a = mPlotList[i], b = mPlotList[i+1];

		// get current from last runs next vars
		currRoadSegVector = nextRoadSegVector;
		currRoadSegVector2D = nextRoadSegVector2D;
		currRoadSegOffset = nextRoadSegOffset;
		currRoadSegNormal = nextRoadSegNormal;
		currRoadSegLength = nextRoadSegLength;

		// advance A to previous B
		a1 = b1;
		a2 = b2;
		aNormal = bNormal;


		if(bindToGraph)
		{
			//mDstNode->invalidate();
			//getDstNode()->validate();
			size_t lastSeg = mRoadSegmentList.size()-1;
			if(mRoadGraph.getDstNode(mRoadSegmentList[lastSeg])->getPosition3D() != mPlotList[mPlotList.size()-1])
				boost::tie(b2, b1) = getDstNode()->getRoadJunction(mRoadSegmentList[lastSeg]);
			else
				boost::tie(b1, b2) = getDstNode()->getRoadJunction(mRoadSegmentList[lastSeg]);
		}
		else
		{
		// calculate vertex positions using the offset
		b1.x = b.x - currRoadSegOffset.x;
		b1.y = b.y;
		b1.z = b.z - currRoadSegOffset.y;
		b2.x = b.x + currRoadSegOffset.x;
		b2.y = b.y;
		b2.z = b.z + currRoadSegOffset.y;
		}

		// calculate b normal 
		bNormal = (currRoadSegNormal + Vector3::UNIT_Y) / 2;

		// create road segment
		uMax += currRoadSegLength;
		buildSegment(a1, a2, aNormal, b1, b2, bNormal, uMin, uMax);
		uMin += currRoadSegLength;
	}

	mManualObject->end();
	mSceneNode->attachObject(mManualObject);

}


void WorldRoad::buildSegment(const Vector3 &a1, const Vector3 &a2, const Vector3 &aNorm,
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

void WorldRoad::destroyRoadObject()
{
	// delete our manual object
	if(mManualObject) {
		mSceneNode->detachObject(mManualObject->getName());
		mSceneNode->getCreator()->destroyManualObject(mManualObject);
		delete mManualObject;
		mManualObject = 0;
	}
}

Vector3 WorldRoad::findNextPoint(const Vector2& cursor, const Vector2& direction, const Degree& dev) const
{
	int sampleIntervals = 5;
	std::vector<Vector3> pointList(sampleIntervals);
	Radian sampleSz(Degree((2 * dev) / (sampleIntervals - 1)));

	Radian angle(dev);
	for(angle = -angle; angle < dev; angle += sampleSz)
	{
		Real cos = Math::Cos(angle);
		Real sin = Math::Sin(angle);
		Vector2 translation(direction.x * cos + direction.y * sin, direction.y * cos + direction.x * sin);

		// i wish I could just normalize it once but that doesn't seem to be acurate
		translation.normalise();
		translation *= mRoadSegSz;

		Vector3 candidate(cursor.x + translation.x, 0, cursor.y + translation.y);
		if(WorldFrame::getSingleton().plotPointOnTerrain(candidate.x, candidate.y, candidate.z))
		{
			pointList.push_back(candidate);
		}
	}

	// find lowest point
	Vector3 lowestDiffPoint;
	Real lowestDiff(std::numeric_limits<Ogre::Real>::max());
	std::vector<Vector3>::const_iterator pIt, pEnd;
	for(pIt = pointList.begin(), pEnd = pointList.end(); pIt != pEnd; pIt++)
	{
		// lowest y diff
		Real currentDiff = std::abs(pIt->y - cursor.y);
		if(currentDiff <= lowestDiff)
		{
			lowestDiffPoint = *pIt;
			lowestDiff = currentDiff; 
		}
		// lowest y
		//if(pIt->y <= lowestDiff)
		//	lowestDiffPoint = *pIt;
	}
	return lowestDiffPoint;
}

/*
WorldRoad::RoadIntersectionState WorldRoad::snap(const Ogre::Real& snapSzSquared, NodeId& nd, RoadId& rd, Vector2& pos)
{
	NodeId snappedToNode;
	bool nodeSnapped = false;
	NodeInterface* dstNode = getDstNode();

	if(getClosestIntersection(rd, pos))
	{
		// intersection!, try and snap to a node on the intersecting road
		if(mRoadGraph.snapToRoadNode(pos, rd, snapSzSquared, snappedToNode))
			nodeSnapped = true;
		else
		{
			if(pos != dstNode->getPosition2D())
			{
				// position the destination node at the point of intersection 
				dstNode->setPosition2D(pos.x, pos.y);
				// Note: we have changed the segment so we must recurse
				//plotRoad();
				WorldRoad::RoadIntersectionState recursiveSnap = snap(snapSzSquared, nd, rd, pos);
				if(recursiveSnap != none) return recursiveSnap;
				else return road;
			}
			else return road;
		}
	}
	else
	{
		//HACK
		mRoadGraph.removeNode(dstNode->mNodeId);
		// no intersection!, try and snap to a node
		nodeSnapped = mRoadGraph.snapToNode(dstNode->getPosition2D(), snapSzSquared, snappedToNode);

		dstNode->mNodeId = mRoadGraph.addNode(dstNode);
	}

	if(nodeSnapped)
	{
		// check if the node is different to existing
		if(snappedToNode != nd)
		{
			// a new road section is proposed which must be considered
			nd = snappedToNode;

			// get the position of the new node & update dst node
			Vector2 p = mRoadGraph.getNode(nd)->getPosition2D();
			dstNode->setPosition2D(p.x, p.y);
			
			// rebuild the point list
			//plotRoad();

			// recursively snap
			return snap(snapSzSquared, nd, rd, pos);
		}
		else
		{
			nd = snappedToNode;
			NodeInterface* nb = mRoadGraph.getNode(nd);
			if(typeid(*nb) == typeid(WorldNode))		
				return world_node;
			else
				return simple_node;
		}
	}
	//LogManager::getSingleton().logMessage("Nadda SAl");
	return none;
}

bool WorldRoad::getClosestIntersection(RoadId& rd, Vector2& pos) const
{
	// for each road segment in this road
	if(mPlotList.size() > 2)
	{
		// get the src and dst points for this segment
		const Vector2 srcPos(mPlotList[0].x, mPlotList[0].z);
		const Vector2 dstPos(mPlotList[1].x, mPlotList[1].z);

		// test the first segment excluding roads connecting to its src node 
		if(mRoadGraph.findClosestIntersection3(srcPos, dstPos, getSrcNode()->mNodeId, rd, pos))
		{
			//LogManager::getSingleton().logMessage("Intersection on seg: 1", LML_CRITICAL);
			return true;
		}
		//else
		//{
		//	std::stringstream oss;
		//	oss << "No int on seg 1: ("<<srcPos<<") ("<<dstPos<<");";
		//	LogManager::getSingleton().logMessage(oss.str(), LML_CRITICAL);
		//}

		for(unsigned int i=1; i<mPlotList.size()-1; i++)
		{
			// get the src and dst points for this segment
			const Vector2 srcPos(mPlotList[i].x, mPlotList[i].z);
			const Vector2 dstPos(mPlotList[i+1].x, mPlotList[i+1].z);

			// test the segment against all roads in the graph 
			if(mRoadGraph.findClosestIntersection(srcPos, dstPos, rd, pos))
			{
				//std::stringstream oss;
				//oss << "Int on seg "<<(i+1)<<": ("<<srcPos<<") ("<<dstPos<<");";
				//LogManager::getSingleton().logMessage(oss.str(), LML_CRITICAL);
				return true;
			}
		}
	}
	//LogManager::getSingleton().logMessage("No intersection on "+StringConverter::toString(mPlotList.size())+" segments.", LML_CRITICAL);
	return false;
}
*/


// Note: should only try to snap to road network, ie those accessible from 
// the simple road graph. 
// Simple Node is the same as a road intersection, either way have to split road

WorldRoad::RoadIntersectionState WorldRoad::snap(const Ogre::Real& snapSzSquared, NodeId& nd, RoadId& rd, Vector2& pos)
{
	NodeId snappedToNode;
	bool nodeSnapped = false;

	// Find Closest Intersection
	if(getClosestIntersection(rd, pos))
	{
		// Intersection!
		// Snap to Road Node
		if(mRoadGraph.snapToRoadNode(pos, rd, snapSzSquared, snappedToNode))
		{
			nodeSnapped = true;
		}
		else
		{
			// INTERSECTION
			return road;
		}
	}
	else
	{
		// No intersection!
		// Snap To Node
		nodeSnapped = mRoadGraph.snapToOtherNode(getDstNode()->mNodeId, snapSzSquared, snappedToNode);
	}

	// if node snapped we alter the direction of our proposed road and must retest it
	while(nodeSnapped)
	{
		// DANGER
		Vector2 snapPos = mRoadGraph.getNode(snappedToNode)->getPosition2D();
		getDstNode()->setPosition2D(snapPos.x, snapPos.y);

		//DOH have to reget the snappedToNode since any set position recreated the road segs
		nodeSnapped = mRoadGraph.snapToOtherNode(getDstNode()->mNodeId, snapSzSquared, snappedToNode);

		// Find Closest Intersection
		if(getClosestIntersection(rd, pos))
		{
			// Snap to Road Node
			NodeId tmp; 
			if(mRoadGraph.snapToRoadNode(pos, rd, snapSzSquared, tmp))
			{
				if(snappedToNode != tmp) {
					snappedToNode = tmp;
					continue;
				}
			}
			else
			{
				// INTERSECTION
				return road;
			}
		}
		if(!nodeSnapped) break;

		// NODE SNAP
		nd = snappedToNode;
		NodeInterface* nb = mRoadGraph.getNode(nd);
		if(typeid(*nb) == typeid(WorldNode))		
			return world_node;
		else
		{
			return simple_node;
		}
	}
	// NO INTERSECTION
	return none;
}

bool WorldRoad::getClosestIntersection(RoadId& rd, Vector2& pos) const
{
	for(size_t i=0; i<mRoadSegmentList.size(); i++)
	{
		NodeId srcNd = mRoadGraph.getSrc(mRoadSegmentList[i]);
		NodeId dstNd = mRoadGraph.getDst(mRoadSegmentList[i]);
		if(mRoadGraph.findClosestIntersection(srcNd, dstNd, rd, pos))
			return true;
	}
	return false;
}

/*
bool WorldRoad::getClosestIntersection(RoadId& rd, Vector2& pos) const
{
	for(size_t i=0; i<mRoadSegmentList.size(); i++)
	{
		// get segment src & dst
		Vector2 srcPos = mRoadGraph.getSrcNode(mRoadSegmentList[i])->getPosition2D();
		Vector2 dstPos = mRoadGraph.getDstNode(mRoadSegmentList[i])->getPosition2D();

		// for each road
		RoadIterator rIt, rEnd;
		boost::tie(rIt, rEnd) = mSimpleRoadGraph.getRoads();
		for(; rIt != rEnd; rIt++)
		{
			RoadInterface* ri = mSimpleRoadGraph.getRoad(*rIt);
			if(typeid(*ri) == typeid(WorldRoad))
			{
				WorldRoad* wr = static_cast<WorldRoad*>(ri);
				if(wr != this)
				{
					wr->findClosestIntersection(srcPos, dstPos, distance, rd, pos);
				}
			}
		}
	}
	return false;
}
*/

bool WorldRoad::hasIntersection()
{
	for(unsigned int i=0; i<mRoadSegmentList.size(); i++)
	{
		if(mRoadGraph.hasIntersection(mRoadSegmentList[i]))
			return true;
	}
	return false;
}

bool WorldRoad::isRoadCycle()
{
	return mIsRoadCycle;
}

void WorldRoad::setRoadCycle(bool cycle)
{
	mIsRoadCycle = cycle;
}

bool WorldRoad::rayCross(const Ogre::Vector2& loc)
{
	validate();
	bool rayCross = false;
	for(unsigned int i=0; i<mPlotList.size()-1; i++)
	{
		if(Geometry::rayCross(loc, Vector2(mPlotList[i].x, mPlotList[i].z), Vector2(mPlotList[i+1].x, mPlotList[i+1].z))) 
			rayCross = !rayCross;
	}
	return rayCross;
}

Ogre::Real WorldRoad::getLengthSquared() const
{
	Vector2 src = getSrcNode()->getPosition2D();
	Vector2 dst = getDstNode()->getPosition2D();
	return (src - dst).squaredLength();
}

const std::vector<RoadId>& WorldRoad::getRoadSegmentList()
{
	validate();
	return mRoadSegmentList;
}

Real WorldRoad::getWidth() const
{
	return mRoadWidth;
}

void WorldRoad::onMoveNode()
{
	invalidate();
	// keep the graph up to date
	plotRoad();
}