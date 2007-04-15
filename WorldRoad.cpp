#include "stdafx.h"
#include "WorldNode.h"
#include "WorldRoad.h"
#include "WorldFrame.h"
#include "SimpleNode.h"
#include "Geometry.h"

int WorldRoad::mRoadCount = 0;
using namespace Ogre;

WorldRoad::WorldRoad(SceneManager* creator, WorldNode* src, WorldNode* dst, RoadGraph& rg, bool bind)
	: mSrcNode(src),
	  mDstNode(dst),
	  mRoadGraph(rg),
	  mRoadSegSz(10), //DEBUG: set to v. big 50 normal is 10
	  mRoadWidth(1.0),
	  mRoadDeviance(30),
	  mManualObject(0)
{
	Ogre::String name("Road"+StringConverter::toString(mRoadCount++));
	// create our scene node
	mSceneNode = creator->getRootSceneNode()->createChildSceneNode(name);

	//omg i should like draw a line from my old node to my new node
	bindToGraph = bind;

	mSrcNode->attach(this);
	mDstNode->attach(this);
}

WorldRoad::~WorldRoad()
{
	destroyRoadObject();
	SceneManager* destroyer = mSceneNode->getCreator();
	destroyer->destroySceneNode(mSceneNode->getName());
	mSrcNode->detach(this);
	mDstNode->detach(this);
	destroyRoadGraph();
}


void WorldRoad::setSrcNode(WorldNode* src)
{
	if(src != mSrcNode)
	{
		mSrcNode->detach(this);
		mSrcNode = src;
		mSrcNode->attach(this);
		invalidate();
	}
}

void WorldRoad::setDstNode(WorldNode* dst)
{
	if(dst != mDstNode)
	{
		mDstNode->detach(this);
		mDstNode = dst;
		mDstNode->attach(this);
		invalidate();
	}
}

NodeInterface* WorldRoad::getSrcNode()
{
	return mSrcNode;
}

NodeInterface* WorldRoad::getDstNode()
{
	return mDstNode;
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
	status = 1;


	// always destroy previous
	mPlotList.clear();

	Ogre::Real segmentSizeSq(mRoadSegSz * mRoadSegSz);
	Ogre::Vector2 direction;
	Ogre::Angle devAngle(mRoadDeviance);
	Vector3 groundClearance(0,0.3,0);
	Vector2 cursor2D(mSrcNode->getPosition2D());
	Vector2 dstPoint2D(mDstNode->getPosition2D());
	int segCount = 0;

	// set our start node
	mPlotList.push_back(mSrcNode->getPosition3D() + groundClearance);

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
	

//#ifdef DEBUG
	//LogManager::getSingleton().logMessage("Plotted "+StringConverter::toString(mPlotList.size())+" segments.", LML_CRITICAL);
//#endif

	mPlotList.push_back(mDstNode->getPosition3D() + groundClearance);

//	oss << "Point "<<(mPlotList.size() - 1)<<": " << mPlotList[mPlotList.size() - 1] << std::endl;
//	LogManager::getSingleton().logMessage(oss.str(), LML_CRITICAL);
}

