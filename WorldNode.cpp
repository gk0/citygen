#include "stdafx.h"
#include "WorldNode.h"
#include "WorldRoad.h"
#include "WorldFrame.h"
#include "MovableText.h"
#include "Triangulate.h"
#include "Geometry.h"

using namespace Ogre;

int WorldNode::mInstanceCount = 0;


WorldNode::WorldNode(RoadGraph &g, RoadGraph &s, SceneManager* creator)
  : mRoadGraph(g),
    mSimpleRoadGraph(s)
{
	mCreator = creator;

	mSimpleNodeId = mSimpleRoadGraph.addNode(this);
	mNodeId = mRoadGraph.addNode(this);

	// set the name
	Ogre::String nodeCount(StringConverter::toString(mInstanceCount++));
	mName = "node"+nodeCount;

	// create our scene node
	mSceneNode = mCreator->getRootSceneNode()->createChildSceneNode(mName);

	mJunctionPlate = 0;
	mDegree = 0;

	// create mesh
	mMesh = mCreator->createEntity(mName+"Mesh", "node.mesh");
//	MaterialPtr mat = (MaterialPtr)MaterialManager::getSingleton().getByName("gk/Hilite/Red2");
	//MaterialPtr mat = (MaterialPtr)MaterialManager::getSingleton().create("gk/Hilite/Red2");
//	mat->getTechnique(0)->getPass(0)->setAmbient(0.1, 0.1, 0.1);
//	mat->getTechnique(0)->getPass(0)->setDiffuse(0.5, 0, 0, 1.0);
//	mat->getTechnique(0)->getPass(0)->setSpecular(0.7, 0.7, 0.7, 0.5);
	mMesh->setMaterialName("gk/Hilite/Red2");

	// create highlight mesh
	mHighlight = mCreator->createEntity(mName+"Highlight", "flange.mesh");
	mHighlight->setMaterialName("gk/Hilite/Yellow");
	mHighlight->setVisible(false);

	// create select mesh
	mSelected = mCreator->createEntity(mName+"Selected", "node.mesh");
	mSelected->setMaterialName("gk/Hilite/Yellow");
	mSelected->setVisible(false);

	// create moveable text label
	mLabel = new MovableText("Label"+mName, nodeCount);
	mLabel->setCharacterHeight(5);
	mLabel->setTextAlignment(MovableText::H_CENTER, MovableText::V_ABOVE); // Center horizontally and display above the node
	mLabel->setAdditionalHeight(4.0f);

	// attach objects
	mSceneNode->attachObject(mMesh);
	mSceneNode->attachObject(mHighlight);
	mSceneNode->attachObject(mSelected);
	mSceneNode->attachObject(mLabel);
}

WorldNode::~WorldNode()
{
	mCreator->destroyEntity(mMesh);
	mCreator->destroyEntity(mHighlight);
	mCreator->destroyEntity(mSelected);
	delete mLabel;
	if(mJunctionPlate)
	{
		delete mJunctionPlate;
		mJunctionPlate = 0;
	}
	mCreator->destroySceneNode(mSceneNode->getName());
}

void WorldNode::setLabel(const String& label)
{
	mLabel->setCaption(label);
}

const String& WorldNode::getLabel() const
{
	return mLabel->getCaption();
}

void WorldNode::showHighlighted(bool highlighted)
{
	mHighlight->setVisible(highlighted);
	//mMesh->setVisible(!highlighted & !(mSelected->getVisible()));
}

void WorldNode::showSelected(bool selected)
{
	mSelected->setVisible(selected && mDegree < 2);
	mMesh->setVisible(!selected && mDegree < 2);
}

void WorldNode::setPosition(const Ogre::Vector3 &pos)
{
	setPosition(pos.x, pos.y, pos.z);
}

void WorldNode::setPosition(Real x, Real y, Real z)
{
	//std::stringstream oss;
	//oss <<"Move Node "<<getLabel()<<": "<<getPosition3D() << "->"<<Vector3(x,y,z);
	//LogManager::getSingleton().logMessage(oss.str(), LML_CRITICAL);

	WorldObject::setPosition(x,y,z);
	// the roads are stored in a vector since the iterator may be trashed by onMoveNode
	// in the case of WorldRoad this is most definite
	std::vector<RoadInterface*> roads;
	RoadIterator2 rIt, rEnd;
	for(boost::tie(rIt, rEnd) = mRoadGraph.getRoadsFromNode(mNodeId); rIt != rEnd; rIt++)
		roads.push_back(mRoadGraph.getRoad(*rIt));

	for(size_t i=0; i<roads.size(); i++)
		roads[i]->onMoveNode();
}

bool WorldNode::setPosition2D(Real x, Real z)
{
	Real y;
	if(WorldFrame::getSingleton().plotPointOnTerrain(x, y, z))
	{
		setPosition(x,y + 0.1,z);
		return true;
	}
	return false;
}

bool WorldNode::setPosition2D(const Ogre::Vector2& pos)
{
	return setPosition2D(pos.x, pos.y);
}

