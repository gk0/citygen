#include "stdafx.h"
#include "WorldNode.h"
#include "WorldRoad.h"
#include "WorldFrame.h"
#include "SimpleNode.h"
#include "Geometry.h"

// fine grain roads currently cause problems due to simple insets
#define FINEGRAIN 1
#define GROUNDCLEARANCE Ogre::Vector3(0,0.3,0)


unsigned int WorldRoad::_instanceCount = 0;
RoadGenParams WorldRoad::_defaultGenParams;

using namespace Ogre;
using namespace std;

WorldRoad::WorldRoad(WorldNode* src, WorldNode* dst, RoadGraph& g, 
					 RoadGraph& s, Ogre::SceneManager *creator, bool bind)
	: _roadGraph(g),
	  _simpleRoadGraph(s)
{
	_genParams = _defaultGenParams;

	_selected = false;
	_length = 0;
	
	_manualObject = 0;
	_debugMOObject = 0;
	
	_simpleRoadGraph.addRoad(src->mSimpleNodeId, dst->mSimpleNodeId, _simpleRoadId);
	_simpleRoadGraph.setRoad(_simpleRoadId, this);

	_name = "road" + StringConverter::toString(_instanceCount++);
	// create our scene node
	_sceneNode = creator->getRootSceneNode()->createChildSceneNode(_name);
	plotRoad();
}

WorldRoad::~WorldRoad()
{
	destroyRoadGraph();
	_simpleRoadGraph.removeRoad(_simpleRoadId);

	if(_debugMOObject)
	{
		_sceneNode->detachObject(_debugMOObject);
		delete _debugMOObject;
		_debugMOObject = 0;
	}

	destroyRoadObject();
	SceneManager* destroyer = _sceneNode->getCreator();
	destroyer->destroySceneNode(_sceneNode->getName());
}

