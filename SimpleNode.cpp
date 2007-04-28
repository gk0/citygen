#include "stdafx.h"
#include "SimpleNode.h"
#include "WorldFrame.h"
#include "RoadGraph.h"
#include "Triangulate.h"
#include "RoadInterface.h"

using namespace Ogre;

SimpleNode::SimpleNode(RoadGraph &g)
 : mRoadGraph(g)
{ }

SimpleNode::SimpleNode(RoadGraph &g, Ogre::Real x, Ogre::Real z)
 : mRoadGraph(g)
{
	setPosition2D(x, z);
}

SimpleNode::SimpleNode(RoadGraph &g, const Ogre::Vector2 &pos)
 : mRoadGraph(g)
{
	setPosition2D(pos.x, pos.y);
}

SimpleNode::SimpleNode(RoadGraph &g, const Ogre::Vector3 &pos)
 : mRoadGraph(g)
{
	setPosition3D(pos.x, pos.y, pos.z);
}

bool SimpleNode::setPosition2D(Ogre::Real x, Ogre::Real z)
{
	Ogre::Real y;
	if(WorldFrame::getSingleton().plotPointOnTerrain(x, y, z))
	{
		setPosition3D(x,y+0.199,z);
		return true;
	}
	return false;
}

void SimpleNode::createJunction(ManualObject* junctionPlate)
{
	// how many roads connect here
	size_t degree = mRoadGraph.getDegree(mNodeId);
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
	case 4:
	case 5:
	case 6:
	case 7:
		//createTJunction(nd, m);
		break;
	default:
		// can't do this aggh
		//return;
		//maybe should try
		break;
	}

	Vector2 nodePos2D = getPosition2D();
	Real height = getPosition3D().y;

	//THINK!!
	//
	RoadId firstRoad, previousRoad, currentRoad;
	NodeId previousNode, currentNode;
	Vector3 previousPoint, currentPoint;
	std::vector<RoadId> roadClockwiseList;
	std::vector<Vector2> pointlist;

	// get the first road and node
	RoadIterator2 rIt2, rEnd2;
	boost::tie(rIt2, rEnd2) = mRoadGraph.getRoadsFromNode(mNodeId); 
	firstRoad = previousRoad = *rIt2;
	previousNode = mRoadGraph.getDst(previousRoad);
	roadClockwiseList.push_back(previousRoad);

	// start with the second road using the first as prev
	// NOTE: a dest node exactly on the src node screws this up
	for(size_t i = 1; i < degree; i++)
	{
		// get next node and road in a counter clockwise direction
		mRoadGraph.getCounterClockwiseMostFromPrev(previousNode, mNodeId, currentNode);
		currentRoad = mRoadGraph.getRoad(mNodeId, currentNode);
		roadClockwiseList.push_back(currentRoad);

		// MADNESS CHECK
		Vector2 tmp = mRoadGraph.getRoadBounaryIntersection(previousRoad, currentRoad);
		pointlist.push_back(madnessCheck(nodePos2D, tmp, 9.0f, 3.0f));

		// advance
		previousRoad = currentRoad;
		previousNode = currentNode;
	}
	// store intersection between last previous and first road
	Vector2 tmp = mRoadGraph.getRoadBounaryIntersection(previousRoad, firstRoad);
	pointlist.push_back(madnessCheck(nodePos2D, tmp, 9.0f, 3.0f));

	// fill the junction data for use by roads
	mRoadJunction.clear();
	for(size_t i=0; i < degree; i++)
	{
		size_t j = (i + 1) % degree;

		// create a junction -> road join pair
		mRoadJunction[roadClockwiseList[j]] = 
			std::make_pair(Vector3(pointlist[j].x, height, pointlist[j].y), 
							Vector3(pointlist[i].x, height, pointlist[i].y));
	}
	roadClockwiseList.clear();

	//Triangulate it 
	std::vector<Vector2> result;
	if(Triangulate::Process(pointlist, result))
	{
		Vector2 poser = getPosition2D();
		for(size_t i=0; i<result.size(); i+=3)
		{
			// we must be relative to the scenenode
			//result[i] -= poser;
			//result[i+1] -= poser;
			//result[i+2] -= poser;

			junctionPlate->position(Vector3(result[i+2].x, height, result[i+2].y));
			junctionPlate->normal(Vector3::UNIT_Y);
			junctionPlate->position(Vector3(result[i+1].x, height, result[i+1].y));
			junctionPlate->normal(Vector3::UNIT_Y);
			junctionPlate->position(Vector3(result[i].x, height, result[i].y));
			junctionPlate->normal(Vector3::UNIT_Y);
		}
	}
}

std::pair<Vector3, Vector3> SimpleNode::getRoadJunction(RoadId rd) 
{
	std::map<RoadId, std::pair<Vector3, Vector3>, road_less_than >::iterator rIt;
	rIt = mRoadJunction.find(rd);
	if(rIt == mRoadJunction.end())
	{
		//size_t degree = mRoadGraph.getDegree(mNodeId);
		//throw new Exception(Exception::ERR_ITEM_NOT_FOUND, "Road not found", "WorldNode::getRoadJunction");
		LogManager::getSingleton().logMessage("SimpleNode road not found.", LML_CRITICAL);
		return std::make_pair(getPosition3D(), getPosition3D());
	}

	return rIt->second;
}

