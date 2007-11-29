#include "stdafx.h"
#include "WorldNode.h"
#include "WorldRoad.h"
#include "WorldFrame.h"
#include "MovableText.h"
#include "Triangulate.h"
#include "Geometry.h"
#include "MeshBuilder.h"

#include <OgreEntity.h>
#include <OgreMeshManager.h>
#include <OgreMaterialManager.h>
#include <OgreStringConverter.h>
#include <tinyxml.h>


using namespace Ogre;
using namespace std;

int WorldNode::_instanceCount = 0;


WorldNode::WorldNode(RoadGraph &g, RoadGraph &s, SceneManager* creator)
  : NodeInterface(g),
    _simpleRoadGraph(s)
{
	_creator = creator;

	mSimpleNodeId = _simpleRoadGraph.addNode(this);
	_nodeId = _roadGraph.addNode(this);

	// set the name
	Ogre::String nodeCount(StringConverter::toString(_instanceCount++));
	_name = "node"+nodeCount;

	// create our scene node
	_sceneNode = _creator->getRootSceneNode()->createChildSceneNode(_name);

	_junctionEntity = 0;
	_degree = 0;

	// create mesh
	_mesh = _creator->createEntity(_name+"Mesh", "node.mesh");
//	MaterialPtr mat = (MaterialPtr)MaterialManager::getSingleton().getByName("gk/Hilite/Red2");
	//MaterialPtr mat = (MaterialPtr)MaterialManager::getSingleton().create("gk/Hilite/Red2");
//	mat->getTechnique(0)->getPass(0)->setAmbient(0.1, 0.1, 0.1);
//	mat->getTechnique(0)->getPass(0)->setDiffuse(0.5, 0, 0, 1.0);
//	mat->getTechnique(0)->getPass(0)->setSpecular(0.7, 0.7, 0.7, 0.5);
	_mesh->setMaterialName("gk/Hilite/Red2");

	// create highlight mesh
	_highlight = _creator->createEntity(_name+"Highlight", "flange.mesh");
	_highlight->setMaterialName("gk/Hilite/Yellow");
	_highlight->setVisible(false);

	// create select mesh
	_selected = _creator->createEntity(_name+"Selected", "node.mesh");
	_selected->setMaterialName("gk/Hilite/Yellow");
	_selected->setVisible(false);

	// create moveable text label
	_label = new MovableText("Label"+_name, nodeCount);
	_label->setCharacterHeight(4);
	_label->setTextAlignment(MovableText::H_CENTER, MovableText::V_ABOVE); // Center horizontally and display above the node
	_label->setAdditionalHeight(4.0f);

	// attach objects
	_sceneNode->attachObject(_mesh);
	_sceneNode->attachObject(_highlight);
	_sceneNode->attachObject(_selected);
	_sceneNode->attachObject(_label);
}

WorldNode::~WorldNode()
{
	_simpleRoadGraph.removeNode(mSimpleNodeId);
	_roadGraph.removeNode(_nodeId);
	_creator->destroyEntity(_mesh);
	_creator->destroyEntity(_highlight);
	_creator->destroyEntity(_selected);
	delete _label;
	if(_junctionEntity)
	{
		_sceneNode->getCreator()->destroyEntity(_junctionEntity);
		MeshManager::getSingleton().remove(_name+"Mesh");
		_junctionEntity = 0;
	}
	_creator->destroySceneNode(_sceneNode->getName());
}

void WorldNode::setLabel(const String& label)
{
	_label->setCaption(label);
}

const String& WorldNode::getLabel() const
{
	return _label->getCaption();
}

void WorldNode::showHighlighted(bool highlighted)
{
	_highlight->setVisible(highlighted);
	//mMesh->setVisible(!highlighted & !(mSelected->getVisible()));
}

void WorldNode::showSelected(bool selected)
{
	_selected->setVisible(selected);
	_label->setVisible(selected || _degree < 2);
	_mesh->setVisible(!selected && _degree < 2);
}

void WorldNode::setPosition(const Ogre::Vector3 &pos)
{
	setPosition(pos.x, pos.y, pos.z);
}