void WorldRoad::build()
{
	Vector2 tmper(getSrcNode()->getPosition2D() - getDstNode()->getPosition2D());
	if(tmper.length() < 0.3)
		return;
	std::vector<Vector3> interpolatedList;

#ifdef FINEGRAIN

	interpolatedList.push_back(getSrcNode()->getPosition3D());
	BOOST_FOREACH(RoadId rd, _roadSegmentList) 
		interpolatedList.push_back(_roadGraph.getDstNode(rd)->getPosition3D());
#else
	// calculate approximate number of points to get desired segment size 
	Real interpolateStep = 1;
	if(_roadSegmentList.size() > 0)
		interpolateStep = 1/((_roadSegmentList.size() * _genParams._sampleSize) / _genParams._segmentDrawSize);

	// extract point list from spline
	for(Real t=0; t<=1; t += interpolateStep)
	{
		interpolatedList.push_back(_spline.interpolate(t));
	}
#endif

	// always destroy previous
	destroyRoadObject();

	// declare the manual object
	_manualObject = new ManualObject(_name); 
	if(_selected) _manualObject->begin("gk/YellowBrickRoad", Ogre::RenderOperation::OT_TRIANGLE_LIST);
	else _manualObject->begin("gk/Road", Ogre::RenderOperation::OT_TRIANGLE_LIST);

	//_debugMOObject = new ManualObject(_name+"do");

	// vars
	Vector3 currRoadSegNormal, nextRoadSegNormal, nextRoadSegVector;
	Vector2 currRoadSegPerp, nextRoadSegPerp, nextRoadSegVector2D;
	Real currRoadSegLength, nextRoadSegLength;
	Real uMin = 0, uMax = 0;
	Vector3 a, a1, a2, aNormal, b, b1, b2, bNormal, c;

	// init
	if(interpolatedList.size() >= 2)
	{
		nextRoadSegVector = interpolatedList[1] - interpolatedList[0];
		nextRoadSegVector2D.x = nextRoadSegVector.x;
		nextRoadSegVector2D.y = nextRoadSegVector.z;

		// use the 3D road segment vector to calculate length for accuracy
		nextRoadSegLength = nextRoadSegVector.length();

		// calculate the offset of length roadWidth
		nextRoadSegPerp = nextRoadSegVector2D.perpendicular();
		if(nextRoadSegLength != 0) nextRoadSegPerp /= nextRoadSegLength;
		else nextRoadSegPerp.normalise();

		// calculate normal
		nextRoadSegNormal = Vector3(nextRoadSegPerp.x, 0, nextRoadSegPerp.y).crossProduct(nextRoadSegVector);
		nextRoadSegNormal.normalise();
		bNormal = (nextRoadSegNormal + Vector3::UNIT_Y) / 2;

		//get the first b from the node
		b = interpolatedList[0];
		boost::tie(b1, b2) = getSrcNode()->getRoadJunction(_roadSegmentList[0]);

		//
		size_t start=0,end=(interpolatedList.size()-2);
		for(size_t i=0; i<(interpolatedList.size()-2); i++)
		{
			Vector2 intsctn;
			Real r,s;
			if(Geometry::lineIntersect(Geometry::V2(b1), Geometry::V2(b2), Geometry::V2(interpolatedList[i]), 
				Geometry::V2(interpolatedList[i+1]), intsctn, r, s)
				&& s >= 0 && s <= 1)
			{
				start = i + 1;
				break;
			}
		}

		for(size_t i=(interpolatedList.size()-2); i>0; i--)
		{
			size_t lastSeg = _roadSegmentList.size()-1;
			Vector3 c1,c2;
			boost::tie(c2, c1) = getDstNode()->getRoadJunction(_roadSegmentList[lastSeg]);
			Vector2 intsctn;
			Real r,s;
			if(Geometry::lineIntersect(Geometry::V2(c1), Geometry::V2(c2), Geometry::V2(interpolatedList[i]), 
				Geometry::V2(interpolatedList[i+1]), intsctn, r, s)
				&& s >= 0 && s <= 1)
			{
				end = i;
				break;
			}
		}


		for(size_t i=start; i<end; i++)
		{
			// for a road segment pointA -> pointB
			a = b;
			b = interpolatedList[i+1], c = interpolatedList[i+2];

			// get current from last runs next vars
			currRoadSegPerp = nextRoadSegPerp;
			currRoadSegNormal = nextRoadSegNormal;
			currRoadSegLength = nextRoadSegLength;

			// advance A to previous B
			a1 = b1;
			a2 = b2;
			aNormal = bNormal;

			// calculate next road segment vectors to get b normal
			nextRoadSegVector = c - b;
			nextRoadSegVector2D.x = nextRoadSegVector.x;
			nextRoadSegVector2D.y = nextRoadSegVector.z;

			// use the 3D road segment vector to calculate length for accuracy
			nextRoadSegLength = nextRoadSegVector.length();

			//TODO this offset is calculates
			// calculate the offset of length roadWidth
			nextRoadSegPerp = nextRoadSegVector2D.perpendicular();
			if(nextRoadSegLength != 0) nextRoadSegPerp /= nextRoadSegLength;
			else nextRoadSegPerp.normalise();

			Vector2 offset(Geometry::calcInsetVector(currRoadSegPerp, nextRoadSegPerp, _genParams._roadWidth));

			// calculate vertex positions using the offset
			b1.x = b.x - offset.x;
			b1.y = b.y;
			b1.z = b.z - offset.y;
			b2.x = b.x + offset.x;
			b2.y = b.y;
			b2.z = b.z + offset.y;

			// calculate normal
			Vector3 nextRoadSegNormal = Vector3(nextRoadSegPerp.x, 0, nextRoadSegPerp.y).crossProduct(nextRoadSegVector);
			nextRoadSegNormal.normalise();

			// calculate b normal 
			bNormal = (currRoadSegNormal + nextRoadSegNormal) / 2;

			// create road segment
			uMax += currRoadSegLength;
			buildSegment(a1, a2, aNormal, b1, b2, bNormal, uMin, uMax);
			uMin += currRoadSegLength;
		}

		// finish end
		size_t i = interpolatedList.size() - 2;

		// for a road segment pointA -> pointB
		a = interpolatedList[i], b = interpolatedList[i+1];

		// get current from last runs next vars
		currRoadSegNormal = nextRoadSegNormal;
		currRoadSegLength = nextRoadSegLength;

		// advance A to previous B
		a1 = b1;
		a2 = b2;
		aNormal = bNormal;

		size_t lastSeg = _roadSegmentList.size()-1;
		boost::tie(b2, b1) = getDstNode()->getRoadJunction(_roadSegmentList[lastSeg]);

		// calculate b normal 
		bNormal = (currRoadSegNormal + Vector3::UNIT_Y) / 2;

		// create road segment
		uMax += currRoadSegLength;
		buildSegment(a1, a2, aNormal, b1, b2, bNormal, uMin, uMax);
		uMin += currRoadSegLength;
	}

	_manualObject->end();
	_sceneNode->attachObject(_manualObject);
 }

 void WorldRoad::buildDebugSegments(const Vector3 &pos, const std::vector<Vector3> &samples)
{
	Vector3 offset(0,3,0);
	for(size_t i=0; i<samples.size(); i++)
	{
		_debugMOObject->position(pos + offset);
		_debugMOObject->position(samples[i] + offset);
	}
}