bool WorldNode::loadXML(const TiXmlHandle& nodeRoot)
{
	Real x, z;
	nodeRoot.Element()->QueryFloatAttribute("x", &x);
	nodeRoot.Element()->QueryFloatAttribute("y", &z);
	setPosition2D(x, z);
	
	return true;
}

Vector2 WorldNode::getPosition2D() const
{
	const Vector3& pos = getPosition();
	return Vector2(pos.x, pos.z);
}

/* this function should return false if the current posiotion 
*/
bool WorldNode::hasRoadIntersection()
{
	// check for any intersections
	RoadIterator2 rIt, rEnd;
	boost::tie(rIt, rEnd) = mRoadGraph.getRoadsFromNode(mNodeId); 
	for(; rIt != rEnd; rIt++)
	{
		//HACKISH
		RoadInterface* ri = mRoadGraph.getRoad(*rIt);
		if(typeid(*ri) == typeid(WorldRoad))
		{
			WorldRoad* wr = static_cast<WorldRoad*>(ri);
			if(wr->hasIntersection()) return true;
		}
	}
	return false;
}

Vector3 WorldNode::getPosition3D() const
{
	return mSceneNode->getPosition();
}

void WorldNode::setPosition3D(Real x, Real y, Real z)
{
	WorldObject::setPosition(x, y, z);
	invalidate();
}

void WorldNode::setPosition3D(const Ogre::Vector3& pos)
{
	WorldObject::setPosition(pos);
	invalidate();
}

bool WorldNode::move(Ogre::Vector2 pos)
{
	Vector3 oldPos(getPosition());
	setPosition2D(pos.x, pos.y);

	// check for any intersections
	if(hasRoadIntersection())
	{
		setPosition(oldPos);
		return false;
	}
	return true;
}

void WorldNode::build()
{
	if(mJunctionPlate)
	{
		delete mJunctionPlate;
		mJunctionPlate = 0;
	}

	// how many roads connect here
	switch(mDegree)
	{
	case 0:
		mLabel->setVisible(true);
		mMesh->setVisible(true);
		return;
	case 1:
		mLabel->setVisible(true);
		mMesh->setVisible(true);
		createTerminus();
		return;
	case 2:
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
		return;
	}
	mLabel->setVisible(false);
	mMesh->setVisible(false);

	Vector2 nodePos2D = getPosition2D();
	Real height = getPosition3D().y + 0.3;

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
	for(size_t i = 1; i < mDegree; i++)
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
	// MADNESS CHECK
	Vector2 tmp = mRoadGraph.getRoadBounaryIntersection(previousRoad, firstRoad);
	pointlist.push_back(madnessCheck(nodePos2D, tmp, 9.0f, 3.0f));

	// fill the junction data for use by roads
	mRoadJunction.clear();
	for(size_t i=0; i < mDegree; i++)
	{
		size_t j = (i + 1) % mDegree;

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
		// declare the manual object
		mJunctionPlate = new ManualObject(mName+"j");
		mJunctionPlate->begin("gk/RoadJunction", Ogre::RenderOperation::OT_TRIANGLE_LIST);

		Vector2 poser = getPosition2D();
		for(size_t i=0; i<result.size(); i+=3)
		{
			// we must be relative to the scenenode
			result[i] -= poser;
			result[i+1] -= poser;
			result[i+2] -= poser;

			mJunctionPlate->position(Vector3(result[i+2].x, 0.3, result[i+2].y));
			mJunctionPlate->normal(Vector3::UNIT_Y);
			mJunctionPlate->position(Vector3(result[i+1].x, 0.3, result[i+1].y));
			mJunctionPlate->normal(Vector3::UNIT_Y);
			mJunctionPlate->position(Vector3(result[i].x, 0.3, result[i].y));
			mJunctionPlate->normal(Vector3::UNIT_Y);
		}
		mJunctionPlate->end();
		mSceneNode->attachObject(mJunctionPlate);
	}
}

void WorldNode::createTerminus()
{
	mRoadJunction.clear();

	// get road
	RoadIterator2 rIt2, rEnd2;
	boost::tie(rIt2, rEnd2) = mRoadGraph.getRoadsFromNode(mNodeId); 
	if(rIt2 != rEnd2)
	{		
		Real h = getPosition3D().y + 0.4;
		Vector2 p1, p2, offset;
		p1 = mRoadGraph.getSrcNode(*rIt2)->getPosition2D();
		p2 = mRoadGraph.getDstNode(*rIt2)->getPosition2D();
		offset = (p2 - p1).perpendicular();
		offset.normalise();
		offset *= mRoadGraph.getRoad(*rIt2)->getWidth();;


		mRoadJunction[*rIt2] = std::make_pair(Vector3(p1.x - offset.x, h, p1.y - offset.y),
						Vector3(p1.x + offset.x, h, p1.y + offset.y));
	}
}