void WorldNode::setPosition(Real x, Real y, Real z)
{
	//stringstream oss;
	//oss <<"Move Node "<<getLabel()<<": "<<getPosition3D() << "->"<<Vector3(x,y,z);
	//LogManager::getSingleton().logMessage(oss.str(), LML_CRITICAL);

	WorldObject::setPosition(x,y,z);
	// the roads are stored in a vector since the iterator may be trashed by onMoveNode
	// in the case of WorldRoad this is most definite
	vector<RoadInterface*> roads;
	RoadIterator2 rIt, rEnd;
	for(boost::tie(rIt, rEnd) = _roadGraph.getRoadsFromNode(_nodeId); rIt != rEnd; rIt++)
		roads.push_back(_roadGraph.getRoad(*rIt));

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
	Real x = 0, z = 0;
	nodeRoot.Element()->QueryFloatAttribute("x", &x);
	nodeRoot.Element()->QueryFloatAttribute("y", &z);
	setPosition2D(x, z);

	String s;
	s = nodeRoot.Element()->Attribute("label");
	setLabel(s);

	return true;
}

Vector2 WorldNode::getPosition2D() const
{
	const Vector3& pos = getPosition();
	return Vector2(pos.x, pos.z);
}

/* this function should return false if the current position 
*/
bool WorldNode::hasRoadIntersection()
{
	// check for any intersections
	RoadIterator2 rIt, rEnd;
	boost::tie(rIt, rEnd) = _roadGraph.getRoadsFromNode(_nodeId); 
	for(; rIt != rEnd; rIt++)
	{
		//HACKISH
		RoadInterface* ri = _roadGraph.getRoad(*rIt);
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
	return _sceneNode->getPosition();
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
	if(_junctionEntity)
	{
		_sceneNode->getCreator()->destroyEntity(_junctionEntity);
		MeshManager::getSingleton().remove(_name+"Mesh");
		_junctionEntity = 0;
	}

	// how many roads connect here
	switch(_degree)
	{
	case 0:
		_label->setVisible(true);
		_mesh->setVisible(true);
		return;
	case 1:
		_label->setVisible(true);
		_mesh->setVisible(true);
		createTerminus();
		return;
	case 2:
		_label->setVisible(false);
		_mesh->setVisible(false);
		break;
	case 3:
		_label->setVisible(false);
		_mesh->setVisible(false);
		if(createTJunction()) return;
		break;
	default:
		// try it
		_label->setVisible(false);
		_mesh->setVisible(false);
		break;
	}
	//_label->setVisible(true);
	//_mesh->setVisible(true);
	

	Vector2 nodePos2D = getPosition2D();
	Real height = getPosition3D().y + GROUNDCLEARANCE.y - 0.1f;

	// get a clockwise list of road intersections
	vector<Vector2> pointlist;
	vector<RoadId> roadCWVec(getClockwiseVecOfRoads());
	for(size_t j,i=0; i < _degree; i++)
	{
		j = (i+1)%_degree;
		Vector2 tmp = _roadGraph.getRoadBounaryIntersection(roadCWVec[i], roadCWVec[j]);
		pointlist.push_back(madnessCheck(nodePos2D, tmp, 9.0f, 3.0f));
	}

	// declare the manual object
	MaterialPtr mat = static_cast<MaterialPtr>(MaterialManager::getSingleton().getByName("gk/RoadJunction"));

	// fill the junction data for use by roads
	_roadJunction.clear();
	_vertexData.reserve(_degree * 8 * 3);
	_indexData.reserve(_degree * 3);
	for(size_t i=0; i < _degree; i++)
	{
		size_t j = (i + 1) % _degree;

		//
		pair<Vector3, Vector3> roadPair(Vector3(pointlist[j].x, height, pointlist[j].y), 
										Vector3(pointlist[i].x, height, pointlist[i].y));

		// create a junction -> road join pair
		_roadJunction[roadCWVec[j]] = roadPair;

		uint16 offset = static_cast<uint16>(_vertexData.size()>>3);
		MeshBuilder::addVData3(_vertexData, roadPair.second - getPosition3D());
		MeshBuilder::addVData3(_vertexData, Vector3::UNIT_Y);
		MeshBuilder::addVData2(_vertexData, 0, 0);
		MeshBuilder::addVData3(_vertexData, roadPair.first - getPosition3D());
		MeshBuilder::addVData3(_vertexData, Vector3::UNIT_Y);
		MeshBuilder::addVData2(_vertexData, 1, 0);
		MeshBuilder::addVData3(_vertexData, 0.0f, GROUNDCLEARANCE.y - 0.1f, 0.0f);
		MeshBuilder::addVData3(_vertexData, Vector3::UNIT_Y);
		MeshBuilder::addVData2(_vertexData, 0.5, 0.5);

		MeshBuilder::addIData3(_indexData, offset, offset + 1, offset + 2);
	}
	MeshBuilder meshBuilder(_name+"Mesh", "custom", this);
	meshBuilder.registerData(mat.get(), _vertexData, _indexData);
	meshBuilder.build();
	_vertexData.clear();
	_indexData.clear();
	_junctionEntity = _sceneNode->getCreator()->createEntity(_name+"Entity",_name+"Mesh");
	_sceneNode->attachObject(_junctionEntity);
	meshBuilder.registerData(mat.get(), _vertexData, _indexData);
}

void WorldNode::createTerminus()
{
	_roadJunction.clear();

	// get road
	RoadIterator2 rIt2, rEnd2;
	boost::tie(rIt2, rEnd2) = _roadGraph.getRoadsFromNode(_nodeId); 
	if(rIt2 != rEnd2)
	{		
		Real h = getPosition3D().y + GROUNDCLEARANCE.y - 0.1f;
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

pair<Vector3, Vector3> WorldNode::getRoadJunction(RoadId rd) 
{
	map<RoadId, pair<Vector3, Vector3>, road_less_than >::iterator rIt;
	rIt = _roadJunction.find(rd);
	if(rIt == _roadJunction.end())
	{
		//size_t degree = _roadGraph.getDegree(mNodeId);
		//throw new Exception(Exception::ERR_ITEM_NOT_FOUND, "Road not found", "WorldNode::getRoadJunction");
		//LogManager::getSingleton().logMessage("Node "+getLabel()+" road not found.", LML_CRITICAL);
		return make_pair(getPosition3D(), getPosition3D());
	}
	return rIt->second;
}

void WorldNode::onMove()
{
	RoadIterator2 rIt, rEnd;
	boost::tie(rIt, rEnd) = _roadGraph.getRoadsFromNode(_nodeId); 
	for(; rIt != rEnd; rIt++)
	{
		_roadGraph.getRoad(*rIt)->onMoveNode();
	}
}

void WorldNode::onAddRoad()
{
	_degree = _roadGraph.getDegree(_nodeId);
	invalidate();
}

void WorldNode::onRemoveRoad()
{
	_degree = _roadGraph.getDegree(_nodeId);
	invalidate();
}

void WorldNode::invalidate()
{
	WorldObject::invalidate();
	// invalidate roads
	RoadIterator2 rIt, rEnd;
	boost::tie(rIt, rEnd) = _roadGraph.getRoadsFromNode(_nodeId); 
	for(; rIt != rEnd; rIt++)
	{
		RoadInterface* ri = _roadGraph.getRoad(*rIt);
		if(typeid(*ri) == typeid(WorldRoad))
			static_cast<WorldRoad*>(ri)->invalidate();
	}
}

bool WorldNode::createTJunction()
{
	_roadJunction.clear();

	vector<RoadId> throughRoads(2);
	size_t joiningRoadInd;
	Vector2 pos2D(getPosition2D());

	// get a clockwise list of road intersections
	bool roadsAreEqualSize = false;
	vector<RoadId> roadCWVec(getClockwiseVecOfRoads());
	vector<Real> roadWidthVec;
	roadWidthVec.reserve(_degree);
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
		Vector2 roadVec0(_roadGraph.getNode(nodeCWVec[0])->getPosition2D() - pos2D);
		Vector2 roadVec1(_roadGraph.getNode(nodeCWVec[1])->getPosition2D() - pos2D);
		Vector2 roadVec2(_roadGraph.getNode(nodeCWVec[2])->getPosition2D() - pos2D);
		roadVec0.normalise();
		roadVec1.normalise();
		roadVec2.normalise();

		// get the angles between the other two roads
		Real cos0 = roadVec1.dotProduct(roadVec2);
		Real cos1 = roadVec0.dotProduct(roadVec2);
		Real cos2 = roadVec0.dotProduct(roadVec1);

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

	//WorldRoad* wr = static_cast<WorldRoad*>(_roadGraph.getRoad(roadCWVec[joiningRoadInd]));
	//WorldNode* ni1 = static_cast<WorldNode*>(wr->getSrcNode());
	//WorldNode* ni2 = static_cast<WorldNode*>(wr->getDstNode());
	//LogManager::getSingleton().logMessage("Road Pair: "+ni1->getLabel()+":"+ni2->getLabel());

	// get height
	Real h = getPosition3D().y + GROUNDCLEARANCE.y - 0.1f;
	Vector2 p1,p2;
	for(size_t k,j,i=0; i<_degree; i++)
	{
		j = (i+1)%_degree;
		k = (i+2)%_degree;

		if(j == joiningRoadInd)
		{
			p1 = _roadGraph.getRoadBounaryIntersection(roadCWVec[j], roadCWVec[k]);
			p2 = _roadGraph.getRoadBounaryIntersection(roadCWVec[i], roadCWVec[j]);
			p1 = madnessCheck(pos2D, p1, 4.0f, 2.0f);
			p2 = madnessCheck(pos2D, p2, 4.0f, 2.0f);
			_roadJunction[roadCWVec[j]] = make_pair(Vector3(p1.x, h, p1.y), Vector3(p2.x, h, p2.y));
		}
		else
		{
			if(k == joiningRoadInd) k = (i+3)%_degree;
			p1 = _roadGraph.getRoadBounaryIntersection(roadCWVec[j], roadCWVec[k]);
			p2 = _roadGraph.getRoadBounaryIntersection(roadCWVec[k], roadCWVec[j]);
			p1 = madnessCheck(pos2D, p1, 4.0f, 2.0f);
			p2 = madnessCheck(pos2D, p2, 4.0f, 2.0f);
			_roadJunction[roadCWVec[j]] = make_pair(Vector3(p1.x, h, p1.y), Vector3(p2.x, h, p2.y));
		}
	}
	return true;
}

int WorldNode::snapInfo(const Real snapSz, Vector2& pos, WorldNode*& wn, WorldRoad*& wr) const
{
	Real snapSzSq(Math::Sqr(snapSz));
	Vector2 nodePos = getPosition2D();
	Real closestDistanceSq = numeric_limits<Real>::max();
	NodeIterator nIt, nEnd;
	for(boost::tie(nIt, nEnd) = _simpleRoadGraph.getNodes();  nIt != nEnd; nIt++)
	{
		if((*nIt)==mSimpleNodeId) continue;
		NodeInterface* ni = _simpleRoadGraph.getNode(*nIt);
		assert(typeid(*ni) == typeid(WorldNode));
			
		Real currentDistanceSq = (nodePos - ni->getPosition2D()).squaredLength();
		if(currentDistanceSq < snapSzSq && currentDistanceSq < closestDistanceSq)
		{
			wn = static_cast<WorldNode*>(ni);
			pos = ni->getPosition2D();
			closestDistanceSq = currentDistanceSq;
		}
	}
	if(closestDistanceSq < numeric_limits<Real>::max())
		return 2;

	RoadId rd;
	if(_roadGraph.findClosestRoad(_nodeId, snapSz, pos, rd))
	{
		assert(typeid(*_roadGraph.getRoad(rd)) == typeid(WorldRoad));
		wr = static_cast<WorldRoad*>(_roadGraph.getRoad(rd));
		return 1;
	}
	else
		return 0;
}

vector<WorldRoad*> WorldNode::getWorldRoads() const
{
	vector<WorldRoad*> wrList;
	RoadIterator2 rIt, rEnd;
	boost::tie(rIt, rEnd) = _simpleRoadGraph.getRoadsFromNode(mSimpleNodeId);
	for(;rIt != rEnd; rIt++)
	{
		RoadInterface* ri = _simpleRoadGraph.getRoad(*rIt);
		assert(typeid(*ri) == typeid(WorldRoad));
		wrList.push_back(static_cast<WorldRoad*>(ri));
	}
	return wrList;
}

vector<RoadId> WorldNode::getClockwiseVecOfRoads()
{
	vector<RoadId> roadClockwiseVec;
	roadClockwiseVec.reserve(_degree);

	// get the first road and node
	RoadIterator2 rIt2, rEnd2;
	boost::tie(rIt2, rEnd2) = _roadGraph.getRoadsFromNode(_nodeId); 
	RoadId currentRoad, previousRoad = *rIt2;
	NodeId currentNode, previousNode = _roadGraph.getDst(previousRoad);
	roadClockwiseVec.push_back(previousRoad);

	// start with the second road using the first as prev
	// NOTE: a dst node exactly on the src node screws this up
	for(size_t i = 1; i < _degree; i++)
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

vector<NodeId> WorldNode::getClockwiseVecOfNodes(const vector<RoadId>& roads)
{
	vector<NodeId> nodeClockwiseVec;
	nodeClockwiseVec.reserve(_degree);

	BOOST_FOREACH(const RoadId rd, roads)
	{
		NodeId dstNode = _roadGraph.getDst(rd);
		if(dstNode != _nodeId) nodeClockwiseVec.push_back(dstNode);
		else nodeClockwiseVec.push_back(_roadGraph.getSrc(rd));
	}
	return nodeClockwiseVec;
}

void WorldNode::exportObject(ExportDoc &doc)
{
	if(_junctionEntity)
		doc.addMesh(_sceneNode, _junctionEntity->getMesh());
}