void WorldRoad::buildSampleFan(const Vector2& cursor, const Vector2& direction, std::vector<Vector3> &samples) const
{
	samples.clear();

	if(_genParams._numOfSamples < 2)
	{
		// single sample case is a straight forward centre sample
		if(_genParams._numOfSamples == 1) {
			samples.reserve(_genParams._numOfSamples);
			Vector2 sample2D = cursor + (direction * _genParams._sampleSize);
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
		Radian sampleSz(Degree((2 * _genParams._sampleDeviance) / (_genParams._numOfSamples - 1)));

		for(Radian angle = -_genParams._sampleDeviance; angle <= _genParams._sampleDeviance; angle += sampleSz)
		{
			Real cos = Math::Cos(angle);
			Real sin = Math::Sin(angle);
			Vector2 translation(direction.x * cos - direction.y * sin, direction.y * cos + direction.x * sin);
			//Vector2 translation = direction.rotate(angle);

			// i wish I could just normalize it once but that doesn't seem to be acurate
			translation.normalise();
			translation *= _genParams._sampleSize;

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
	return _simpleRoadGraph.getSrcNode(_simpleRoadId);
}

NodeInterface* WorldRoad::getDstNode() const
{
	return _simpleRoadGraph.getDstNode(_simpleRoadId);
}
/*
//simple
void WorldRoad::createRoadObject()
{
	//omg i should like draw a line from my old node to my new node
	Vector3 offset(0,3,0);
	mManualObject = new ManualObject(_name); 
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
	_plotList.clear();

	Ogre::Real distanceToJoin, sampleSize(_genParams._sampleSize);
	Ogre::Real sampleSize2(_genParams._sampleSize * 2);
	Ogre::Vector2 direction;
	Vector2 srcCursor2D(getSrcNode()->getPosition2D());
	Vector2 dstCursor2D(getDstNode()->getPosition2D());
	Vector3 srcCursor3D(getSrcNode()->getPosition3D());
	Vector3 dstCursor3D(getDstNode()->getPosition3D());
	Vector3 nextSrcCursor3D, nextDstCursor3D;
	Vector2 segVector2D;
	int segCount = 0;

	// set our start node
	std::vector<Vector3> plotList2;
	_plotList.push_back(getSrcNode()->getPosition3D() + GROUNDCLEARANCE);
	plotList2.push_back(getDstNode()->getPosition3D() + GROUNDCLEARANCE);

	std::stringstream oss;
	std::vector<Vector3> samples;
	oss << "Point "<<(_plotList.size() - 1)<<": " << _plotList[_plotList.size() - 1] << std::endl;

	// select the algorithm used to select the ideal sample
	Vector3 (WorldRoad::*pt2SelectSample)(const Vector3&, const std::vector<Vector3>&, const Vector3&) = 0;                // C++
	switch(_genParams._algorithm)
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
	if(_debugMOObject)
	{
		_sceneNode->detachObject(_debugMOObject);
		delete _debugMOObject;
		_debugMOObject = 0;
	}
	if(_genParams._debug) 
	{
		_debugMOObject = new ManualObject(_name+"Debug");
		_debugMOObject->begin("gk/Hilite/Red", Ogre::RenderOperation::OT_LINE_LIST);
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
				if(_genParams._debug) buildDebugSegments(srcCursor3D, samples);
				nextSrcCursor3D = (*this.*pt2SelectSample)(srcCursor3D, samples, dstCursor3D);
				nextSrcCursor3D += GROUNDCLEARANCE;

				// add our nodes to the road graph
				_plotList.push_back(nextSrcCursor3D);
				
				// increment road segment count
				segCount++;
			}

			// dst --> src
			{
				// get target direction
				segVector2D = -segVector2D;

				// get the samples
				buildSampleFan(dstCursor2D, segVector2D, samples);
				if(_genParams._debug) buildDebugSegments(dstCursor3D, samples);
				nextDstCursor3D = (*this.*pt2SelectSample)(dstCursor3D, samples, srcCursor3D);
				nextDstCursor3D += GROUNDCLEARANCE;

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
	_length = (segCount * sampleSize) + distanceToJoin;

	if(distanceToJoin > sampleSize)
	{	
		if(_plotList.size() >= 2 && plotList2.size() >= 2)
		{
			// even out segment size a bit
			Vector3 srcDst = _plotList[(_plotList.size()-2)] - _plotList[(_plotList.size()-1)];
			Vector3 dstSrc = plotList2[plotList2.size()-2] - plotList2[plotList2.size()-1];
			Real srcDstL(srcDst.length()), dstSrcL(dstSrc.length());
			Real avgL = (srcDstL + dstSrcL + distanceToJoin) / 4;
			srcDst.normalise();
			dstSrc.normalise();
			_plotList[_plotList.size()-1] = _plotList[_plotList.size()-2] - (srcDst*avgL);
			plotList2[plotList2.size()-1] = plotList2[plotList2.size()-2] - (dstSrc*avgL);

			// update the cursors
			srcCursor3D = _plotList[_plotList.size()-1];
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
		Real dist = _genParams._sampleSize * Math::Sin(_genParams._sampleDeviance);
		Real step = (2 * dist) / (_genParams._numOfSamples - 1);
		Vector2 stepVector = midLineVector * step;

		// initial sample
		Vector2 midCursor2D = midPoint - (dist * midLineVector);
		Vector3 midCursor3D;

		// fill array of sample
		std::vector<Vector3> samples;
		if(WorldFrame::getSingleton().plotPointOnTerrain(midCursor2D, midCursor3D))
			samples.push_back(midCursor3D);

		for(uint16 i=0; i<(_genParams._numOfSamples - 1); i++)
		{
			midCursor2D += stepVector;
			if(WorldFrame::getSingleton().plotPointOnTerrain(midCursor2D, midCursor3D))
				samples.push_back(midCursor3D);
		}

		if(_genParams._debug) 
		{
			buildDebugSegments(srcCursor3D, samples);
			buildDebugSegments(dstCursor3D, samples);
		}
		
		// select best sample
		nextSrcCursor3D = (*this.*pt2SelectSample)(srcCursor3D, samples, dstCursor3D);
		_plotList.push_back(nextSrcCursor3D + GROUNDCLEARANCE);
	}
	else
	{
		if(_plotList.size() >= 2 && plotList2.size() >= 2)
		{
			// even out segment size a bit
			Vector3 srcDst = _plotList[(_plotList.size()-2)] - _plotList[(_plotList.size()-1)];
			Vector3 dstSrc = plotList2[plotList2.size()-2] - plotList2[plotList2.size()-1];
			Real srcDstL(srcDst.length()), dstSrcL(dstSrc.length());
			Real avgL = (srcDstL + dstSrcL + distanceToJoin) / 3;
			srcDst.normalise();
			dstSrc.normalise();
			_plotList[_plotList.size()-1] = _plotList[_plotList.size()-2] - (srcDst*avgL);
			plotList2[plotList2.size()-1] = plotList2[plotList2.size()-2] - (dstSrc*avgL);

			// update the cursors
			srcCursor3D = _plotList[_plotList.size()-1];
			dstCursor3D = plotList2[plotList2.size()-1];
			srcCursor2D = Vector2(srcCursor3D.x, srcCursor3D.z);
			dstCursor2D = Vector2(dstCursor3D.x, dstCursor3D.z);
		}
	}
	

	//join src-->dst & dst-->src lists
	for(int i=static_cast<int>(plotList2.size()-1); i>=0; i--)
		_plotList.push_back(plotList2[i]);

	if(_genParams._debug)
	{
		_debugMOObject->end(); 
		_sceneNode->attachObject(_debugMOObject); 
	}

	// build a spline from our plot list
	_spline.clear();
	_spline.setAutoCalculate(false);
	BOOST_FOREACH(Vector3& v, _plotList) _spline.addPoint(v);
	_spline.recalcTangents();

	// make the graph
	createRoadGraph();
}

void WorldRoad::createRoadGraph()
{
	// always delete the last graph
	destroyRoadGraph();

#ifdef FINEGRAIN
	// um this could be more complicated than i originally thought,
	// first and last are special cases
	// calculate approximate number of points to get desired segment size 
	Real interpolateStep = 1;
	if(_plotList.size() > 1)
		interpolateStep = 1/(((_plotList.size()-1) * _genParams._sampleSize) / _genParams._segmentDrawSize);

	// set prevNodeId to source node
	NodeId prevNodeId = getSrcNode()->_nodeId;

	// extract point list from spline
	for(Real t=interpolateStep; t<=(1-interpolateStep); t += interpolateStep)
	{
		// create node
		SimpleNode* bn = new SimpleNode(_roadGraph, _spline.interpolate(t));
		NodeId currNodeId = _roadGraph.addNode(bn);

		// create road
		RoadId roadSegId;
		_roadGraph.addRoad(prevNodeId, currNodeId, this, roadSegId);
		_roadSegmentList.push_back(roadSegId);

		// update prevNodeId
		prevNodeId = currNodeId;
	}

	// create final road segment
	RoadId roadSegId;
	_roadGraph.addRoad(prevNodeId, getDstNode()->_nodeId, this, roadSegId);
	_roadSegmentList.push_back(roadSegId);

	// print seg size report
	/*stringstream os;
	BOOST_FOREACH(RoadId rd, _roadSegmentList)
	{

		os << (_roadGraph.getSrcNode(rd)->getPosition2D() - _roadGraph.getDstNode(rd)->getPosition2D()).length() << ",";
	}
	LogManager::getSingleton().logMessage(os.str());*/
#else

	if(_plotList.size() == 2)
	{
		// create first road segment
		RoadId roadSegId;
		_roadGraph.addRoad(getSrcNode()->_nodeId, getDstNode()->_nodeId, this, roadSegId);
		_roadSegmentList.push_back(roadSegId);
	}
	else
	{
		// create first road segment
		SimpleNode* bn = new SimpleNode(_roadGraph, _plotList[1]);
		NodeId cursorNode = _roadGraph.addNode(bn);
		RoadId roadSegId;
		_roadGraph.addRoad(getSrcNode()->_nodeId, cursorNode, this, roadSegId);
		_roadSegmentList.push_back(roadSegId);

		// create each road segment
		for(unsigned int i=2; i<(_plotList.size()-1); i++)
		{
			// add our nodes to the road graph
			SimpleNode* bn = new SimpleNode(_roadGraph, _plotList[i]);
			NodeId nextNodeId = _roadGraph.addNode(bn);
			_roadGraph.addRoad(cursorNode, nextNodeId, this, roadSegId);
			_roadSegmentList.push_back(roadSegId);

			// advance cursor
			cursorNode = nextNodeId;
		}

		// create last road segment
		_roadGraph.addRoad(cursorNode, getDstNode()->_nodeId, this, roadSegId);
		_roadSegmentList.push_back(roadSegId);
	}
#endif
}

void WorldRoad::destroyRoadGraph()
{
	// clear previous road segs.
	if(_roadSegmentList.size() > 0) 
		_roadGraph.removeRoad(_roadSegmentList[0]);
	for(unsigned int i=1; i<_roadSegmentList.size(); i++)
	{
		NodeId nd = _roadGraph.getSrc(_roadSegmentList[i]);
		_roadGraph.removeRoad(_roadSegmentList[i]);
		delete _roadGraph.getNode(nd);
		_roadGraph.removeNode(nd);
	}

	// clear the list
	_roadSegmentList.clear();
}



void WorldRoad::buildSegment(const Vector3 &a1, const Vector3 &a2, const Vector3 &aNorm,
			const Vector3 &b1, const Vector3 &b2, const Vector3 &bNorm, Real uMin, Real uMax)
{
	// create road segment
	_manualObject->position(a1);
	_manualObject->normal(aNorm);
	_manualObject->textureCoord(uMin, 0);
	_manualObject->position(a2);
	_manualObject->normal(aNorm);
	_manualObject->textureCoord(uMin, 1);
	_manualObject->position(b1);
	_manualObject->normal(bNorm);
	_manualObject->textureCoord(uMax, 0);

	_manualObject->position(a2);
	_manualObject->normal(aNorm);
	_manualObject->textureCoord(uMin, 1);
	_manualObject->position(b2);
	_manualObject->normal(bNorm);
	_manualObject->textureCoord(uMax, 1);
	_manualObject->position(b1);
	_manualObject->normal(bNorm);
	_manualObject->textureCoord(uMax, 0);
}

void WorldRoad::destroyRoadObject()
{
	// delete our manual object
	if(_manualObject) {
		_sceneNode->detachObject(_manualObject->getName());
		_sceneNode->getCreator()->destroyManualObject(_manualObject);
		delete _manualObject;
		_manualObject = 0;
	}
}
/*
bool WorldRoad::getClosestIntersection(RoadId& rd, Vector2& pos) const
{
	for(size_t i=0; i<mRoadSegmentList.size(); i++)
	{
		NodeId srcNd = _roadGraph.getSrc(mRoadSegmentList[i]);
		NodeId dstNd = _roadGraph.getDst(mRoadSegmentList[i]);
		if(_roadGraph.findClosestIntersection(srcNd, dstNd, rd, pos))
			return true;
	}
	return false;
}
*/
bool WorldRoad::hasIntersection()
{
	for(unsigned int i=0; i<_roadSegmentList.size(); i++)
	{
		if(_roadGraph.hasIntersection(_roadSegmentList[i]))
			return true;
	}
	return false;
}

bool WorldRoad::isRoadCycle()
{
	return _isRoadCycle;
}

void WorldRoad::setRoadCycle(bool cycle)
{
	_isRoadCycle = cycle;
}

bool WorldRoad::rayCross(const Ogre::Vector2& loc)
{
	validate();
	bool rayCross = false;
	for(unsigned int i=0; i<_plotList.size()-1; i++)
	{
		if(Geometry::rayCross(loc, Vector2(_plotList[i].x, _plotList[i].z), Vector2(_plotList[i+1].x, _plotList[i+1].z))) 
			rayCross = !rayCross;
	}
	return rayCross;
}

RoadGenParams WorldRoad::getDefaultGenParams()
{
	return _defaultGenParams;
}

RoadGenParams WorldRoad::getGenParams()
{
	return _genParams;
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
	return _roadSegmentList;
}

Real WorldRoad::getWidth() const
{
	return _genParams._roadWidth;
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
				_genParams._algorithm = static_cast<RoadPlotAlgorithm>(alg);
			}else if(key == "sampleSize")
				element->QueryFloatAttribute("value", &_genParams._sampleSize);
			else if(key == "sampleDeviance"){
				Real sd;
				element->QueryFloatAttribute("value", &sd);
				_genParams._sampleDeviance = Degree(sd);
			}else if(key == "roadWidth")
				element->QueryFloatAttribute("value", &_genParams._roadWidth);
			else if(key == "segmentDrawSize")
				element->QueryFloatAttribute("value", &_genParams._segmentDrawSize);
			else if(key == "numOfSamples") {
				int nos;
				element->QueryIntAttribute("value", &nos);
				_genParams._numOfSamples = static_cast<uint16>(nos);
			}else if(key == "debug") {
				int d;
				element->QueryIntAttribute("value", &d);
				_genParams._debug = (d == 1);
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
	for(aIt = _attachments.begin(), aEnd = _attachments.end(); aIt != aEnd; aIt++)
	{
		(*aIt)->invalidate();
	}
	WorldFrame::getSingleton().modify(true);
}

void WorldRoad::invalidate()
{
	_valid = false;
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
	algorithm->SetAttribute("value", _genParams._algorithm);
	genParams->LinkEndChild(algorithm);

	TiXmlElement *sampleSize = new TiXmlElement("sampleSize");  
	sampleSize->SetDoubleAttribute("value", _genParams._sampleSize);
	genParams->LinkEndChild(sampleSize);

	TiXmlElement *segmentDrawSize = new TiXmlElement("segmentDrawSize");  
	segmentDrawSize->SetDoubleAttribute("value", _genParams._segmentDrawSize);
	genParams->LinkEndChild(segmentDrawSize);

	TiXmlElement *sampleDeviance = new TiXmlElement("sampleDeviance");  
	sampleDeviance->SetDoubleAttribute("value", _genParams._sampleDeviance.valueDegrees());
	genParams->LinkEndChild(sampleDeviance);

	TiXmlElement *roadWidth = new TiXmlElement("roadWidth");  
	roadWidth->SetDoubleAttribute("value", _genParams._roadWidth);
	genParams->LinkEndChild(roadWidth);

	TiXmlElement *numOfSamples = new TiXmlElement("numOfSamples");  
	numOfSamples->SetAttribute("value", _genParams._numOfSamples);
	genParams->LinkEndChild(numOfSamples);

	TiXmlElement *debug = new TiXmlElement("debug");  
	debug->SetAttribute("value", _genParams._debug);
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

void WorldRoad::setDefaultGenParams(const RoadGenParams& g)
{
	_defaultGenParams = g;
}

void WorldRoad::setGenParams(const RoadGenParams& g)
{
	_genParams = g;
	onMoveNode();	// well it isn't but i want to do what it does
}

void WorldRoad::setWidth(const Ogre::Real& w)
{
	_genParams._roadWidth = w;
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
	for(i=0; i<(_roadSegmentList.size()-1); i++) 
	{
		Vector2 src = _roadGraph.getSrcNode(_roadSegmentList[i])->getPosition2D();
		Vector2 dst = _roadGraph.getDstNode(_roadSegmentList[i])->getPosition2D();
		Real l = (src-dst).length();
		if(l<len) len = l;
	}
	Real minSnapSize = std::min((len/2), snapSz);
	Real snapSzSq(Math::Sqr(minSnapSize));
	//snapSzSq = 0;

	for(i=0; i<(_roadSegmentList.size()-1); i++) 
	{
		NodeId srcNd = _roadGraph.getSrc(_roadSegmentList[i]);
		NodeId dstNd = _roadGraph.getDst(_roadSegmentList[i]);
		intersection = _roadGraph.findClosestIntscnConnected(srcNd, dstNd, minSnapSize, pos, rd);
		if(intersection) break;
	}
	if(!intersection)
	{
		NodeId srcNd = _roadGraph.getSrc(_roadSegmentList[i]);
		Vector2 dstPos = _roadGraph.getDstNode(_roadSegmentList[i])->getPosition2D();
		intersection = _roadGraph.findClosestIntscn(srcNd, dstPos, minSnapSize, pos, rd);
	}

	//TODO: actually it might be an idea to have a full exclude list for 
	// this segment and to gradually increase the minimized snap size to
	// the original size as the check progresses from src to dst
	// maybe???


	if(!intersection) pos = getDstNode()->getPosition2D();

	snapSzSq = Math::Sqr(snapSz);		// give node snap full snapSz
	Real closestDistanceSq = std::numeric_limits<Real>::max();
	NodeIterator nIt, nEnd;
	for(boost::tie(nIt, nEnd) = _simpleRoadGraph.getNodes();  nIt != nEnd; nIt++)
	{
		NodeInterface* ni = _simpleRoadGraph.getNode(*nIt);
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
		RoadInterface* ri = _roadGraph.getRoad(rd);
		assert(typeid(*ri) == typeid(WorldRoad));
		wr = static_cast<WorldRoad*>(_roadGraph.getRoad(rd));
		return 1;
	}
	else return 0;
}


void WorldRoad::getMidPointAndDirection(Ogre::Vector2 &pos, Ogre::Vector2 &dir) const
{
	size_t N = _roadSegmentList.size();
	assert(N>0);
	RoadInterface* ri;
	
	if(N%2==0)	// if even
	{	
		ri = _roadGraph.getRoad(_roadSegmentList[N/2]);
		pos = ri->getSrcNode()->getPosition2D();
	}
	else		// else odd
	{
		ri = _roadGraph.getRoad(_roadSegmentList[(N/2)-1]);
		pos = (ri->getSrcNode()->getPosition2D() + ri->getDstNode()->getPosition2D())/2;
	}
	dir = ri->getDstNode()->getPosition2D() - ri->getSrcNode()->getPosition2D();
}

Ogre::Vector3 WorldRoad::getMidPoint()
{
	return _spline.interpolate(0.5f);
}
