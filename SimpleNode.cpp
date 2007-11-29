#include "stdafx.h"
#include "SimpleNode.h"
#include "WorldFrame.h"
#include "RoadGraph.h"
#include "Triangulate.h"
#include "RoadInterface.h"
#include "MeshBuilder.h"

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
		setPosition3D(x,y+0.299,z);
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

	Vector2 nodePos2D = getPosition2D();
	Real height = getPosition3D().y;

	// get a clockwise list of road intersections
	vector<Vector2> pointlist;
	vector<RoadId> roadCWVec(getClockwiseVecOfRoads());
	for(size_t j,i=0; i < degree; i++)
	{
		j = (i+1)%degree;
		Vector2 tmp = _roadGraph.getRoadBounaryIntersection(roadCWVec[i], roadCWVec[j]);
		pointlist.push_back(madnessCheck(nodePos2D, tmp, 9.0f, 3.0f));
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
		pair<Vector3, Vector3> roadPair(Vector3(pointlist[j].x, height, pointlist[j].y), 
			Vector3(pointlist[i].x, height, pointlist[i].y));

		// create a junction -> road join pair
		_roadJunction[roadCWVec[j]] = roadPair;

		uint16 offset = static_cast<uint16>(_vertexData.size()>>3);
		MeshBuilder::addVData3(_vertexData, roadPair.second);
		MeshBuilder::addVData3(_vertexData, Vector3::UNIT_Y);
		MeshBuilder::addVData2(_vertexData, 0, 0);
		MeshBuilder::addVData3(_vertexData, roadPair.first);
		MeshBuilder::addVData3(_vertexData, Vector3::UNIT_Y);
		MeshBuilder::addVData2(_vertexData, 1, 0);
		MeshBuilder::addVData3(_vertexData, getPosition3D());
		MeshBuilder::addVData3(_vertexData, Vector3::UNIT_Y);
		MeshBuilder::addVData2(_vertexData, 0.5, 0.5);

		MeshBuilder::addIData3(_indexData, offset, offset + 1, offset + 2);
	}
}

void SimpleNode::build(MeshBuilder& meshBuilder, Material* mat)
{
	meshBuilder.registerData(mat, _vertexData, _indexData);
}

void SimpleNode::createJunction(ManualObject* junctionPlate)
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

	Vector2 nodePos2D = getPosition2D();
	Real height = getPosition3D().y;

	// get a clockwise list of road intersections
	vector<Vector2> pointlist;
	vector<RoadId> roadCWVec(getClockwiseVecOfRoads());
	for(size_t j,i=0; i < degree; i++)
	{
		j = (i+1)%degree;
		Vector2 tmp = _roadGraph.getRoadBounaryIntersection(roadCWVec[i], roadCWVec[j]);
		pointlist.push_back(madnessCheck(nodePos2D, tmp, 9.0f, 3.0f));
	}

	// fill the junction data for use by roads
	_roadJunction.clear();
	for(size_t i=0; i < degree; i++)
	{
		size_t j = (i + 1) % degree;

		//
		pair<Vector3, Vector3> roadPair(Vector3(pointlist[j].x, height, pointlist[j].y), 
										Vector3(pointlist[i].x, height, pointlist[i].y));

		// create a junction -> road join pair
		_roadJunction[roadCWVec[j]] = roadPair;
		junctionPlate->position(roadPair.second);
		junctionPlate->normal(Vector3::UNIT_Y);
		junctionPlate->textureCoord(0,0);
		junctionPlate->position(roadPair.first);
		junctionPlate->normal(Vector3::UNIT_Y);
		junctionPlate->textureCoord(1,0);
		junctionPlate->position(getPosition3D());
		junctionPlate->normal(Vector3::UNIT_Y);
		junctionPlate->textureCoord(0.5, 0.5);
	}
}

pair<Vector3, Vector3> SimpleNode::getRoadJunction(RoadId rd) 
{
	map<RoadId, pair<Vector3, Vector3>, road_less_than >::iterator rIt;
	rIt = _roadJunction.find(rd);
	if(rIt == _roadJunction.end())
	{
		//size_t degree = _roadGraph.getDegree(mNodeId);
		//throw new Exception(Exception::ERR_ITEM_NOT_FOUND, "Road not found", "WorldNode::getRoadJunction");
		LogManager::getSingleton().logMessage("SimpleNode road not found.", LML_CRITICAL);
		return make_pair(getPosition3D(), getPosition3D());
	}

	return rIt->second;
}

