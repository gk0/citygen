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
	  mSimpleRoadGraph(s)
{
	mGenParams.algorithm = EvenElevationDiff;
	mGenParams.sampleSize = 12; //DEBUG: set to v. big 50 normal is 10
	mGenParams.roadWidth = 1.0;
	mGenParams.sampleDeviance= 25;
	mGenParams.numOfSamples = 5;
	mGenParams.debug = false;
	mGenParams.segmentDrawSize = 2;

	mSelected = false;
	
	mManualObject = 0;
	mDebugObject = 0;
	
	mSimpleRoadGraph.addRoad(src->mSimpleNodeId, dst->mSimpleNodeId, mSimpleRoadId);
	mSimpleRoadGraph.setRoad(mSimpleRoadId, this);

	mName = "road" + StringConverter::toString(mRoadCount++);
	// create our scene node
	mSceneNode = creator->getRootSceneNode()->createChildSceneNode(mName);
	plotRoad();
}

WorldRoad::~WorldRoad()
{
	destroyRoadGraph();
	mSimpleRoadGraph.removeRoad(mSimpleRoadId);

	if(mDebugObject)
	{
		mSceneNode->detachObject(mDebugObject);
		delete mDebugObject;
		mDebugObject = 0;
	}

	destroyRoadObject();
	SceneManager* destroyer = mSceneNode->getCreator();
	destroyer->destroySceneNode(mSceneNode->getName());
}

