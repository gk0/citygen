#include "stdafx.h"
#include "SimpleNode.h"
#include "WorldFrame.h"
#include "RoadGraph.h"
#include "Triangulate.h"
#include "RoadInterface.h"
#include "MeshBuilder.h"
#include "Geometry.h"

#include <OgreManualObject.h>

using namespace Ogre;
using namespace std;


SimpleNode::SimpleNode(RoadGraph &g)
 : NodeInterface(g)
{ }

SimpleNode::SimpleNode(RoadGraph &g, Ogre::Real x, Ogre::Real z)
 : NodeInterface(g)
{
	setPosition2D(x, z);
}

SimpleNode::SimpleNode(RoadGraph &g, const Ogre::Vector2 &pos)
 : NodeInterface(g)
{
	setPosition2D(pos.x, pos.y);
}

SimpleNode::SimpleNode(RoadGraph &g, const Ogre::Vector3 &pos)
 : NodeInterface(g)
{
	setPosition3D(pos.x, pos.y, pos.z);
}

bool SimpleNode::setPosition2D(Ogre::Real x, Ogre::Real z)
{
	Ogre::Real y;
	if(WorldFrame::getSingleton().plotPointOnTerrain(x, y, z))
	{
		setPosition3D(x, y+GROUNDCLEARANCE, z);
		return true;
	}
	return false;
}

void SimpleNode::prebuild()
{
	// how many roads connect here
	size_t degree = _roadGraph.getDegree(_nodeId);
	switch(degree)
	{
	case 0:
		return;
	case 1:
		createTerminus();
		return;
	case 2:
		// use generic
		break;
	case 3:
		if(createTJunction()) return;
		break;
	default:
		//maybe should try
		break;
	}

	// get a clockwise list of road intersections
	vector<Vector3> pointlist;
	vector<RoadId> roadCWVec;
	pointlist.reserve(degree);
	roadCWVec.reserve(degree);

	// initial vars
	NodeId lastNodeId = _roadGraph.getFirstAdjacent(_nodeId);
	Real lastRoadInset = _roadGraph.getRoad(_roadGraph.getRoadId(_nodeId, lastNodeId))->getWidth();
	Vector3 lastRoadVec = _roadGraph.getNode(lastNodeId)->getPosition3D() - getPosition3D();

	for(size_t i=0; i < degree; i++)
	{
		NodeId currNodeId;
		_roadGraph.getCounterClockwiseMostFromPrev(lastNodeId, _nodeId, currNodeId);
		Vector3 curRoadVec = _roadGraph.getNode(currNodeId)->getPosition3D() - getPosition3D();
		RoadId curRoadId = _roadGraph.getRoadId(_nodeId, currNodeId);
		Real curRoadInset = _roadGraph.getRoad(curRoadId)->getWidth();

		Vector3 intersectionPoint = Geometry::calcBoundedBisector(lastRoadVec,curRoadVec,lastRoadInset,curRoadInset);
		pointlist.push_back(intersectionPoint + getPosition3D());
		roadCWVec.push_back(curRoadId);

		lastNodeId = currNodeId;
		lastRoadVec = curRoadVec;
		lastRoadInset = curRoadInset;
	}

	// fill the junction data for use by roads
	_roadJunction.clear();
	_vertexData.clear();
	_indexData.clear();
	_vertexData.reserve(degree * 8 * 3);
	_indexData.reserve(degree * 3);
	for(size_t i=0; i < degree; i++)
	{
		size_t j = (i + 1) % degree;

		//
		pair<Vector3, Vector3> roadPair(pointlist[i],pointlist[j]);

		// create a junction -> road join pair
		_roadJunction[roadCWVec[i]] = roadPair;

		uint16 offset = static_cast<uint16>(_vertexData.size()>>3);
		MeshBuilder::addVData3(_vertexData, roadPair.first);
		MeshBuilder::addVData3(_vertexData, Vector3::UNIT_Y);
		MeshBuilder::addVData2(_vertexData, 1, 0);
		MeshBuilder::addVData3(_vertexData, roadPair.second);
		MeshBuilder::addVData3(_vertexData, Vector3::UNIT_Y);
		MeshBuilder::addVData2(_vertexData, 0, 0);
		MeshBuilder::addVData3(_vertexData, getPosition3D());
		MeshBuilder::addVData3(_vertexData, Vector3::UNIT_Y);
		// this is a little costly but such is life
		Real v = (((pointlist[i] + pointlist[j])/2)-getPosition3D()).length()
			/ _roadGraph.getRoad(roadCWVec[i])->getWidth();
		MeshBuilder::addVData2(_vertexData, 0.5, std::min(1.0f,v/4));
		MeshBuilder::addIData3(_indexData, offset, offset + 1, offset + 2);
	}
}

void SimpleNode::build(MeshBuilder& meshBuilder, Material* mat)
{
	meshBuilder.registerData(mat, _vertexData, _indexData);
}

pair<Vector3, Vector3> SimpleNode::getRoadJunction(RoadId rd) 
{
	map<RoadId, pair<Vector3, Vector3>, road_less_than >::iterator rIt;
	rIt = _roadJunction.find(rd);
	if(rIt == _roadJunction.end())
	{
		//size_t degree = _roadGraph.getDegree(mNodeId);
		throw Exception(Exception::ERR_ITEM_NOT_FOUND, "Road not found", "SimpleNode::getRoadJunction");
		//LogManager::getSingleton().logMessage("SimpleNode road not found.", LML_CRITICAL);
		return make_pair(getPosition3D(), getPosition3D());
	}

	return rIt->second;
}