std::pair<Vector3, Vector3> WorldNode::getRoadJunction(RoadId rd) 
{
	std::map<RoadId, std::pair<Vector3, Vector3>, road_less_than >::iterator rIt;
	rIt = mRoadJunction.find(rd);
	if(rIt == mRoadJunction.end())
	{
		//size_t degree = mRoadGraph.getDegree(mNodeId);
		//throw new Exception(Exception::ERR_ITEM_NOT_FOUND, "Road not found", "WorldNode::getRoadJunction");
		LogManager::getSingleton().logMessage("Node "+getLabel()+" road not found.", LML_CRITICAL);
		return std::make_pair(getPosition3D(), getPosition3D());
	}

	return rIt->second;
}

void WorldNode::onMove()
{
	RoadIterator2 rIt, rEnd;
	boost::tie(rIt, rEnd) = mRoadGraph.getRoadsFromNode(mNodeId); 
	for(; rIt != rEnd; rIt++)
	{
		mRoadGraph.getRoad(*rIt)->onMoveNode();
	}
}

void WorldNode::onAddRoad()
{
	mDegree = mRoadGraph.getDegree(mNodeId);
	invalidate();
}

void WorldNode::onRemoveRoad()
{
	mDegree = mRoadGraph.getDegree(mNodeId);
	invalidate();
}

void WorldNode::invalidate()
{
	WorldObject::invalidate();
	// invalidate roads
	RoadIterator2 rIt, rEnd;
	boost::tie(rIt, rEnd) = mRoadGraph.getRoadsFromNode(mNodeId); 
	for(; rIt != rEnd; rIt++)
	{
		RoadInterface* ri = mRoadGraph.getRoad(*rIt);
		if(typeid(*ri) == typeid(WorldRoad))
			static_cast<WorldRoad*>(ri)->invalidate();
	}
}


bool WorldNode::createTJunction()
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
	Real h = getPosition3D().y + 0.3;

	// create simple junction for through road
	Vector2 p1 = mRoadGraph.getRoadBounaryIntersection(throughRoads[0], throughRoads[1]);
	Vector2 p2 = mRoadGraph.getRoadBounaryIntersection(throughRoads[1], throughRoads[0]);

	mRoadJunction[throughRoads[0]] = std::make_pair(Vector3(p1.x, h, p1.y), Vector3(p2.x, h, p2.y));
	mRoadJunction[throughRoads[1]] = std::make_pair(Vector3(p2.x, h, p2.y), Vector3(p1.x, h, p1.y));

	// create the juntion for the joining road
	NodeId ccwNd;
	mRoadGraph.getCounterClockwiseMostFromPrev(mRoadGraph.getDst(joiningRoad), mNodeId, ccwNd);
	if(mRoadGraph.getDst(throughRoads[0]) == ccwNd)
	{
		p1 = mRoadGraph.getRoadBounaryIntersection(joiningRoad, throughRoads[0]);
		p2 = mRoadGraph.getRoadBounaryIntersection(throughRoads[1], joiningRoad);
		mRoadJunction[joiningRoad] = std::make_pair(Vector3(p1.x, h, p1.y), Vector3(p2.x, h, p2.y));
	}
	else
	{
		p1 = mRoadGraph.getRoadBounaryIntersection(joiningRoad, throughRoads[1]);
		p2 = mRoadGraph.getRoadBounaryIntersection(throughRoads[0], joiningRoad);
		//mRoadJunction[joiningRoad] = std::make_pair(Vector3(p2.x, h, p2.y), Vector3(p1.x, h, p1.y));
		mRoadJunction[joiningRoad] = std::make_pair(Vector3(p1.x, h, p1.y), Vector3(p2.x, h, p2.y));
	}
	return true;
}

int WorldNode::snapInfo(const Real snapSz, Vector2& pos, WorldNode*& wn, WorldRoad*& wr) const
{
	Real snapSzSq(Math::Sqr(snapSz));
	Vector2 nodePos = getPosition2D();
	Real closestDistanceSq = std::numeric_limits<Real>::max();
	NodeIterator nIt, nEnd;
	for(boost::tie(nIt, nEnd) = mSimpleRoadGraph.getNodes();  nIt != nEnd; nIt++)
	{
		if((*nIt)==mSimpleNodeId) continue;
		NodeInterface* ni = mSimpleRoadGraph.getNode(*nIt);
		assert(typeid(*ni) == typeid(WorldNode));
			
		Real currentDistanceSq = (nodePos - ni->getPosition2D()).squaredLength();
		if(currentDistanceSq < snapSzSq && currentDistanceSq < closestDistanceSq)
		{
			wn = static_cast<WorldNode*>(ni);
			pos = ni->getPosition2D();
			closestDistanceSq = currentDistanceSq;
		}
	}
	if(closestDistanceSq < std::numeric_limits<Real>::max())
		return 2;

	RoadId rd;
	if(mRoadGraph.findClosestRoad(mNodeId, snapSz, pos, rd))
	{
		assert(typeid(*mRoadGraph.getRoad(rd)) == typeid(WorldRoad));
		wr = static_cast<WorldRoad*>(mRoadGraph.getRoad(rd));
		return 1;
	}
	else
		return 0;

}