bool SimpleNode::createTJunction()
{
	mRoadJunction.clear();

	std::vector<RoadId> throughRoads(2);
	RoadId joiningRoad;
	Ogre::Real joiningRoadWidth;

	//
	RoadIterator2 rIt, rIt2, rEnd;
	boost::tie(rIt, rEnd) = mRoadGraph.getRoadsFromNode(mNodeId); 
	joiningRoad = *rIt;
	joiningRoadWidth = mRoadGraph.getRoad(*rIt)->getWidth();

	// 
	rIt++;
	if(mRoadGraph.getRoad(*rIt)->getWidth() == joiningRoadWidth)
	{
		rIt2 = rIt;
		rIt2++;
		if(mRoadGraph.getRoad(*rIt2)->getWidth() != joiningRoadWidth)
		{
			throughRoads[0] = joiningRoad;
			throughRoads[1] = *rIt;
			joiningRoad = *rIt2;
		}
		else 
			return false;	// all roads same width not a T-Junction
	}
	else
	{
		rIt2 = rIt;
		rIt2++;
		if(mRoadGraph.getRoad(*rIt2)->getWidth() == joiningRoadWidth)
		{
			throughRoads[0] = joiningRoad;
			throughRoads[1] = *rIt2;
			joiningRoad = *rIt;
		}
		else if(mRoadGraph.getRoad(*rIt)->getWidth() == mRoadGraph.getRoad(*rIt2)->getWidth())
		{
			throughRoads[0] = *rIt;
			throughRoads[1] = *rIt2;
		}
		else return false; 	// all roads different width not a T-Junction
	}

	// get height
	Real h = getPosition3D().y;
	Vector2 nodePos2D = getPosition2D();

	// create simple junction for through road
	Vector2 p1 = mRoadGraph.getRoadBounaryIntersection(throughRoads[0], throughRoads[1]);
	p1 = madnessCheck(nodePos2D, p1, 9.0f, 3.0f);
	Vector2 p2 = mRoadGraph.getRoadBounaryIntersection(throughRoads[1], throughRoads[0]);
	p2 = madnessCheck(nodePos2D, p2, 9.0f, 3.0f);

	mRoadJunction[throughRoads[0]] = std::make_pair(Vector3(p1.x, h, p1.y), Vector3(p2.x, h, p2.y));
	mRoadJunction[throughRoads[1]] = std::make_pair(Vector3(p2.x, h, p2.y), Vector3(p1.x, h, p1.y));

	// create the juntion for the joining road
	NodeId ccwNd;
	mRoadGraph.getCounterClockwiseMostFromPrev(mRoadGraph.getDst(joiningRoad), mNodeId, ccwNd);
	if(mRoadGraph.getDst(throughRoads[0]) == ccwNd)
	{
		p1 = mRoadGraph.getRoadBounaryIntersection(joiningRoad, throughRoads[0]);
		p1 = madnessCheck(nodePos2D, p1, 9.0f, 3.0f);
		p2 = mRoadGraph.getRoadBounaryIntersection(throughRoads[1], joiningRoad);
		p2 = madnessCheck(nodePos2D, p2, 9.0f, 3.0f);
		mRoadJunction[joiningRoad] = std::make_pair(Vector3(p1.x, h, p1.y), Vector3(p2.x, h, p2.y));
	}
	else
	{
		p1 = mRoadGraph.getRoadBounaryIntersection(joiningRoad, throughRoads[1]);
		p1= madnessCheck(nodePos2D, p1, 9.0f, 3.0f);
		p2 = mRoadGraph.getRoadBounaryIntersection(throughRoads[0], joiningRoad);
		p2 = madnessCheck(nodePos2D, p2, 9.0f, 3.0f);
		mRoadJunction[joiningRoad] = std::make_pair(Vector3(p1.x, h, p1.y), Vector3(p2.x, h, p2.y));
	}
	return true;
}



void SimpleNode::createTerminus()
{
	mRoadJunction.clear();

	// get road
	RoadIterator2 rIt2, rEnd2;
	boost::tie(rIt2, rEnd2) = mRoadGraph.getRoadsFromNode(mNodeId); 
	if(rIt2 != rEnd2)
	{		
		Real h = getPosition3D().y;
		Vector2 p1, p2, offset;
		p1 = mRoadGraph.getSrcNode(*rIt2)->getPosition2D();
		p2 = mRoadGraph.getDstNode(*rIt2)->getPosition2D();
		offset = (p2 - p1).perpendicular();
		offset.normalise();
		offset *= mRoadGraph.getRoad(*rIt2)->getWidth();

		mRoadJunction[*rIt2] = std::make_pair(Vector3(p1.x - offset.x, h, p1.y - offset.y),
						Vector3(p1.x + offset.x, h, p1.y + offset.y));
	}
}