void WorldRoad::build()
{
	std::vector<Vector3> interpolatedList;

	// build a spline from our plot list
	SimpleSpline roadSpline;
	roadSpline.setAutoCalculate(false);
	for(size_t i=0; i<mPlotList.size(); i++)
	{
		roadSpline.addPoint(mPlotList[i]);
	}

	// calculate approximate number of points to get desired segment size 
	Real interpolateStep = 1;
	if(mRoadSegmentList.size() > 0)
		interpolateStep = 1/((mRoadSegmentList.size() * mGenParams.sampleSize) / mGenParams.segmentDrawSize);

	// extract point list from spline
	roadSpline.recalcTangents();
	for(Real t=0; t<=1; t += interpolateStep)
	{
		interpolatedList.push_back(roadSpline.interpolate(t));
	}

	// always destroy previous
	destroyRoadObject();

	// declare the manual object
	mManualObject = new ManualObject(mName); 
	if(mSelected) mManualObject->begin("gk/YellowBrickRoad", Ogre::RenderOperation::OT_TRIANGLE_LIST);
	else mManualObject->begin("gk/Road", Ogre::RenderOperation::OT_TRIANGLE_LIST);

	// vars
	Vector3 currRoadSegNormal, nextRoadSegNormal;
	Vector3 currRoadSegVector, nextRoadSegVector;
	Vector2 currRoadSegVector2D, nextRoadSegVector2D;
	Vector2 currRoadSegOffset, nextRoadSegOffset;
	Real currRoadSegLength, nextRoadSegLength;
	Real uMin = 0, uMax = 0;
	Vector3 a, a1, a2, aNormal, b, b1, b2, bNormal, c;

	// init
	if(interpolatedList.size() >= 2)
	{
		nextRoadSegVector = interpolatedList[1] - interpolatedList[0];
		nextRoadSegVector2D.x = nextRoadSegVector.x;
		nextRoadSegVector2D.y = nextRoadSegVector.z;

		// use the 2D road seg vector to calculate length for speed
		nextRoadSegLength = nextRoadSegVector2D.length();

		// calculate the offset of length roadWidth
		nextRoadSegOffset = nextRoadSegVector2D.perpendicular();
		nextRoadSegOffset.normalise();
		nextRoadSegOffset *= mGenParams.roadWidth;

		// calculate normal
		nextRoadSegNormal = Vector3(nextRoadSegOffset.x, 0, nextRoadSegOffset.y).crossProduct(nextRoadSegVector);
		nextRoadSegNormal.normalise();
		bNormal = (nextRoadSegNormal + Vector3::UNIT_Y) / 2;


//get the first b from the node
		b = interpolatedList[0];
		boost::tie(b1, b2) = getSrcNode()->getRoadJunction(mRoadSegmentList[0]);
	}

	for(size_t i=0; i<(interpolatedList.size()-2); i++)
	{
		// for a road segment pointA -> pointB
		a = b;
		b = interpolatedList[i+1], c = interpolatedList[i+2];

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
		nextRoadSegOffset *= mGenParams.roadWidth;

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
	if(interpolatedList.size() >= 2)
	{
		size_t i = interpolatedList.size() - 2;

		// for a road segment pointA -> pointB
		a = interpolatedList[i], b = interpolatedList[i+1];

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

		//mDstNode->invalidate();
		//getDstNode()->validate();
		size_t lastSeg = mRoadSegmentList.size()-1;
		boost::tie(b2, b1) = getDstNode()->getRoadJunction(mRoadSegmentList[lastSeg]);

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

 void WorldRoad::buildDebugSegments(const Vector3 &pos, const std::vector<Vector3> &samples)
{
	Vector3 offset(0,3,0);
	for(size_t i=0; i<samples.size(); i++)
	{
		mDebugObject->position(pos + offset);
		mDebugObject->position(samples[i] + offset);
	}
}

void WorldRoad::buildSampleFan(const Vector2& cursor, const Vector2& direction, std::vector<Vector3> &samples) const
{
	samples.clear();

	if(mGenParams.numOfSamples < 2)
	{
		// single sample case is a straight forward centre sample
		if(mGenParams.numOfSamples == 1) {
			samples.reserve(mGenParams.numOfSamples);
			Vector2 sample2D = cursor + (direction * mGenParams.sampleSize);
			Vector3 candidate(sample2D.x, 0, sample2D.y);
			if(WorldFrame::getSingleton().plotPointOnTerrain(candidate.x, candidate.y, candidate.z))
				samples.push_back(candidate);
		}
	}
	else
	{
		//Radian sampleSz(Degree((2 * mGenParams.sampleDeviance) / (mGenParams.numOfSamples - 1)));

		//for(Radian angle = -mGenParams.sampleDeviance; angle < mGenParams.sampleDeviance; angle += sampleSz)
		//{
		//	Real cos = Math::Cos(angle);
		//	Real sin = Math::Sin(angle);
		//	Vector2 translation(direction.x * cos + direction.y * sin, direction.y * cos + direction.x * sin);

		//	// i wish I could just normalize it once but that doesn't seem to be acurate
		//	translation.normalise();
		//	translation *= mGenParams.sampleSize;

		//	Vector3 candidate(cursor.x + translation.x, 0, cursor.y + translation.y);
		//	if(WorldFrame::getSingleton().plotPointOnTerrain(candidate.x, candidate.y, candidate.z))
		//	{
		//		samples.push_back(candidate);
		//	}
		//}

		//direction.normalise();
		Radian sampleSz(Degree((2 * mGenParams.sampleDeviance) / (mGenParams.numOfSamples - 1)));

		for(Radian angle = -mGenParams.sampleDeviance; angle <= mGenParams.sampleDeviance; angle += sampleSz)
		{
			Real cos = Math::Cos(angle);
			Real sin = Math::Sin(angle);
			Vector2 translation(direction.x * cos - direction.y * sin, direction.y * cos + direction.x * sin);
			//Vector2 translation = direction.rotate(angle);

			// i wish I could just normalize it once but that doesn't seem to be acurate
			translation.normalise();
			translation *= mGenParams.sampleSize;

			Vector3 candidate(cursor.x + translation.x, 0, cursor.y + translation.y);
			if(WorldFrame::getSingleton().plotPointOnTerrain(candidate.x, candidate.y, candidate.z))
			{
				samples.push_back(candidate);
			}
		}
	}
}

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

	Ogre::Real distanceToJoin, sampleSize(mGenParams.sampleSize);
	Ogre::Real sampleSize2(mGenParams.sampleSize * 2);
	Ogre::Vector2 direction;
	Vector3 groundClearance(0,0.3,0);
	Vector2 srcCursor2D(getSrcNode()->getPosition2D());
	Vector2 dstCursor2D(getDstNode()->getPosition2D());
	Vector3 srcCursor3D(getSrcNode()->getPosition3D());
	Vector3 dstCursor3D(getDstNode()->getPosition3D());
	Vector3 nextSrcCursor3D, nextDstCursor3D;
	Vector2 segVector2D;
	int segCount = 0;

	// set our start node
	std::vector<Vector3> plotList2;
	mPlotList.push_back(getSrcNode()->getPosition3D() + groundClearance);
	plotList2.push_back(getDstNode()->getPosition3D() + groundClearance);

	std::stringstream oss;
	std::vector<Vector3> samples;
	oss << "Point "<<(mPlotList.size() - 1)<<": " << mPlotList[mPlotList.size() - 1] << std::endl;

	// select the algorithm used to select the ideal sample
	Vector3 (WorldRoad::*pt2SelectSample)(const Vector3&, const std::vector<Vector3>&, const Vector3&) = 0;                // C++
	switch(mGenParams.algorithm)
	{
	case EvenElevationDiff:
		pt2SelectSample = &WorldRoad::selectEvenElevationDiff;
		break;
	case MinimumElevationDiff:
		pt2SelectSample = &WorldRoad::selectMinElevationDiff;
		break;
	case MinimumElevation:
		pt2SelectSample = &WorldRoad::selectMinElevation;
		break;
	}

	// DEBUG Object
	if(mDebugObject)
	{
		mSceneNode->detachObject(mDebugObject);
		delete mDebugObject;
		mDebugObject = 0;
	}
	if(mGenParams.debug) 
	{
		mDebugObject = new ManualObject(mName+"Debug");
		mDebugObject->begin("gk/Hilite/Red", Ogre::RenderOperation::OT_LINE_LIST);
	}

	while(true)
	{
		distanceToJoin = (srcCursor2D - dstCursor2D).length(); 
		// if outside range of dest point
		if(distanceToJoin > sampleSize2)
		{
			// src --> dst
			{
				// get target direction
				segVector2D = dstCursor2D - srcCursor2D;
				segVector2D.normalise();

				// get the samples
				buildSampleFan(srcCursor2D, segVector2D, samples);	
				if(mGenParams.debug) buildDebugSegments(srcCursor3D, samples);
				nextSrcCursor3D = (*this.*pt2SelectSample)(srcCursor3D, samples, dstCursor3D);
				nextSrcCursor3D += groundClearance;

				// add our nodes to the road graph
				mPlotList.push_back(nextSrcCursor3D);
				
				// increment road segment count
				segCount++;
			}

			// dst --> src
			{
				// get target direction
				segVector2D = -segVector2D;

				// get the samples
				buildSampleFan(dstCursor2D, segVector2D, samples);
				if(mGenParams.debug) buildDebugSegments(dstCursor3D, samples);
				nextDstCursor3D = (*this.*pt2SelectSample)(dstCursor3D, samples, srcCursor3D);
				nextDstCursor3D += groundClearance;

				// add our nodes to the road graph
				plotList2.push_back(nextDstCursor3D);

				// increment road segment count
				segCount++;
			}

			// advance cursors
			srcCursor3D = nextSrcCursor3D;
			dstCursor3D = nextDstCursor3D;
			srcCursor2D = Vector2(nextSrcCursor3D.x, nextSrcCursor3D.z);
			dstCursor2D = Vector2(nextDstCursor3D.x, nextDstCursor3D.z);
		}
		else
			break;
	}
	
	// if distance between src and dst cursors is less than a seg length then 
	// another point is required
	distanceToJoin = (srcCursor2D - dstCursor2D).length(); 
	if(distanceToJoin > sampleSize)
	{	
		if(mPlotList.size() >= 2 && plotList2.size() >= 2)
		{
			// even out segment size a bit
			Vector3 srcDst = mPlotList[(mPlotList.size()-2)] - mPlotList[(mPlotList.size()-1)];
			Vector3 dstSrc = plotList2[plotList2.size()-2] - plotList2[plotList2.size()-1];
			Real srcDstL(srcDst.length()), dstSrcL(dstSrc.length());
			Real avgL = (srcDstL + dstSrcL + distanceToJoin) / 4;
			srcDst.normalise();
			dstSrc.normalise();
			mPlotList[mPlotList.size()-1] = mPlotList[mPlotList.size()-2] - (srcDst*avgL);
			plotList2[plotList2.size()-1] = plotList2[plotList2.size()-2] - (dstSrc*avgL);

			// update the cursors
			srcCursor3D = mPlotList[mPlotList.size()-1];
			dstCursor3D = plotList2[plotList2.size()-1];
			srcCursor2D = Vector2(srcCursor3D.x, srcCursor3D.z);
			dstCursor2D = Vector2(dstCursor3D.x, dstCursor3D.z);
		}

		// find the mid point between the two cursors
		Vector2 midPoint = (srcCursor2D + dstCursor2D) / 2;

		// find the direction vector
		segVector2D = dstCursor2D - srcCursor2D;
		Vector2 midLineVector = segVector2D.perpendicular();
		midLineVector.normalise();

		// get max distance from the midPoint, direction & step vector
		Real dist = mGenParams.sampleSize * Math::Sin(mGenParams.sampleDeviance);
		Real step = (2 * dist) / (mGenParams.numOfSamples - 1);
		Vector2 stepVector = midLineVector * step;

		// initial sample
		Vector2 midCursor2D = midPoint - (dist * midLineVector);
		Vector3 midCursor3D;

		// fill array of sample
		std::vector<Vector3> samples;
		if(WorldFrame::getSingleton().plotPointOnTerrain(midCursor2D, midCursor3D))
			samples.push_back(midCursor3D);

		for(uint16 i=0; i<(mGenParams.numOfSamples - 1); i++)
		{
			midCursor2D += stepVector;
			if(WorldFrame::getSingleton().plotPointOnTerrain(midCursor2D, midCursor3D))
				samples.push_back(midCursor3D);
		}

		if(mGenParams.debug) 
		{
			buildDebugSegments(srcCursor3D, samples);
			buildDebugSegments(dstCursor3D, samples);
		}
		
		// select best sample
		nextSrcCursor3D = (*this.*pt2SelectSample)(srcCursor3D, samples, dstCursor3D);
		mPlotList.push_back(nextSrcCursor3D + groundClearance);
	}
	else
	{
		if(mPlotList.size() >= 2 && plotList2.size() >= 2)
		{
			// even out segment size a bit
			Vector3 srcDst = mPlotList[(mPlotList.size()-2)] - mPlotList[(mPlotList.size()-1)];
			Vector3 dstSrc = plotList2[plotList2.size()-2] - plotList2[plotList2.size()-1];
			Real srcDstL(srcDst.length()), dstSrcL(dstSrc.length());
			Real avgL = (srcDstL + dstSrcL + distanceToJoin) / 3;
			srcDst.normalise();
			dstSrc.normalise();
			mPlotList[mPlotList.size()-1] = mPlotList[mPlotList.size()-2] - (srcDst*avgL);
			plotList2[plotList2.size()-1] = plotList2[plotList2.size()-2] - (dstSrc*avgL);

			// update the cursors
			srcCursor3D = mPlotList[mPlotList.size()-1];
			dstCursor3D = plotList2[plotList2.size()-1];
			srcCursor2D = Vector2(srcCursor3D.x, srcCursor3D.z);
			dstCursor2D = Vector2(dstCursor3D.x, dstCursor3D.z);
		}
	}
	

	//join src-->dst & dst-->src lists
	for(int i=static_cast<int>(plotList2.size()-1); i>=0; i--)
		mPlotList.push_back(plotList2[i]);

	if(mGenParams.debug)
	{
		mDebugObject->end(); 
		mSceneNode->attachObject(mDebugObject); 
	}
	createRoadGraph();
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
		SimpleNode* bn = new SimpleNode(mRoadGraph, mPlotList[1]);
		NodeId cursorNode = mRoadGraph.addNode(bn);
		RoadId roadSegId;
		mRoadGraph.addRoad(getSrcNode()->mNodeId, cursorNode, this, roadSegId);
		mRoadSegmentList.push_back(roadSegId);

		// create each road segment
		for(unsigned int i=2; i<(mPlotList.size()-1); i++)
		{
			// add our nodes to the road graph
			SimpleNode* bn = new SimpleNode(mRoadGraph, mPlotList[i]);
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
/*
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

const RoadGenParams& WorldRoad::getGenParams()
{
	return mGenParams;
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
	return mGenParams.roadWidth;
}


bool WorldRoad::loadXML(const TiXmlHandle& roadRoot)
{
	// Load GenParams, not Smart Enough to do our graph, see parent
	{
		TiXmlElement *element = roadRoot.FirstChild("genparams").FirstChild().Element();

		for(; element; element=element->NextSiblingElement())
		{
			std::string key = element->Value();
			
			if(key == "algorithm"){
				int alg;
				element->QueryIntAttribute("value", &alg);
				mGenParams.algorithm = static_cast<RoadPlotAlgorithm>(alg);
			}else if(key == "sampleSize")
				element->QueryFloatAttribute("value", &mGenParams.sampleSize);
			else if(key == "sampleDeviance"){
				Real sd;
				element->QueryFloatAttribute("value", &sd);
				mGenParams.sampleDeviance = Degree(sd);
			}else if(key == "roadWidth")
				element->QueryFloatAttribute("value", &mGenParams.roadWidth);
			else if(key == "numOfSamples") {
				int nos;
				element->QueryIntAttribute("value", &nos);
				mGenParams.numOfSamples = static_cast<uint16>(nos);
			}else if(key == "debug") {
				int d;
				element->QueryIntAttribute("value", &d);
				mGenParams.debug = (d == 1);
			}
		}
	}
	plotRoad();
	invalidate();
	return true;
}

void WorldRoad::onMoveNode()
{
	invalidate();
	// keep the graph up to date
	plotRoad();

	std::set<WorldObject*>::iterator aIt, aEnd;
	for(aIt = mAttachments.begin(), aEnd = mAttachments.end(); aIt != aEnd; aIt++)
	{
		(*aIt)->invalidate();
	}
	WorldFrame::getSingleton().modify(true);
}

void WorldRoad::invalidate()
{
	mValid = false;
/*	std::set<WorldObject*>::iterator aIt, aEnd;
	for(aIt = mAttachments.begin(), aEnd = mAttachments.end(); aIt != aEnd; aIt++)
	{
		(*aIt)->invalidate();
	}*/
}

TiXmlElement* WorldRoad::saveXML()
{
	// GraphML compliant, edge not road
	TiXmlElement *root = new TiXmlElement("edge"); 
	root->SetAttribute("source", (int) getSrcNode());
	root->SetAttribute("target", (int) getDstNode());

	TiXmlElement *genParams = new TiXmlElement("genparams"); 
	root->LinkEndChild(genParams);

	TiXmlElement *algorithm = new TiXmlElement("algorithm");  
	algorithm->SetAttribute("value", mGenParams.algorithm);
	genParams->LinkEndChild(algorithm);

	TiXmlElement *sampleSize = new TiXmlElement("sampleSize");  
	sampleSize->SetDoubleAttribute("value", mGenParams.sampleSize);
	genParams->LinkEndChild(sampleSize);

	TiXmlElement *sampleDeviance = new TiXmlElement("sampleDeviance");  
	sampleDeviance->SetDoubleAttribute("value", mGenParams.sampleDeviance.valueDegrees());
	genParams->LinkEndChild(sampleDeviance);

	TiXmlElement *roadWidth = new TiXmlElement("roadWidth");  
	roadWidth->SetAttribute("value", mGenParams.roadWidth);
	genParams->LinkEndChild(roadWidth);

	TiXmlElement *numOfSamples = new TiXmlElement("numOfSamples");  
	numOfSamples->SetAttribute("value", mGenParams.numOfSamples);
	genParams->LinkEndChild(numOfSamples);

	TiXmlElement *debug = new TiXmlElement("debug");  
	debug->SetAttribute("value", mGenParams.debug);
	genParams->LinkEndChild(debug);

	return root;
}

Vector3 WorldRoad::selectEvenElevationDiff(const Vector3 &lastSample, const std::vector<Vector3> &samples, const Vector3 &target)
{
	// find total difference in height
	Real totalRatio = (target.y - lastSample.y) / ((target - lastSample).length());
	Real lowestRatioDiff(std::numeric_limits<Ogre::Real>::max());

	Vector3 selectedPoint;
	std::vector<Vector3>::const_iterator sIt, sEnd;
	for(sIt = samples.begin(), sEnd = samples.end(); sIt != sEnd; sIt++)
	{
		// lowest difference in (elevation diff / distance) ratio
		Real currentRatio = (sIt->y - lastSample.y)/((*sIt - lastSample).length());
		Real currentRatioDiff = std::abs(totalRatio - currentRatio);
		if(currentRatioDiff <= lowestRatioDiff)
		{
			selectedPoint = *sIt;
			lowestRatioDiff = currentRatioDiff; 
		}
	}
	return selectedPoint;
}

Vector3 WorldRoad::selectMinElevationDiff(const Vector3 &lastSample, const std::vector<Vector3> &samples, const Vector3 &target)
{
	// find lowest defference in height
	Vector3 lowestDiffPoint;
	Real lowestDiff(std::numeric_limits<Ogre::Real>::max());
	std::vector<Vector3>::const_iterator sIt, sEnd;
	for(sIt = samples.begin(), sEnd = samples.end(); sIt != sEnd; sIt++)
	{
		// lowest y diff
		Real currentDiff = std::abs(sIt->y - lastSample.y);
		if(currentDiff <= lowestDiff)
		{
			lowestDiffPoint = *sIt;
			lowestDiff = currentDiff; 
		}
	}
	return lowestDiffPoint;
}

Vector3 WorldRoad::selectMinElevation(const Vector3 &lastSample, const std::vector<Vector3> &samples, const Vector3 &target)
{
	// find lowest defference in height
	Vector3 lowestPoint;
	Real lowest(std::numeric_limits<Ogre::Real>::max());
	std::vector<Vector3>::const_iterator sIt, sEnd;
	for(sIt = samples.begin(), sEnd = samples.end(); sIt != sEnd; sIt++)
	{
		// lowest y
		if(sIt->y <= lowest)
		{
			lowestPoint = *sIt;
			lowest = sIt->y;
		}
	}
	return lowestPoint;
}

void WorldRoad::setGenParams(const RoadGenParams& g)
{
	mGenParams = g;
	onMoveNode();	// well it isn't but i want to do what it does
}

void WorldRoad::setWidth(const Ogre::Real& w)
{
	mGenParams.roadWidth = w;
}

// Note: should only try to snap to road network, ie those accessible from 
// the simple road graph. 
// Simple Node is the same as a road intersection, either way have to split road
int WorldRoad::snapInfo(Ogre::Real snapSz, Vector2& pos, WorldNode*& wn, WorldRoad*& wr)
{
	size_t i;
	RoadId rd;
	bool intersection=false;

	// calculate the minimum segment size, i know its a bit of work and its not 
	// ideal but its not that bad and is only a tiny percentage of the work
	Real len = std::numeric_limits<Real>::max();
	for(i=0; i<(mRoadSegmentList.size()-1); i++) 
	{
		Vector2 src = mRoadGraph.getSrcNode(mRoadSegmentList[i])->getPosition2D();
		Vector2 dst = mRoadGraph.getDstNode(mRoadSegmentList[i])->getPosition2D();
		Real l = (src-dst).length();
		if(l<len) len = l;
	}
	snapSz = std::min((len-0.01f), snapSz);
	Real snapSzSq(Math::Sqr(snapSz));

	for(i=0; i<(mRoadSegmentList.size()-1); i++) 
	{
		NodeId srcNd = mRoadGraph.getSrc(mRoadSegmentList[i]);
		NodeId dstNd = mRoadGraph.getDst(mRoadSegmentList[i]);
		intersection = mRoadGraph.findClosestIntscnConnected(srcNd, dstNd, snapSz, pos, rd);
		if(intersection) break;
	}
	if(!intersection)
	{
		NodeId srcNd = mRoadGraph.getSrc(mRoadSegmentList[i]);
		Vector2 dstPos = mRoadGraph.getDstNode(mRoadSegmentList[i])->getPosition2D();
		intersection = mRoadGraph.findClosestIntscn(srcNd, dstPos, snapSz, pos, rd);
	}

	if(!intersection) pos = getDstNode()->getPosition2D();

	Real closestDistanceSq = std::numeric_limits<Real>::max();
	NodeIterator nIt, nEnd;
	for(boost::tie(nIt, nEnd) = mSimpleRoadGraph.getNodes();  nIt != nEnd; nIt++)
	{
		NodeInterface* ni = mSimpleRoadGraph.getNode(*nIt);
		assert(typeid(*ni) == typeid(WorldNode));
		Vector2 nPos = ni->getPosition2D();
		if(nPos == getDstNode()->getPosition2D()) continue;
			
		Real currentDistanceSq = (pos - nPos).squaredLength();
		if(currentDistanceSq < snapSzSq && currentDistanceSq < closestDistanceSq)
		{
			wn = static_cast<WorldNode*>(ni);
			pos = nPos;
			closestDistanceSq = currentDistanceSq;
		}
	}
	if(closestDistanceSq < std::numeric_limits<Real>::max())
		return 2;
	else if(intersection == true)
	{
		//TODO: interpolate point on spline
		return 1;
	}
	else return 0;
}