void WorldRoad::createRoadGraph()
{
	// always delete the last graph
	destroyRoadGraph();

	if(mPlotList.size() == 2)
	{
		// create first road segment
		RoadId roadSegId;
		mRoadGraph.addRoad(mSrcNode->mNodeId, mDstNode->mNodeId, this, roadSegId);
		mRoadSegmentList.push_back(roadSegId);
	}
	else
	{
		// create first road segment
		SimpleNode* bn = new SimpleNode();
		bn->setPosition3D(mPlotList[1]);
		NodeId cursorNode = mRoadGraph.addNode(bn);
		RoadId roadSegId;
		mRoadGraph.addRoad(mSrcNode->mNodeId, cursorNode, this, roadSegId);
		mRoadSegmentList.push_back(roadSegId);

		// create each road segment
		for(unsigned int i=2; i<(mPlotList.size()-1); i++)
		{
			// add our nodes to the road graph
			SimpleNode* bn = new SimpleNode();
			bn->setPosition3D(mPlotList[i]);
			NodeId nextNodeId = mRoadGraph.addNode(bn);
			mRoadGraph.addRoad(cursorNode, nextNodeId, this, roadSegId);
			mRoadSegmentList.push_back(roadSegId);

			// advance cursor
			cursorNode = nextNodeId;
		}

		// create last road segment
		mRoadGraph.addRoad(cursorNode, mDstNode->mNodeId, this, roadSegId);
		mRoadSegmentList.push_back(roadSegId);
	}
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
	// always destroy previous
	destroyRoadGraph();
	destroyRoadObject();

	plotRoad();
	if(bindToGraph) createRoadGraph();

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

		// calculate vertex positions using the offset
		b = mPlotList[0];
		b1.x = b.x - nextRoadSegOffset.x;
		b1.y = b.y;
		b1.z = b.z - nextRoadSegOffset.y;
		b2.x = b.x + nextRoadSegOffset.x;
		b2.y = b.y;
		b2.z = b.z + nextRoadSegOffset.y;
	}

	for(size_t i=0; i<(mPlotList.size()-2); i++)
	{
		// for a road segment pointA -> pointB
		a = mPlotList[i], b = mPlotList[i+1], c = mPlotList[i+2];

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

		// calculate vertex positions using the offset
		b1.x = b.x - currRoadSegOffset.x;
		b1.y = b.y;
		b1.z = b.z - currRoadSegOffset.y;
		b2.x = b.x + currRoadSegOffset.x;
		b2.y = b.y;
		b2.z = b.z + currRoadSegOffset.y;

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


WorldRoad::RoadIntersectionState WorldRoad::snap(const Ogre::Real& snapSzSquared, NodeId& nd, RoadId& rd, Vector2& pos)
{
	plotRoad();
	status = 2;
	NodeId snappedToNode;
	bool nodeSnapped = false;

	if(getClosestIntersection(rd, pos))
	{
		// intersection!, try and snap to a node on the intersecting road
		if(mRoadGraph.snapToRoadNode(pos, rd, snapSzSquared, snappedToNode))
			nodeSnapped = true;
		else
		{
			if(pos != mDstNode->getPosition2D())
			{
				// position the destination node at the point of intersection 
				mDstNode->setPosition2D(pos.x, pos.y);
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
		// no intersection!, try and snap to a node
		nodeSnapped = mRoadGraph.snapToNode(mDstNode->getPosition2D(), snapSzSquared, snappedToNode);
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
			mDstNode->setPosition2D(p.x, p.y);
			
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
	//LogManager::getSingleton().logMessage("Nadda SAl", LML_CRITICAL);
	return none;
}

bool WorldRoad::getClosestIntersection(RoadId& rd, Vector2& pos) const
{
	// for each road segment in this road
	if(mPlotList.size() < 2) return false;
	else
	{
		// get the src and dst points for this segment
		const Vector2 srcPos(mPlotList[0].x, mPlotList[0].z);
		const Vector2 dstPos(mPlotList[1].x, mPlotList[1].z);

		// test the first segment excluding roads connecting to its src node 
		if(mRoadGraph.findClosestIntersection3(srcPos, dstPos, mSrcNode->mNodeId, rd, pos))
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
	}

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
	//LogManager::getSingleton().logMessage("No intersection on "+StringConverter::toString(mPlotList.size())+" segments.", LML_CRITICAL);
	return false;
}


//doh must rewrite this function
// and also review the invalidate business could be plotting far too much

bool WorldRoad::hasIntersection()
{
	for(unsigned int i=0; i<mRoadSegmentList.size(); i++)
	{
		if(mRoadGraph.hasIntersection(mRoadSegmentList[i]))
			return true;
	}
	return false;
}

/*
//doh must rewrite this function
// and also review the invalidate business could be plotting far too much

bool WorldRoad::hasIntersection()
{
	bool hasIntersection = false;

	// remove our road segments from the graph so we don't intersect with them
	destroyRoadGraph();

	// for each road segment in this road
	for(unsigned int i=0; i<mPlotList.size()-1; i++)
	{
		// get the src and dst points for this segment
		const Vector2 srcPos(mPlotList[i].x, mPlotList[i].z);
		const Vector2 dstPos(mPlotList[i+1].x, mPlotList[i+1].z);

		// test the road segment in the graph and exclude the mSrcNode intersection
		Vector2 pos;
		if(mRoadGraph.hasIntersection(srcPos, dstPos, pos))
		{
			// if first segment
			if(i == 0)
			{
				// exclude mSrcNode intersection
				if(pos != mSrcNode->getPosition2D())
				{
					hasIntersection = true;
					break;
				}
			}
			// if last segment
			else if(i == (mPlotList.size()-2))
			{
				// exclude mDstNode intersection
				if(pos != mDstNode->getPosition2D()) 
				{
					hasIntersection = true;
					break;
				}
			}
			else
			{
				hasIntersection = true;
				break;
			}
			
		}
	}
	
	// add our selves back to the road graph
	createRoadGraph();
	
	return hasIntersection;
}*/

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
	Vector2 src = mSrcNode->getPosition2D();
	Vector2 dst = mDstNode->getPosition2D();
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
