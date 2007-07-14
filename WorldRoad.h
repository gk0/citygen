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

typedef struct {
	RoadPlotAlgorithm algorithm;
	Ogre::Real sampleSize;
	Ogre::Degree sampleDeviance;
	Ogre::Real roadWidth;
	Ogre::uint16 numOfSamples;
	bool debug;
	Ogre::Real segmentDrawSize;
} RoadGenParams;

class WorldRoad : public WorldObject, public RoadInterface
{
private:
	static int mRoadCount;

	RoadId mSimpleRoadId;

	Ogre::ManualObject *mManualObject;
	Ogre::ManualObject *mDebugObject;
	Ogre::MovableText *mLabel;
	Ogre::String mName;
	bool mSelected;

	bool mIsRoadCycle;
//	WorldNode *mSrcNode, *mDstNode;

	RoadGraph &mRoadGraph;
	RoadGraph &mSimpleRoadGraph;

	RoadGenParams mGenParams;

	std::vector<RoadId> mRoadSegmentList;
	std::vector<Ogre::Vector3> mPlotList;
	void buildSegment(const Ogre::Vector3 &a1, const Ogre::Vector3 &a2, const Ogre::Vector3 &aNorm,
		const Ogre::Vector3 &b1, const Ogre::Vector3 &b2, const Ogre::Vector3 &bNorm, Ogre::Real uMin, Ogre::Real uMax);


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
		mSelected = show;
		invalidate();
	}

	const RoadGenParams& getGenParams();
	void setGenParams(const RoadGenParams& g);
	void setWidth(const Ogre::Real& w);

	bool loadXML(const TiXmlHandle& roadRoot);
	TiXmlElement* saveXML();

	int snapInfo(Ogre::Real snapSz, Ogre::Vector2& pos, WorldNode*& wn, WorldRoad*& wr);

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

};

#endif