bool SimpleNode::createTJunction()
{
	assert(_roadGraph.getDegree(_nodeId) == 3);
	_roadJunction.clear();

	bool roadsAreEqualSize = false;
	vector<RoadId> throughRoads(2);
	size_t joiningRoadInd;
	vector<RoadId> roadCWVec;
	vector<Vector3> roadVec;
	vector<Real> roadWidthVec;

	// init
	{
		roadCWVec.reserve(3);
		roadVec.reserve(3);
		roadWidthVec.reserve(3);
		NodeId currNodeId = _roadGraph.getFirstAdjacent(_nodeId);
		RoadId currRoadId = _roadGraph.getRoadId(_nodeId, currNodeId);
		roadCWVec.push_back(currRoadId);
		roadVec.push_back((_roadGraph.getNode(currNodeId)->getPosition3D() - getPosition3D()).normalisedCopy());
		roadWidthVec.push_back(_roadGraph.getRoad(currRoadId)->getWidth());

		for(size_t i=1; i < 3; i++)
		{
			_roadGraph.getCounterClockwiseMostFromPrev(currNodeId, _nodeId, currNodeId);
			currRoadId = _roadGraph.getRoadId(_nodeId, currNodeId);
			roadCWVec.push_back(currRoadId);
			roadVec.push_back((_roadGraph.getNode(currNodeId)->getPosition3D() - getPosition3D()).normalisedCopy());
			roadWidthVec.push_back(_roadGraph.getRoad(currRoadId)->getWidth());
		}
	}

	if(roadWidthVec[0] == roadWidthVec[1])
	{
		if(roadWidthVec[2] != roadWidthVec[0]) joiningRoadInd = 2;
		else roadsAreEqualSize = true;
	}
	else
	{
		if(roadWidthVec[2] == roadWidthVec[0]) joiningRoadInd = 1;
		else if(roadWidthVec[2] == roadWidthVec[1]) joiningRoadInd = 0;
		else return false;
	}
	// distinction cannot be made by road width
	if(roadsAreEqualSize)
	{
		// get the angles between the other two roads
		Real cos0 = roadVec[1].x * roadVec[2].x + roadVec[1].z * roadVec[2].z;
		Real cos1 = roadVec[0].x * roadVec[2].x + roadVec[0].z * roadVec[2].z;
		Real cos2 = roadVec[0].x * roadVec[1].x + roadVec[0].z * roadVec[1].z;

		// choose the road with the least open angle to the others
		if(cos0 < cos1)
		{
			if(cos0 < cos2) joiningRoadInd = 0;
			else joiningRoadInd = 2;
		}
		else
		{
			if(cos1 < cos2) joiningRoadInd = 1;
			else joiningRoadInd = 2;
		}
	}

	// get height
	Real h = getPosition3D().y;
	Vector3 p1,p2;
	for(size_t k,j,i=0; i<3; i++)
	{
		j = (i+1)%3;
		k = (i+2)%3;

		if(j == joiningRoadInd)
		{
			p1 = getPosition3D() + Geometry::calcBoundedBisector(roadVec[i], roadVec[j], roadWidthVec[i], roadWidthVec[j]);
			p2 = getPosition3D() + Geometry::calcBoundedBisector(roadVec[j], roadVec[k], roadWidthVec[j], roadWidthVec[k]);
			_roadJunction[roadCWVec[j]] = make_pair(Vector3(p1.x, h, p1.z), Vector3(p2.x, h, p2.z));
		}
		else
		{
			if(k == joiningRoadInd) k = i;
			p1 = getPosition3D() - Geometry::calcBoundedBisector(roadVec[j], roadVec[k], roadWidthVec[j], roadWidthVec[k]);
			p2 = getPosition3D() - Geometry::calcBoundedBisector(roadVec[k], roadVec[j], roadWidthVec[k], roadWidthVec[j]);
			_roadJunction[roadCWVec[j]] = make_pair(Vector3(p1.x, h, p1.z), Vector3(p2.x, h, p2.z));
		}
	}
	return true;
}

void SimpleNode::createTerminus()
{
	_roadJunction.clear();

	Vector2 pos(getPosition2D());
	NodeId connectedNodeId = _roadGraph.getFirstAdjacent(_nodeId);
	RoadId roadId = _roadGraph.getRoadId(_nodeId, connectedNodeId);
	Real roadInset = _roadGraph.getRoad(roadId)->getWidth();
	Vector2 roadVec = _roadGraph.getNode(connectedNodeId)->getPosition2D() - pos;
	roadVec.normalise();

	Real h = getPosition3D().y;
	Vector2 offset = roadVec.perpendicular();
	offset *= roadInset;
	roadVec *= roadInset;
	_roadJunction[roadId] = make_pair(Vector3(pos.x + offset.x - roadVec.x, h, pos.y + offset.y - roadVec.y),
		Vector3(pos.x - offset.x - roadVec.x, h, pos.y - offset.y - roadVec.y));
}
