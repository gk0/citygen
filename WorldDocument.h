#ifndef WORLDDOCUMENT_H
#define WORLDDOCUMENT_H

#include "stdafx.h"
#include "RoadGraph.h"
#include "CityCell.h"


typedef std::list< CityCell >::iterator CityCellIterator;


class WorldDocument : public wxDocument 
{
	DECLARE_DYNAMIC_CLASS(WorldDocument)
private:
	RoadGraph mRoadGraph;
	std::list< CityCell > mCityCells;
//	std::vector< GrowthGenParams > mCellParams;

public:
	//Default Constructor
	WorldDocument();

	std::ostream& SaveObject(std::ostream& stream);
	std::istream& LoadObject(std::istream& stream);

	NodeDescriptor createNode(const Ogre::Vector2& pos) { return mRoadGraph.createNode(pos); }
	NodeDescriptor createNode(const Ogre::Vector2& pos, void* ptrData) { return mRoadGraph.createNode(pos, ptrData); }
	void* getNodeData1(const NodeDescriptor nd) { return mRoadGraph.getNodeData1(nd); }
	Ogre::Vector2 getNodePosition(const NodeDescriptor nd) { return mRoadGraph.getNodePosition(nd); }
	void setNodeData1(const NodeDescriptor nd, void* ptrData) { return mRoadGraph.setNodeData1(nd, ptrData); }
	std::pair<const NodeIterator, const NodeIterator> getNodes() { return mRoadGraph.getNodes(); }
	void moveNode(const NodeDescriptor nd, const Ogre::Vector2& pos) { mRoadGraph.moveNode(nd, pos); }
	void removeNode(const NodeDescriptor nd) { mRoadGraph.removeNode(nd); }

	RoadDescriptor createRoad(const NodeDescriptor nd1, const NodeDescriptor nd2) { return mRoadGraph.createRoad(nd1, nd2); }
	RoadDescriptor createRoad(const NodeDescriptor nd1, const NodeDescriptor nd2, void* ptrData) { return mRoadGraph.createRoad(nd1, nd2, ptrData); }
	bool findRoad(const NodeDescriptor nd1, const NodeDescriptor nd2, RoadDescriptor& rd) { return mRoadGraph.findRoad(nd1, nd2, rd); }
	void* getRoadData1(const RoadDescriptor rd) { return mRoadGraph.getRoadData1(rd); }
	std::pair<const RoadIterator, const RoadIterator> getRoads() { return mRoadGraph.getRoads(); }
	void removeRoad(const NodeDescriptor nd1, const NodeDescriptor nd2) {  mRoadGraph.removeRoad(nd1, nd2); }
	std::pair<const RoadIterator2, const RoadIterator2> getRoadsFromNode(const NodeDescriptor& nd) { return mRoadGraph.getRoadsFromNode(nd); }

	NodeDescriptor getRoadSource(const RoadDescriptor rd) { return mRoadGraph.getRoadSource(rd); }
	NodeDescriptor getRoadTarget(const RoadDescriptor rd) { return mRoadGraph.getRoadTarget(rd); }
	void setRoadData1(const RoadDescriptor rd, void* ptrData) { mRoadGraph.setRoadData1(rd, ptrData); }

	Ogre::Vector2 findPrimitiveCenter(const Primitive& p) { return mRoadGraph.findPrimitiveCenter(p); }
	bool pointInPolygon(const Ogre::Vector2& loc, const Primitive& p) { return mRoadGraph.pointInPolygon(loc, p); }

	void divideCells();
	bool pickCell(const Ogre::Vector2& loc, CityCell* &cell);
	std::pair<CityCellIterator, CityCellIterator> getCells();


};

#endif
