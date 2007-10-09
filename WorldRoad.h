#ifndef WORLDROAD_H
#define WORLDROAD_H

#include "stdafx.h"
#include "WorldObject.h"
#include "RoadInterface.h"
#include "RoadGraph.h"
#include "MovableText.h"

class WorldNode;
class WorldCanvas;

enum RoadPlotAlgorithm
{
	EvenElevationDiff,
	MinimumElevationDiff,
	MinimumElevation
};

struct RoadGenParams 
{
	RoadPlotAlgorithm	_algorithm;
	Ogre::Real			_sampleSize;
	Ogre::Degree		_sampleDeviance;
	Ogre::Real			_roadWidth;
	Ogre::uint16		_numOfSamples;
	bool				_debug;
	Ogre::Real			_segmentDrawSize;

	RoadGenParams()
	{
		_algorithm = EvenElevationDiff;
		_sampleSize = 7;
		_sampleDeviance = 25;
		_roadWidth = 1.0;
		_numOfSamples = 5;
		_debug = false;
		_segmentDrawSize = 2;
	}
};

class WorldRoad : public WorldObject, public RoadInterface
{
private:
	static unsigned int		_instanceCount;
	static RoadGenParams	_defaultGenParams;

	RoadId					_simpleRoadId;

	Ogre::ManualObject*		_manualObject;
	Ogre::ManualObject*		_debugMOObject;
	Ogre::MovableText*		_label;
	Ogre::String			_name;
	bool					_selected;
	Ogre::Real				_length;

	bool					_isRoadCycle;

	RoadGraph&				_roadGraph;
	RoadGraph&				_simpleRoadGraph;

	RoadGenParams			_genParams;

	std::vector<RoadId>		_roadSegmentList;
	Ogre::SimpleSpline		_spline;
	std::vector<Ogre::Vector3> _plotList;

public:
	WorldRoad(WorldNode* src, WorldNode* dst, RoadGraph& g, 
					 RoadGraph& s, Ogre::SceneManager *creator, bool bind = true);
	virtual ~WorldRoad();

	NodeInterface* getSrcNode() const;
	NodeInterface* getDstNode() const;

	bool hasIntersection();

	bool isRoadCycle();
	void setRoadCycle(bool cycle);

	bool rayCross(const Ogre::Vector2& loc);
	Ogre::Real getLengthSquared() const;
	const std::vector<RoadId>& getRoadSegmentList();
	Ogre::Real getWidth() const;
	
	void onMoveNode();

	void invalidate();
	void showSelected(bool show)
	{
		_selected = show;
		invalidate();
	}

	static RoadGenParams getDefaultGenParams();
	RoadGenParams getGenParams();
	static void setDefaultGenParams(const RoadGenParams& g);
	void setGenParams(const RoadGenParams& g);

	void setWidth(const Ogre::Real& w);

	bool loadXML(const TiXmlHandle& roadRoot);
	TiXmlElement* saveXML();

	int snapInfo(Ogre::Real snapSz, Ogre::Vector2& pos, WorldNode*& wn, WorldRoad*& wr);

	void getMidPointAndDirection(Ogre::Vector2 &pos, Ogre::Vector2 &dir) const;

	Ogre::Real getLength() const { return _length; }
	Ogre::Vector3 getMidPoint();

	// compare by length
	bool operator>(const WorldRoad& right)
	{
		return this->getLength() < right.getLength();
	}

	friend int operator>(const WorldRoad& left, const WorldRoad& right)
	{
		return left.getLength() < right.getLength();
	}

	static void resetInstanceCount() { _instanceCount = 0; }

private:
	void build();
	void destroyRoadObject();

	void createRoadGraph();
	void destroyRoadGraph();
	void plotRoad();

	void buildSampleFan(const Ogre::Vector2& cursor, const Ogre::Vector2& direction, 
		std::vector<Ogre::Vector3> &samples) const;

	Ogre::Vector3 selectMinElevation(const Ogre::Vector3 &lastSample, 
		const std::vector<Ogre::Vector3> &samples, const Ogre::Vector3 &target);
	
	Ogre::Vector3 selectMinElevationDiff(const Ogre::Vector3 &lastSample, 
		const std::vector<Ogre::Vector3> &samples, const Ogre::Vector3 &target);

	Ogre::Vector3 selectEvenElevationDiff(const Ogre::Vector3 &lastSample, 
		const std::vector<Ogre::Vector3> &samples, const Ogre::Vector3 &target);

	void buildDebugSegments(const Ogre::Vector3 &pos, const std::vector<Ogre::Vector3> &samples);

	void buildSegment(const Ogre::Vector3 &a1, const Ogre::Vector3 &a2, const Ogre::Vector3 &aNorm,
		const Ogre::Vector3 &b1, const Ogre::Vector3 &b2, const Ogre::Vector3 &bNorm, Ogre::Real uMin, Ogre::Real uMax);

};

#endif
