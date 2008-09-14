#ifndef WORLDROAD_H
#define WORLDROAD_H

#include "stdafx.h"
#include "WorldObject.h"
#include "RoadInterface.h"
#include "RoadGraph.h"
#include "Region.h"
#include "MovableText.h"
#include "ExportDoc.h"

class WorldNode;
class WorldCanvas;
class TiXmlElement;
class TiXmlHandle;

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

	RoadGenParams();
};

class WorldRoad : public WorldObject, public RoadInterface, public Ogre::ManualResourceLoader
{
private:
	static unsigned int		_instanceCount;
	static RoadGenParams	_defaultGenParams;

	RoadId					_simpleRoadId;

	Ogre::Entity*			_entity;
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

	std::vector<Ogre::Real> _vertexData;
	std::vector<Ogre::uint16> _indexData;

	std::vector<Ogre::Vector3> _leftVertices;
	std::vector<Ogre::Vector3> _rightVertices;

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

	void setHighlighted(bool highlighted);
	void setSelected(bool selected);


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

	void exportObject(ExportDoc &doc);
	void loadResource(Ogre::Resource* r) {}
	void prebuild();
	void build();

	void addLeftBoundary(citygen::Region& region);
	void addRightBoundary(citygen::Region& region);

	Ogre::Real getBasicLength2D() const
	{
		Ogre::Vector2 tmpVec(getSrcNode()->getPosition2D() - getDstNode()->getPosition2D());
		return tmpVec.length();
	}

private:

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

	void addVertexData(const Ogre::Vector3 &p1, const Ogre::Vector3 &p2, const Ogre::Vector3 &norm, Ogre::Real uTex);
	void buildMeshData(const std::vector<Ogre::Vector3> &pointList);
};

#endif