bool SimpleNode::createTJunction()
{
	_roadJunction.clear();
	size_t joiningRoadInd;

	// get a clockwise list of road intersections
	bool roadsAreEqualSize = false;
	vector<RoadId> roadCWVec(getClockwiseVecOfRoads());
	size_t degree = roadCWVec.size();
	vector<Real> roadWidthVec;
	roadWidthVec.reserve(degree);
	BOOST_FOREACH(RoadId rd, roadCWVec) 
		roadWidthVec.push_back(_roadGraph.getRoad(rd)->getWidth());

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
		vector<NodeId> nodeCWVec(getClockwiseVecOfNodes(roadCWVec));
		Vector2 pos2D(getPosition2D());
		Vector2 roadVec0(pos2D - _roadGraph.getNode(nodeCWVec[0])->getPosition2D());
		Vector2 roadVec1(pos2D - _roadGraph.getNode(nodeCWVec[1])->getPosition2D());
		Vector2 roadVec2(pos2D - _roadGraph.getNode(nodeCWVec[2])->getPosition2D());
		roadVec0.normalise();
		roadVec1.normalise();
		roadVec2.normalise();
		Real cos0 = roadVec1.dotProduct(roadVec2);
		Real cos1 = roadVec0.dotProduct(roadVec2);
		Real cos2 = roadVec0.dotProduct(roadVec1);
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
	Vector2 p1,p2;
	for(size_t k,j,i=0; i<degree; i++)
	{
		j = (i+1)%degree;
		k = (i+2)%degree;

		if(j == joiningRoadInd)
		{
			p1 = _roadGraph.getRoadBounaryIntersection(roadCWVec[j], roadCWVec[k]);
			p2 = _roadGraph.getRoadBounaryIntersection(roadCWVec[i], roadCWVec[j]);
			_roadJunction[roadCWVec[j]] = make_pair(Vector3(p1.x, h, p1.y), Vector3(p2.x, h, p2.y));
		}
		else
		{
			if(k == joiningRoadInd) k = (i+3)%degree;
			p1 = _roadGraph.getRoadBounaryIntersection(roadCWVec[j], roadCWVec[k]);
			p2 = _roadGraph.getRoadBounaryIntersection(roadCWVec[k], roadCWVec[j]);
			_roadJunction[roadCWVec[j]] = make_pair(Vector3(p1.x, h, p1.y), Vector3(p2.x, h, p2.y));
		}
	}
	return true;
}



void SimpleNode::createTerminus()
{
	_roadJunction.clear();

	// get road
	RoadIterator2 rIt2, rEnd2;
	boost::tie(rIt2, rEnd2) = _roadGraph.getRoadsFromNode(_nodeId); 
	if(rIt2 != rEnd2)
	{		
		Real h = getPosition3D().y;
		Vector2 p1, p2, offset, roadVec;
		p1 = _roadGraph.getSrcNode(*rIt2)->getPosition2D();
		p2 = _roadGraph.getDstNode(*rIt2)->getPosition2D();
		roadVec = (p1-p2);
		roadVec.normalise();
		offset = roadVec.perpendicular();
		offset *= _roadGraph.getRoad(*rIt2)->getWidth();
		roadVec *= _roadGraph.getRoad(*rIt2)->getWidth();
		_roadJunction[*rIt2] = make_pair(Vector3(p1.x + offset.x + roadVec.x, h, p1.y + offset.y + roadVec.y),
						Vector3(p1.x - offset.x + roadVec.x, h, p1.y - offset.y + roadVec.y));
	}
}

vector<RoadId> SimpleNode::getClockwiseVecOfRoads()
{
	vector<RoadId> roadClockwiseVec;
	size_t degree = _roadGraph.getDegree(_nodeId);
	roadClockwiseVec.reserve(degree);

	// get the first road and node
	RoadIterator2 rIt2, rEnd2;
	boost::tie(rIt2, rEnd2) = _roadGraph.getRoadsFromNode(_nodeId); 
	RoadId currentRoad, previousRoad = *rIt2;
	NodeId currentNode, previousNode = _roadGraph.getDst(previousRoad);
	roadClockwiseVec.push_back(previousRoad);

	// start with the second road using the first as prev
	// NOTE: a dst node exactly on the src node screws this up
	for(size_t i = 1; i < degree; i++)
	{
		// get next node and road in a counter clockwise direction
		_roadGraph.getCounterClockwiseMostFromPrev(previousNode, _nodeId, currentNode);
		currentRoad = _roadGraph.getRoad(_nodeId, currentNode);
		roadClockwiseVec.push_back(currentRoad);

		// advance
		previousRoad = currentRoad;
		previousNode = currentNode;
	}
	return roadClockwiseVec;
}

vector<NodeId> SimpleNode::getClockwiseVecOfNodes(const vector<RoadId>& roads)
{
	vector<NodeId> nodeClockwiseVec;
	nodeClockwiseVec.reserve(roads.size());

	BOOST_FOREACH(const RoadId rd, roads)
	{
		NodeId dstNode = _roadGraph.getDst(rd);
		if(dstNode != _nodeId) nodeClockwiseVec.push_back(dstNode);
		else nodeClockwiseVec.push_back(_roadGraph.getSrc(rd));
	}
	return nodeClockwiseVec;
}

