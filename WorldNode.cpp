#include "stdafx.h"
#include "WorldNode.h"
#include "WorldRoad.h"
#include "WorldFrame.h"
#include "MovableText.h"

using namespace Ogre;

int WorldNode::mInstanceCount = 0;


WorldNode::WorldNode(SceneManager* creator, const String& name)
{
	mCreator = creator;
	init(name, name);
}

WorldNode::WorldNode(SceneManager* creator)
{
	mCreator = creator;
	// set the name
	Ogre::String nodeCount(StringConverter::toString(mInstanceCount++));
	Ogre::String name("node"+nodeCount);
	init(name, nodeCount);
}

WorldNode::WorldNode(SceneManager* creator, const Vector3& pos)
{
	mCreator = creator;

	// set the name
	Ogre::String nodeCount(StringConverter::toString(mInstanceCount++));
	Ogre::String name("node"+nodeCount);
	init(name, nodeCount);
	setPosition(pos);
}

void WorldNode::init(const String& name, const String& label)
{
	// create our scene node
	mSceneNode = mCreator->getRootSceneNode()->createChildSceneNode(name);

	// create mesh
	mMesh = mCreator->createEntity(name+"Mesh", "node.mesh" );
//	MaterialPtr mat = (MaterialPtr)MaterialManager::getSingleton().getByName("gk/Hilite/Red2");
	//MaterialPtr mat = (MaterialPtr)MaterialManager::getSingleton().create("gk/Hilite/Red2");
//	mat->getTechnique(0)->getPass(0)->setAmbient(0.1, 0.1, 0.1);
//	mat->getTechnique(0)->getPass(0)->setDiffuse(0.5, 0, 0, 1.0);
//	mat->getTechnique(0)->getPass(0)->setSpecular(0.7, 0.7, 0.7, 0.5);
	mMesh->setMaterialName("gk/Hilite/Red2");

	// create highlight mesh
	mHighlight = mCreator->createEntity(name+"Highlight", "flange.mesh" );
	mHighlight->setMaterialName("gk/Hilite/Yellow");
	mHighlight->setVisible(false);

	// create select mesh
	mSelected = mCreator->createEntity(name+"Selected", "node.mesh" );
	mSelected->setMaterialName("gk/Hilite/Yellow");
	mSelected->setVisible(false);

	// create moveable text label
	mLabel = new MovableText("Label"+name, label);
	mLabel->setCharacterHeight(5);
	mLabel->setTextAlignment(MovableText::H_CENTER, MovableText::V_ABOVE); // Center horizontally and display above the node
	mLabel->setAdditionalHeight( 4.0f );

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
	mSelected->setVisible(selected);
	mMesh->setVisible(!selected);
}

void WorldNode::setPosition(const Ogre::Vector3 &pos)
{
	WorldObject::setPosition(pos);
	invalidate();
}

void WorldNode::setPosition(Real x, Real y, Real z)
{
	WorldObject::setPosition(x,y,z);
	invalidate();
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

void WorldNode::attach(WorldObject* wo)
{
	WorldObject::attach(wo);
	// if its a road add it to our road list
	if(typeid(*wo) == typeid(WorldRoad))
		mRoads.push_back(static_cast<WorldRoad*>(wo));
}

void WorldNode::detach(WorldObject* wo)
{
	WorldObject::detach(wo);
	if(typeid(*wo) == typeid(WorldRoad))
	{
		WorldRoad *wr = static_cast<WorldRoad*>(wo); 
		std::vector<WorldRoad*>::iterator rIt, rEnd;
		for(rIt = mRoads.begin(), rEnd = mRoads.end(); rIt != rEnd; )
		{
			if((*rIt) == wr)
			{
				rIt = mRoads.erase(rIt); 
				rEnd = mRoads.end();
			}
			else rIt++;
		}
	}
}


/* this function should return false if the current posiotion 
*/
bool WorldNode::hasRoadIntersection()
{
	// check for any intersections
	std::vector<WorldRoad*>::iterator rIt, rEnd;
	for(rIt = mRoads.begin(), rEnd = mRoads.end(); rIt != rEnd; rIt++)
		if((*rIt)->hasIntersection()) return true;
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
	std::vector<WorldRoad*>::iterator rIt, rEnd;
	for(rIt = mRoads.begin(), rEnd = mRoads.end(); rIt != rEnd; rIt++)
		(*rIt)->validate();

	if(hasRoadIntersection())
	{
		setPosition(oldPos);
		for(rIt = mRoads.begin(), rEnd = mRoads.end(); rIt != rEnd; rIt++)
			(*rIt)->validate();
		return false;
	}
	return true;
}
